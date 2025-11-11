package com.smartcontrolx

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.media.projection.MediaProjectionManager
import android.net.wifi.WifiManager
import android.os.Bundle
import android.text.format.Formatter
import android.util.Log
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var projectionManager: MediaProjectionManager
    private lateinit var ipAddressTextView: TextView
    private lateinit var startButton: Button

    companion object {
        private const val TAG = "SmartControlX_Main"
        // Load the native C++ library
        init {
            System.loadLibrary("smartcontrolx")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main) // Assuming a simple layout with a button and text view

        projectionManager = getSystemService(Context.MEDIA_PROJECTION_SERVICE) as MediaProjectionManager
        ipAddressTextView = findViewById(R.id.ipAddressTextView)
        startButton = findViewById(R.id.startButton)

        // Display the device's local IP address
        displayLocalIpAddress()

        startButton.setOnClickListener {
            if (SmartControlXService.isRunning) {
                stopService()
            } else {
                requestMediaProjection()
            }
        }

        // Update button text based on service status
        updateUi(SmartControlXService.isRunning)
    }

    private fun displayLocalIpAddress() {
        val wm = applicationContext.getSystemService(WIFI_SERVICE) as WifiManager
        @Suppress("DEPRECATION")
        val ipAddress: String = Formatter.formatIpAddress(wm.connectionInfo.ipAddress)
        ipAddressTextView.text = getString(R.string.ip_address_label, ipAddress)
    }

    private fun updateUi(isRunning: Boolean) {
        startButton.text = if (isRunning) getString(R.string.stop_control) else getString(R.string.start_control)
    }

    private fun requestMediaProjection() {
        Log.d(TAG, "Requesting MediaProjection permission.")
        // Create the screen capture permission request intent
        val captureIntent = projectionManager.createScreenCaptureIntent()
        mediaProjectionLauncher.launch(captureIntent)
    }

    private val mediaProjectionLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == Activity.RESULT_OK) {
            Log.d(TAG, "MediaProjection permission granted. Starting service.")
            val intent = Intent(this, SmartControlXService::class.java).apply {
                putExtra(SmartControlXService.EXTRA_RESULT_CODE, result.resultCode)
                putExtra(SmartControlXService.EXTRA_RESULT_DATA, result.data)
            }
            startForegroundService(intent)
            updateUi(true)
        } else {
            Log.e(TAG, "MediaProjection permission denied.")
            Toast.makeText(this, "Screen capture permission denied.", Toast.LENGTH_SHORT).show()
            updateUi(false)
        }
    }

    private fun stopService() {
        Log.d(TAG, "Stopping service.")
        val intent = Intent(this, SmartControlXService::class.java)
        stopService(intent)
        updateUi(false)
    }

    // JNI function declarations (implemented in C++)
    external fun nativeStartServer(width: Int, height: Int, bitrate: Int, resultData: Intent): Int
    external fun nativeStopServer()
    external fun nativeInjectInput(type: Int, x: Int, y: Int, keycode: Int, action: Int)
}
