<!-- This is a readme file -->
# Smart fan controller
Edvin Mohlin em223tu - Time needed to replicate: 2 hours - 2 days

This is an IoT project to create a controller that controls a small USB powered desk fan. This controller can be used to control the fan using physical buttons and also remotely over the internet. The controller also incorporates an automatic mode that controls the fan depending on the temperature and the presence of a human being. The remote communication and data collection of the fan is done over MQTT and sent to a local server running Home Assistant.  

In order to remove the need for hard coding the WiFi credentials and the server address, the controller can act as a WiFi access point with a web server for recieveing the needed information. These are then saved in non-volatile storage so that it can be used to reconnect to the server if it loses power. In order to connect to a different network, a physical button can be pressed to start the web server again and receive the new credentials.
![image](https://github.com/EMohlin7/iot/blob/master/config.png?raw=true)

# Objective

This project was chosen because during the summer it can get very hot inside, especially with a big computer spewing out extra heat into the room. A fan could somewhat remedy this and with an automatic fan you wouldn't event have to remeber to turn it on. The temperature and humdity data collected could also be usefull for other stuff. By sending the data to Home Assistant it could be used to also automatically control other devices, such as humidifiers for example.

# Material

This table includes the materials used in this project.

| Component  |  Description | Cost |
| ------------- | ------------- | ------------- |
| [ESP32 Development board](https://www.amazon.se/AZDelivery-V4-Development-osoldered-Efterföljarmodul/dp/B08BTS62L7) | Microcontroller | 119 SEK for 3 pieces |
| [USB A to micro B](https://www.electrokit.com/usb-kabel-a-hane-micro-b-5p-hane-1.8m) | Power and data cable for the controller | 39 SEK |
| [DHT11](https://www.electrokit.com/digital-temperatur-och-fuktsensor-dht11) | Temperature and humidity sensor | 49 SEK |
| [PIR sensor](https://www.electrokit.com/pir-rorelsedetektor-hc-sr501) | Movement sensor | 55 SEK |
| [Relay module](https://www.amazon.se/Arceli-stycken-enkanalig-rel%C3%A4modulkort-Arduino-rel%C3%A4/dp/B07BVXT1ZK) | Relay | 100 SEK for 5 pieces |
| [LEDs](https://www.electrokit.com/lysdiodsats-5mm-5-farger-300st) | Red, Green and Yellow LED | 129 SEK for 300 pieces|
| [Switches](https://www.electrokit.com/tryckknapp-pcb-6x6x6mm-svart) | Three switches | 6.50 SEK times 3 |
| [USB](https://www.electrokit.com/usb-a-hona-monterad-pa-kort) | Female USB connector to deliver power | 16 SEK |
| [Resistors](https://www.electrokit.com/motstandssats-10r-1m-x20-600-st) | Three 1k resitors and one 10k resistor | 99 SEK for 600 pieces |
| [Capacitor](https://www.amazon.se/BOJACK-v%C3%A4rde-keramisk-kondensator-sortimentsats/dp/B07PP7SFY8) | 100n bypass capacitor for DHT11| 145 SEK for 600 pieces |
| [Raspberry Pi 4](https://www.electrokit.com/raspberry-pi-4-model-b/4gb) | Used to host Home Assistant | 729 SEK |
| [Fan](https://www.netonnet.se/art/hem-fritid/klimat-varme/flaktar/bordsflakt/on-ufn-100-usb-fan/1026068.17565/) | USB powered fan | 99 SEK |

Along with the items in the table, something to build it in is needed. This was not included in the table because it can vary greatly and the specifics are not crucial. I used a small plastic box to put everything in and soldered it together but a simple breadboard could also be used. 


# Computer setup

This section describes the steps for installing and setting up the needed SDK and environment. The steps taken here assume that you are using Ubuntu but any Debian based Linux distrubution should work fine with the exception of installing Visual Studio Code.

## IDE
The IDE used to develop this project was Visual Studio Code. Any IDE can be used and if you are replicating this an IDE won't be necessary. To install Visual Studio Code on Ubuntu, open the terminal and write 
```bash
sudo snap install --classic code
```

## SDK and environment
The SDK used to develop the software is ESP-IDF. There are a few steps needed to install the SDK but the official [documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation) explains it well. 

Flex and Bison also have to be installed. These are used for the code that parses the input from the credentials web server. Run
```bash
sudo apt install flex bison
```
to install them.

Make sure that your user is part of the "dialout" group. This is needed for your user to be able to interact with the serial ports used to flash the firmware onto the ESP32. In the terminal run
```bash
groups
```
to list the groups your user is part of. If your user is part of the dialout group, dialout will be in the shown list. If your user is not part of the dialout group, run 
```bash
sudo usermod -a -G dialout $USER
```
to add it to the group. The computer will have to be restarted for this change to take into effect.

If you installed ESP-IDF in its default path, run
```bash
. $HOME/esp/esp-idf/export.sh
```
to initialize the ESP-IDF environment. 

## Build and run the code
When the environment is initialized, run
```bash
git clone --recurse-submodules git@github.com:EMohlin7/iot.git
cd iot
```
to clone the repository and move inside it.

Plug the ESP32 board into a USB port on your computer and run
```bash
idf.py build
idf.py flash
```
to build the program and flash it onto the ESP32. 

When the program has been flashed onto the ESP32 the USB cable can be plugged into any USB power outlet since no more data has to be sent to the ESP32.


# Hardware setup

This sections shows the circuit schematic and calculations for LED current flow.

## Circuit diagram

![Schematic](https://github.com/EMohlin7/iot/blob/master/schematic.svg?raw=true)

## Electrical calculations

The same 1K ohm resistors were used to limit the current for all LEDs. The same value was used for all of them because of simplicity. Since different colours of LEDs have different forward voltages, the LEDs will have different current flow and therefore vary in brightness when using the same value resistors.

| Colour | Typical forward voltage | 
| ------------- | ------------- |
| Red | 1.8V |
| Yellow | 2.0V | 
| Green | 2.2V  |  

The ESP32 provides 3.3V when powering the LEDs. Given this information we can calculate the current for the different LEDs.

$$I_R = {3.3V - 1.8V \over 10^3\Omega} = 1.5mA$$

$$I_Y = {3.3V - 2.0V \over 10^3\Omega} = 1.3mA$$

$$I_G = {3.3V - 2.2V \over 10^3\Omega} = 1.1mA$$

This shows that the red LED will shine slightly brighter than the other ones.

# Platform
Everything in this section is done and hosted on the Raspberry Pi.
The platform used is Home Assistant. Home Assistant is run locally and can show the state of and interact remotely with devices. It can also be used to script automations using data from devices, to for example activate a humidifier if this controller's sensor reports that the air is dry. 

Home Assistant was chosen as the platform because it is hosted locally which removes the dependence of a third party and is therefore safer and more reliable. It also has very good integration with a lot of other devices and a good scripting system to automate your smart home devices.

## Install the MQTT broker
The broker used is Mosquitto. To install the broker and start it, run
```bash
sudo apt install mosquitto
mosquitto -d 
```

## Install and setup Home Assistant
Home Assistant can be installed in multiple ways, the easiest way is supposedly to install their Home Assistant OS. To still have a normal Raspberry Pi user experience however, Home Assistant was installed in a container using Docker instead. The official Docker [documentation](https://docs.docker.com/engine/install/raspberry-pi-os/) shows how to install docker on a Raspberry Pi. 

With Docker installed follow [these](https://www.home-assistant.io/installation/linux#install-home-assistant-container) steps to install Home Assistant on your Raspberry Pi. 



When Home Assistant is installed, the MQTT integration has to be added to Home Assistant.

- Browse to your Home Assistant instance.
- Go to Settings > Devices & Services.
- In the bottom right corner, select the Add Integration button.
- From the list, select MQTT.
- Follow the instructions on screen to complete the setup.


With MQTT added to Home Assistant, the fan controller should automatically appear inside Home Assistant when connected to the MQTT broker. 


# The code
The code is written in C using the ESP-IDF SDK. The code used for reading the DHT11 sensor is taken from [this repository](https://github.com/Anacron-sec/esp32-DHT11). ESP-IDF is built on FreeRTOS, which of course is an RTOS, making it easy to divide the program into different parts that all can be run in parallell and synced with eachother. This is for example useful for letting the program work as normal no matter if it is connected to the MQTT broker or hosting a web server.  

Calibration data for the temperature sensor can be changed inside the file **config.h**. Only one-point calibration is supported. Other settings are also editable inside this file, such as IO-pins and data send interval. 

# Transmitting the data

The data is transmitted to the server over WiFi using MQTT. MQTT was used because it works well, is easy to implement and has good integration with Home Assistant. WiFi was selected because the controller will be used in a home environment where WiFi usually is availiable and the controller is not battery powered so power draw is not a concern. The temperature and humidity data is sent once every ten seconds. Interactions and changes in presence is sent immediately to and from the controller.



# Presenting the data

Home Assistant uses an internal database to automatically save the data. Settings for this can of course be configured but the default data save interval is 5 seconds. Automation of the controller is done locally using the built in temperature and movement sensors to ensure it can operate automatically even when not connected to a server. Using Home Assistant the controller can of course also be automatically controlled using the bigger dataset available there.

### Home Assistant dashboard
![dashboard](https://github.com/EMohlin7/iot/blob/master/dashboard.png?raw=true)

### Home Assistant data visualization
![data](https://github.com/EMohlin7/iot/blob/master/data.png?raw=true)



# Finalizing the design
![image](https://github.com/EMohlin7/iot/blob/master/image.jpg?raw=true)

The mounting of the DHT11 could be better to improve the airflow over the sensor to make sure it is not effected by the other components. To ease component placement and make styling nicer, one could 3d-print a custom box to contain it instead of making holes in an existing one. All in all, the project went very well and fullfills the intended purpose.   


