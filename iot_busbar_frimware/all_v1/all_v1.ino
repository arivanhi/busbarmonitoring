#include <Wire.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>
#include <Preferences.h>

// --- Definisi Pin ---
#define PIN_BTN_UP 32
#define PIN_BTN_DOWN 33
#define PIN_BTN_RESET 25
#define buzzer 13

// --- Pengaturan OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Sensor & HTTP ---
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
HTTPClient http;

// --- Variabel Global ---
Preferences preferences;
float calibrationOffset = 0.0;
float currentTemp = 0.0;
String serverEndpoint;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 250;

// === VARIABEL BARU ===
float alarmThreshold = 50.0; // Nilai default 50 C

// --- Fungsi setupOLED() (Tidak Berubah) ---
void setupOLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Alokasi SSD1306 gagal"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED OK!");
  display.display();
  delay(1000);
}

// --- Fungsi updateOLED() (Tidak Berubah) ---
void updateOLED(String line1, String line2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(line1);
  display.setTextSize(1);
  display.setCursor(0, 18);
  display.print(line2);
  display.display();
}


// === FUNGSI checkButtons() (DIMODIFIKASI TOTAL) ===
void checkButtons() {
  // Cek apakah jeda debounce sudah lewat
  if ((millis() - lastDebounceTime) < debounceDelay) {
    return; // Belum saatnya, keluar
  }

  // Baca status semua tombol
  bool upPressed = (digitalRead(PIN_BTN_UP) == LOW);
  bool downPressed = (digitalRead(PIN_BTN_DOWN) == LOW);
  bool resetPressed = (digitalRead(PIN_BTN_RESET) == LOW);

  // --- LOGIKA KOMBINASI TOMBOL ---
  if (resetPressed) {
    if (upPressed) {
      // Kombinasi: RESET + UP -> Tambah Treshold
      alarmThreshold += 1.0; // Tambah 1 derajat
      preferences.putFloat("threshold", alarmThreshold);
      Serial.print("New Threshold: "); Serial.println(alarmThreshold);
      updateOLED("Alarm:", String(alarmThreshold, 0) + " C");
      lastDebounceTime = millis();
      delay(500); // Tampilkan di OLED selama 0.5 detik
    } 
    else if (downPressed) {
      // Kombinasi: RESET + DOWN -> Kurangi Treshold
      alarmThreshold -= 1.0; // Kurangi 1 derajat
      preferences.putFloat("threshold", alarmThreshold);
      Serial.print("New Threshold: "); Serial.println(alarmThreshold);
      updateOLED("Alarm:", String(alarmThreshold, 0) + " C");
      lastDebounceTime = millis();
      delay(500); // Tampilkan di OLED selama 0.5 detik
    }
    else {
      // --- LOGIKA RESET WIFI ---
      // Hanya RESET yang ditekan (UP dan DOWN tidak)
      Serial.println("Reset button pressed. Hold for 3s...");
      updateOLED("Resetting", "Hold 3s...");
      delay(3000); // Tunggu 3 detik

      // Cek lagi setelah 3 detik
      // Pastikan UP/DOWN tidak ditekan selama menunggu
      if (digitalRead(PIN_BTN_RESET) == LOW && digitalRead(PIN_BTN_UP) == HIGH && digitalRead(PIN_BTN_DOWN) == HIGH) {
        Serial.println("Resetting WiFi settings!");
        updateOLED("WiFi Reset!", "Restarting...");
        WiFiManager wifiManager;
        wifiManager.resetSettings();
        delay(1000);
        ESP.restart();
      } else {
        Serial.println("Reset/Combo cancelled.");
      }
      lastDebounceTime = millis(); // Setel ulang timer debounce
    }
  }
  // --- LOGIKA TOMBOL TUNGGAL (OFFSET) ---
  else if (upPressed) {
    // Hanya UP -> Tambah Offset
    calibrationOffset += 0.5;
    preferences.putFloat("offset", calibrationOffset);
    Serial.print("New Offset: "); Serial.println(calibrationOffset);
    updateOLED("Offset:", String(calibrationOffset, 1));
    lastDebounceTime = millis();
    delay(500);
  } 
  else if (downPressed) {
    // Hanya DOWN -> Kurangi Offset
    calibrationOffset -= 0.5;
    preferences.putFloat("offset", calibrationOffset);
    Serial.print("New Offset: "); Serial.println(calibrationOffset);
    updateOLED("Offset:", String(calibrationOffset, 1));
    lastDebounceTime = millis();
    delay(500);
  }
}


