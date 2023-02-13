# Fun-with-fans
Fun-with-fans is the individual project of the IoT course of the master's degree in Engineering in Computer Science at Sapienza University of Rome. Nowadays consumption for heating buildings is a big problem due to rising prices, for this reason I think about to build an intelligent system that allows to turn on and turn off for example a fan in an intelligent way, in order to reduce the energy consumption. In particular in this project this device is tought for the workstation: it checks if there is someone on the location and ,if the temperature is too hot, it automatically turn a fan. If one of these 2 conditions stops being valid, the fan turns off. 

# Architecture
The IoT device is developed using RIOT-OS as operating system and with a STM NUCLEO-f401re as board. The system sends data to the cloud and store there in a database, in particular the cloud-based services are based on Amazon Web Services (AWS); it is composed by **2 sensors** and **2 actuators** (light led and fan blade motor).
## Sensors
The sensors of the system are an *ultrasonic sensor* and a *digital temperature sensor*.
* **Ultrasonic Sensor:** by means of this sensor it is possible to check whether someone is sitting on the chair at the workstation. The HC-SR04 ultrasonic sensor uses sonar to determine distance to an object like bats or dolphins do and it operates in a distance range going from 2cm to 400 cm so it is perfect for our purpose. it is positioned on the back of the chair at neck height, so as to be able to correctly verify the presence of a person.
*  **Digital temperature-humidty Sensor:** it is a basic, low-cost digital temperature and humidity sensor. It uses a capacitive humidity sensor and a thermistor to measure the surrounding air, and spits out a digital signal on the data pin (no analog input pins needed). With this sensor it is possible to measure the post office ambient temperature. 
### Periodicity of measurements
The system is designed to reduce expenses and energy costs, therefore the ultrasonic sensor does not continuously estimate the data, but checks for the presence of someone every minute: 
* if it does not detect an object in a space within 10cm, the device starts sleep-mode and measure again after 1 minute.
* on the contrary if there is someone, the temperature sensor starts to do its measurements on the ambient.

So the ultrasonic sensor works every 1 minutes, and the temperature sensor works only there is a person of the workstation.

## Actuators
The actuators of the system are a *light led* and a *fan blade motor*.
* **Fan Blade Motor:** it is the simulation of the fan (or whatever heating system).
*  **Light Led**: it is a simple light.

After the measurements carried out by the sensors previously exposed, if there is a person in the workstation and the temperature exceeds the pre-fixed threshold (in our case we have chosen the temperature of 28°), the fan starts working and at the same time the led that signals that the fan is on turns on.
If one of the two conditions is no longer verified (therefore if the temperature falls below the threshold or the person gets up from the chair and there is no one else) the fan and the led switch off.

![alt text](https://github.com/StefanoRucci/Fun-with-fan/blob/[branch]/image.jpg?raw=true)

# Network
