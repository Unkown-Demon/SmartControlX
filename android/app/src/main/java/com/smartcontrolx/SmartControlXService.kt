package com.smartcontrolx

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.hardware.display.DisplayManager
import android.hardware.display.VirtualDisplay
import android.media.projection.MediaProjection
import android.media.projection.MediaProjectionManager
import android.os.Build
import android.os.IBinder
import android.util.DisplayMetrics
import android.util.Log
import android.view.WindowManager
import androidx.core.app.NotificationCompat

class SmartControlXService : Service() {

    private lateinit var projectionManager: MediaProjectionManager
    private var mediaProjection: MediaProjection? = null
    private var virtualDisplay: VirtualDisplay? = null

    private var screenWidth = 1280
    private var screenHeight = 720
    private val screenDensity = 1
    private val bitrate = 5000000 // 5 Mbps

    companion object {
        private const val TAG = "SmartControlX_Service"
        private const val NOTIFICATION_CHANNEL_ID = "SmartControlX_Channel"
        private const val NOTIFICATION_ID = 1
        const val EXTRA_RESULT_CODE = "extra_result_code"
        const val EXTRA_RESULT_DATA = "extra_result_data"
        @Volatile var isRunning = false
    }

    override fun onCreate() {
        super.onCreate()
        projectionManager = getSystemService(Context.MEDIA_PROJECTION_SERVICE) as MediaProjectionManager
        isRunning = true
        Log.d(TAG, "Service created.")
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (intent == null) {
            Log.e(TAG, "Service started with null intent. Stopping.")
            stopSelf()
            return START_NOT_STICKY
        }

        val resultCode = intent.getIntExtra(EXTRA_RESULT_CODE, 0)
        val resultData = intent.getParcelableExtra<Intent>(EXTRA_RESULT_DATA)

        if (resultCode != 0 && resultData != null) {
            startForeground(NOTIFICATION_ID, createNotification())
            startProjection(resultCode, resultData)
        } else {
            Log.e(TAG, "Missing MediaProjection data. Stopping.")
            stopSelf()
        }

        return START_STICKY
    }

    private fun createNotification(): Notification {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                NOTIFICATION_CHANNEL_ID,
                "SmartControlX Service",
                NotificationManager.IMPORTANCE_LOW
            )
            (getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager).createNotificationChannel(channel)
        }

        return NotificationCompat.Builder(this, NOTIFICATION_CHANNEL_ID)
            .setContentTitle("SmartControlX")
            .setContentText("Screen sharing is active.")
            .setSmallIcon(R.drawable.ic_launcher_foreground) // Placeholder icon
            .build()
    }

    private fun startProjection(resultCode: Int, resultData: Intent) {
        mediaProjection = projectionManager.getMediaProjection(resultCode, resultData)
        mediaProjection?.registerCallback(MediaProjectionCallback(), null)

        // Get screen dimensions
        val metrics = DisplayMetrics()
        val windowManager = getSystemService(Context.WINDOW_SERVICE) as WindowManager
        windowManager.defaultDisplay.getMetrics(metrics)
        screenWidth = metrics.widthPixels
        screenHeight = metrics.heightPixels
        val densityDpi = metrics.densityDpi

        Log.d(TAG, "Screen size: ${screenWidth}x${screenHeight}, Density: $densityDpi")

        // 1. Start the C++ server and get the Surface from the encoder
        val surface = nativeStartServer(screenWidth, screenHeight, bitrate, resultData)

        // 2. Create VirtualDisplay using the Surface
        virtualDisplay = mediaProjection?.createVirtualDisplay(
            "SmartControlX_VirtualDisplay",
            screenWidth,
            screenHeight,
            densityDpi,
            DisplayManager.VIRTUAL_DISPLAY_FLAG_AUTO_MIRROR,
            surface,
            null,
            null
        )
        Log.d(TAG, "VirtualDisplay created.")
    }

    private fun stopProjection() {
        virtualDisplay?.release()
        virtualDisplay = null
        mediaProjection?.stop()
        mediaProjection = null
        nativeStopServer()
        Log.d(TAG, "Projection and server stopped.")
    }

    override fun onDestroy() {
        super.onDestroy()
        stopProjection()
        isRunning = false
        Log.d(TAG, "Service destroyed.")
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    private inner class MediaProjectionCallback : MediaProjection.Callback() {
        override fun onStop() {
            Log.d(TAG, "MediaProjection stopped by system or user.")
            stopProjection()
            stopSelf()
        }
    }

    // JNI function declarations (implemented in C++)
    // NOTE: nativeStartServer will now return a Surface object (or a pointer to it)
    // For simplicity in this mock, we'll assume the C++ side handles the Surface creation and returns it.
    // In a real NDK project, the C++ side would use AMediaCodec_createInputSurface and pass the ANativeWindow* back,
    // which is then wrapped into a Surface object in Kotlin.
    external fun nativeStartServer(width: Int, height: Int, bitrate: Int, resultData: Intent): android.view.Surface
    external fun nativeStopServer()
}
