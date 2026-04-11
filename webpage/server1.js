const express = require('express');
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json());

let sensorData = {
  soil: 0,
  temp: 0,
  hum: 0,
  pump: 0,
  crop: 0
};

let command = "";

/* RECEIVE DATA */
app.get('/update', (req, res) => {
  sensorData = req.query;

  console.log("\n📥 Data Received:");
  console.log(sensorData);
  console.log("From:", req.ip);
  console.log("---------------------");

  res.send("OK");
});

/* SEND DATA TO WEB */
app.get('/data', (req, res) => {
  res.json(sensorData);
});

/* COMMAND ROUTES */
app.get('/manualOn', (req, res) => {
  command = "MAN_ON";
  res.send("OK");
});

app.get('/manualOff', (req, res) => {
  command = "MAN_OFF";
  res.send("OK");
});

app.get('/auto', (req, res) => {
  command = "AUTO";
  res.send("OK");
});

app.get('/setCrop', (req, res) => {
  command = `CROP:${req.query.id}`;
  res.send("OK");
});

/* ESP FETCHES COMMAND */
app.get('/command', (req, res) => {
  res.send(command);
  command = "";
});

/* START SERVER */
const PORT = 3000;
const HOST = "0.0.0.0";

app.listen(PORT, HOST, () => {
  console.log("🚀 Backend Running");
  console.log("Listening on http://10.86.240.105:3000");
});