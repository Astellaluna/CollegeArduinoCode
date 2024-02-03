#include <ArduinoJson.h>

#define USE_AVG
const int sharpLEDPin = 7;   // Arduino digital pin 7 connect to sensor LED.
const int sharpVoPin = A5;   // Arduino analog pin 5 connect to sensor Vo.

// For averaging last N raw voltage readings.
#ifdef USE_AVG
#define N 100
static unsigned long VoRawTotal = 0;
static int VoRawCount = 0;
#endif // USE_AVG

static float Voc = 0.6; // Set the typical output voltage in Volts when there is zero dust.
const float K = 0.5; // typical sensitivity, units of V per 100ug/m3

//Part for MQ7 & MQ135
const int mq7Pin = A0;
const int mq7PWM = 5;

float MQ7analog = 0;
float sensor_volt = 0;
float ratio;
float RS_gas = 0;
float ppm = 0;
float R2 = 2450;
//float R0 = 0;  // Get the value of R0 via in H2
float R0 = 43164.55;

int testing = 0;

void setup() {
  Serial.begin(57600);
  pinMode(mq7PWM, OUTPUT);
  pinMode(sharpLEDPin, OUTPUT);
  Serial.println("Starting...");
}

void loop() {

  //MQ7Calib();

  digitalWrite(sharpLEDPin, LOW); // Turn on the dust sensor LED by setting digital pin LOW.
  delayMicroseconds(280); // Wait 0.28ms before taking a reading of the output voltage as per spec.
  int VoRaw = analogRead(sharpVoPin); // Record the output voltage. This operation takes around 100 microseconds.
  digitalWrite(sharpLEDPin, HIGH);  // Turn the dust sensor LED off by setting digital pin HIGH.
  delayMicroseconds(9620);  // Wait for remainder of the 10us cycle = 10000 - 280 - 100 microseconds.
  float Vo = VoRaw;
#ifdef USE_AVG
  VoRawTotal += VoRaw;
  VoRawCount++;
  if ( VoRawCount >= N ) {
    Vo = 1.0 * VoRawTotal / N;
    VoRawCount = 0;
    VoRawTotal = 0;
  }
  else {
    return;
  }
#endif // USE_AVG
  Vo = Vo / 1024.0 * 5.0; // Compute the output voltage in Volts.
  // Convert to Dust Density in units of ug/m3.
  float dV = Vo - Voc;
  if ( dV < 0 ) {
    dV = 0;
    Voc = Vo;
  }
  float dustDensity = dV / K * 100.0;

  //MQ7========================================
  analogWrite(mq7PWM, 255);
  delay(60000);
  analogWrite(mq7PWM, 72);
  delay(90000);
  analogWrite(mq7PWM, 255);
  MQ7analog = analogRead(mq7Pin);
  sensor_volt = (MQ7analog / 1024 * 5.0);
  RS_gas = (5.0 - sensor_volt) / sensor_volt;
  ratio = RS_gas / R0; //Replace R0 with the value found using the calibration code
  float x = 1538.46 * ratio;
  ppm = pow(x, -1.709);
  int testing = MQ7analog;

  //JSON Stuff
  DynamicJsonBuffer jbuffer;
  JsonObject& root = jbuffer.createObject();
  root["MQ7ppm"] = ppm;
  root["PMppm"] = dustDensity;
  root["test"] = testing;
  root.printTo(Serial);
  root.remove("MQ7ppm");
  root.remove("PMppm");
  root.remove("test");
  Serial.println();
}

/*void MQ7Calib() {
  analogWrite(mq7PWM, 255);
  delay(60000);
  analogWrite(mq7PWM, 72);
  delay(90000);
  analogWrite(mq7PWM, 255);
  MQ7analog = analogRead(mq7Pin);
  Serial.print("Analog Value: ");
  Serial.println(MQ7analog);

  sensor_volt = (MQ7analog / 1024 * 5.0);
  RS_gas = ((5.0 * R2) / sensor_volt) - R2;
  R0 = RS_gas;

  Serial.print("R0: ");
  Serial.println(R0);
  delay(100);
  }*/
