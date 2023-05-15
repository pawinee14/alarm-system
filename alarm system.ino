
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FirebaseESP8266.h>

HTTPClient http;
WiFiClient client;
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
TinyGPSPlus gps;
static const int RXPin = D3, TXPin = D4;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

static const String WIFI_SSID = "Araya_2.4G";                        //Wi-Fi ssid connection *****
static const String WIFI_PASS = "29102535";                     //Wi-Fi password connection ****
#define LINE_TOKEN "9JfLjMn1GgRAPuoQkrIt96s1yqJAiJQHyOkFimU6Hat"  //Token Line

#define FIREBASE_HOST "esp8266i-67afe-default-rtdb.firebaseio.com/"
#define FIREBASE_KEY "4X7nxeMbpoEFvk2c94OdsXGskPFSq1okOMnm73gg"
FirebaseData firebaseData;

unsigned long previousMillis = 0;
const long interval = 1000;  //send data every 1 sec
unsigned long previousMillis2 = 0;
const long interval2 = 1000;

String _lat;
String _lng;
float adx_y; /*เอียง*/
float adx_x;
float adx_z;
int vib;
int rem;
int rem_;
int state;
String str_state[3] = { "UnLock", "Lock  " };
int buttonState[4];
int lastButtonState[4];
int alert;
int alert_count;
String data_send;
int gps_send;
int line_control = 0;
int line_control_ = 0;
int remote_ = 0;

void funtionRun() {
  Serial.println("Line Send VIB LAT:" + _lat + ",LONG:" + _lng + "");
  // Line_Notify(LINE_TOKEN, "LINE BOT");
  Line_Notify(LINE_TOKEN, "locatine is :https://www.google.com/maps?q=" + _lat + "," + _lng + "");
}

void Line_Notify(String token, String message) {
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;
  }
  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(token) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + message).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + message;
  // Serial.println(req);
  client.print(req);
  delay(20);
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    //Serial.println(line);
  }
}

