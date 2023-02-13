# Fun-with-fans
Fun-with-fans is the individual project of the IoT course of the master's degree in Engineering in Computer Science at Sapienza University of Rome. Nowadays consumption for heating buildings is a big problem due to rising prices, for this reason I think about to build an intelligent system that allows to turn on and turn off for example a fan in an intelligent way, in order to reduce the energy consumption.

# Architecture
The IoT device is developed using RIOT-OS as operating system and with a STM NUCLEO-f401re as board. The system sends data to the cloud and store there in a database, in particular the cloud-based services are based on Amazon Web Services (AWS); it is composed by 2 sensors and 2 actuators (light led and fan blade motor).
## Sensors
The sensors of the system are an ultrasonic sensor and a digital temperature sensor.
* Ultrasonic Sensor
gfdgd
