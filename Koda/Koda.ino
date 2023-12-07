/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID "TMPL4k1rrmy-9"
#define BLYNK_TEMPLATE_NAME "IoT Kontroler za Temperaturo in Vlažnost"
#define BLYNK_AUTH_TOKEN "iG3Jf_BBBFk-YKR7cKgqwrecjCrF5ILN"
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "wifi";
char pass[] = "password";

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Preferences.h>
#include <AceButton.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

using namespace ace_button;
Preferences pref;

Adafruit_BME280 bme;

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SH1106 display(21, 22);

// Setpoint and setHumi values (in Celsius)
float setTemp = 0;
float setHumi = 0;
float currentTemp = 0;
float currentHumi = 0;

// define the GPIO connected with Relays and Buttons
#define RelayPin1 18  //D18
#define RelayPin2 19  //D19
#define RelayPin3 5  //D5

#define ButtonPin1 25  //D25
#define ButtonPin2 26  //D26 
#define ButtonPin3 27  //D27
#define ButtonPin4 13  //D21

#define wifiLed   2   //D2

//Change the virtual pins according the rooms
#define VPIN_Text           V0
#define VPIN_Mode           V1
#define VPIN_currentTemp    V2
#define VPIN_currentHumi    V3
#define VPIN_setTemp        V4
#define VPIN_setHumi        V5
#define VPIN_Grelec         V6
#define VPIN_Vlazilec       V7
#define VPIN_Ventilator     V8

// Relay and Mode State
bool grelecState = LOW; //Define integer to remember the toggle state for Grelec
bool vlazilecState = LOW; 
bool ventilatorState = LOW;//Define integer to remember the toggle state for Vlazilec
bool modeState = LOW; //Define integer to remember the mode

int wifiFlag = 0;

char auth[] = BLYNK_AUTH_TOKEN;

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);
ButtonConfig config4;
AceButton button4(&config4);

void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);
void handleEvent4(AceButton*, uint8_t, uint8_t);

BlynkTimer timer;

// When App button is pushed - switch the state

BLYNK_WRITE(VPIN_Grelec) {
  grelecState = param.asInt();
  digitalWrite(RelayPin1, !grelecState);
  pref.putBool("Grelec", grelecState);
}

BLYNK_WRITE(VPIN_Vlazilec) {
  vlazilecState = param.asInt();
  digitalWrite(RelayPin2, !vlazilecState);
  pref.putBool("Vlazilec", vlazilecState);
}

BLYNK_WRITE(VPIN_Ventilator) {
  ventilatorState = param.asInt();
  digitalWrite(RelayPin3, !ventilatorState);
  pref.putBool("Ventilator", ventilatorState);
}

BLYNK_WRITE(VPIN_Mode) {
  modeState = param.asInt();
  pref.putBool("Mode", modeState);
}

BLYNK_WRITE(VPIN_setTemp) {
  setTemp = param.asFloat();
  pref.putBool("setemp", setTemp);
}

BLYNK_WRITE(VPIN_setHumi) {
  setHumi = param.asFloat();
  pref.putBool("Humidity", setHumi);
}

void checkBlynkStatus() { // called every 2 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    wifiFlag = 1;
    Serial.println("Blynk Not Connected");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 2);
    digitalWrite(wifiLed, LOW);
  }
  if (isconnected == true) {
    wifiFlag = 0;
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(2, 2);
    display.println(" Blynk IoT Connected ");
    digitalWrite(wifiLed, HIGH);
    Blynk.virtualWrite(VPIN_Text, "IoT Temperature & Humidity Controller");
  }
  display.display();
  delay(2000);
}