// --- FUNGSI SETUP UTAMA ---
void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(PIN_BTN_UP, INPUT_PULLUP);
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  setupOLED();

  if (!mlx.begin(MLX90614_I2CADDR, &Wire)) {
    Serial.println("Error connecting to MLX sensor.");
    updateOLED("MLX FAIL!", "Check wiring");
    while (1);
  }
  Serial.println("MLX Sensor OK!");

  // === PERUBAHAN DI SINI ===
  // 3. Ambil data tersimpan (Offset DAN Treshold)
  preferences.begin("sensor-app", false);
  calibrationOffset = preferences.getFloat("offset", 0.0);
  serverEndpoint = preferences.getString("server_ip", "");
  // Ambil "threshold", jika tidak ada, gunakan nilai default (50.0)
  alarmThreshold = preferences.getFloat("threshold", 50.0); 

  Serial.print("Loaded offset: "); Serial.println(calibrationOffset);
  Serial.print("Loaded endpoint: "); Serial.println(serverEndpoint);
  Serial.print("Loaded Threshold: "); Serial.println(alarmThreshold); // Tampilkan

  // 4. Inisialisasi WiFiManager
  WiFiManager wifiManager;
  WiFiManagerParameter custom_server_ip("server_ip", "Alamat API Server", serverEndpoint.c_str(), 100);
  wifiManager.addParameter(&custom_server_ip);

  updateOLED("Connecting", "WiFi...");
  if (!wifiManager.autoConnect("SensorConfigAP")) {
    Serial.println("Failed to connect and hit timeout");
    updateOLED("WiFi FAIL", "Restarting...");
    delay(3000);
    ESP.restart();
  }

  // 5. Jika Koneksi WiFi Berhasil
  Serial.println("WiFi Connected!");
  updateOLED("IP:", WiFi.localIP().toString());
  delay(1000);

  // 6. Ambil dan Simpan nilai dari parameter kustom
  serverEndpoint = custom_server_ip.getValue();
  preferences.putString("server_ip", serverEndpoint);

  Serial.print("New endpoint saved: "); Serial.println(serverEndpoint);
  updateOLED("Server:", "CONFIGURED");
  delay(2000);

  Serial.println("Setup complete. Starting loop.");
}

// --- FUNGSI LOOP UTAMA ---
void loop() {
  checkButtons(); // Cek tombol (termasuk kombo)

  float tempC = mlx.readObjectTempC();
  if (isnan(tempC)) {
    Serial.println("Failed to read from MLX sensor!");
    Wire.begin();
    delay(100);
    tempC = mlx.readObjectTempC();
    if (isnan(tempC)) {
      updateOLED("T: ERROR", "Re-reading...");
      delay(1000);
      return;
    }
  }

  currentTemp = tempC + calibrationOffset;

  // === PERUBAHAN DI SINI ===
  // Gunakan variabel 'alarmThreshold' (bukan angka 50)
  if (currentTemp > alarmThreshold) {
    digitalWrite(buzzer, HIGH);
  } else {
    digitalWrite(buzzer, LOW);
  }

  // === PERUBAHAN DI SINI ===
  // Tampilkan Treshold (A) dan Offset (O) di baris kedua
  String line2 = "A:" + String(alarmThreshold, 0) + " O:" + String(calibrationOffset, 1);
  updateOLED("T: " + String(currentTemp, 1) + " C", line2);

  // --- Logika HTTP POST (Tidak Berubah) ---
  if (WiFi.status() == WL_CONNECTED && serverEndpoint.length() > 0) {
    StaticJsonDocument<100> doc;
    doc["temperature"] = currentTemp;
    doc["offset"] = calibrationOffset;
    doc["threshold"] = alarmThreshold; // Kirim treshold juga ke server
    String jsonPayload;
    serializeJson(doc, jsonPayload);

    http.begin(serverEndpoint);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected or server endpoint not set.");
  }

  delay(500);
}