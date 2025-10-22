#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MLX90614.h>

#define SCREEN_WIDTH 128    // Lebar display OLED
#define SCREEN_HEIGHT 32    // Tinggi display OLED

#define OLED_RESET -1       // Pin Reset (-1 jika berbagi pin reset Arduino)
#define SCREEN_ADDRESS 0x3C // Alamat I2C display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Inisialisasi satu objek sensor MLX90614
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(115200);

  // Inisialisasi display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Alokasi SSD1306 gagal"));
    for (;;); // Jangan lanjutkan, loop selamanya
  }

  // Inisialisasi sensor
  if (!mlx.begin()) {
    Serial.println("Error terhubung ke sensor MLX. Cek kabel.");
    // Tampilkan pesan error di OLED juga
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("MLX Sensor NOT FOUND!");
    display.display();
    while (1);
  }

  Serial.println("Sensor MLX dan OLED siap.");
  
  // Tampilkan pesan awal di OLED
  display.clearDisplay();
  display.setTextSize(2); // Ukuran teks 2x agar mudah dibaca
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 8); // Posisi agak ke tengah
  display.print("READY");
  display.display();
  delay(2000);
}

void loop() {
  // Baca suhu dari sensor
  // PENTING: Gunakan readObjectTempC() untuk suhu objek, bukan readAmbientTempC()
  float objectTempC = mlx.readObjectTempC();
  
  // Tampilkan ke Serial Monitor (untuk debug)
  Serial.print("Object Temperature: ");
  Serial.print(objectTempC);
  Serial.println(" *C");

  // Tampilkan ke Layar OLED
  display.clearDisplay();
  display.setTextSize(1);      // Gunakan font yang lebih besar
  display.setCursor(0, 8);   // Atur posisi kursor (x, y)

  display.print("Obj: ");
  display.print(objectTempC, 1); // Tampilkan suhu dengan 1 angka desimal
  display.print(" C");

  display.display(); // Kirim buffer ke layar
  
  delay(1000); // Jeda 1 detik
}