BLYNK_CONNECTED() {
  // update the latest state to the server
  Blynk.virtualWrite(VPIN_Text, "IoT Temperature & Humidity Controller");
  Blynk.virtualWrite(VPIN_Mode, modeState);
  Blynk.syncVirtual(VPIN_currentTemp);
  Blynk.syncVirtual(VPIN_currentHumi);
  Blynk.syncVirtual(VPIN_setTemp);
  Blynk.syncVirtual(VPIN_setHumi);
  Blynk.virtualWrite(VPIN_Grelec, grelecState);
  Blynk.virtualWrite(VPIN_Vlazilec, vlazilecState);
  Blynk.virtualWrite(VPIN_Ventilator, ventilatorState);

}

void setup()
{
  Serial.begin(115200);
  //Open namespace in read-write mode
  pref.begin("Relay_State", false);

  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(wifiLed, OUTPUT);

  pinMode(ButtonPin1, INPUT_PULLUP);
  pinMode(ButtonPin2, INPUT_PULLUP);
  pinMode(ButtonPin3, INPUT_PULLUP);
  pinMode(ButtonPin4, INPUT_PULLUP);

  display.begin(SH1106_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay();
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  //display.display();

  //During Starting all Relays should TURN OFF
  digitalWrite(RelayPin1, !grelecState);
  digitalWrite(RelayPin2, !vlazilecState);
  digitalWrite(RelayPin3, !ventilatorState);

  // Enabling DHT sensor
    bool status;
  status = bme.begin(0x76);
  digitalWrite(wifiLed, LOW);

  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);
  config4.setEventHandler(button4Handler);

  button1.init(ButtonPin1);
  button2.init(ButtonPin2);
  button3.init(ButtonPin3);
  button4.init(ButtonPin4);

  //Blynk.begin(auth, ssid, pass);
  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  timer.setInterval(1000L, sendSensor); // Sending Sensor Data to Blynk Cloud every 1 second
  Blynk.config(auth);
  delay(1000);

  getRelayState(); // Get the last state of Relays and Set values of Temp & Humidity

  Blynk.virtualWrite(VPIN_Grelec, grelecState);
  Blynk.virtualWrite(VPIN_Vlazilec, vlazilecState);
  Blynk.virtualWrite(VPIN_Ventilator, ventilatorState);
  Blynk.virtualWrite(VPIN_setTemp, setTemp);
  Blynk.virtualWrite(VPIN_setHumi, setHumi);
}

void readSensor() {

  currentTemp = bme.readTemperature();
  currentHumi = bme.readHumidity();
  if (isnan(currentHumi) || isnan(currentTemp)) {
    Serial.println("Failed to read from BME sensor!");
    return;
  }
}

void sendSensor()
{
  readSensor();
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(VPIN_Text, "IoT Temperature & Humidity Controller");
  Blynk.virtualWrite(VPIN_currentTemp, currentTemp);
  Blynk.virtualWrite(VPIN_currentHumi, currentHumi);

}

void getRelayState()
{
  //Serial.println("reading data from NVS");
  modeState = pref.getBool("Mode", 0);
  Blynk.virtualWrite(VPIN_Mode, modeState);
  delay(100);
  grelecState = pref.getBool("Grelec", 0);
  digitalWrite(RelayPin1, !grelecState);
  Blynk.virtualWrite(VPIN_Grelec, grelecState);
  delay(100);
  vlazilecState = pref.getBool("Vlazilec", 0);
  digitalWrite(RelayPin2, !vlazilecState);
  Blynk.virtualWrite(VPIN_Vlazilec, vlazilecState);
  delay(100);
  ventilatorState = pref.getBool("Ventilator", 0);
  digitalWrite(RelayPin3, !ventilatorState);
  Blynk.virtualWrite(VPIN_Ventilator, ventilatorState);
  delay(100);
  setTemp = pref.getBool("setemp", 0);
  Blynk.virtualWrite(VPIN_setTemp, setTemp);
  delay(100);
  setHumi = pref.getBool("Vlaznost", 0);
  Blynk.virtualWrite(VPIN_setHumi, setHumi);
  delay(100);
}


