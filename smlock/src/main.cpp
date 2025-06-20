#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Keypad.h>
#include <ESP32Servo.h>

// WiFi credentials
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// Firebase project credentials
#define FIREBASE_HOST "https://smartlock-844b9-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "WC9koLDXvUDBxslqAywSw1UzF8DhiWFaZWjm8weO"

// Firebase objects
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Keypad configuration (4x4)
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Servo motor configuration
Servo doorServo;
#define SERVO_PIN 18
#define LOCK_POSITION 0     // Khóa cửa: 90 độ
#define UNLOCK_POSITION 90    // Mở khóa: 0 độ

// System variables
String inputPIN = "";
const String CORRECT_PIN = "888888";
bool isLocked = true;
bool webUnlocked = false;
unsigned long lastUnlockTime = 0;
unsigned long lastFirebaseCheck = 0;
unsigned long lastStatusUpdate = 0;
const unsigned long AUTO_LOCK_DELAY = 10000;
const unsigned long FIREBASE_CHECK_INTERVAL = 1000; // Tăng interval để giảm tải
const unsigned long STATUS_UPDATE_INTERVAL = 1000;

// LED and buzzer pins
#define RED_LED_PIN 2
#define GREEN_LED_PIN 15
#define BUZZER_PIN 16
#define BUZZER_CHANNEL 0

bool firebaseConnected = false;
bool wifiConnected = false;
int lastFirebaseStatus = -1;
bool servoInitialized = false;

// Function declarations
void handleKeypadInput(char key);
void checkPIN(String pin);
void unlockDoor(bool fromWeb = false);
void lockDoor(bool fromWeb = false);
void checkFirebaseCommands();
void updateFirebaseStatus(bool force = false);
void playTone(int freq, int dur);
void playSuccessSound();
void playErrorSound();
void checkWiFiConnection();
void syncStatus();
void initializeServo();

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Setup PWM for buzzer
  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

  // Initialize servo
  initializeServo();

  // Initial state - locked
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  wifiConnected = true;

  // Configure Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  firebaseConnected = true;

  // Initialize Firebase with current status
  updateFirebaseStatus(true);
  delay(1000);
  
  // Read initial Firebase status
  if (Firebase.getInt(firebaseData, "status")) {
    lastFirebaseStatus = firebaseData.intData();
  }
}

void loop() {
  // Check WiFi connection
  checkWiFiConnection();
  
  // Handle keypad input
  char key = keypad.getKey();
  if (key) {
    handleKeypadInput(key);
  }

  // Check Firebase commands
  if (firebaseConnected && (millis() - lastFirebaseCheck > FIREBASE_CHECK_INTERVAL)) {
    lastFirebaseCheck = millis();
    checkFirebaseCommands();
  }

  // Periodic status sync
  if (firebaseConnected && (millis() - lastStatusUpdate > STATUS_UPDATE_INTERVAL)) {
    lastStatusUpdate = millis();
    syncStatus();
  }

  // Auto-lock after 10 seconds
  if (!isLocked && (millis() - lastUnlockTime > AUTO_LOCK_DELAY)) {
    lockDoor();
  }

  delay(100);
}

void initializeServo() {
  if (!servoInitialized) {
    doorServo.attach(SERVO_PIN);
    delay(100);
    if (doorServo.read() != LOCK_POSITION) {
      doorServo.write(LOCK_POSITION);
      delay(500);
    }
    doorServo.detach();
    servoInitialized = true;
    Serial.println("Servo initialized - Door LOCKED at 90 degrees");
  }
}

void checkWiFiConnection() {
  static unsigned long lastWiFiCheck = 0;
  
  if (millis() - lastWiFiCheck > 10000) {
    lastWiFiCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED && wifiConnected) {
      wifiConnected = false;
      firebaseConnected = false;
      Serial.println("WiFi connection lost!");
    } else if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
      wifiConnected = true;
      Serial.println("WiFi reconnected!");
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);
      firebaseConnected = true;
      updateFirebaseStatus(true);
    }
  }
}

void handleKeypadInput(char key) {
  playTone(800, 50);
  
  if (key >= '0' && key <= '9') {
    if (inputPIN.length() < 6) {
      inputPIN += key;
    } else {
      playErrorSound();
    }
  } else if (key == '#') {
    if (inputPIN.length() > 0) {
      checkPIN(inputPIN);
    } else {
      playTone(400, 200);
    }
  } else if (key == 'C' || key == '*') {
    inputPIN = "";
    playTone(600, 100);
  } else if (key == 'D') {
    if (inputPIN.length() > 0) {
      inputPIN = inputPIN.substring(0, inputPIN.length() - 1);
      playTone(600, 100);
    }
  }
}

void checkPIN(String pin) {
  if (pin == CORRECT_PIN) {
    unlockDoor();
    Serial.println("PIN correct - Door UNLOCKED");
  } else {
    playErrorSound();
    delay(1000);
    inputPIN = "";
    Serial.println("Wrong PIN!");
  }
}

void unlockDoor(bool fromWeb) {
  if (isLocked) {
    doorServo.attach(SERVO_PIN);
    if (doorServo.read() != UNLOCK_POSITION) {
      doorServo.write(UNLOCK_POSITION);
      delay(500);
    }
    doorServo.detach();
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    playSuccessSound();
    isLocked = false;
    webUnlocked = fromWeb;
    lastUnlockTime = millis();
    inputPIN = "";
    Serial.println("Door UNLOCKED at 0 degrees");
    updateFirebaseStatus(true);
  }
}

void lockDoor(bool fromWeb) {
  if (!isLocked) {
    doorServo.attach(SERVO_PIN);
    if (doorServo.read() != LOCK_POSITION) {
      doorServo.write(LOCK_POSITION);
      delay(500);
    }
    doorServo.detach();
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    playTone(600, 300);
    isLocked = true;
    webUnlocked = false;
    Serial.println("Door LOCKED at 90 degrees");
    updateFirebaseStatus(true);
  }
}

void checkFirebaseCommands() {
  if (Firebase.getInt(firebaseData, "status")) {
    int firebaseStatus = firebaseData.intData();
    if (firebaseStatus != lastFirebaseStatus) {
      lastFirebaseStatus = firebaseStatus;
      if (firebaseStatus == 1 && isLocked) {
        unlockDoor(true);
        Serial.println("Unlocked remotely via web");
      } else if (firebaseStatus == 0 && !isLocked) {
        lockDoor(true);
        Serial.println("Locked remotely via web");
      }
    }
  } else {
    Serial.println("Failed to read Firebase status");
  }
}

void updateFirebaseStatus(bool force) {
  if (firebaseConnected) {
    int statusValue = isLocked ? 0 : 1;
    if (force || statusValue != lastFirebaseStatus) {
      if (Firebase.setInt(firebaseData, "status", statusValue)) {
        lastFirebaseStatus = statusValue;
      } else {
        Serial.println("Failed to update Firebase");
      }
    }
  }
}

void syncStatus() {
  if (Firebase.getInt(firebaseData, "status")) {
    int firebaseStatus = firebaseData.intData();
    int localStatus = isLocked ? 0 : 1;
    if (firebaseStatus != localStatus) {
      updateFirebaseStatus(true);
    }
  }
}

void playTone(int freq, int dur) {
  ledcWriteTone(BUZZER_CHANNEL, freq);
  delay(dur);
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void playSuccessSound() {
  playTone(1000, 120);
  delay(80);
  playTone(1300, 120);
  delay(80);
  playTone(1600, 200);
}

void playErrorSound() {
  for (int i = 0; i < 3; i++) {
    playTone(300, 200);
    delay(150);
  }
}