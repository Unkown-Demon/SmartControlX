#include "network_manager.h"
#include <android/log.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>

#define LOG_TAG "SmartControlX_Net"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Protocol ports
constexpr int VIDEO_PORT = 8000;
constexpr int CONTROL_PORT = 8001;
constexpr int DISCOVERY_PORT = 8002;

// Helper function to send data reliably
static bool sendAll(int socket, const void* data, size_t size) {
    const char* buffer = (const char*)data;
    size_t totalSent = 0;
    while (totalSent < size) {
        ssize_t sent = send(socket, buffer + totalSent, size - totalSent, 0);
        if (sent < 0) {
            ALOGE("Error sending data: %s", strerror(errno));
            return false;
        }
        totalSent += sent;
    }
    return true;
}

NetworkManager::NetworkManager(JNIEnv* env, jobject resultData)
    : mEnv(env), mResultData(resultData), mRunning(false),
      mVideoSocket(-1), mControlSocket(-1), mVideoListener(-1), mControlListener(-1) {
    // Detach the JNIEnv from the current thread to avoid issues with new threads
    // The new threads will need to attach to the JVM to use JNI functions.
    // For simplicity in this mock, we'll rely on the fact that we won't be using JNI
    // in the network threads, except for potential input injection (which is a placeholder).
    // In a real app, we'd store a JavaVM* and attach/detach JNIEnv in the threads.
    ALOGI("NetworkManager initialized.");
}

NetworkManager::~NetworkManager() {
    stopServer();
}

std::string NetworkManager::generatePin() {
    srand(time(0));
    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << (rand() % 10000);
    return ss.str();
}

