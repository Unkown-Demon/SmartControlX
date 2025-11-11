#ifndef SMARTCONTROLX_VIDEO_ENCODER_H
#define SMARTCONTROLX_VIDEO_ENCODER_H

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <android/native_window.h>
#include <thread>
#include <atomic>

class NetworkManager; // Forward declaration

class VideoEncoder {
public:
    VideoEncoder(int width, int height, int bitrate, NetworkManager* networkManager);
    ~VideoEncoder();

    ANativeWindow* startEncoder();
    void stopEncoder();

private:
    int mWidth;
    int mHeight;
    int mBitrate;
    NetworkManager* mNetworkManager;

    AMediaCodec* mEncoder;
    AMediaFormat* mFormat;
    std::thread mEncoderThread;
    std::atomic<bool> mRunning;

    void encodingLoop();
    void sendFrame(const uint8_t* data, size_t size, bool isKeyFrame);
};

#endif //SMARTCONTROLX_VIDEO_ENCODER_H
