# Fun-with-fans
Fun-with-fans is the individual project of the IoT course of the master's degree in Engineering in Computer Science at Sapienza University of Rome. Nowadays consumption for heating buildings is a big problem due to rising prices, for this reason I think about to build an intelligent system that allows to turn on and turn off for example a fan in an intelligent way, in order to reduce the energy consumption. In particular in this project this device is thought for the workstation: it checks if there is someone on the location and ,if the temperature is too hot, it automatically turn a fan. If one of these 2 conditions stops being valid, the fan turns off. 

# Architecture
The IoT device is developed using RIOT-OS as operating system and with a STM NUCLEO-f401re as board. The system sends data to the cloud and store there in a database, in particular the cloud-based services are based on Amazon Web Services (AWS). The device is composed by **2 sensors** and **2 actuators**. The datasheets are available [here](https://github.com/StefanoRucci/Fun-with-fans/tree/main/datasheets).
## Sensors
The sensors of the system are an *ultrasonic sensor* and a *digital temperature sensor*.
* **Ultrasonic Sensor:** by means of this sensor it is possible to check whether someone is sitting on the chair at the workstation. The HC-SR04 ultrasonic sensor uses sonar to determine distance to an object like bats or dolphins do and it operates in a distance range going from 2cm to 400 cm so it is perfect for our purpose. it is positioned on the back of the chair at neck height, so as to be able to correctly verify the presence of a person.
*  **Digital temperature-humidty Sensor:** it is a basic, low-cost digital temperature and humidity sensor. It uses a capacitive humidity sensor and a thermistor to measure the surrounding air, and spits out a digital signal on the data pin (no analog input pins needed). With this sensor it is possible to measure the post office ambient temperature. 
### Periodicity of measurements
The system is designed to reduce expenses and energy costs, therefore the ultrasonic sensor does not continuously estimate the data, but checks for the presence of someone every minute: 
* if it does not detect an object in a space within 20cm, the device starts sleep-mode and measure again after 1 minute.
* on the contrary if there is someone, the temperature sensor starts to do its measurements on the ambient.

So the ultrasonic sensor works every 1 minute, and the temperature sensor works only if there is a person of the workstation.

## Actuators
The actuators of the system are a *light led* and a *fan blade motor*.
* **Fan Blade Motor:** it is the simulation of the fan (or whatever heating system).
*  **Light Led**: it is a simple light.

After the measurements carried out by the sensors previously exposed, if there is a person in the workstation and the temperature exceeds the pre-fixed threshold (in our case we have chosen the temperature of 28°), the fan starts working and at the same time the led that signals that the fan is on turns on.
If one of the two conditions is no longer verified (therefore if the temperature falls below the threshold or the person gets up from the chair and there is no one else) the fan and the led switch off.

# Network
![alt text](https://github.com/StefanoRucci/Fun-with-fans/blob/main/Network.avif?raw=true)

This is a schema of the compontents that make up the system both at the *IoT device level* and at the *cloud level*. The board is connected through **MQTT-SN** to a local broker called *Mosquitto* hosted on the machine, the connection is carried out using IPv6 and RIOT-OS tap interfaces. So the board exchanges messages with Mosquitto by means of MQTT-SN, a communication protocol based on a publish/subscribe mechanism on a **topic** to which the board must be subscribed (in this case the topic is "*temp*"). Mosquitto exchanges messages using **MQTT** with the AWS ecosystem through a **transparent bridge** that converts from MQTT-SN to MQTT, it is a python script that works as a bridge between Mosquitto and AWS IoT Core: the role of this component is to "take" the messages published on the mosquitto broker by the board and relay them to the broker situated on the cloud broker, in this case the AWS IoT Core one.

After this phase the data collected by the board arrive to **AWS** cloud service, and they have to be stored in a permanent way in a database. AWS allows us to use a NoSQL database called *DynamoDB*, and after to have created a table inside the microservice, we store the data received on the topic temp. After this in order to show this data on the web dashboard we have to use the **REST API**, by means of this is possible to extract the data from the table with a **lambda function**.

## Network Performance
The messages on the network, and in particular the messages published on the topic, are in the format "*tXX.XhYY.Y*", and so they refer to the data of the temperature and the humidity. For example a possible message is "*t20.0h59.0*". So the lenght of a single message sent on the network is of 10 bytes, to transmit data from the board to mosquitto, it was chosen MQTT-SN because of its peculiarities: it reduces the size of the message payload and removes the need for a permanent connection by using UDP as the transport protocol.

# Walkthrough of the system
## Mosquitto (MQTT-SN Broker)
The first thing to do is to setup the MQTT-SN broker and in order to do this you have to clone this [repository](https://github.com/eclipse/mosquitto.rsmb) to your machine. After that you have to og inside this folder and do the following steps:

```
> cd rsmb/src
> make
```
After this, like it is explained in the guide in Mosquitto repository, you have to create a file called *config.conf* and write the following lines inside it:
```
# add some debug output
trace_output protocol
# listen for MQTT-SN traffic on UDP port 1885
listener 1885 INADDR_ANY mqtts
ipv6 true
# listen to MQTT connections on tcp port 1886
listener 1886 INADDR_ANY
ipv6 true
```
So now we can start the broker with the following command always in the previous terminal:
```
> ./broker_mqtts config.conf
```
## Transparent bridge 
Now let's start the bridge from MQTT-SN and MQTT. Since that is not supported the direct bridge between RSMB and our service, we have to create it with a Python script. You have to copy the file called *transparent_bridge.py* and launch it with the command:
```
python3 transparent_bridge.py
```
Now we have the MQTT-SN broker and the trasparent bridge running, they are waiting for a message. Let'see the prototype and how to make it works.

## Prototype
The system is composed by the sensors and the actuators seen above, but there are also:
* Two **resistor** of 220 Ohm.
* A **diode** rectifier.
* A **NPN transistor**

They are connected with the nucleo board like in the following schema:

<img src="https://github.com/StefanoRucci/Fun-with-fans/blob/main/Circuit.jpg" width=70% height=70%>

Once the circuit is built, we have to flash the firmware on the board. First of all we clone the [RIOT repository](https://github.com/RIOT-OS/RIOT) on our machine, and so in a terminal do the following steps:
```
> sudo ./RIOT/dist/tools/tapsetup/tapsetup -c 2 
> sudo ip a a 2000:2::1/64 dev tapbr0
```
After this you have to go inside the folder where there is the code (*main.c* and *Makefile*) and run the command:
```
> make flash term all
```
And now your system is running local and speak with to the Mosquitto broker. Let's see how to setup AWS in order to send the data to the cloud and store them inside DynamoDB table.

## DynamoDB
Now first of all on AWS you have to create a table where the values sent on the topic have to be stored.

## AWS IoT Core
Once the table was created, you have to setup a thing on AWS that can store automatically messages arriving on the broker. From the AWS IoT Core homepage you have to follow the steps of *connect a new device*. At the end of the configuration you will have the files to configure the trasnparent bridge in the right way. Once this is done, by means of a rule (using role engine of AWS IoT Core) it is possible to store automatically the messages arriving on the broker inside the DynamoDB table created before.

## Lambda Function
Last, thanks to this lambda function, it is possible to extract the data stored inside the table, so that it is possible to do available for the user.

# Links
Check my [Linkedln profile](https://www.linkedin.com/in/stefano-rucci-74b5b6220/)!
[Here](https://www.hackster.io/stefano-rucci/fun-with-fans-c9f7bd) you can find the blog post.

# Gallery
![alt text](https://github.com/StefanoRucci/Fun-with-fans/blob/main/images/img1.jpeg?raw=true)
![alt text](https://github.com/StefanoRucci/Fun-with-fans/blob/main/images/img2.jpeg?raw=true)
![alt text](https://github.com/StefanoRucci/Fun-with-fans/blob/main/images/img3.jpeg?raw=true)
![alt text](https://github.com/StefanoRucci/Fun-with-fans/blob/main/images/img4.jpeg?raw=true)
![alt text](https://github.com/StefanoRucci/Fun-with-fans/blob/main/images/img5.jpeg?raw=true)
