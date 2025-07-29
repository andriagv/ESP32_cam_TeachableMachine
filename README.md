# ESP32-CAM Teachable Machine

A comprehensive ESP32-CAM web interface with Teachable Machine integration for real-time image recognition and pose detection.

## 🚀 Features

- **Real-time Camera Streaming**: Live video feed from ESP32-CAM
- **Teachable Machine Integration**: Support for both image recognition and pose detection models
- **Modern Web Interface**: Beautiful gradient design with responsive layout
- **Camera Controls**: Adjustable resolution, quality, brightness, contrast, and flash
- **Static IP Configuration**: Pre-configured for easy access at `172.20.10.4`
- **Mobile Responsive**: Works seamlessly on both desktop and mobile devices

## 📋 Requirements

- ESP32-CAM module
- MicroSD card (for PSRAM models)
- WiFi network access
- Web browser for interface access

## 🔧 Setup Instructions

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

### 3. Network Configuration
The project uses static IP configuration:
- **IP Address**: `172.20.10.4`
- **Gateway**: `172.20.10.1`
- **Subnet**: `255.255.255.0`

## 🌐 Access URLs

- **Main Interface**: http://172.20.10.4
- **Video Stream**: http://172.20.10.4:81/stream
- **Single Capture**: http://172.20.10.4/capture
- **Status API**: http://172.20.10.4/status
- **Control API**: http://172.20.10.4/control

## 🤖 Teachable Machine Integration

### Image Recognition
1. Train your model on [Teachable Machine](https://teachablemachine.withgoogle.com/)
2. Export as TensorFlow.js model
3. Host the model files (model.json, metadata.json, weights.bin)
4. Enter the model URL in the web interface
5. Click "Start Recognition"

### Pose Detection
1. Train a pose detection model on Teachable Machine
2. Export as TensorFlow.js model
3. Select "Pose Detection" in the interface
4. Enter the model URL and start recognition

## 🎛️ Camera Controls

- **Flash Control**: Adjust LED brightness (0-255)
- **Resolution**: Choose from QQVGA to UXGA
- **Quality**: JPEG quality setting (10-63)
- **Brightness**: Image brightness adjustment (-2 to 2)
- **Contrast**: Image contrast adjustment (-2 to 2)
- **H-Mirror**: Horizontal mirror toggle
- **V-Flip**: Vertical flip toggle

## 📱 Web Interface Features

- **Modern Design**: Gradient backgrounds and smooth animations
- **Responsive Layout**: Adapts to different screen sizes
- **Real-time Updates**: Live camera feed with recognition results
- **Settings Panel**: Collapsible control panel
- **Error Handling**: User-friendly error messages

## 🔄 API Endpoints

### Control Commands
- `GET /control?cmd=restart` - Restart ESP32-CAM
- `GET /control?cmd=ip` - Get IP addresses
- `GET /control?cmd=mac` - Get MAC address
- `GET /control?cmd=flash&val=value` - Control flash LED
- `GET /control?var=framesize&val=value` - Set resolution
- `GET /control?var=quality&val=value` - Set JPEG quality

### Camera Parameters
- `framesize`: 0-10 (QQVGA to UXGA)
- `quality`: 10-63 (JPEG quality)
- `brightness`: -2 to 2
- `contrast`: -2 to 2
- `hmirror`: 0 or 1
- `vflip`: 0 or 1
- `flash`: 0-255

## 🛠️ Troubleshooting

### Common Issues
1. **Camera not initializing**: Check PSRAM availability and power supply
2. **WiFi connection failed**: Verify SSID and password
3. **Stream not loading**: Check network connectivity and firewall settings
4. **Model not loading**: Ensure model URL is accessible and files are properly hosted

### Debug Information
- Serial output available at 115200 baud
- Check Serial Monitor for detailed error messages
- LED indicators show connection status

## 📄 License

This project is open source and available under the MIT License.

## 👨‍💻 Author

**Andria Gvaramia** - Initial development and web interface enhancements

## 📞 Support

For issues and questions:
1. Check the troubleshooting section
2. Review the code comments for detailed explanations
3. Ensure all hardware connections are correct

---

**Note**: This project is designed for educational and development purposes. Ensure proper security measures when deploying in production environments.