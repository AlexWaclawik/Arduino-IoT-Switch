/*
 * LTE Switch for use with SIM7000. Code is taken and edited from Timothy Woo, whose
 * repo can be found here -> https://github.com/botletics/SIM7000-LTE-Shield
 */

// import libraries
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

// define identifiers
#define SIMCOM_7000
#define FONA_PWRKEY 6
#define FONA_RST 7
#define FONA_DTR 8
#define FONA_RI 9
#define FONA_TX 10
#define FONA_RX 11
#define T_ALERT 12

// software serial
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// define global variables
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char imei[16] = {0};
char replybuffer[255];

Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();

void setup() {
  // default state
  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH);
  pinMode(FONA_PWRKEY, OUTPUT);
  // power on module
  fona.powerOn(FONA_PWRKEY);

  Serial.begin(9600);
  Serial.println(F("FONA basic test"));
  Serial.println(F("Initializing...(May take several seconds)"));

  // software serial
  // default SIM7000 baud rate
  Serial.println(F("Configuring to 9600 baud"));
  fonaSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  fonaSS.begin(9600);
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // Don't proceed if it couldn't find the device
  }

  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case SIM7000:
      Serial.println(F("SIM7000")); break;
  }

  // print module IMEI number
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  // set modem to full functionality (AT+CFUN=1)
  fona.setFunctionality(1);

  // Configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
  //fona.setNetworkSettings(F("your APN"), F("your username"), F("your password"));
  // For Hologram SIM card
  fona.setNetworkSettings(F("iot.1nce.net"), F("bart@volairsim.com"));
  
  printMenu();  
}

void printMenu(void) {
  Serial.println(F("-------------------------------------"));
  Serial.println(F("[?] Print this menu"));
  Serial.println(F("[a] Read the ADC, 0V-VBAT for SIM7000"));
  Serial.println(F("[b] Read supply voltage")); // Will also give battery % charged for LiPo battery
  Serial.println(F("[C] Read the SIM CCID"));
  Serial.println(F("[U] Unlock SIM with PIN code"));
  Serial.println(F("[i] Read signal strength (RSSI)"));
  Serial.println(F("[n] Get network status"));

  // Phone
  Serial.println(F("[c] Make phone Call"));
  Serial.println(F("[A] Get call status"));
  Serial.println(F("[h] Hang up phone"));
  Serial.println(F("[p] Pick up phone"));

  // SMS
  Serial.println(F("[N] Number of SMS's"));
  Serial.println(F("[r] Read SMS #"));
  Serial.println(F("[R] Read all SMS"));
  Serial.println(F("[d] Delete SMS #"));
  Serial.println(F("[s] Send SMS"));
  
  // Time
  Serial.println(F("[y] Enable local time stamp"));
  Serial.println(F("[Y] Enable NTP time sync")); // Need to use "G" command first!
  Serial.println(F("[t] Get network time")); // Works just by being connected to network

  // Data Connection
  Serial.println(F("[G] Enable GPRS"));
  Serial.println(F("[g] Disable GPRS"));
  // Serial.println(F("[w] Read webpage (GPRS)"));
  // Serial.println(F("[W] Post to website (GPRS)"));
  Serial.println(F("[1] Get connection info")); // See what connection type and band you're on!

  // GPS
  Serial.println(F("[O] Turn GPS on)"));
  Serial.println(F("[o] Turn GPS off"));
  Serial.println(F("[L] Query GPS location"));
  
  // Serial.println(F("[S] Create serial passthru tunnel"));
  Serial.println(F("-------------------------------------"));
  Serial.println(F(""));
}

