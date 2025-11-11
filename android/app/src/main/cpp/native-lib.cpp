#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h>
#include <android/surface_control.h":// Include necessary headers
#include "video_encoder.h"
#include "network_manager.h"

#define LOG_TAG "SmartControlX_JNI"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Global pointers to C++ components
static VideoEncoder* g_encoder = nullptr;
static NetworkManager* g_network_manager = nullptr;

// JNI function to start the server (encoder and network)
extern "C" JNIEXPORT jobject JNICALL
Java_com_smartcontrolx_SmartControlXService_nativeStartServer(
        JNIEnv* env,
        jobject /* this */,
        jint width,
        jint height,
        jint bitrate,
        jobject resultData) {

    ALOGI("nativeStartServer called: %dx%d @ %d bps", width, height, bitrate);

    // 1. Initialize Network Manager
    // Pass the JNI environment and service object for potential callbacks (e.g., input injection)
    g_network_manager = new NetworkManager(env, resultData);
    if (!g_network_manager->startServer()) {
        ALOGE("Failed to start network server.");
        delete g_network_manager;
        g_network_manager = nullptr;
        return nullptr;
    }

    // 2. Initialize Video Encoder
    g_encoder = new VideoEncoder(width, height, bitrate, g_network_manager);
    ANativeWindow* window = g_encoder->startEncoder();

    if (!window) {
        ALOGE("Failed to start video encoder or get native window.");
        g_network_manager->stopServer();
        delete g_network_manager;
        g_network_manager = nullptr;
        delete g_encoder;
        g_encoder = nullptr;
        return nullptr;
    }

    // 3. Convert ANativeWindow* to android.view.Surface object to return to Kotlin
    jobject surface = ANativeWindow_toSurface(env, window);
    if (!surface) {
        ALOGE("Failed to convert ANativeWindow to Surface.");
        g_encoder->stopEncoder();
        g_network_manager->stopServer();
        delete g_network_manager;
        g_network_manager = nullptr;
        delete g_encoder;
        g_encoder = nullptr;
        return nullptr;
    }

    ALOGI("Server started successfully. Returning Surface object.");
    return surface;
}

// JNI function to stop the server
extern "C" JNIEXPORT void JNICALL
Java_com_smartcontrolx_SmartControlXService_nativeStopServer(
        JNIEnv* env,
        jobject /* this */) {

    ALOGI("nativeStopServer called.");

    if (g_encoder) {
        g_encoder->stopEncoder();
        delete g_encoder;
        g_encoder = nullptr;
    }

    if (g_network_manager) {
        g_network_manager->stopServer();
        delete g_network_manager;
        g_network_manager = nullptr;
    }
}

// JNI function to inject input (called from Kotlin/Java side)
extern "C" JNIEXPORT void JNICALL
Java_com_smartcontrolx_MainActivity_nativeInjectInput(
        JNIEnv* env,
        jobject /* this */,
        jint type,
        jint x,
        jint y,
        jint keycode,
        jint action) {
    // This function is a placeholder. In the final design, the input injection
    // logic will be on the C++ side, called by the NetworkManager's control thread.
    // However, for a complete example, we'll keep the JNI declaration here.
    ALOGI("nativeInjectInput called: Type=%d, X=%d, Y=%d, KeyCode=%d, Action=%d", type, x, y, keycode, action);
    // In a real app, this would call a Kotlin/Java method to use InputManager.
}
