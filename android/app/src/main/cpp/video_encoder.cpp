#include "video_encoder.h"
#include "network_manager.h"
#include <android/log.h>
#include <unistd.h>

#define LOG_TAG "SmartControlX_Encoder"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Timeouts
constexpr long TIMEOUT_US = 10000; // 10ms

VideoEncoder::VideoEncoder(int width, int height, int bitrate, NetworkManager* networkManager)
    : mWidth(width), mHeight(height), mBitrate(bitrate), mNetworkManager(networkManager),
      mEncoder(nullptr), mFormat(nullptr), mRunning(false) {
    ALOGI("VideoEncoder initialized: %dx%d @ %d bps", mWidth, mHeight, mBitrate);
}

VideoEncoder::~VideoEncoder() {
    stopEncoder();
}

ANativeWindow* VideoEncoder::startEncoder() {
    // 1. Configure MediaFormat
    mFormat = AMediaFormat_new();
    AMediaFormat_setString(mFormat, AMEDIAFORMAT_KEY_MIME, "video/avc"); // H.264
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_WIDTH, mWidth);
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_HEIGHT, mHeight);
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_BIT_RATE, mBitrate);
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_FRAME_RATE, 30); // 30 FPS
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1); // 1 second between key frames
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, 0x7F000789); // COLOR_FormatSurface

    // 2. Create MediaCodec
    mEncoder = AMediaCodec_createEncoderByType("video/avc");
    if (!mEncoder) {
        ALOGE("Failed to create H.264 encoder.");
        AMediaFormat_delete(mFormat);
        mFormat = nullptr;
        return nullptr;
    }

    // 3. Configure and Start
    media_status_t status = AMediaCodec_configure(mEncoder, mFormat, nullptr, nullptr, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
    if (status != AMEDIA_OK) {
        ALOGE("Failed to configure encoder: %d", status);
        AMediaCodec_delete(mEncoder);
        AMediaFormat_delete(mFormat);
        mEncoder = nullptr;
        mFormat = nullptr;
        return nullptr;
    }

    status = AMediaCodec_start(mEncoder);
    if (status != AMEDIA_OK) {
        ALOGE("Failed to start encoder: %d", status);
        AMediaCodec_delete(mEncoder);
        AMediaFormat_delete(mFormat);
        mEncoder = nullptr;
        mFormat = nullptr;
        return nullptr;
    }

    // 4. Get Input Surface
    ANativeWindow* window = AMediaCodec_createInputSurface(mEncoder);
    if (!window) {
        ALOGE("Failed to get input surface.");
        AMediaCodec_stop(mEncoder);
        AMediaCodec_delete(mEncoder);
        AMediaFormat_delete(mFormat);
        mEncoder = nullptr;
        mFormat = nullptr;
        return nullptr;
    }

    // 5. Start Encoding Thread
    mRunning = true;
    mEncoderThread = std::thread(&VideoEncoder::encodingLoop, this);

    ALOGI("Encoder started successfully.");
    return window;
}

void VideoEncoder::stopEncoder() {
    if (mRunning) {
        mRunning = false;
        if (mEncoderThread.joinable()) {
            mEncoderThread.join();
        }
    }

    if (mEncoder) {
        AMediaCodec_stop(mEncoder);
        AMediaCodec_delete(mEncoder);
        mEncoder = nullptr;
    }

    if (mFormat) {
        AMediaFormat_delete(mFormat);
        mFormat = nullptr;
    }
    ALOGI("Encoder stopped.");
}

void VideoEncoder::encodingLoop() {
    AMediaCodecBufferInfo info;
    ssize_t status;

    // Send SPS/PPS (Codec Specific Data) once at the start
    // This is typically available after the first key frame, but we can try to get it from the format
    // after the encoder is started.
    AMediaFormat* outputFormat = AMediaCodec_getOutputFormat(mEncoder);
    if (outputFormat) {
        ALOGI("Got initial output format.");
        // The client needs the SPS/PPS data to initialize the decoder.
        // This data is contained in the "csd-0" and "csd-1" keys of the output format.
        // In a real implementation, we would extract and send this data here.
        // For this mock, we assume the client can handle receiving it with the first keyframe.
        AMediaFormat_delete(outputFormat);
    }

    while (mRunning) {
        status = AMediaCodec_dequeueOutputBuffer(mEncoder, &info, TIMEOUT_US);

        if (status >= 0) {
            // Buffer available
            if (info.size > 0) {
                uint8_t* buffer = nullptr;
                size_t bufferSize = 0;
                AMediaCodec_getOutputBuffer(mEncoder, status, &buffer, &bufferSize);

                if (buffer && mNetworkManager) {
                    // Check for key frame (sync frame)
                    bool isKeyFrame = (info.flags & AMEDIACODEC_BUFFER_FLAG_SYNC_FRAME) != 0;
                    sendFrame(buffer, info.size, isKeyFrame);
                }
            }

            // Release the buffer back to the codec
            AMediaCodec_releaseOutputBuffer(mEncoder, status, false);

            if ((info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) != 0) {
                ALOGI("End of stream reached.");
                break;
            }
        } else if (status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
            ALOGI("Output buffers changed.");
        } else if (status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            outputFormat = AMediaCodec_getOutputFormat(mEncoder);
            ALOGI("Output format changed to: %s", AMediaFormat_toString(outputFormat));
            // Send new SPS/PPS data if needed
            AMediaFormat_delete(outputFormat);
        } else if (status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            // ALOGI("No output buffer available yet.");
        } else {
            ALOGE("Unexpected status from dequeueOutputBuffer: %zd", status);
        }
    }
}

void VideoEncoder::sendFrame(const uint8_t* data, size_t size, bool isKeyFrame) {
    // The NetworkManager handles the actual socket transmission.
    // The protocol is: [4-byte size] [NAL Unit Data]
    if (mNetworkManager) {
        mNetworkManager->sendVideoFrame(data, size);
    }
}
