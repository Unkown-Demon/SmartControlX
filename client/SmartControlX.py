import sys
import json
import asyncio
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLineEdit, QPushButton, QLabel, QStatusBar
)
from PyQt6.QtGui import QImage, QPixmap, QPainter, QKeyEvent, QMouseEvent
from PyQt6.QtCore import Qt, QThread, pyqtSignal, QTimer

# Import other client modules (will be created next)
from StreamReceiver import StreamReceiver
from ControlSender import ControlSender
from AutoDiscovery import AutoDiscovery

class VideoWidget(QWidget):
    """A widget that displays video frames."""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.image = None

    def setImage(self, image: QImage):
        self.image = image
        self.update() # Trigger a repaint

    def paintEvent(self, event):
        if self.image:
            painter = QPainter(self)
            # Scale the image to fit the widget while maintaining aspect ratio
            scaled_image = self.image.scaled(self.size(), Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation)
            # Center the image
            x = (self.width() - scaled_image.width()) // 2
            y = (self.height() - scaled_image.height()) // 2
            painter.drawImage(x, y, scaled_image)

class MainWindow(QMainWindow):
    """Main application window."""
    frame_received = pyqtSignal(QImage)
    status_updated = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("SmartControlX Client")
        self.setGeometry(100, 100, 800, 600)
        self.setStyleSheet("background-color: #1e1e1e; color: #d4d4d4;")

        # Load settings
        with open("settings.json", "r") as f:
            self.settings = json.load(f)

        # --- UI Components ---
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.layout = QVBoxLayout(self.central_widget)

        # Top connection bar
        self.connection_bar = QHBoxLayout()
        self.ip_input = QLineEdit(self.settings.get("default_ip", "192.168.1.1"))
        self.ip_input.setPlaceholderText("Enter Android Device IP")
        self.connect_button = QPushButton("Connect")
        self.discover_button = QPushButton("Discover")
        self.connection_bar.addWidget(self.ip_input)
        self.connection_bar.addWidget(self.discover_button)
        self.connection_bar.addWidget(self.connect_button)
        self.layout.addLayout(self.connection_bar)

        # Video display
        self.video_widget = VideoWidget()
        self.layout.addWidget(self.video_widget, 1) # Give it stretch factor

        # Status bar
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)
        self.status_label = QLabel("Disconnected")
        self.status_bar.addWidget(self.status_label)

        # --- Networking and Control ---
        self.stream_receiver = None
        self.control_sender = None
        self.discovery = None
        self.is_connected = False
        self.is_recording = False

        # --- Signal/Slot Connections ---
        self.connect_button.clicked.connect(self.toggle_connection)
        self.discover_button.clicked.connect(self.start_discovery)
        self.frame_received.connect(self.video_widget.setImage)
        self.status_updated.connect(self.update_status)

        # --- FPS and Ping Timer ---
        self.fps_counter = 0
        self.ping_timer = QTimer(self)
        self.ping_timer.timeout.connect(self.update_stats)
        self.ping_timer.start(5000) # Every 5 seconds

    def toggle_connection(self):
        if not self.is_connected:
            self.start_connection()
        else:
            self.stop_connection()

    def start_connection(self):
        ip = self.ip_input.text()
        video_port = self.settings["default_port"]
        control_port = self.settings["control_port"]

        self.update_status(f"Connecting to {ip}...")
        self.connect_button.setEnabled(False)

        # Start the networking threads/tasks
        self.stream_receiver = StreamReceiver(ip, video_port, self.frame_received, self.status_updated)
        self.control_sender = ControlSender(ip, control_port, self.status_updated)

        # Start the asyncio event loop in a separate thread
        self.asyncio_thread = QThread()
        self.stream_receiver.moveToThread(self.asyncio_thread)
        self.control_sender.moveToThread(self.asyncio_thread)

        self.asyncio_thread.started.connect(self.stream_receiver.run)
        self.asyncio_thread.started.connect(self.control_sender.run)

        self.stream_receiver.finished.connect(self.asyncio_thread.quit)
        self.asyncio_thread.finished.connect(self.asyncio_thread.deleteLater)

        self.asyncio_thread.start()

        # Check connection status after a delay
        QTimer.singleShot(2000, self.check_initial_connection)

    def check_initial_connection(self):
        if self.stream_receiver and self.stream_receiver.is_running and self.control_sender and self.control_sender.is_running:
            self.is_connected = True
            self.connect_button.setText("Disconnect")
            self.update_status("Connected")
        else:
            self.update_status("Connection Failed")
            self.stop_connection()
        self.connect_button.setEnabled(True)

    def stop_connection(self):
        self.is_connected = False
        if self.stream_receiver:
            self.stream_receiver.stop()
        if self.control_sender:
            self.control_sender.stop()
        if hasattr(self, "asyncio_thread") and self.asyncio_thread.isRunning():
            self.asyncio_thread.quit()
            self.asyncio_thread.wait()

        self.connect_button.setText("Connect")
        self.connect_button.setEnabled(True)
        self.update_status("Disconnected")
        self.video_widget.setImage(None) # Clear screen

    def start_discovery(self):
        self.update_status("Discovering devices...")
        self.discovery = AutoDiscovery(self.settings["discovery_port"])
        self.discovery.device_found.connect(self.device_discovered)
        self.discovery.start()

    def device_discovered(self, ip):
        self.update_status(f"Device found at {ip}")
        self.ip_input.setText(ip)
        self.discovery.stop()
        self.discovery = None

    def update_status(self, message):
        self.status_label.setText(message)

    def update_stats(self):
        if self.is_connected and self.stream_receiver:
            fps = self.stream_receiver.get_fps()
            ping = self.control_sender.get_ping()
            self.update_status(f"Connected | FPS: {fps:.1f} | Ping: {ping:.1f} ms")

    # --- Input Event Handling ---
    def keyPressEvent(self, event: QKeyEvent):
        if self.is_connected and self.control_sender:
            # Handle Ctrl+M for recording
            if event.key() == Qt.Key.Key_M and event.modifiers() == Qt.KeyboardModifier.ControlModifier:
                self.toggle_recording()
                return
            self.control_sender.send_key_event(event.key(), 1) # 1 for press

    def keyReleaseEvent(self, event: QKeyEvent):
        if self.is_connected and self.control_sender:
            self.control_sender.send_key_event(event.key(), 0) # 0 for release

    def mousePressEvent(self, event: QMouseEvent):
        if self.is_connected and self.control_sender:
            self.control_sender.send_mouse_event(event.pos().x(), event.pos().y(), event.button(), 1) # 1 for press

    def mouseReleaseEvent(self, event: QMouseEvent):
        if self.is_connected and self.control_sender:
            self.control_sender.send_mouse_event(event.pos().x(), event.pos().y(), event.button(), 0) # 0 for release

    def mouseMoveEvent(self, event: QMouseEvent):
        if self.is_connected and self.control_sender:
            self.control_sender.send_mouse_event(event.pos().x(), event.pos().y(), event.buttons(), 2) # 2 for move

    def toggle_recording(self):
        if not self.is_recording:
            if self.stream_receiver:
                self.stream_receiver.start_recording("record.mp4")
                self.is_recording = True
                self.update_status("Recording started...")
        else:
            if self.stream_receiver:
                self.stream_receiver.stop_recording()
                self.is_recording = False
                self.update_status("Recording stopped. Saved to record.mp4")

    def closeEvent(self, event):
        self.stop_connection()
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
