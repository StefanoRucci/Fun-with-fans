import paho.mqtt.client as mqtt #to interact with Mosquitto broker
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient #to interact with AWS broker

import json
import paho.mqtt.publish as publish

TOPIC_OUT1 = "temp"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(TOPIC_OUT1)

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    message = {}
    message['message'] = str(msg.payload)[2:-1]
    messageJson = json.dumps(message)
    myAWSIoTMQTTClient.publish(msg.topic, messageJson, 1)
    print("Done")

#AWS parameters, these files are created when we created a new thing in AWS
host = "a2k8u2qbj3a7s7-ats.iot.us-east-1.amazonaws.com"
rootCAPath = "root-CA.crt"
certificatePath = "fun.cert.pem"
privateKeyPath = "fun.private.key"
clientId = "basicPubSub"
port=8883

#connection to AWS
myAWSIoTMQTTClient = None
myAWSIoTMQTTClient = AWSIoTMQTTClient(clientId)
myAWSIoTMQTTClient.configureEndpoint(host, port)
myAWSIoTMQTTClient.configureCredentials(rootCAPath, privateKeyPath, certificatePath)
myAWSIoTMQTTClient.connect()

#connection to Mosquitto
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1886, 60)
client.loop_forever()