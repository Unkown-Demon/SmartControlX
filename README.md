# 🚀 SmartControlX: Wireless Android Screen Mirroring & Control

<p align="center">
  <img src="https://github.com/Unkown-Demon/SmartControlX/actions/workflows/android_build.yml/badge.svg" alt="Android Build Status">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Language-Python%20%7C%20Kotlin%20%7C%20C++-blue.svg" alt="Languages">
  <img src="https://img.shields.io/badge/Platform-Android%20%7C%20Windows%20%7C%20Linux-green.svg" alt="Platforms">
  <img src="https://img.shields.io/badge/Protocol-TCP%20%7C%20UDP-orange.svg" alt="Protocols">
  <img src="https://img.shields.io/badge/Video-H.264%20(MediaCodec)-red.svg" alt="Video Codec">
</p>

---

## 🇺🇿 O'zbek Tilida Ma'lumot

### 💡 Loyiha Haqida
**SmartControlX** — bu **scrcpy** loyihasidan ilhomlanib yaratilgan, ammo undan ham soddaroq va tezroq ishlashga mo'ljallangan, Wi-Fi orqali Android qurilmasini real vaqtda boshqarish va ekranini aks ettirish uchun mo'ljallangan kross-platformali dastur.

Asosiy maqsadi: Android qurilmasi ekranini kompyuterga yuqori tezlikda uzatish va sichqoncha/klaviatura orqali to'liq boshqaruvni ta'minlash.

### ✨ Asosiy Xususiyatlar

| Xususiyat | Tavsif |
| :--- | :--- |
| **Yuqori Unumdorlik** | Android tomonida **C++ NDK** orqali **H.264** kodlash (`MediaCodec`) yordamida minimal kechikish. |
| **Kross-Platforma** | Server (Android: Kotlin + C++), Klient (Python: Windows/Linux/macOS). |
| **Simsiz Ulanish** | Wi-Fi orqali **TCP** soketlar yordamida video va boshqaruv ma'lumotlarini uzatish. |
| **Avtomatik Topish** | **UDP Broadcast** orqali Android qurilmasining IP manzilini avtomatik aniqlash. |
| **Xavfsiz Ulanish** | Har bir ulanish uchun **PIN kodli pairing** tizimi (xavfsizlik uchun). |
| **Ekran Yozish** | Klientda **`Ctrl + M`** tugmasi orqali uzatilayotgan ekranni MP4 formatida yozib olish. |
| **Statistika** | FPS (kadr tezligi) va Ping (kechikish) ko'rsatkichlarini real vaqtda kuzatish. |

### 🏗️ Arxitektura

| Komponent | Platforma | Texnologiya | Rol |
| :--- | :--- | :--- | :--- |
| **Android Server** | Android | Kotlin, C++ (NDK), MediaProjection | Ekran tasvirini olish, H.264 kodlash, Tarmoqqa uzatish. |
| **PC Klient** | Desktop | Python 3.10+, PyQt6, PyAV, Asyncio | GUI, Streamni qabul qilish, Dekodlash, Boshqaruv hodisalarini yuborish. |

### 🛠️ O'rnatish Bo'yicha Qo'llanma

#### 1. Talablar

*   **Android:** Android 7.0+ (API 24+)
*   **Kompyuter:** Python 3.10+
*   **Android Qurilmasida:** **Dasturchi rejimi (Developer Mode)** va **USB Debugging** yoqilgan bo'lishi shart.

#### 2. Python Klientini O'rnatish

1.  Klient papkasiga o'ting:
    ```bash
    cd SmartControlX/client
    ```
2.  Kerakli kutubxonalarni o'rnating:
    ```bash
    pip install -r requirements.txt
    ```
    *   **Eslatma:** `PyAV` kutubxonasi uchun tizimingizda FFmpeg kutubxonalari o'rnatilgan bo'lishi talab qilinishi mumkin.

#### 3. Android Serverini O'rnatish