void DisplayData()  {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(5, 7);
  display.println("----------");
  display.setCursor(5, 20);
  display.print(int(currentTemp));
  display.print((char)247);
  display.println("C");
  display.setCursor(60, 20);
  display.print(": ");
  display.print(int(currentHumi));
  display.println("%");
  display.setCursor(5, 34);
  display.println("----------");

  display.setTextSize(1);
  display.setCursor(0, 57);
  display.print("Temp:");
  display.print(int(setTemp));
  display.print((char)247);
  display.println("C");
  display.setTextSize(1);
  display.setCursor(80, 57);
  display.print("Vlaz:");
  display.print(int(setHumi));
  display.println("%");


  if (modeState == 1) {
    display.setTextSize(1);
    display.setCursor(24, 45);
    display.print("Automatic Mode");
    if (currentTemp < setTemp) {
      grelecState = 1;
      digitalWrite(RelayPin1, !grelecState);
      pref.putBool("Grelec", grelecState);
      Serial.println("Grelec ON");
      Blynk.virtualWrite(VPIN_Grelec, grelecState);
    } else {
      grelecState = 0;
      digitalWrite(RelayPin1, !grelecState);
      pref.putBool("Grelec", grelecState);
      Serial.println("Grelec OFF");
      Blynk.virtualWrite(VPIN_Grelec, grelecState);
    } if (currentHumi < setHumi) {
      vlazilecState = 1;
      digitalWrite(RelayPin2, !vlazilecState);
      Serial.println("Vlazilec ON");
      pref.putBool("Vlazilec", vlazilecState);
      Blynk.virtualWrite(VPIN_Vlazilec, vlazilecState);
    } else {
      vlazilecState = 0;
      digitalWrite(RelayPin2, !vlazilecState);
      Serial.println("Vlazilec OFF");
      pref.putBool("Vlazilec", vlazilecState);
      Blynk.virtualWrite(VPIN_Vlazilec, vlazilecState);
          } 
      if (setHumi + 15 < currentHumi) {
      ventilatorState = 1;
      digitalWrite(RelayPin3, !ventilatorState);
      Serial.println("Ventilator ON");
      pref.putBool("Ventilator", ventilatorState);
      Blynk.virtualWrite(VPIN_Ventilator, ventilatorState);
    } else {
      ventilatorState = 0;
      digitalWrite(RelayPin3, !ventilatorState);
      Serial.println("Ventilator OFF");
      pref.putBool("Ventilator", ventilatorState);
      Blynk.virtualWrite(VPIN_Ventilator, ventilatorState);
    }
  } else {
    display.setTextSize(1);
    display.setCursor(38, 45);
    display.print("Hand Mode");
  }
  display.display();
}


void loop()
{
  
  Blynk.run();
  timer.run();
  DisplayData();
  button1.check();
  button2.check();
  button3.check();
  button4.check();
}

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Mode");
  switch (eventType) {
    case AceButton::kEventReleased:
      modeState = !modeState;
      pref.putBool("Mode", modeState);
      Blynk.virtualWrite(VPIN_Mode, modeState);
      break;
  }
}
void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Grelec");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin1, grelecState);
      grelecState = !grelecState;
      pref.putBool("Grelec", grelecState);
      Blynk.virtualWrite(VPIN_Grelec, grelecState);
      break;
  }
}
void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Vlazilec");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin2, vlazilecState);
      vlazilecState = !vlazilecState;
      pref.putBool("VPIN_Vlazilec", vlazilecState);
      Blynk.virtualWrite(VPIN_Vlazilec, vlazilecState);
      break;
  }
}
void button4Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("Ventilator");
  switch (eventType) {
    case AceButton::kEventReleased:
      digitalWrite(RelayPin3, ventilatorState);
      ventilatorState = !ventilatorState;
      pref.putBool("VPIN_Ventilator", ventilatorState);
      Blynk.virtualWrite(VPIN_Ventilator, ventilatorState);
      break;
  }
}