void loop() {
  Serial.print(F("FONA> "));
  while (! Serial.available() ) {
    if (fona.available()) {
      Serial.write(fona.read());
    }
  }

  char command = Serial.read();
  Serial.println(command);

  switch (command) {
    case '?': {
        printMenu();
        break;
    }

    case 'a': {
        // read the ADC
        uint16_t adc;
        if (! fona.getADCVoltage(&adc)) {
          Serial.println(F("Failed to read ADC"));
        } else {
          Serial.print(F("ADC = ")); Serial.print(adc); Serial.println(F(" mV"));
        }
        break;
    }

    case 'b': {
        // Read the battery voltage and percentage
        uint16_t vbat;
        if (! fona.getBattVoltage(&vbat)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VBat = ")); Serial.print(vbat); Serial.println(F(" mV"));
        }

        if ( type != SIM7500 ) {
          if (! fona.getBattPercent(&vbat)) {
            Serial.println(F("Failed to read Batt"));
          } else {
            Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));
          }
        }

        break;
      }

    case 'U': {
        // Unlock the SIM with a PIN code
        char PIN[5];
        flushSerial();
        Serial.println(F("Enter 4-digit PIN"));
        readline(PIN, 3);
        Serial.println(PIN);
        Serial.print(F("Unlocking SIM card: "));
        if (! fona.unlockSIM(PIN)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }

    case 'C': {
        // read the CCID
        fona.getSIMCCID(replybuffer);  // make sure replybuffer is at least 21 bytes!
        Serial.print(F("SIM CCID = ")); Serial.println(replybuffer);
        break;
      }

    case 'i': {
        // read the RSSI
        uint8_t n = fona.getRSSI();
        int8_t r;

        Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
        if (n == 0) r = -115;
        if (n == 1) r = -111;
        if (n == 31) r = -52;
        if ((n >= 2) && (n <= 30)) {
          r = map(n, 2, 30, -110, -54);
        }
        Serial.print(r); Serial.println(F(" dBm"));

        break;
      }

    case 'n': {
        // read the network/cellular status
        uint8_t n = fona.getNetworkStatus();
        Serial.print(F("Network status "));
        Serial.print(n);
        Serial.print(F(": "));
        if (n == 0) Serial.println(F("Not registered"));
        if (n == 1) Serial.println(F("Registered (home)"));
        if (n == 2) Serial.println(F("Not registered (searching)"));
        if (n == 3) Serial.println(F("Denied"));
        if (n == 4) Serial.println(F("Unknown"));
        if (n == 5) Serial.println(F("Registered roaming"));
        break;
      }

    /*** SMS ***/
    case 'N': {
        // read the number of SMS's!
        int8_t smsnum = fona.getNumSMS();
        if (smsnum < 0) {
          Serial.println(F("Could not read # SMS"));
        } else {
          Serial.print(smsnum);
          Serial.println(F(" SMS's on SIM card!"));
        }
        break;
    }
    case 'r': {
        // read an SMS
        flushSerial();
        Serial.print(F("Read #"));
        uint8_t smsn = readnumber();
        Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);

        // Retrieve SMS sender address/phone number.
        if (! fona.getSMSSender(smsn, replybuffer, 250)) {
          Serial.println("Failed!");
          break;
        }
        Serial.print(F("FROM: ")); Serial.println(replybuffer);

        // Retrieve SMS value.
        uint16_t smslen;
        if (! fona.readSMS(smsn, replybuffer, 250, &smslen)) { // pass in buffer and max len!
          Serial.println("Failed!");
          break;
        }
        Serial.print(F("***** SMS #")); Serial.print(smsn);
        Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
        Serial.println(replybuffer);
        Serial.println(F("*****"));

        break;
      }
    case 'R': {
        // read all SMS
        int8_t smsnum = fona.getNumSMS();
        uint16_t smslen;
        int8_t smsn;

        if ( (type == SIM5320A) || (type == SIM5320E) ) {
          smsn = 0; // zero indexed
          smsnum--;
        } else {
          smsn = 1;  // 1 indexed
        }

        for ( ; smsn <= smsnum; smsn++) {
          Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
          if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
            Serial.println(F("Failed!"));
            break;
          }
          // if the length is zero, its a special case where the index number is higher
          // so increase the max we'll look at!
          if (smslen == 0) {
            Serial.println(F("[empty slot]"));
            smsnum++;
            continue;
          }

          Serial.print(F("***** SMS #")); Serial.print(smsn);
          Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
          Serial.println(replybuffer);
          Serial.println(F("*****"));
        }
        break;
      }

    case 'd': {
        // delete an SMS
        flushSerial();
        Serial.print(F("Delete #"));
        uint8_t smsn = readnumber();

        Serial.print(F("\n\rDeleting SMS #")); Serial.println(smsn);
        if (fona.deleteSMS(smsn)) {
          Serial.println(F("OK!"));
        } else {
          Serial.println(F("Couldn't delete"));
        }
        break;
      }

    case 's': {
        // send an SMS!
        char sendto[21], message[141];
        flushSerial();
        Serial.print(F("Send to #"));
        readline(sendto, 20);
        Serial.println(sendto);
        Serial.print(F("Type out one-line message (140 char): "));
        readline(message, 140);
        Serial.println(message);
        if (!fona.sendSMS(sendto, message)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
        }

        break;
      }

    /*** Time ***/
    case 'y': {
        // enable network time sync
        if (!fona.enableRTC(true))
          Serial.println(F("Failed to enable"));
        break;
      }

    case 'Y': {
        // enable NTP time sync
        if (!fona.enableNTPTimeSync(true, F("pool.ntp.org")))
          Serial.println(F("Failed to enable"));
        break;
      }

    case 't': {
        // read the time
        char buffer[23];

        fona.getTime(buffer, 23);  // make sure replybuffer is at least 23 bytes!
        Serial.print(F("Time = ")); Serial.println(buffer);
        break;
      }


    /******* GPS *******/
    case 'o': {
        // turn GPS off
        if (!fona.enableGPS(false))
          Serial.println(F("Failed to turn off"));
        break;
      }
    case 'O': {
        // turn GPS on
        if (!fona.enableGPS(true))
          Serial.println(F("Failed to turn on"));
        break;
      }
    case 'L': {
        /*
        // Uncomment this block if all you want to see is the AT command response
        // check for GPS location
        char gpsdata[120];
        fona.getGPS(0, gpsdata, 120);
        if (type == SIM808_V1)
          Serial.println(F("Reply in format: mode,longitude,latitude,altitude,utctime(yyyymmddHHMMSS),ttff,satellites,speed,course"));
        else if ( (type == SIM5320A) || (type == SIM5320E) || (type == SIM7500) )
          Serial.println(F("Reply in format: [<lat>],[<N/S>],[<lon>],[<E/W>],[<date>],[<UTC time>(yyyymmddHHMMSS)],[<alt>],[<speed>],[<course>]"));
        else
          Serial.println(F("Reply in format: mode,fixstatus,utctime(yyyymmddHHMMSS),latitude,longitude,altitude,speed,course,fixmode,reserved1,HDOP,PDOP,VDOP,reserved2,view_satellites,used_satellites,reserved3,C/N0max,HPA,VPA"));
        
        Serial.println(gpsdata);

        break;
        */

        float latitude, longitude, speed_kph, heading, altitude;

        // Uncomment the definitions below and the line immediately below the definitions if you want
        // to parse UTC time data as well, or use the line below it if you don't care
//        float second;
//        uint16_t year;
//        uint8_t month, day, hour, minute;
//        if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude, &year, &month, &day, &hour, &minute, &second)) {
        if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) { // Use this line instead if you don't want UTC time
          Serial.println(F("---------------------"));
          Serial.print(F("Latitude: ")); Serial.println(latitude, 6);
          Serial.print(F("Longitude: ")); Serial.println(longitude, 6);
          Serial.print(F("Speed: ")); Serial.println(speed_kph);
          Serial.print(F("Heading: ")); Serial.println(heading);
          Serial.print(F("Altitude: ")); Serial.println(altitude);
          // Comment out the stuff below if you don't care about UTC time
          /*
          Serial.print(F("Year: ")); Serial.println(year);
          Serial.print(F("Month: ")); Serial.println(month);
          Serial.print(F("Day: ")); Serial.println(day);
          Serial.print(F("Hour: ")); Serial.println(hour);
          Serial.print(F("Minute: ")); Serial.println(minute);
          Serial.print(F("Second: ")); Serial.println(second);
          */
          Serial.println(F("---------------------"));
        }

        break;
      }

    /******* GPRS *******/
    case 'g': {
        // turn GPRS off
        if (!fona.enableGPRS(false))
          Serial.println(F("Failed to turn off"));
        break;
      }
    case 'G': {
        // turn GPRS off first for SIM7500/7600
        #if defined(SIMCOM_7500) || defined(SIMCOM_7600)
          fona.enableGPRS(false);
        #endif
        
        // turn GPRS on
        if (!fona.enableGPRS(true))
          Serial.println(F("Failed to turn on"));
        break;
      }

