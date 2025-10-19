const express = require("express");
const bodyParser = require("body-parser");
const fs = require("fs");
const path = require("path");

const app = express();
const port = 5000; // ESP32 akan mengirim data ke port ini
app.use(bodyParser.json());

// Path ke file log kita
const dataLogPath = path.join(__dirname, "data.json");

// 1. ENDPOINT UNTUK MENERIMA DATA DARI ESP32
app.post("/api/data", (req, res) => {
	// Ambil data dari ESP32 (misal: { "temperature": 50.2, "offset": 0.1 })
	const sensorData = req.body;

	// Buat entri data baru dengan timestamp UTC (Format Standar)
	const newDataEntry = {
		timestamp: new Date().toISOString(),
		...sensorData, // Salin semua data dari ESP32 (temperature, offset)
	};

	console.log("Menerima data baru:", newDataEntry);

	// Baca file data.json yang ada
	fs.readFile(dataLogPath, "utf8", (err, data) => {
		let dataArray = []; // Inisialisasi sebagai array kosong

		if (err) {
			// Jika file tidak ada (ENOENT), tidak apa-apa, kita buat baru.
			if (err.code !== "ENOENT") {
				console.error("Gagal membaca file:", err);
				return res.status(500).send("Server error");
			}
			console.log("File data.json tidak ditemukan, akan membuat file baru.");
		} else {
			// Jika file ada, parse datanya
			try {
				if (data) dataArray = JSON.parse(data);
				if (!Array.isArray(dataArray)) dataArray = []; // Reset jika korup
			} catch (parseErr) {
				console.error("Gagal mem-parsing data.json, akan di-reset:", parseErr);
				dataArray = [];
			}
		}

		// Tambahkan (push) data baru ke dalam array
		dataArray.push(newDataEntry);

		// Tulis kembali (overwrite) file dengan array yang sudah di-update
		fs.writeFile(dataLogPath, JSON.stringify(dataArray, null, 2), (err) => {
			if (err) {
				console.error("Gagal menyimpan file:", err);
				return res.status(500).send("Server error");
			}

			console.log("Data berhasil ditambahkan ke data.json");
			res.status(200).send({ message: "Data diterima dan disimpan" });
		});
	});
});

// 2. ENDPOINT UNTUK DIBACA OLEH REACT (FRONTEND)
app.get("/api/getdata", (req, res) => {
	fs.readFile(dataLogPath, "utf8", (err, data) => {
		if (err) {
			if (err.code === "ENOENT") return res.status(200).send([]); // Kirim array kosong
			console.error("Gagal membaca file:", err);
			return res.status(500).send("Server error");
		}
		res.setHeader("Content-Type", "application/json");
		res.send(data || "[]"); // Kirim data mentah (string JSON)
	});
});

// Jalankan server
app.listen(port, () => {
	console.log(`Server API berjalan di http://localhost:${port}`);
	console.log("Menunggu data dari ESP32 di endpoint /api/data");
});
