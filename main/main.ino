/*
 * Author: Alex Waclawik
 * Github: https://github.com/AlexWaclawik/Arduino-IoT-Switch
 * Version: 1.1
 * This project is for a remote AC outlet switch that is made using 
 * an Arduino UNO-like microcontroller, and the Blynk IoT library.
 */

// these Blynk definitions ALWAYS have to be first
#define BLYNK_TEMPLATE_ID "TEMPLATE-ID"
#define BLYNK_DEVICE_NAME "DEVICE NAME"
#define BLYNK_AUTH_TOKEN "YOUR-AUTH-TOKEN"
#define BLYNK_PRINT Serial

/* 
 * IMPORTANT: Configuration of Blynk Heartbeat Interval
 * Please read the Blynk Heartbeat Configuration section of the README
 * as configuring this is very important for your application
 * https://github.com/AlexWaclawik/Arduino-IoT-Switch/tree/testing#operation
 */
#define BLYNK_HEARTBEAT 300 

// define the SIM type
#define TINY_GSM_MODEM_SIM7000

// libraries
#include <SPI.h>
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>

#define SerialMon Serial

// pin definitions
#define TX 10 // Microcontroller RX
#define RX 11 // Microcontroller TX
#define RELAY_PIN1 12 // Relay Control Pin
// just one relay pin needed for now
// if you want to use the other one, just uncomment all the relevant lines
//#define RELAY_PIN2 XX

// initialize software serial
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(TX, RX);

// configure GPRS settings (user and pass may be optional depending on your SIM card)
const char apn[] = "iot.1nce.net"; // currently set for 1NCE SIMs
const char user[] = "";
const char pass[] = "";
// configure Blynk authentication token
const char auth[] = "YOUR-AUTH-TOKEN";

// initialize modem
TinyGsm modem(SerialAT);

// define global variables
bool startup = true;
float deviceUptime = 0;
bool relayStatus = false;
float relayStartTime = 0;


/*
 * called whenever a widget writes to virtual pin V1
 * it will switch the relay on and off, turn on the status LED, and push a notification
 * lastly it will call the updateDeviceTime() function which will update the uptime
 */
BLYNK_WRITE(V1) {
  // retrieve value from virtual pin V1
  uint8_t pinValue = param.asInt();
  if (pinValue != 0) {
    // turn on relay
    digitalWrite(RELAY_PIN1, LOW);
    //digitalWrite(RELAY_PIN2, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    SerialMon.println("RELAY ON");
    // turn on Blynk LED
    Blynk.virtualWrite(V4, 255);
    // push notification that the relay is ON
    Blynk.logEvent("relay", "The relay is ON");
    // update device uptime
    updateDeviceTime();
    // start relay uptime
    relayStatus = true;
    relayStartTime = deviceUptime;
  }
  else {
    // turn off relay
    digitalWrite(RELAY_PIN1, HIGH);
    //digitalWrite(RELAY_PIN2, HIGH);
    digitalWrite(LED_BUILTIN, LOW);
    SerialMon.println("RELAY OFF");
    // turn off Blynk LED
    Blynk.virtualWrite(V4, 0);
    // push notification that the relay is OFF
    Blynk.logEvent("relay", "The relay is OFF");
    // update device uptime
    updateDeviceTime();
    // stop relay uptime
    relayStatus = false;
  }
}


/*
 * called whenever a widget reads virtual pin V7
 * first it will update the device uptime then use it to calulate the relay uptime
 * after which it will then write it to virtual pin V6
 * if relay is not on, it will instead return a value of 0
 */
BLYNK_WRITE(V7) {
  uint8_t pinValue = param.asInt();
  if (pinValue != 0) {
    if (relayStatus) {
      updateDeviceTime();
      Blynk.virtualWrite(V6, deviceUptime - relayStartTime);
      SerialMon.println("Relay Uptime: ");
      SerialMon.print(deviceUptime - relayStartTime);
    }
  }
}


/*
 * called whenever a widget reads virtual pin V8 and returns signal quality to virtual pin V9
 * for more information: https://github.com/AlexWaclawik/Arduino-IoT-Switch/tree/testing#operation
 */
BLYNK_WRITE(V8) {
  uint8_t pinValue = param.asInt();
  if (pinValue != 0) {
    String signalCond = "";
    String signalColor = "";
    uint8_t signalQuality = 0;
    signalQuality = modem.getSignalQuality();
    if (signalQuality == 99) {
      signalCond = "UNKNOWN";
      // grey color
      signalColor = "#A0A0A0";
    }
    // RSSI less than -93 dBm
    else if (signalQuality < 10) {
      signalCond = "MARGINAL";
      // red color
      signalColor = "#FF0000";
    }
    // RSSI greater than -95 dBm and less than -83 dBm
    else if (signalQuality > 9 && signalQuality < 15) {
      signalCond = "OK";
      // orange color
      signalColor = "#FF7B00";
    }
    // RSSI greater than -85 dBm and less than -73 dBm
    else if (signalQuality > 14 && signalQuality < 20) {
      signalCond = "GOOD";
      // light green color
      signalColor = "#6EFF00";
    }
    // RSSI greater than -75 dBm
    else if (signalQuality > 19) {
      signalCond = "EXCELLENT";
      // brighter darker green color
      signalColor = "#04D700";
    }
    Blynk.virtualWrite(V9, signalCond);
    Blynk.virtualWrite(V10, 255);
    Blynk.setProperty(V10, "color", signalColor);
    SerialMon.println("Signal Quality: ");
    SerialMon.print(signalCond);
  }
}


/*
 * updates the device uptime in hours, and
 * then pushes the uptime to virtual pin V5
 */
void updateDeviceTime() {
  deviceUptime = (((millis() / 1000) / 60) / 60);
  Blynk.virtualWrite(V5, deviceUptime);
}


/*
 * setup and initializes device on startup (or reboot) and will only run once
 */
void setup() {
  // configure relay and ensure it is off
  pinMode(RELAY_PIN1, OUTPUT);
  digitalWrite(RELAY_PIN1, HIGH);
  // pinMode(RELAY_PIN2, OUTPUT);
  // digitalWrite(RELAY_PIN2, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // set console baud rate
  SerialMon.begin(9600);
  delay(1000);

  // set GSM module baud rate
  SerialAT.begin(9600);
  delay(6000);

  // initialize modem
  SerialMon.println("Initializing modem...");
  modem.restart();
  modem.waitForNetwork(600000L);

  // get modem info
  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  // connect to Blynk
  Blynk.begin(auth, modem, apn, user, pass);

  // verify heartbeat is configured correctly
  SerialMon.print("Heartbeat (sec): ");
  SerialMon.println(BLYNK_HEARTBEAT);
}


/*
 * main function that loops during device operation
 * On startup, the device will sync the current value of the virtual pin V1 and V8 
 * which will call the write functions for those respective pins
 * this ensures that if the relay was ON when the device loses power, it will resume on reboot
 * it also makes the device check signal quality on startup
 */
void loop() {
  Blynk.run();
  if (startup) {
    // check relay status
    Blynk.syncVirtual(V1);
    // checks signal quality
    Blynk.syncVirtual(V8);
    startup = false;
    // sends push notification that device has rebooted
    Blynk.logEvent("startup","The device has been rebooted");
  }
}
