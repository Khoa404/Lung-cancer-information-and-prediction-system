#  Lung Cancer Information and Prediction System

> A comprehensive system optimized for real-world operation, aimed at supporting early detection of lung cancer signs through AI and secure data management.


##  Overview

This is a complete system ready for real-world operation. The system integrates:

- ** Hardware**: Devices that interact directly with users to collect medical data
- ** Processing Block**: Data processing and management system (C++, Python)
- ** AI Model**: YOLO model to predict lung cancer signs (Precision: 98.4%, Recall: 98.4%)
- ** Firebase**: Secure cloud database for storing patient information
- ** Web Interface**: Professional web interface for doctors and patients

---

##  Key Features

 **Hardware Integration**: Direct communication with medical devices (ESP32, LCD, Keypad, LTE Module)  
 **High-Accuracy AI Prediction**: YOLOv8 model detects lung cancer signs  
 **Secure Storage**: Patient data protected on Firebase  
 **Professional Interface**: Easy-to-use web app for doctors and patients  
 **SMS Alerts**: Automatic notifications to patients via SMS  
 **Production Ready**: System optimized for real-world environment  

---

##  Technologies Used

| Component | Technology | Notes |
|-----------|-----------|--------|
| **Processing Block** | C++, Python | Image processing and AI |
| **AI Model** | YOLO (Object Detection) | Cancer cell detection |
| **Database** | Firebase Realtime DB | Patient data storage |
| **Frontend** | HTML, CSS, JavaScript | Web interface |
| **Hardware** | LCD, Keypad, LEDs | Display and control |
| **Connectivity** | WiFi, LTE (SIM76xx) | Network connection |

---

##  Project Structure

```
Lung-cancer-information-and-prediction-system/
├── hardware/
│   ├── esp32_cam/           # Firmware code for ESP32-CAM
│   ├── CameraWebServer/     # Web server on device
│   └── MKE_M21_SIM768x_demo/# LTE Module demo
├── processing_block/
│   ├── server.py            # AI processing server
│   ├── best_10k_n.pt        # YOLOv8 AI model
│   ├── CT_images/           # Input CT images
│   └── results/             # Prediction results
├── KhoiXuLy/                # Data processing block
│   ├── server.py
│   ├── firebase_key.json    # Firebase key
│   
├── web/
│   ├── index.html           # Web interface
│   └── web_model/           # Web model
├── datasheet/               # Technical documentation
└── README.md               # This documentation
```

---

##  Workflow

```
┌─────────────────────────────────────────────────────────────────┐
│ 1. Enter patient ID                                             │
│    (Via Keypad / Web Interface)                                 │
└────────────────────┬────────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────────┐
│ 2. Collect CT image                                             │
│    (From ESP32-CAM or Upload)                                   │
└────────────────────┬────────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────────┐
│ 3. Preprocessing & AI Prediction (YOLOv8)                       │
│    (Processing Block - Python Server)                           │
└────────────────────┬────────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────────┐
│ 4. Update results                                               │
│    ├─ → Firebase (Storage)                                      │
│    ├─ → Web (Display)                                           │
│    └─ → ESP32 (Control)                                         │
└────────────────────┬────────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────────┐
│ 5. Effects / Alerts                                             │
│    ├─ LCD Display (Result)                                      │
│    ├─ Turn on LED (If positive)                                 │
│    └─ Send SMS (Patient notification)                           │
└─────────────────────────────────────────────────────────────────┘
```

##  Performance

| Metric | Value |
|--------|-------|
| Precision | 98.4% |
| Recall | 98.4% |
| F1-Score | 98.4% |
| Processing Time | < 2 seconds |


## 🎥 Demo 

- **[Demo Videos](https://drive.google.com/drive/folders/1ZxyfhrfRWlsEHcXAybjVt0QtV9cxcDsR?usp=drive_link)**

---