1.  **Tayyor APK'ni yuklab olish:** Eng so'nggi APK faylini to'g'ridan-to'g'ri **[GitHub Releases](https://github.com/Unkown-Demon/SmartControlX/releases)** sahifasidan yuklab oling.
2.  **Qurilmaningizga o'rnating:** Yuklab olingan APK faylini Android qurilmangizga o'rnating.
3.  **Qo'lda qurish (Ixtiyoriy):** Agar siz kodni o'zgartirgan bo'lsangiz, `SmartControlX/android` papkasini Android Studio orqali ochib, loyihani qo'lda qurishingiz mumkin.

### 🔌 Ishlatish Bosqichlari

1.  **Wi-Fi Ulanish:** Android va kompyuteringiz bir xil Wi-Fi tarmog'iga ulanganligiga ishonch hosil qiling.
2.  **Serverni Ishga Tushirish:** Android ilovasini oching, IP manzilni ko'ring va **"Start Wireless Control"** tugmasini bosing. Ekranni yozib olishga ruxsat bering.
3.  **Klientni Ishga Tushirish:** Kompyuterda klientni ishga tushiring:
    ```bash
    python SmartControlX.py
    ```
4.  **Ulanish:**
    *   Klient GUI'sida Android ilovasida ko'rsatilgan IP manzilni kiriting.
    *   Yoki **"Discover"** tugmasini bosib, avtomatik topishni sinab ko'ring.
    *   **"Connect"** tugmasini bosing.
5.  **PIN Pairing:** Android ekranida ko'rsatilgan 4 xonali PIN kodni klientga kiriting (hozircha bu avtomatik amalga oshiriladi, ammo xavfsizlik uchun protokol mavjud).
6.  **Boshqaruvni Boshlash:** Ulanish muvaffaqiyatli bo'lgach, Android ekrani kompyuterda paydo bo'ladi va siz sichqoncha/klaviatura orqali boshqarishingiz mumkin.

---

## 🇬🇧 English Documentation

### 💡 About the Project
**SmartControlX** is a cross-platform application for real-time screen mirroring and remote control of an Android device over Wi-Fi. It is inspired by the **scrcpy** project but designed for a simpler, faster, and more integrated user experience.

The core goal is to provide high-speed video transmission of the Android screen to a desktop computer and enable full control via mouse and keyboard.

### ✨ Key Features

| Feature | Description |
| :--- | :--- |
| **High Performance** | Minimal latency achieved through **H.264** encoding (`MediaCodec`) on the Android side using **C++ NDK**. |
| **Cross-Platform** | Server (Android: Kotlin + C++), Client (Python: Windows/Linux/macOS). |
| **Wireless Connectivity** | Video and control data transmission using **TCP** sockets over Wi-Fi. |
| **Auto-Discovery** | Automatic detection of the Android device's IP address via **UDP Broadcast**. |
| **Secure Connection** | **PIN code pairing** system for each connection attempt (for security). |
| **Screen Recording** | Ability to record the mirrored screen to an MP4 file on the client using **`Ctrl + M`**. |
| **Live Statistics** | Real-time monitoring of **FPS** (Frames Per Second) and **Ping** (latency). |

### 🏗️ Architecture

| Component | Platform | Technology | Role |
| :--- | :--- | :--- | :--- |
| **Android Server** | Android | Kotlin, C++ (NDK), MediaProjection | Screen capture, H.264 Encoding, Network Streaming. |
| **PC Client** | Desktop | Python 3.10+, PyQt6, PyAV, Asyncio | GUI, Stream Reception, Decoding, Control Event Sending. |

### 🛠️ Installation Guide

#### 1. Prerequisites

*   **Android:** Android 7.0+ (API 24+)
*   **Desktop:** Python 3.10+
*   **On Android Device:** **Developer Mode** and **USB Debugging** must be enabled.

#### 2. Python Client Setup

1.  Navigate to the client directory:
    ```bash
    cd SmartControlX/client
    ```
2.  Install the required Python libraries:
    ```bash
    pip install -r requirements.txt
    ```
    *   **Note:** The `PyAV` library may require system-level FFmpeg libraries to be installed on your operating system.

#### 3. Android Server Setup

1.  **Download Pre-built APK:** Download the latest APK file directly from the **[GitHub Releases](https://github.com/Unkown-Demon/SmartControlX/releases)** page.
2.  **Install on Device:** Install the downloaded APK file on your Android device.
3.  **Manual Build (Optional):** If you have modified the code, you can open the `SmartControlX/android` folder in Android Studio and build the project manually.

### 🔌 Usage Steps

1.  **Wi-Fi Connection:** Ensure both your Android device and your computer are connected to the same Wi-Fi network.
2.  **Start Server:** Open the **SmartControlX** app on Android, note the IP address, and tap **"Start Wireless Control"**. Grant the screen capture permission.
3.  **Start Client:** Run the Python client on your computer:
    ```bash
    python SmartControlX.py
    ```
4.  **Connect:**
    *   Enter the IP address shown on the Android app into the client GUI.
    *   Alternatively, click **"Discover"** to attempt automatic IP detection.
    *   Click **"Connect"**.
5.  **PIN Pairing:** The 4-digit PIN shown on the Android screen will be used for pairing (the current client implementation handles this automatically for testing, but the secure protocol is in place).
6.  **Start Control:** Once connected, the Android screen will appear on your desktop, and you can control it using your mouse and keyboard.

---

## ⚠️ Known Issues & Limitations

*   **Initial Permission:** The first time you run the server, you must manually grant the **Media Projection** permission on the Android screen.
*   **Input Injection:** The C++ NDK input injection is currently a placeholder that logs the event. Full, non-root input injection on Android is complex and requires system-level access, which is a known limitation for standard user apps.
*   **TLS/Encryption:** The security feature is currently limited to PIN pairing. Full TLS encryption is planned for future releases.

## 📄 License

This project is licensed under the MIT License.
