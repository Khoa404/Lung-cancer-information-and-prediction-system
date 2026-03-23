#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h> // Thêm thư viện này để truy cập Firestore (HTTPS)

// === SIM768x Setup ===
HardwareSerial sim(1); // UART1: RX=16, TX=17
#define SIM_RX 16
#define SIM_TX 17

// === WiFi ===
#define WIFI_SSID "Nguyen Thi Mai"
#define WIFI_PASSWORD "13051969"

// === OLED ===
#define SDA_PIN 21
#define SCL_PIN 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32 // Đã sửa thành 32 theo code test của bạn
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === LED ===
#define LED_PIN 4

// === Keypad ===
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {18, 19, 32, 33};
byte colPins[COLS] = {25, 26, 27, 14};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// === Server URLs ===
const String SERVER_URL = "http://192.168.1.129:5000/process";
const String ADD_PATIENT_URL = "http://192.168.1.129:5000/add_patient";

// === SMS Config ===
String phoneNumber = "+84968059853";  // SỐ NHẬN CẢNH BÁO MẶC ĐỊNH
bool smsSent = false; 

String inputCode = "";
String targetPhone = ""; 

// Forward Declarations
void checkPatient(String id);
void processImage(String id);
void addNewPatient(String id);
void displayMsg(String text);
void initSIM768x();
void sendAT(String cmd);
void readSIMResponse();
void sendSMS(String phone, String message);

void setup() {
  Serial.begin(115200);
  sim.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);

  pinMode(LED_PIN, OUTPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Khởi động màn hình
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED Failed"));
    for(;;);
  }
  
  // --- QUAN TRỌNG: CÀI ĐẶT MÀU CHỮ ---
  display.setTextColor(SSD1306_WHITE); 
  // -----------------------------------
  
  displayMsg("Connecting WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  initSIM768x();

  displayMsg("Nhap ma BN (6 so)");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == '#') {
      inputCode = "";
      smsSent = false;
      displayMsg("Reset!");
      delay(500);
      displayMsg("Nhap ma BN (6 so)");
    } 
    else if (key == '*') {
      checkPatient(inputCode);
    } 
    else if (key == 'B') {
      processImage(inputCode);
    } 
    else if (key == 'A') {
      addNewPatient(inputCode);
    } 
    else if (isDigit(key) && inputCode.length() < 6) {
      inputCode += key;
      display.clearDisplay();
      display.setCursor(20, 10);
      display.setTextSize(2); // Hiển thị số to hơn cho dễ nhìn
      display.print(inputCode);
      display.display();
    }
  }
  readSIMResponse();
}

// === Hàm hiển thị thông báo lên OLED ===
void displayMsg(String text) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE); // Đảm bảo luôn set màu trắng
  display.setCursor(0, 10);
  display.println(text);
  display.display();
}

// === Khởi động SIM768x ===
void initSIM768x() {
  Serial.println("Init SIM...");
  sendAT("AT");
  sendAT("ATE0");
  sendAT("AT+CMGF=1");          
  sendAT("AT+CSCS=\"GSM\"");    
  sendAT("AT+CSMP=17,167,0,0");
  sendAT("AT+CNMI=2,2,0,0,0");  
  delay(1000);
}

void sendAT(String cmd) {
  sim.println(cmd);
  delay(500);
}

void readSIMResponse() {
  if (sim.available()) {
    String response = sim.readString();
    Serial.println("[SIM] " + response);
  }
}

void sendSMS(String phone, String message) {
  if (phone.startsWith("0")) phone = "+84" + phone.substring(1);
  if (!phone.startsWith("+")) phone = "+" + phone;

  Serial.println("Gui SMS den: " + phone);

  if (!sim) return;

  sim.print("AT+CMGS=\"");
  sim.print(phone);
  sim.println("\"");
  delay(1000);

  sim.print(message);
  delay(500);
  sim.write(26);
  delay(5000);
}

// === Kiểm tra bệnh nhân (Firestore HTTPS) ===
void checkPatient(String id) {
  if (id.length() != 6 || WiFi.status() != WL_CONNECTED) return;
  
  displayMsg("Checking...");

  // Tạo client bảo mật cho HTTPS
  WiFiClientSecure client;
  client.setInsecure(); // Bỏ qua kiểm tra chứng chỉ SSL (Quan trọng cho ESP32)

  String url = "https://firestore.googleapis.com/v1/projects/da2-dudoanungthuphoi/databases/(default)/documents/patients/" + id;
  HTTPClient http;
  
  // Sử dụng client bảo mật
  http.begin(client, url); 
  
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    
    const char* name = doc["fields"]["name"]["stringValue"];

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Ten BN:");
    display.setCursor(0, 15);
    display.println(name); // Xuống dòng in tên cho rõ
    display.display();
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Khong tim thay!");
    display.setCursor(0, 15);
    display.println("Bam A de them");
    display.display();
  }
  http.end();
}

// === Xử lý ảnh ===
void processImage(String id) {
  if (id.length() != 6 || WiFi.status() != WL_CONNECTED) return;

  smsSent = false;
  displayMsg("Dang xu ly...");

  WiFiClient client; // HTTP thường
  HTTPClient http;
  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{\"id\":\"" + id + "\"}";
  int httpCode = http.POST(jsonData);

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(4096); 
    deserializeJson(doc, payload);

    const char* result = doc["result"];
    const char* phoneFromServer = doc["phone"]; 
    targetPhone = String(phoneFromServer);

    if (strcmp(result, "positive") == 0) {
      digitalWrite(LED_PIN, HIGH);
      displayMsg("DUONG TINH!");
      
      if (targetPhone.length() > 8) {
         String msg = "CANH BAO: BN " + id + " duong tinh voi ung thu phoi!";
         sendSMS(targetPhone, msg);
      } else {
         Serial.println("Khong co SDT!");
      }
      
    } else {
      digitalWrite(LED_PIN, LOW);
      displayMsg("AM TINH!");
    }
  } else {
    displayMsg("Loi Server!");
  }
  http.end();
}

// === Thêm bệnh nhân mới ===
void addNewPatient(String id) {
  if (id.length() != 6 || WiFi.status() != WL_CONNECTED) return;

  displayMsg("Dang them moi...");

  WiFiClient client;
  HTTPClient http;
  http.begin(client, ADD_PATIENT_URL);
  http.addHeader("Content-Type", "application/json");

  String jsonData = "{\"id\":\"" + id + "\"}";
  int httpCode = http.POST(jsonData);

  if (httpCode == 201) {
    displayMsg("Them thanh cong!");
    delay(1000);
    displayMsg("Nhap ma BN (6 so)");
  } else {
    displayMsg("Loi them moi!");
    delay(1000);
  }
  http.end();
}