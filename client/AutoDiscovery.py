import socket
import threading
import time
from PyQt6.QtCore import QObject, pyqtSignal

class AutoDiscovery(QObject):
    """Uses UDP broadcast to discover the Android server's IP address."""
    device_found = pyqtSignal(str)

    def __init__(self, port, parent=None):
        super().__init__(parent)
        self.port = port
        self.is_running = False
        self.thread = None

    def start(self):
        self.is_running = True
        self.thread = threading.Thread(target=self._run_discovery)
        self.thread.start()

    def stop(self):
        self.is_running = False
        if self.thread and self.thread.is_alive():
            # Send a dummy packet to unblock the socket if it's blocked on recvfrom
            try:
                s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                s.sendto(b'STOP', ('127.0.0.1', self.port))
                s.close()
            except:
                pass
            self.thread.join()

    def _run_discovery(self):
        # 1. Listener Socket (for server response)
        listener = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind(('', self.port))
        listener.settimeout(1) # Timeout for non-blocking check

        # 2. Sender Socket (for broadcast)
        sender = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sender.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sender.settimeout(1)

        message = b'SMARTCONTROLX_DISCOVERY_REQUEST'

        while self.is_running:
            try:
                # Broadcast the request
                sender.sendto(message, ('<broadcast>', self.port))

                # Wait for a response
                try:
                    data, addr = listener.recvfrom(1024)
                    if data.decode() == 'SMARTCONTROLX_DISCOVERY_RESPONSE':
                        self.device_found.emit(addr[0])
                        self.is_running = False # Stop after finding one
                        break
                except socket.timeout:
                    pass # Continue broadcasting
                except Exception as e:
                    print(f"Discovery error: {e}")
                    break

                time.sleep(1) # Wait a second before next broadcast

            except Exception as e:
                print(f"Broadcast error: {e}")
                break

        listener.close()
        sender.close()
