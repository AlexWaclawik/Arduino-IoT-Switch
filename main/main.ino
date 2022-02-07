/*
 * Author: Alex Waclawik
 * Github: https://github.com/AlexWaclawik/Arduino-IoT-Switch
 * Version: 1.1 (Test Branch)
 * This project is for a remote AC outlet switch that is made using an Arduino microcontroller.
 * 
 * IMPORTANT: Adjust BLYNK_HEARTBEAT (in sec) in BlynkGsmClient.h to suit your application. 
 * newHeartbeatInterval * 2.3 formula will be applied. For example, for a 300 sec heartbeat 
 * you will get a notification regarding the connection status with 11.5 min delay in worst case.
 */

// these Blynk definitions ALWAYS have to be first
#define BLYNK_TEMPLATE_ID "TEMPLATE-ID"
#define BLYNK_DEVICE_NAME "DEVICE-NAME"
#define BLYNK_AUTH_TOKEN "YOUR-AUTH-TOKEN"
#define BLYNK_PRINT Serial

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
const char apn[] = "iot.1nce.net";
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
 * called whenever a widget writes to virtual pin V1.
 * It will switch the relay on and off, as well as turn the status LED.
 * Lastly it will call the updateDeviceTime() function which will update the uptime.
 */
BLYNK_WRITE(V1) {
  // retrieve value from virtual pin V1
  int pinValue = param.asInt();
  if (pinValue != 0) {
    // turn on relay
    digitalWrite(RELAY_PIN1, LOW);
    //digitalWrite(RELAY_PIN2, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    SerialMon.println("RELAY ON");
    // turn on Blynk LED
    Blynk.virtualWrite(V4, 255);
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
 * if relay is not on, it will instead return a value of -1.0
 */
BLYNK_READ(V7) {
  if (relayStatus) {
    updateDeviceTime();
    Blynk.virtualWrite(V6, deviceUptime - relayStartTime);
  }
  else {
    Blynk.virtualWrite(V6, -1.0);
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
  delay(10);

  // set GSM module baud rate
  SerialAT.begin(9600);
  delay(6000);

  // initialize modem
  SerialMon.println("Initializing modem...");
  modem.restart();
  delay(8000);

  // get modem info
  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  // connect to Blynk
  Blynk.begin(auth, modem, apn, user, pass);

  // verify heartbeat is set correctly
  SerialMon.print("Heartbeat (sec): ");
  SerialMon.println(BLYNK_HEARTBEAT);
}


/*
 * main function that loops during device operation
 */
void loop() {
  Blynk.run();
  // syncs the current value of the virtual pin V1 which will trigger a call of the BLYNK_WRITE(V1) function above
  // this ensures that if the relay was ON when the device loses power, on reboot it will resume
  if (startup) {
    Blynk.syncVirtual(V1);
    startup = false;
  }
}