/*
    case 'w': {
        // read website URL
        uint16_t statuscode;
        int16_t length;
        char url[80];

        flushSerial();
        Serial.println(F("NOTE: in beta! Use small webpages to read!"));
        Serial.println(F("URL to read (e.g. www.adafruit.com/testwifi/index.html):"));
        Serial.print(F("http://")); readline(url, 79);
        Serial.println(url);

        Serial.println(F("****"));
        if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
          Serial.println("Failed!");
          break;
        }
        while (length > 0) {
          while (fona.available()) {
            char c = fona.read();

            // Serial.write is too slow, we'll write directly to Serial register!
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
            loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty
            UDR0 = c;
#else
            Serial.write(c);
#endif
            length--;
            if (! length) break;
          }
        }
        Serial.println(F("\n****"));
        fona.HTTP_GET_end();
        break;
      }

    case 'W': {
        // Post data to website
        uint16_t statuscode;
        int16_t length;
        char url[80];
        char data[80];

        flushSerial();
        Serial.println(F("NOTE: in beta! Use simple websites to post!"));
        Serial.println(F("URL to post (e.g. httpbin.org/post):"));
        Serial.print(F("http://")); readline(url, 79);
        Serial.println(url);
        Serial.println(F("Data to post (e.g. \"foo\" or \"{\"simple\":\"json\"}\"):"));
        readline(data, 79);
        Serial.println(data);

        Serial.println(F("****"));
        if (!fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length)) {
          Serial.println("Failed!");
          break;
        }
        while (length > 0) {
          while (fona.available()) {
            char c = fona.read();

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
            loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty
            UDR0 = c;
#else
            Serial.write(c);
#endif

            length--;
            if (! length) break;
          }
        }
        Serial.println(F("\n****"));
        fona.HTTP_POST_end();
        break;
      }
*/
    case '1': {
        // Get connection type, cellular band, carrier name, etc.
        fona.getNetworkInfo();        
        break;
      }