bool NetworkManager::startServer() {
    mPinCode = generatePin();
    ALOGI("Generated PIN: %s", mPinCode.c_str());

    // 1. Setup Video Listener
    mVideoListener = socket(AF_INET, SOCK_STREAM, 0);
    if (mVideoListener < 0) { ALOGE("Video socket failed: %s", strerror(errno)); return false; }
    int opt = 1;
    setsockopt(mVideoListener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in videoAddr;
    memset(&videoAddr, 0, sizeof(videoAddr));
    videoAddr.sin_family = AF_INET;
    videoAddr.sin_addr.s_addr = INADDR_ANY;
    videoAddr.sin_port = htons(VIDEO_PORT);

    if (bind(mVideoListener, (struct sockaddr*)&videoAddr, sizeof(videoAddr)) < 0) { ALOGE("Video bind failed: %s", strerror(errno)); return false; }
    if (listen(mVideoListener, 1) < 0) { ALOGE("Video listen failed: %s", strerror(errno)); return false; }
    ALOGI("Video server listening on port %d", VIDEO_PORT);

    // 2. Setup Control Listener
    mControlListener = socket(AF_INET, SOCK_STREAM, 0);
    if (mControlListener < 0) { ALOGE("Control socket failed: %s", strerror(errno)); return false; }
    setsockopt(mControlListener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in controlAddr;
    memset(&controlAddr, 0, sizeof(controlAddr));
    controlAddr.sin_family = AF_INET;
    controlAddr.sin_addr.s_addr = INADDR_ANY;
    controlAddr.sin_port = htons(CONTROL_PORT);

    if (bind(mControlListener, (struct sockaddr*)&controlAddr, sizeof(controlAddr)) < 0) { ALOGE("Control bind failed: %s", strerror(errno)); return false; }
    if (listen(mControlListener, 1) < 0) { ALOGE("Control listen failed: %s", strerror(errno)); return false; }
    ALOGI("Control server listening on port %d", CONTROL_PORT);

    // 3. Start Server Threads
    mRunning = true;
    mVideoThread = std::thread(&NetworkManager::videoServerLoop, this);
    mControlThread = std::thread(&NetworkManager::controlServerLoop, this);
    mDiscoveryThread = std::thread(&NetworkManager::discoveryLoop, this);

    return true;
}

void NetworkManager::stopServer() {
        if (mRunning) {
            mRunning = false;

            // Close sockets to unblock accept()
            if (mVideoListener >= 0) { close(mVideoListener); mVideoListener = -1; }
            if (mControlListener >= 0) { close(mControlListener); mControlListener = -1; }
            if (mVideoSocket >= 0) { close(mVideoSocket); mVideoSocket = -1; }
            if (mControlSocket >= 0) { close(mControlSocket); mControlSocket = -1; }

            // Close a dummy socket to unblock the discovery thread's recvfrom
            int dummy_sock = socket(AF_INET, SOCK_DGRAM, 0);
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            addr.sin_port = htons(DISCOVERY_PORT);
            sendto(dummy_sock, "STOP", 4, 0, (struct sockaddr*)&addr, sizeof(addr));
            close(dummy_sock);

            if (mVideoThread.joinable()) { mVideoThread.join(); }
            if (mControlThread.joinable()) { mControlThread.join(); }
            if (mDiscoveryThread.joinable()) { mDiscoveryThread.join(); }
        }
    ALOGI("Network server stopped.");
}

void NetworkManager::discoveryLoop() {
    ALOGI("Discovery thread started.");
    int discoverySocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (discoverySocket < 0) {
        ALOGE("Discovery socket failed: %s", strerror(errno));
        return;
    }

    int opt = 1;
    setsockopt(discoverySocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(DISCOVERY_PORT);

    if (bind(discoverySocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ALOGE("Discovery bind failed: %s", strerror(errno));
        close(discoverySocket);
        return;
    }

    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    const char* response = "SMARTCONTROLX_DISCOVERY_RESPONSE";
    const char* request = "SMARTCONTROLX_DISCOVERY_REQUEST";

    while (mRunning) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = recvfrom(discoverySocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&clientAddr, &clientLen);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            if (strcmp(buffer, request) == 0) {
                ALOGI("Received discovery request from %s. Sending response.", inet_ntoa(clientAddr.sin_addr));
                sendto(discoverySocket, response, strlen(response), 0, (struct sockaddr*)&clientAddr, clientLen);
            }
        } else if (bytesRead < 0) {
            if (mRunning) {
                ALOGE("Discovery recvfrom failed: %s", strerror(errno));
            }
            break;
        }
    }
    close(discoverySocket);
    ALOGI("Discovery thread finished.");
}

void NetworkManager::videoServerLoop() {
    ALOGI("Video server thread started.");
    while (mRunning) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        ALOGI("Waiting for video client connection...");
        int clientSocket = accept(mVideoListener, (struct sockaddr*)&clientAddr, &clientLen);

        if (!mRunning) break; // Check if stopServer was called while blocking

        if (clientSocket < 0) {
            ALOGE("Video accept failed: %s", strerror(errno));
            continue;
        }

        ALOGI("Video client connected from %s", inet_ntoa(clientAddr.sin_addr));

        // Close previous connection if any
        if (mVideoSocket >= 0) { close(mVideoSocket); }
        mVideoSocket = clientSocket;

        // 1. Send initial configuration (width, height, SPS/PPS)
        // For simplicity, we'll send a JSON string with basic info.
        // In a real app, this would include the H.264 SPS/PPS data.
        std::string config_msg = "{\"width\":1280, \"height\":720, \"codec\":\"H.264\"}\n";
        sendAll(mVideoSocket, config_msg.c_str(), config_msg.length());

        // The encoder thread will now start sending data to mVideoSocket.
        // This thread will just wait for the connection to close.
        char buffer[1];
        while (mRunning && recv(mVideoSocket, buffer, 1, MSG_PEEK) > 0) {
            usleep(100000); // Wait 100ms
        }

        ALOGI("Video client disconnected.");
        close(mVideoSocket);
        mVideoSocket = -1;
    }
    ALOGI("Video server thread finished.");
}

void NetworkManager::controlServerLoop() {
    ALOGI("Control server thread started.");
    while (mRunning) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        ALOGI("Waiting for control client connection...");
        int clientSocket = accept(mControlListener, (struct sockaddr*)&clientAddr, &clientLen);

        if (!mRunning) break;

        if (clientSocket < 0) {
            ALOGE("Control accept failed: %s", strerror(errno));
            continue;
        }

        ALOGI("Control client connected from %s", inet_ntoa(clientAddr.sin_addr));

        // Close previous connection if any
        if (mControlSocket >= 0) { close(mControlSocket); }
        mControlSocket = clientSocket;

        // 1. PIN Pairing
        if (!performPinPairing(mControlSocket)) {
            ALOGE("PIN pairing failed. Disconnecting control client.");
            close(mControlSocket);
            mControlSocket = -1;
            continue;
        }
        ALOGI("PIN pairing successful.");

        // 2. Control Loop
        // Simple fixed-size event protocol: [1-byte type] [4-byte x] [4-byte y] [4-byte keycode] [4-byte action] = 17 bytes
        constexpr size_t EVENT_SIZE = 17;
        uint8_t eventBuffer[EVENT_SIZE];

        while (mRunning) {
            ssize_t bytesRead = recv(mControlSocket, eventBuffer, EVENT_SIZE, MSG_WAITALL);

            if (bytesRead == 0) {
                ALOGI("Control client disconnected gracefully.");
                break;
            } else if (bytesRead < 0) {
                ALOGE("Error reading control data: %s", strerror(errno));
                break;
            } else if (bytesRead == EVENT_SIZE) {
                // Parse and inject input
                uint8_t type = eventBuffer[0];
                int x = *(int*)&eventBuffer[1];
                int y = *(int*)&eventBuffer[5];
                int keycode = *(int*)&eventBuffer[9];
                int action = *(int*)&eventBuffer[13];

                injectInput(type, x, y, keycode, action);
            } else {
                ALOGE("Received incomplete control packet: %zd bytes", bytesRead);
            }
        }

        close(mControlSocket);
        mControlSocket = -1;
    }
    ALOGI("Control server thread finished.");
}

