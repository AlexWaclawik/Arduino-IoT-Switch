## Arduino IoT Switch

This project is for a remote AC outlet switch that is made using an Arduino microcontroller. This
was achieved using the [Blynk IoT platform and libraries](https://docs.blynk.io/en/), which creates a neat mobile application
interface for interacting with the microcontroller.

<p align="right"></p>

### Components

1. [ELEGOO UNO R3 Board ATmega328P with USB Cable (Arduino UNO Clone)](https://www.elegoo.com/products/elegoo-uno-r3-board)
	- Acts as the main board, and is fully compatible with Arduino.
![board](media/board-modem.jpg =250x250)
2. [Botletics™ SIM7000A LTE CAT-M1/NB-IoT + GPS Shield Kit](https://www.botletics.com/products/sim7000-shield)
	- This kit acts as a modem. It also offers expanability for the future.
![antenna](media/antenna.jpg =250x250)
3. [1NCE FlexSIM Card](https://1nce.com/en/sim-cards/)
	- This pre-paid SIM card is offered for 10 years with a 500 MB limit, making it perfect for low-bandwith IoT devices.
	- TCP was chosen over SMS, as the 1NCE SIM Card only has an SMS limit of 250 messages, which is not enough for the 10 year period.
4. [MCIGICM 2 Channel DC 5V Relay Module for Arduino UNO](https://www.amazon.com/gp/product/B072BY3KJF?psc=1)
	- Opto-isolated relay for maximum safety of the outlet.
![relay](media/relay.jpg =250x250)
5. [Adafruit 328 Lithium Ion Battery, 3.7V, 2500mAh](https://www.adafruit.com/product/328)
	- Along with the power supply, provides additonal power for activating the relay.
![battery](media/battery.jpg =250x250)

<p align="right"></p>

### Operation

<p align="right"></p>

### Dependancies

-	[TinyGSM](https://github.com/vshymanskyy/TinyGSM)
-	[Blynk](https://github.com/blynkkk/blynk-library)

To Install:
1. [Download the latest release from here]()
2. Unzip the contents. The necessary libraries are located in ```/include```
3. Place the dependancies into your Arduino Libraries Folder
	- On Windows: ```C:\Users\user\Documents\Arduino\libraries```
	- On Linux: ``` ~/home/user/sketchbook/libraries```
	- On Mac: ``` ~/Documents/Arduino/libraries```

<p align="right"></p>

### Credits

1. [Project idea inspired by video from GaryExplains](https://www.youtube.com/watch?v=7swG4XVSx50)
2. [Code for manipulating relay pins taken from GaryExplains](https://github.com/garyexplains/examples/blob/master/MKR1000/mysmartswitch.ino)

<p align="right"></p>