void setup() {
  //WiFi.mode(WIFI_STA);
  Serial.begin(9600);
  ss.begin(GPSBaud);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  if (!accel.begin()) {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1)
      ;
  }
  accel.setRange(ADXL345_RANGE_16_G);
  Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);  //FIREBASE
  pinMode(D5, INPUT);
  pinMode(D0, INPUT);
  pinMode(D7, OUTPUT);
  digitalWrite(D7, HIGH);
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void loop() {
  delay(500);
  firebase_Data_location();
  delay(500);
  firebase_Data_running();

  sensors_event_t event;
  accel.getEvent(&event);
  adx_x = event.acceleration.x;
  adx_y = event.acceleration.y;
  adx_z = event.acceleration.z;
  vib = digitalRead(D5);

  if (line_control == 0 && line_control_ == 0) {
    rem = digitalRead(D0);
    buttonState[3] = rem;
  }


  Serial.println("TS: " + String(buttonState[3]));


  if (adx_x > 3 || adx_x < -3) { /*เอียง*/
    //Line_Notify(LINE_TOKEN, " Front or Back https://www.google.com/maps?q=" + _lat + "," + _lng + ""); /*เอียง*/
    buttonState[2] = 1;

  } else if (adx_y > 3 || adx_y < -3) { /*เอียง*/
    //Line_Notify(LINE_TOKEN, "Right or Left https://www.google.com/maps?q=" + _lat + "," + _lng + ""); /*เอียง*/
    buttonState[2] = 1;
  } else {
    buttonState[2] = 0;
  }

  if (buttonState[2] != lastButtonState[2]) {
    if (buttonState[2] == 1 && state == 1) {
      Serial.println("Line Send adx_y LAT:" + _lat + ",LONG:" + _lng + "");
      Serial.println("Line Send adx_x LAT:" + _lat + ",LONG:" + _lng + "");
      Serial.println("Line Send adx_z LAT:" + _lat + ",LONG:" + _lng + "");

      if (adx_x > 3 || adx_x < -3) {                                                                       /*เอียง*/
        Line_Notify(LINE_TOKEN, " Front or Back https://www.google.com/maps?q=" + _lat + "," + _lng + ""); /*เอียง*/
        buttonState[2] = 1;

      } else if (adx_y > 3 || adx_y < -3) {                                                               /*เอียง*/
        Line_Notify(LINE_TOKEN, "Right or Left https://www.google.com/maps?q=" + _lat + "," + _lng + ""); /*เอียง*/
        buttonState[2] = 1;
      }

      alert = 1;
    }
    lastButtonState[2] = buttonState[2];
  }

  if (buttonState[3] != lastButtonState[3]) {
    if (buttonState[3] == 1) {
      if (state == 0) {
        Line_Notify(LINE_TOKEN, "OPEN");
        state = 1;
        Play_Tone(D6, 2000, 50);
        delay(50);
        Play_Tone(D6, 2000, 50);
      } else {
        state = 0;
        Line_Notify(LINE_TOKEN, "OFF");
        alert = 0;
        Play_Tone(D6, 2000, 50);
      }
    }
    lastButtonState[3] = buttonState[3];
  }

  unsigned long currentMillis2 = millis();
  if (currentMillis2 - previousMillis2 >= interval2) {
    previousMillis2 = currentMillis2;

    if (state == 1 && alert == 1) {
      Play_Tone(D6, 2000, 50);
      digitalWrite(D7, LOW);
    } else {
      digitalWrite(D7, HIGH);
    }

    //    gps_send += 1;
    //    if (gps_send >= 10800) {
    //      Line_Notify(LINE_TOKEN, "https://www.google.com/maps?q=" + _lat + "," + _lng + "");
    //      gps_send = 0;
    //    }

    data_send = "GPS:" + String(gps.charsProcessed()) + ",lat=" + _lat + ",lng=" + _lng + ",VIB=" + String(vib) + ",REM=" + String(rem) + ",State=" + str_state[state] + ",x,y:" + String(adx_x, 0) + "," + String(adx_y, 0) + ",Alert:" + String(alert_count) + ",GPSSEND:" + String(gps_send) + "";
    Serial.println(data_send);
    if (vib == 1 && state == 1) { /*สั่น*/
      buttonState[1] = 1;
      alert = 1;
    } else {
      buttonState[1] = 0;
    }

    if (buttonState[1] != lastButtonState[1]) {
      if (buttonState[1] == 1) {
        Serial.println("Line Send VIB LAT:" + _lat + ",LONG:" + _lng + "");
        Line_Notify(LINE_TOKEN, "Vibration detected https://www.google.com/maps?q=" + _lat + "," + _lng + ""); /*สั่น*/
      }
      lastButtonState[1] = buttonState[1];
    }
  }
  smartDelay(50);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (alert == 1) {
      alert_count += 1;
    }
    if (alert_count >= 10) {
      alert = 0;
      alert_count = 0;
    }
    _lat = String(gps.location.lat(), 6);
    _lng = String(gps.location.lng(), 6);
  }
}

void Play_Tone(int _pin, int _frequency, int _length) {
  pinMode(_pin, OUTPUT);
  analogWriteFreq(_frequency);
  analogWrite(_pin, 512);
  delay(_length);
  analogWrite(_pin, 0);
  pinMode(_pin, INPUT);
}

void firebase_Data_location() {
  if (Firebase.getString(firebaseData, "value")) {
    FirebaseJson &json = firebaseData.jsonObject();
    FirebaseJsonData data;
    json.get(data, "/location");
    int value_location = data.doubleValue;
    Serial.println(value_location);
    if (value_location == 1) {
      funtionRun();
      FirebaseJson data_;
      data_.set("location", 0);
      Firebase.updateNode(firebaseData, "/value", data_);
    }
  }
}

void firebase_Data_running() {
  if (Firebase.getString(firebaseData, "value1")) {
    FirebaseJson &json = firebaseData.jsonObject();
    FirebaseJsonData data;
    json.get(data, "/running");
    int value_running = data.doubleValue;
    Serial.println(value_running);
    if (value_running == 1) {
      line_control = 1;
      if (rem == 1) {
        line_control = 0;
        FirebaseJson data_;
        data_.set("running", 0);
        Firebase.updateNode(firebaseData, "/value1", data_);
      } else if (rem == 0 && buttonState[3] == 0 && line_control_ == 0) {
        buttonState[3] = 1;
      } else if (rem == 0 && buttonState[3] == 1 && line_control_ == 0) {
        line_control_ = 1;
        buttonState[3] = 0;
      }
    } else {
      line_control = 0;
      if (rem == 0 && buttonState[3] == 0 && line_control_ == 1) {
        buttonState[3] = 1;
      }else if(rem == 0 && buttonState[3] == 1 && line_control_ == 1){
        line_control_ = 0;
        buttonState[3] = 0;
      }
    }
  }
}