bool NetworkManager::performPinPairing(int clientSocket) {
    // 1. Send PIN to client
    std::string pinMsg = "PIN:" + mPinCode;
    if (!sendAll(clientSocket, pinMsg.c_str(), pinMsg.length())) {
        return false;
    }
    ALOGI("Sent PIN to client: %s", mPinCode.c_str());

    // 2. Receive PIN from client (expecting 4 bytes)
    char receivedPin[5] = {0};
    ssize_t bytesRead = recv(clientSocket, receivedPin, 4, MSG_WAITALL);

    if (bytesRead != 4) {
        ALOGE("Failed to receive PIN from client. Read %zd bytes.", bytesRead);
        return false;
    }

    receivedPin[4] = '\0';
    ALOGI("Received PIN from client: %s", receivedPin);

    // 3. Compare
    return mPinCode == receivedPin;
}

void NetworkManager::injectInput(int type, int x, int y, int keycode, int action) {
    // This is the critical part that requires JNI to call back into the Java/Kotlin
    // layer to use the Android InputManager, as it's not directly accessible from NDK.
    // For a complete, compilable example, we will just log the event.
    // A real implementation would require attaching the thread to the JVM and finding
    // the appropriate Java method to call.

    // Placeholder for actual input injection logic
    ALOGI("Input Event: Type=%d, X=%d, Y=%d, KeyCode=%d, Action=%d", type, x, y, keycode, action);

    // In a real application, you would:
    // 1. Get JavaVM* from JNI_OnLoad
    // 2. Attach the current thread to the JVM: jint res = vm->AttachCurrentThread(&env, NULL);
    // 3. Find the Java class and method for input injection (e.g., in SmartControlXService)
    // 4. Call the method: env->CallVoidMethod(serviceObject, methodId, ...);
    // 5. Detach the thread: vm->DetachCurrentThread();
}

void NetworkManager::sendVideoFrame(const uint8_t* data, size_t size) {
    if (mVideoSocket < 0) {
        // ALOGE("Video socket not connected.");
        return;
    }

    // Protocol: [4-byte size] [NAL Unit Data]
    uint32_t frameSize = htonl(size); // Convert to network byte order (big-endian)

    // Send size
    if (!sendAll(mVideoSocket, &frameSize, sizeof(frameSize))) {
        ALOGE("Failed to send frame size. Disconnecting video socket.");
        close(mVideoSocket);
        mVideoSocket = -1;
        return;
    }

    // Send data
    if (!sendAll(mVideoSocket, data, size)) {
        ALOGE("Failed to send frame data. Disconnecting video socket.");
        close(mVideoSocket);
        mVideoSocket = -1;
        return;
    }
}