#if defined(SIMCOM_2G) || defined(SIMCOM_7000) || defined(SIMCOM_7070)
    case '2': {
        // Post data to website via 2G or LTE CAT-M/NB-IoT
        
        float temperature = analogRead(A0)*1.23; // Change this to suit your needs
        
        uint16_t battLevel;
        if (! fona.getBattVoltage(&battLevel)) battLevel = 3800; // Use dummy voltage if can't read

        // Create char buffers for the floating point numbers for sprintf
        // Make sure these buffers are long enough for your request URL
        char URL[150];
        char body[100];
        char tempBuff[16];
      
        // Format the floating point numbers as needed
        dtostrf(temperature, 1, 2, tempBuff); // float_val, min_width, digits_after_decimal, char_buffer

        #else
            // Construct the appropriate URL's and body, depending on request type
            // Use IMEI as device ID for this example
            
            // GET request
            sprintf(URL, "dweet.io/dweet/for/%s?temp=%s&batt=%i", imei, tempBuff, battLevel); // No need to specify http:// or https://
    //        sprintf(URL, "http://dweet.io/dweet/for/%s?temp=%s&batt=%i", imei, tempBuff, battLevel); // But this works too

            if (!fona.postData("GET", URL))
              Serial.println(F("Failed to complete HTTP GET..."));
            
            // POST request
            /*
            sprintf(URL, "http://dweet.io/dweet/for/%s", imei);
            sprintf(body, "{\"temp\":%s,\"batt\":%i}", tempBuff, battLevel);
            
            if (!fona.postData("POST", URL, body)) // Can also add authorization token parameter!
              Serial.println(F("Failed to complete HTTP POST..."));
            */
          
        #endif

        break;
      }
    /*****************************************/

    case 'S': {
        Serial.println(F("Creating SERIAL TUBE"));
        while (1) {
          while (Serial.available()) {
            delay(1);
            fona.write(Serial.read());
          }
          if (fona.available()) {
            Serial.write(fona.read());
          }
        }
        break;
      }

    default: {
        Serial.println(F("Unknown command"));
        printMenu();
        break;
      }
  }
  // flush input
  flushSerial();
  while (fona.available()) {
    Serial.write(fona.read());
  }
}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}

uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
