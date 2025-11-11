import asyncio
import av
import struct
import time
from PyQt6.QtCore import QObject, pyqtSignal, QThread
from PyQt6.QtGui import QImage

class StreamReceiver(QObject):
    """Handles video stream reception, H.264 decoding, and frame passing to GUI."""
    finished = pyqtSignal()

    def __init__(self, ip, port, frame_signal, status_signal, parent=None):
        super().__init__(parent)
        self.ip = ip
        self.port = port
        self.frame_signal = frame_signal
        self.status_signal = status_signal
        self.is_running = False
        self.loop = None

        # PyAV components
        self.codec = None
        self.decoder = None
        self.output_container = None # For recording

        # FPS calculation
        self.frame_count = 0
        self.start_time = time.time()
        self.fps = 0.0

    def run(self):
        """Starts the asyncio event loop and the main receiving task."""
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)
        self.is_running = True
        try:
            self.loop.run_until_complete(self.receive_stream())
        except asyncio.CancelledError:
            pass
        finally:
            self.loop.close()
            self.is_running = False
            self.finished.emit()

    async def receive_stream(self):
        """Asynchronously connects to the video server and processes the stream."""
        try:
            reader, writer = await asyncio.open_connection(self.ip, self.port)
            self.status_signal.emit(f"Video stream connected to {self.ip}:{self.port}")
        except ConnectionRefusedError:
            self.status_signal.emit(f"Connection refused by server at {self.ip}:{self.port}")
            return
        except Exception as e:
            self.status_signal.emit(f"Error connecting video stream: {e}")
            return

        # Initialize H.264 decoder
        self.codec = av.CodecContext.create('h264', 'r')
        self.codec.thread_type = 'AUTO' # Use multi-threading for decoding

        while self.is_running:
            try:
                # 1. Read 4-byte size prefix (network byte order - big endian)
                size_bytes = await reader.readexactly(4)
                nal_size = struct.unpack('>I', size_bytes)[0]

                # 2. Read NAL unit data
                nal_data = await reader.readexactly(nal_size)

                # 3. Decode the frame
                packet = av.Packet(nal_data)
                frames = self.codec.decode(packet)

                for frame in frames:
                    # Convert AVFrame to QImage
                    img = frame.to_image(format='rgb24')
                    qimage = QImage(img.tobytes(), img.width, img.height, QImage.Format.Format_RGB888)
                    self.frame_signal.emit(qimage)

                    # Handle recording
                    if self.output_container:
                        for packet in self.output_container.mux(frame):
                            self.output_container.stream.write(packet)

                    # Update FPS
                    self.frame_count += 1
                    if time.time() - self.start_time >= 1.0:
                        self.fps = self.frame_count / (time.time() - self.start_time)
                        self.frame_count = 0
                        self.start_time = time.time()

            except asyncio.IncompleteReadError:
                self.status_signal.emit("Video stream disconnected (Incomplete Read)")
                break
            except ConnectionResetError:
                self.status_signal.emit("Video stream disconnected (Connection Reset)")
                break
            except Exception as e:
                self.status_signal.emit(f"Video stream error: {e}")
                break

        writer.close()
        await writer.wait_closed()
        self.stop()

    def start_recording(self, filename):
        """Starts recording the decoded frames to a file."""
        if self.output_container:
            self.status_signal.emit("Recording already in progress.")
            return

        try:
            # Create output container for MP4
            self.output_container = av.open(filename, mode='w')
            stream = self.output_container.add_stream('mpeg4', rate=30) # Use mpeg4 for compatibility
            stream.width = self.codec.width
            stream.height = self.codec.height
            stream.pix_fmt = 'yuv420p' # Standard pixel format for video
            self.status_signal.emit(f"Recording started to {filename}")
        except Exception as e:
            self.status_signal.emit(f"Error starting recording: {e}")
            self.output_container = None

    def stop_recording(self):
        """Stops recording and closes the output file."""
        if self.output_container:
            try:
                # Flush stream
                for packet in self.output_container.mux():
                    self.output_container.stream.write(packet)
                self.output_container.close()
                self.output_container = None
                self.status_signal.emit("Recording stopped and file saved.")
            except Exception as e:
                self.status_signal.emit(f"Error stopping recording: {e}")
                self.output_container = None

    def get_fps(self):
        return self.fps

    def stop(self):
        """Stops the receiver and closes resources."""
        self.is_running = False
        if self.loop:
            self.loop.call_soon_threadsafe(self.loop.stop)
        if self.output_container:
            self.stop_recording()
        if self.codec:
            self.codec.close()
            self.codec = None
