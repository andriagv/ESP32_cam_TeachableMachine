# ESP32-CAM Teachable Machine

A comprehensive ESP32-CAM web interface with Teachable Machine integration for real-time image recognition and pose detection.

## Features

- **Real-time Camera Streaming**: Live video feed from ESP32-CAM
- **Teachable Machine Integration**: Support for both image recognition and pose detection models
- **Static IP Configuration**: Pre-configured for easy access at `172.20.10.4`

## Setup Instructions

### 1. Hardware Setup
- Connect ESP32-CAM to your computer via USB
- Insert microSD card (if using PSRAM model)
- Ensure proper power supply

### 2. Software Configuration
1. Open `ESP32-CAM_TeachableMachine.ino` in Arduino IDE
2. Update WiFi credentials in the code:
   ```cpp
   const char* ssid = "your_wifi_ssid";
   const char* password = "your_wifi_password";
   ```
3. Upload the code to your ESP32-CAM

## Access URLs

- **Main Interface**: http://172.20.10.4
- **Video Stream**: http://172.20.10.4:81/stream
- **Single Capture**: http://172.20.10.4/capture
- **Status API**: http://172.20.10.4/status
- **Control API**: http://172.20.10.4/control

## Teachable Machine Integration

### Image Recognition
Train your model on [Teachable Machine](https://teachablemachine.withgoogle.com/)
Click "Start Recognition"

##  API Endpoints

### Control Commands
- `GET /control?cmd=restart` - Restart ESP32-CAM
- `GET /control?cmd=ip` - Get IP addresses
- `GET /control?cmd=mac` - Get MAC address
- `GET /control?cmd=flash&val=value` - Control flash LED
- `GET /control?var=framesize&val=value` - Set resolution
- `GET /control?var=quality&val=value` - Set JPEG quality

### Debug Information
- Serial output available at 115200 baud
- Check Serial Monitor for detailed error messages
- LED indicators show connection status


**Note**: This project is designed for educational and development purposes. Ensure proper security measures when deploying in production environments.
