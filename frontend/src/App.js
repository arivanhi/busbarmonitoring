import React, { useState, useEffect } from "react";
import "./App.css";

function App() {
	const [sensorData, setSensorData] = useState(null);
	const [logCount, setLogCount] = useState(0);
	const [error, setError] = useState(null);

	const fetchData = () => {
		fetch("/api/getdata")
			.then((response) => response.json())
			.then((dataArray) => {
				if (Array.isArray(dataArray) && dataArray.length > 0) {
					const latestData = dataArray[dataArray.length - 1];
					setSensorData(latestData);
					setLogCount(dataArray.length);
					setError(null);
				} else {
					setSensorData(null);
					setLogCount(0);
					setError("Menunggu data pertama dari sensor...");
				}
			})
			.catch((error) => {
				console.error("Error fetching data:", error);
				setError("Gagal terhubung ke server API.");
			});
	};

	useEffect(() => {
		fetchData();
		const interval = setInterval(fetchData, 2000);
		return () => clearInterval(interval);
	}, []);

	if (error && !sensorData) {
		return <div className="App-header">{error}</div>;
	}
	if (!sensorData) {
		return <div className="App-header">Loading data sensor...</div>;
	}

	return (
		<div className="App-header">
			<h1>Busbar Monitor</h1>
			<div className="card">
				<div className="label">Suhu Objek</div>
				<span className="temp">{sensorData.temperature.toFixed(1)}</span>
				<span className="unit">&deg;C</span>
				<div className="offset">(Offset: {sensorData.offset.toFixed(1)})</div>
				<div className="timestamp">
					Terakhir update:{" "}
					{new Date(sensorData.timestamp).toLocaleString("id-ID", {
						timeZone: "Asia/Jakarta",
						hour: "2-digit",
						minute: "2-digit",
						second: "2-digit",
						day: "2-digit",
						month: "short",
						year: "numeric",
					})}
				</div>
			</div>
			{/* <div className="log-count">Total data ter-log: {logCount}</div> */}
		</div>
	);
}

export default App;
