#ifndef SMARTCONTROLX_NETWORK_MANAGER_H
#define SMARTCONTROLX_NETWORK_MANAGER_H

#include <jni.h>
#include <thread>
#include <atomic>
#include <string>

class NetworkManager {
public:
    NetworkManager(JNIEnv* env, jobject resultData);
    ~NetworkManager();

    bool startServer();
    void stopServer();
    void sendVideoFrame(const uint8_t* data, size_t size);

private:
    JNIEnv* mEnv;
    jobject mResultData; // MediaProjection result data (for input injection)

    std::thread mVideoThread;
    std::thread mControlThread;
    std::thread mDiscoveryThread;
    std::atomic<bool> mRunning;

    int mVideoSocket;
    int mControlSocket;
    int mVideoListener;
    int mControlListener;

    std::string mPinCode;

    void videoServerLoop();
    void controlServerLoop();
    void discoveryLoop();
    void handleClientConnection(int clientSocket, bool isVideoChannel);
    std::string generatePin();
    bool performPinPairing(int clientSocket);
    void injectInput(int type, int x, int y, int keycode, int action);
};

#endif //SMARTCONTROLX_NETWORK_MANAGER_H
