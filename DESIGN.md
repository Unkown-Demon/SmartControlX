# SmartControlX Project Design Document

## 1. Overview
SmartControlX is a cross-platform application for real-time screen mirroring and remote control of an Android device over a Wi-Fi network. It is inspired by `scrcpy` but aims for a simpler, faster, and more integrated solution using a mixed-language architecture (C++, Kotlin, Python).

## 2. Architecture

| Component | Platform | Language/Framework | Role |
| :--- | :--- | :--- | :--- |
| **Android Server** | Android | Kotlin, C++ (NDK), MediaProjection, MediaCodec | Screen capture, H.264 encoding, Network streaming (Video/Control), Input injection. |
| **PC Client** | Windows/Linux | Python 3.10+, PyQt6, PyAV, Asyncio | Network receiving, H.264 decoding, GUI display, User input capture, Control event sending. |

## 3. Communication Protocol

The application will use two separate TCP sockets for reliable, ordered data transfer, which is crucial for both video streaming and control events.

### 3.1. Video Stream Channel (TCP Port 8000)
- **Protocol:** TCP
- **Purpose:** Transmit H.264 encoded video frames.
- **Data Format:** Each frame will be prefixed with a 4-byte little-endian integer indicating the size of the NAL unit (Network Abstraction Layer unit).
  - `[4-byte size] [NAL Unit Data]`
- **Initial Handshake:** The server will send a JSON string containing initial configuration (e.g., screen width, height, rotation, H.264 SPS/PPS data) upon connection.

### 3.2. Control Channel (TCP Port 8001)
- **Protocol:** TCP
- **Purpose:** Transmit control events (mouse, keyboard) from the client to the server.
- **Data Format:** A simple, fixed-size binary structure for each event type to minimize overhead.
  - **Mouse Event:** `[1-byte type] [4-byte x] [4-byte y] [1-byte button/action]`
  - **Key Event:** `[1-byte type] [4-byte keycode] [1-byte action]`
  - **Type Codes:** 0x01 (Mouse), 0x02 (Key), 0x03 (Touch - future), 0x04 (Configuration/Ping).

## 4. Android Server Design (C++ NDK + Kotlin)

### 4.1. Kotlin Layer (Server UI & MediaProjection)
- **`SmartControlXService.kt` (Foreground Service):** Manages the service lifecycle, handles the `MediaProjection` request, and obtains the `Surface` for encoding.
- **`MainActivity.kt`:** Simple UI to display the device's IP address and a "Start Wireless Control" button.
- **Workflow:**
    1. User clicks "Start".
    2. `MainActivity` requests `MediaProjection` permission.
    3. On success, the `MediaProjection` object is passed to the `SmartControlXService`.
    4. The service calls a JNI function to initialize the C++ NDK encoder and networking.

### 4.2. C++ NDK Layer (Encoding & Networking)
- **`video_encoder.cpp`:**
    - Uses `AMediaCodec` to create an H.264 encoder.
    - Calls `AMediaCodec_createInputSurface()` to get the `Surface`.
    - The `Surface` is passed back to Kotlin to be used by `MediaProjection`'s `VirtualDisplay`.
    - As encoded buffers are available, they are read and streamed.
- **`network_manager.cpp`:**
    - Manages two TCP sockets (Video and Control).
    - Video stream thread: Reads encoded NAL units and sends them over the Video Channel.
    - Control receiver thread: Listens on the Control Channel, parses events, and calls JNI functions to inject input.
- **Input Injection:** JNI bridge to call Android's `InputManager` or similar low-level APIs (requires `adb` shell access for testing or root/system permissions for full control, which is a known limitation for non-system apps, but we will target the standard `InputManager` approach for initial design).

## 5. PC Client Design (Python)

### 5.1. GUI (PyQt6)
- **Main Window:** Dark theme (as requested).
- **Input Field:** For manually entering the Android device's IP address.
- **Status Bar:** Displays connection status, FPS, and Ping.
- **Video Display Widget:** Uses PyAV to decode and display the H.264 stream in real-time.

### 5.2. Networking (Asyncio)
- **`StreamReceiver.py`:** Manages the Video Channel (Port 8000).
    - Asynchronously reads the 4-byte size prefix and then the NAL unit data.
    - Passes NAL units to the decoder.
- **`ControlSender.py`:** Manages the Control Channel (Port 8001).
    - Asynchronously sends serialized mouse/keyboard events.
- **Auto-Discovery:** Uses UDP broadcast on a separate port (e.g., 8002) to find the server's IP.

### 5.3. Decoding and Display (PyAV)
- **`VideoDecoder.py`:**
    - Uses PyAV's `av.CodecContext.create('h264', 'r')` to initialize the decoder.
    - Feeds NAL units to the decoder.
    - Receives decoded frames and passes them to the GUI for display.

### 5.4. Control Input
- Captures global mouse and keyboard events (e.g., using `pynput` or PyQt's event system) and translates them into the Control Channel protocol format.

## 6. Security and Features

- **PIN Pairing:** The server generates a random 4-digit PIN and sends it to the client during the initial handshake. The client must send the correct PIN back on the Control Channel to proceed.
- **Screen Recording:** A global hotkey (Ctrl+M) will trigger PyAV to start writing decoded frames to an MP4 file (`record.mp4`).
- **Settings:** A `settings.json` file will store configuration like default IP, desired FPS, and bitrate.
- **Ekrani ishlamasa ham:** The design relies on `MediaProjection` which requires a working screen for the initial permission prompt. The requirement "Ekrani ishlamasa ham" (even if the screen is not working) is a known limitation for non-root/non-system apps using `MediaProjection`. However, the server will be designed to run as a Foreground Service, so once the initial permission is granted, the streaming will continue even if the screen is turned off. For a truly screen-off experience, the user must have **Developer Mode and USB Debugging** enabled, which allows for initial setup via ADB, similar to `scrcpy`. We will document this requirement.

## 7. Project Structure (Refined)

```
SmartControlX/
├── android/
│   ├── app/
│   │   ├── src/
│   │   │   ├── main/
│   │   │   │   ├── java/
│   │   │   │   │   └── com/smartcontrolx/
│   │   │   │   │       ├── MainActivity.kt
│   │   │   │   │       └── SmartControlXService.kt
│   │   │   │   └── cpp/
│   │   │   │       ├── CMakeLists.txt
│   │   │   │       ├── native-lib.cpp (JNI bridge)
│   │   │   │       ├── network_manager.cpp
│   │   │   │       ├── network_manager.h
│   │   │   │       ├── video_encoder.cpp
│   │   │   │       └── video_encoder.h
│   ├── build.gradle
│   └── settings.gradle
├── client/
│   ├── SmartControlX.py (Main application, GUI)
│   ├── StreamReceiver.py (Video Channel handling, PyAV decoding)
│   ├── ControlSender.py (Control Channel handling)
│   ├── AutoDiscovery.py (UDP broadcast logic)
│   ├── settings.json (Configuration file)
│   └── requirements.txt (Python dependencies)
├── README.md
└── DESIGN.md
```
