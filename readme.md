# Dashboard Sensor Suhu ESP32 (Full Stack)

Proyek ini adalah sistem pemantauan suhu _full-stack_ yang menggunakan ESP32 untuk membaca sensor suhu non-kontak (MLX90614) dan mengirimkan datanya ke _backend_ Node.js. Backend mencatat data ke file `data.json`, dan _frontend_ React menampilkan data secara _real-time_ di _dashboard_ web.

## Arsitektur Proyek

- **ESP32**: Membaca sensor, terhubung ke WiFi menggunakan `WiFiManager` (untuk konfigurasi), dan mengirim data (HTTP POST) ke _backend_.
- **Backend** (`/backend`): Server Node.js/Express yang:
  1.  Menerima data dari ESP32 di _endpoint_ `/api/data`.
  2.  Menyimpan data dengan _timestamp_ ke file `data.json`.
  3.  Menyajikan data ke _frontend_ di _endpoint_ `/api/getdata`.
- **Frontend** (`/frontend`): Aplikasi React yang:
  1.  Mengambil data dari `/api/getdata` _backend_ setiap 2 detik.
  2.  Menampilkan suhu, offset, dan total log terbaru.

---

## 🚀 Cara Menjalankan (di Komputer Baru)

Berikut adalah langkah-langkah untuk menjalankan proyek ini di komputer baru setelah melakukan `git clone`.

### 1. Prasyarat

- Pastikan Anda telah menginstal [Node.js](https://nodejs.org/) (versi 16 atau lebih baru direkomendasikan). Ini juga akan menginstal `npm`.
- [Git](https://git-scm.com/) terinstal.
- ESP32 Anda sudah di-flash dengan firmware yang sesuai.

### 2. Kloning Repositori

Buka terminal dan kloning repositori ini:

```bash
git clone https://github.com/arivanhi/busbarmonitoring.git
cd busbarmonitoring
```

### 3. Cara instalasi

Pastikan berada di folder busbarmonitoring

1. Install dependencies untuk root (concurrently)

```bash
   npm install
```

2. Install dependencies untuk backend (Express, dll)

```bash
   npm install --prefix backend
```

3. Install dependencies untuk frontend (React, dll)

```bash
   npm install --prefix frontend
```

4. jalankan server

```bash
   npm run dev
```
