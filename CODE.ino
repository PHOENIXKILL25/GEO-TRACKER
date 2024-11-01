#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <MPU6050.h>
#include <math.h>

// WiFi credentials
const char* ssid = "SSID-NAME";
const char* password = "SSID-PASSWORD";

// MPU6050 and GPS setup
#define MPU6050_ADDR 0x68  // I2C Address for MPU6050
#define RXPin 16
#define TXPin 17
MPU6050 mpu6050(MPU6050_ADDR);
HardwareSerial gpsSerial(1);

// HTML page content
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Geo Tracker</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; }
    .value { font-size: 2em; }
    #compass { transform-origin: 50% 50%; }
  </style>
</head>
<body>
  <h1>Geo Tracker</h1>
  <svg id="compass" width="200" height="200" viewBox="0 0 100 100">
    <line x1="50" y1="50" x2="50" y2="10" stroke="red" stroke-width="2"/>
    <circle cx="50" cy="50" r="48" stroke="black" stroke-width="2" fill="none"/>
  </svg>
  <div>
    <div>Pitch: <span class="value" id="pitch">0</span> °</div>
    <div>Roll: <span class="value" id="roll">0</span> °</div>
    <div>Yaw: <span class="value" id="yaw">0</span> °</div>
    <div>Temperature: <span class="value" id="temperature">0</span> °C</div>
    <div>Latitude: <span class="value" id="latitude">0</span></div>
    <div>Longitude: <span class="value" id="longitude">0</span></div>
  </div>
  <script>
    setInterval(function() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('pitch').innerText = data.pitch.toFixed(2);
          document.getElementById('roll').innerText = data.roll.toFixed(2);
          document.getElementById('yaw').innerText = data.yaw.toFixed(2);
          document.getElementById('temperature').innerText = data.temperature.toFixed(2);
          document.getElementById('latitude').innerText = data.latitude;
          document.getElementById('longitude').innerText = data.longitude;
          document.getElementById('compass').style.transform = 'rotate(' + data.yaw + 'deg)';
        });
    }, 1000);
  </script>
</body>
</html>
)rawliteral";

AsyncWebServer server(80);

float latitude = 0.0;
float longitude = 0.0;
float pitch = 0.0, roll = 0.0, yaw = 0.0, temperature = 0.0;
float alpha = 0.9;
float elapsedTime = 0.1;
float gyroAngleZ = 0.0;

void readMPU6050Data(int16_t& accX, int16_t& accY, int16_t& accZ) {
  int16_t gyroX, gyroY, gyroZ;
  mpu6050.getMotion6(&accX, &accY, &accZ, &gyroX, &gyroY, &gyroZ);

  float accXg = accX / 16384.0;
  float accYg = accY / 16384.0;
  float accZg = accZ / 16384.0;

  float accAngleX = atan(accYg / sqrt(pow(accXg, 2) + pow(accZg, 2))) * 180 / PI;
  float accAngleY = atan(-accXg / sqrt(pow(accYg, 2) + pow(accZg, 2))) * 180 / PI;

  pitch = alpha * (pitch + gyroX * elapsedTime) + (1.0 - alpha) * accAngleX;
  roll = alpha * (roll + gyroY * elapsedTime) + (1.0 - alpha) * accAngleY;
  gyroAngleZ += gyroZ * elapsedTime;

  pitch = fmod(pitch + 180, 360) - 180;
  roll = fmod(roll + 180, 360) - 180;
  yaw = fmod(gyroAngleZ + 180, 360) - 180;

  temperature = mpu6050.getTemperature() / 340.0 + 36.53;
}

void parseGPSData(String data) {
  if (data.startsWith("$GPGGA")) {
    String parts[15];
    int i = 0;
    char *str = strdup(data.c_str());
    const char *delimiter = ",";
    char *token = strtok(str, delimiter);

    while (token != NULL) {
      parts[i++] = token;
      token = strtok(NULL, delimiter);
    }

    latitude = atof(parts[2].c_str());
    longitude = atof(parts[4].c_str());
    free(str);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  gpsSerial.begin(9600, SERIAL_8N1, RXPin, TXPin);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  mpu6050.initialize();
  if (!mpu6050.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }
  Serial.println("MPU6050 connected");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    int16_t xAccel, yAccel, zAccel;
    readMPU6050Data(xAccel, yAccel, zAccel);

    String json = "{\"pitch\":" + String(pitch) + ",\"roll\":" + String(roll) + ",\"yaw\":" + String(yaw) +
                  ",\"temperature\":" + String(temperature) + ",\"latitude\":" + String(latitude) + 
                  ",\"longitude\":" + String(longitude) + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  if (gpsSerial.available() > 0) {
    String gpsData = gpsSerial.readStringUntil('\n');
    parseGPSData(gpsData);
  }
  
  // Serial Debugging
  int16_t xAccel, yAccel, zAccel;
  readMPU6050Data(xAccel, yAccel, zAccel);
  Serial.print("Pitch: ");
  Serial.print(pitch);
  Serial.print("°, Roll: ");
  Serial.print(roll);
  Serial.print("°, Yaw: ");
  Serial.print(yaw);
  Serial.print("°, Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Latitude: ");
  Serial.print(latitude);
  Serial.print(", Longitude: ");
  Serial.println(longitude);

  delay(1000);
}
