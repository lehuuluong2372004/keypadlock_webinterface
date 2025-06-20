# Smart Lock System

A comprehensive IoT smart lock system built with ESP32, featuring both physical keypad input and remote web-based control through Firebase Real-time Database.

## 🚀 Features

### Hardware Features
- **4x4 Matrix Keypad**: Physical PIN entry (6-digit PIN: `888888`)
- **Servo Motor Control**: Automated door lock/unlock mechanism
- **LED Status Indicators**: 
  - Red LED: Door locked
  - Green LED: Door unlocked
- **Audio Feedback**: Buzzer for key presses, success/error sounds
- **Auto-lock**: Automatically locks after 10 seconds of being unlocked

### Software Features
- **WiFi Connectivity**: Connects to wireless network for remote access
- **Firebase Integration**: Real-time synchronization with cloud database
- **Web Interface**: Browser-based control panel for remote operation
- **Real-time Status Updates**: Bidirectional communication between hardware and web interface
- **Connection Monitoring**: Automatic WiFi and Firebase reconnection
- **Security**: PIN-based authentication with input validation

## 🛠️ Hardware Requirements

- **ESP32 DevKit C V4** - Main microcontroller
- **4x4 Membrane Keypad** - PIN input interface
- **SG90 Servo Motor** - Door lock mechanism
- **2x LEDs** (Red & Green) - Status indicators
- **Buzzer** - Audio feedback
- **Jumper Wires & Breadboard** - Connections

## 📋 Pin Configuration

| Component | ESP32 Pin | Function |
|-----------|-----------|----------|
| Keypad Rows | 13, 12, 14, 27 | R1-R4 |
| Keypad Cols | 26, 25, 33, 32 | C1-C4 |
| Servo Motor | 18 | PWM Control |
| Red LED | 2 | Lock Status |
| Green LED | 15 | Unlock Status |
| Buzzer | 16 | Audio Feedback |

## 🔧 Software Dependencies

### Arduino Libraries
```ini
Firebase ESP32 client
chris--a/Keypad@^3.1.1
ESP32Servo@^1.1.1
```

### Web Technologies
- HTML5
- CSS3
- JavaScript (ES6+)
- Firebase SDK v9.22.2

## 📱 Installation & Setup

### 1. Hardware Assembly
1. Connect components according to the pin configuration table
2. Ensure proper power supply (5V for servo, 3.3V for ESP32)
3. Verify all connections before powering on

### 2. Firebase Configuration
1. Create a new Firebase project at [Firebase Console](https://console.firebase.google.com/)
2. Enable Realtime Database
3. Update Firebase credentials in both `main.cpp` and `script.js`:
   ```cpp
   #define FIREBASE_HOST "your-project-url"
   #define FIREBASE_AUTH "your-auth-token"
   ```

### 3. Arduino IDE Setup
1. Install ESP32 board support
2. Install required libraries via Library Manager
3. Update WiFi credentials in `main.cpp`:
   ```cpp
   #define WIFI_SSID "Your_WiFi_Name"
   #define WIFI_PASSWORD "Your_WiFi_Password"
   ```
4. Upload code to ESP32

### 4. Web Interface Deployment
1. Host the HTML files on any web server
2. Update Firebase configuration in `script.js`
3. Access the web interface through your browser

## 🎮 Usage

### Physical Operation
1. **Unlock**: Enter PIN `888888` followed by `#`
2. **Clear Input**: Press `C` or `*`
3. **Delete Last Digit**: Press `D`
4. **Auto-lock**: Door automatically locks after 10 seconds

### Web Interface
1. Open the web interface in your browser
2. Monitor real-time lock status
3. Use remote unlock/lock buttons when connected to Firebase
4. Enter PIN using the virtual keypad

## 🔐 Security Features

- **6-digit PIN protection** (default: `888888`)
- **Input validation** and length restrictions
- **Audio/visual feedback** for user actions
- **Auto-lock mechanism** prevents doors being left unlocked
- **Firebase authentication** for remote access

## 📊 System Architecture

```
[ESP32 Hardware] ←→ [WiFi Network] ←→ [Firebase Database] ←→ [Web Interface]
```

- **ESP32**: Physical control and sensor management
- **Firebase**: Real-time data synchronization
- **Web Interface**: Remote monitoring and control

## 🚨 Troubleshooting

### Common Issues

**WiFi Connection Failed**
- Verify SSID and password
- Check signal strength
- Restart ESP32

**Firebase Connection Error**
- Verify Firebase credentials
- Check database rules
- Ensure internet connectivity

**Servo Not Moving**
- Check power supply (servo needs 5V)
- Verify pin connections
- Test servo separately

**Keypad Not Responding**
- Check row/column pin connections
- Verify keypad wiring
- Test individual keys

## 🔄 System States

| State | Red LED | Green LED | Servo Position | Description |
|-------|---------|-----------|----------------|-------------|
| Locked | ON | OFF | 90° | Default secure state |
| Unlocked | OFF | ON | 0° | Temporary access state |
| Error | Blinking | OFF | - | System error/invalid PIN |

## 📈 Future Enhancements

- [ ] Multiple user PINs with different access levels
- [ ] Access logging and history
- [ ] Mobile app development
- [ ] Biometric authentication integration
- [ ] Battery backup system
- [ ] Encrypted communication
- [ ] Email/SMS notifications

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👥 Authors

- **Le Huu Luong** - *Initial work*

## 🙏 Acknowledgments

- Firebase team for the excellent real-time database
- Arduino community for ESP32 support
- Contributors to the open-source libraries used

## 📞 Support

For support and questions:
- Create an issue in this repository
- Check the troubleshooting section
- Review the code comments for implementation details

---

**⚠️ Security Notice**: Change the default PIN (`888888`) before deployment in production environments. This system is designed for educational and prototyping purposes.
