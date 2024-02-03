#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define FIREBASE_HOST "-"
#define FIREBASE_AUTH "-"
const char* ssid     = "-";
const char* password = "-";

//For Date&Time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;

//From Arduino
String COppm;
String PMValue;
String testing;

void setup() {
  Serial.begin(57600);
  WiFiconnect();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  timeClient.begin();
  timeClient.setTimeOffset(28800);
  delay(10);
}

void loop() {
  checkInfoFromArduino();
  DateAndTime();

  /*Firebase.setString("OutputSensors/MQ7/" + dayStamp + "/" + timeStamp, COppm);
  if (Firebase.failed()) {
    return;
  }*/
    Firebase.setString("OutputSensors/MQ7/" + dayStamp + "/" + timeStamp, testing);
  if (Firebase.failed()) {
    return;
  }
  Firebase.setString("OutputSensors/PM/" + dayStamp + "/" + timeStamp, PMValue);
  if (Firebase.failed()) {
    return;
  }

  Serial.println("COAnalog: " + testing);
  Serial.println("COppm: " + COppm);
  Serial.println("PM2.5ppm: " + PMValue);
  delay(150011);


}

void checkInfoFromArduino()
{
  bool StringReady;
  String json;

  while (Serial.available()) {
    json = Serial.readString();
    StringReady = true;
  }
  if (StringReady) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json);
    if (!root.success()) {
      //Serial.println("parseObject() failed");
      return ;
    }
    float MQ7PPM = root["MQ7ppm"];
    float PMSen = root["PMppm"];
    float analog = root["test"];
    COppm = MQ7PPM;
    PMValue = PMSen;
    testing = analog;
    root.remove("MQ7ppm");
    root.remove("PMppm");
    root.remove("test");
  }
}

void DateAndTime() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();

  //Extract Date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);

  //Extract Time
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  timeStamp.remove(5);
}


void WiFiconnect() {
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
