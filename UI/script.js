// Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyASHBfOP8kDDg33QTZbXWt6ofehsxBidps",
  authDomain: "smartlock-844b9.firebaseapp.com",
  databaseURL: "https://smartlock-844b9-default-rtdb.firebaseio.com/",
  projectId: "smartlock-844b9",
  storageBucket: "smartlock-844b9.firebasestorage.app",
  messagingSenderId: "361347722439",
  appId: "1:361347722439:web:e384d1fcbce9be77ef62e2",
  measurementId: "G-LLX766MZ8B"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);
const database = firebase.database();

let display = document.getElementById('numberDisplay');
let statusElement = document.getElementById('status');
let connectionStatus = document.getElementById('connectionStatus');
let lastSyncElement = document.getElementById('lastSync');
let unlockBtn = document.getElementById('unlockBtn');
let lockBtn = document.getElementById('lockBtn');

const CORRECT_PIN = "888888";
let currentLockState = null; // null = unknown, false = locked, true = unlocked
let currentInput = "";
let isConnected = false;
let autoLockTimer = null;

// Firebase references
const statusRef = database.ref('status');
const connectedRef = database.ref('.info/connected');

// Initialize system
function init() {
    console.log('Smart Lock Web Interface Starting...');
    
    // Monitor Firebase connection
    connectedRef.on('value', function(snapshot) {
        isConnected = snapshot.val() === true;
        updateConnectionStatus();
    });
    
    // Listen for status changes from Firebase
    statusRef.on('value', function(snapshot) {
        const firebaseStatus = snapshot.val();
        if (firebaseStatus !== null) {
            updateLockState(firebaseStatus);
            updateLastSync();
        }
    });
    
    // Initialize UI
    updateDisplay();
}

// Update connection status
function updateConnectionStatus() {
    if (isConnected) {
        connectionStatus.textContent = '🟢 Connected';
        connectionStatus.className = 'connection-status connected';
        unlockBtn.disabled = false;
        lockBtn.disabled = false;
    } else {
        connectionStatus.textContent = '🔴 Disconnected';
        connectionStatus.className = 'connection-status disconnected';
        unlockBtn.disabled = true;
        lockBtn.disabled = true;
    }
}

// Update lock state from Firebase
function updateLockState(firebaseStatus) {
    const newState = firebaseStatus === 1;
    
    if (currentLockState !== newState) {
        currentLockState = newState;
        updateStatusDisplay();
        
        // Clear any existing auto-lock timer
        if (autoLockTimer) {
            clearTimeout(autoLockTimer);
            autoLockTimer = null;
        }
        
        console.log('Lock state updated:', currentLockState ? 'UNLOCKED' : 'LOCKED');
    }
}

// Update status display
function updateStatusDisplay() {
    if (currentLockState === true) {
        statusElement.textContent = "UNLOCKED";
        statusElement.className = "status unlocked";
        unlockBtn.disabled = true;
        lockBtn.disabled = false;
    } else if (currentLockState === false) {
        statusElement.textContent = "LOCKED";
        statusElement.className = "status locked";
        unlockBtn.disabled = false;
        lockBtn.disabled = true;
    } else {
        statusElement.textContent = "CHECKING...";
        statusElement.className = "status checking";
        unlockBtn.disabled = true;
        lockBtn.disabled = true;
    }
}

// Update last sync time
function updateLastSync() {
    const now = new Date();
    lastSyncElement.textContent = now.toLocaleTimeString();
}

// Update Firebase status
function updateFirebaseStatus(status) {
    const statusValue = status === "UNLOCKED" ? 1 : 0;
    statusRef.set(statusValue)
        .then(() => {
            console.log("Firebase updated:", statusValue, "(" + status + ")");
        })
        .catch((error) => {
            console.error("Firebase update error:", error);
        });
}

// Add number/character to display
function addNumber(value) {
    if (value === 'D') {
        // Delete last character (backspace)
        deleteLastCharacter();
    } else if (value === 'C') {
        // Clear all
        clearAll();
    } else if (value === '#') {
        // Check password
        checkPassword();
    } else {
        // Add character (max 6 characters)
        if (currentInput.length < 6) {
            currentInput += value;
            updateDisplay();
        }
    }
}

// Update display
function updateDisplay() {
    // Show asterisks for security
    display.value = '*'.repeat(currentInput.length);
}

// Delete last character
function deleteLastCharacter() {
    if (currentInput.length > 0) {
        currentInput = currentInput.slice(0, -1);
        updateDisplay();
    }
}

// Clear all input
function clearAll() {
    currentInput = "";
    display.value = "";
}

// Check password
function checkPassword() {
    if (currentInput === CORRECT_PIN) {
        // Correct password
        currentLockState = true;
        statusElement.textContent = "UNLOCKED";
        statusElement.className = "status unlocked";
        
        // Update Firebase
        updateFirebaseStatus("UNLOCKED");
        
        // Auto-lock after 5 seconds (same as ESP32)
        autoLockTimer = setTimeout(() => {
            lockDevice();
        }, 5000);
        
    } else {
        // Wrong password
        statusElement.textContent = "WRONG PASSWORD";
        statusElement.className = "status error";
        
        // Reset after 2 seconds
        setTimeout(() => {
            updateStatusDisplay();
        }, 2000);
    }
    
    // Clear input after checking
    clearAll();
}

// Lock device
function lockDevice() {
    currentLockState = false;
    statusElement.textContent = "LOCKED";
    statusElement.className = "status locked";
    
    // Update Firebase
    updateFirebaseStatus("LOCKED");
    
    // Clear auto-lock timer
    if (autoLockTimer) {
        clearTimeout(autoLockTimer);
        autoLockTimer = null;
    }
    
    clearAll();
}

// Remote unlock
function remoteUnlock() {
    if (!isConnected) {
        alert('Not connected to Firebase!');
        return;
    }
    
    updateFirebaseStatus("UNLOCKED");
    console.log('Remote unlock command sent');
}

// Remote lock
function remoteLock() {
    if (!isConnected) {
        alert('Not connected to Firebase!');
        return;
    }
    
    updateFirebaseStatus("LOCKED");
    console.log('Remote lock command sent');
}

// Initialize when page loads
document.addEventListener('DOMContentLoaded', function() {
    init();
});