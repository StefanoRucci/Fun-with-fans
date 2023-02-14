#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "xtimer.h"
#include "thread.h"
#include "fmt.h"
#include "dht.h"
#include "dht_params.h"

#include "shell.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"


//Definition for Emcute and MQTT Broker
#ifndef EMCUTE_ID
#define EMCUTE_ID           ("nucleo-board")
#endif
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)
#define NUMOFSUBS           (1U)
#define TOPIC_MAXLEN        (64U)
#define MQTT_TOPIC "temp"
#define MQTT_QoS (EMCUTE_QOS_0)

#define SERVER_ADDR ("2000:2::1")
#define SERVER_PORT 1885
#define IPV6_PREFIX_LEN (64U)
#define DEFAULT_INTERFACE ("4")
#define DEVICE_IP_ADDR ("2000:2::2")

//Definition of Array used as Stack by threads
static char stack[THREAD_STACKSIZE_DEFAULT];

//Definition of Emcute Data Structure
static emcute_sub_t subscriptions[NUMOFSUBS]; 
//static char topics[NUMOFSUBS][TOPIC_MAXLEN];

#define UNUSED(x) (void)(x)

int res;

static void *emcute_thread(void *arg)
{
    (void)arg;
    emcute_run(CONFIG_EMCUTE_DEFAULT_PORT, EMCUTE_ID);
    return NULL;    /* should never be reached */
}

//Function that setup the address on the board
static int address_setup(char *name, char *ip_address){

    netif_t *iface = netif_get_by_name(name); //getting the interface where to add the address
    if(iface == NULL){
        puts("No such Interface");
        return 1;
    }

    ipv6_addr_t ip_addr;
    uint16_t flag = GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID | (IPV6_PREFIX_LEN << 8U); //setting the flag for the set function
    
    //Parsing IPv6 Address from String 
    if(ipv6_addr_from_str(&ip_addr, ip_address) == NULL){
        puts("Error in parsing ipv6 address");
        return 1;
    }

    //Set Interface Options 
    if(netif_set_opt(iface, NETOPT_IPV6_ADDR, flag, &ip_addr, sizeof(ip_addr)) < 0){
            puts("Error in Adding ipv6 address");
            return 1;
        }
    printf("Added %s with prefix %d to interface %s\n", ip_address, IPV6_PREFIX_LEN, name);
    return 0;
}

//Function to connect to the Mosquitto Broker
static int connect_broker(void){
    
    sock_udp_ep_t gw = { .family = AF_INET6, .port = SERVER_PORT }; //creating the socket
    char *topic = MQTT_TOPIC;
    char *message = "connected";
    size_t len = strlen(message);
    
    //Parsing IPv6 Address from String
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, SERVER_ADDR) == NULL) {
        puts("error parsing IPv6 address");
        return 1;
    }
    
    //Connecting to broker
    puts("Inizializzo connessione con il broker");
    if (emcute_con(&gw, true, topic, message, len, 0) != EMCUTE_OK) {
        printf("error: unable to connect to [%s]:%i\n", SERVER_ADDR, (int)gw.port);
        return 1;
    }

    printf("Successfully connected to gateway at [%s]:%i\n",
           SERVER_ADDR, (int)gw.port);

    //Setting Up the subscription
    subscriptions[0].topic.name = MQTT_TOPIC;

    //Subscribing to topic
    if (emcute_sub(&subscriptions[0], MQTT_QoS) != EMCUTE_OK) {
        printf("error: unable to subscribe to %s\n", subscriptions[0].topic.name);
        return 1;
    }

    printf("Now subscribed to %s\n", subscriptions[0].topic.name);

    return 0;
}

//Function to publish on Mosquitto broker
static int publish(char *t, char *message){
    emcute_topic_t topic;
    topic.name = t;
    //printf("%s lunghezza %d\n", message, strlen(message)); This was used to check the lenght of the messages

    if(emcute_pub(&topic, message, strlen(message), MQTT_QoS) != EMCUTE_OK){ //publishing on the broker
        printf("cannot publish data\n");
        return 1;
    }
    return 0;

}

//The core of the code
static int start(void)
{

    //Set up the digital pin of the fan
    gpio_t pin_fan = GPIO_PIN(PORT_B, 5);  //PORT BOARD D4, this is the fan
    if (gpio_init(pin_fan, GPIO_OUT)) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_B, 5);
        return -1;
    }

    //Set up the digital pin of the led
    gpio_t pin_led = GPIO_PIN(PORT_A, 8);  //PORT BOARD D7, this is the led
    if (gpio_init(pin_led, GPIO_OUT)) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_A, 8);
        return -1;
    }

    //Set up the digital pin of the trigger
    gpio_t trigger_pin = GPIO_PIN(PORT_A, 9);  //PORT BOARD D8, this is the trigger
    if (gpio_init(trigger_pin, GPIO_OUT)) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_A, 9);
        return -1;
    }

    //Set up the digital pin of the echo
    gpio_t echo_pin = GPIO_PIN(PORT_C, 7);  //PORT BOARD D9, this is the echo
    if (gpio_init(echo_pin, GPIO_OUT)) {
        printf("Error to initialize GPIO_PIN(%d %d)\n", PORT_C, 7);
        return -1;
    }

    //Set up the digital pin of the temperature sensor
    dht_params_t my_params;
    my_params.pin = GPIO_PIN(PORT_A, 10);  //PORT BOARD D2, this is the temperature sensor
    my_params.in_mode = DHT_PARAM_PULL;

    
    dht_t dev;
    if(dht_init(&dev, &my_params) == DHT_OK){
        puts("RIOT smart-fan application starts");
        puts("DHT sensor connected");
        puts("~~~~~~~~~~~~~~~~~~~~~~");
    }  

    volatile uint32_t echo_time_start;
    volatile uint32_t echo_time;


    void echo_cb(void* arg){
        UNUSED(arg);
	    int val = gpio_read(echo_pin);
	    uint32_t echo_time_stop;

	    if(val){
		    echo_time_start = xtimer_now_usec();
	    } else{
		    echo_time_stop = xtimer_now_usec();
		    echo_time = echo_time_stop - echo_time_start;
	    }
    }   

	gpio_init(trigger_pin, GPIO_OUT);
	gpio_init_int(echo_pin, GPIO_IN_PD, GPIO_BOTH, &echo_cb, NULL);


	uint32_t distance;

    int16_t temp, hum;
    while(1){


        echo_time = 0;
		gpio_clear(trigger_pin);
		xtimer_usleep(20);
		gpio_set(trigger_pin);

		xtimer_usleep(100*1000);

        distance = echo_time/58;
        
		printf("distance=%ldcm\n", distance); //print the distance value

        //check the distance, if there is nobody turn off the fun and led and continue
        if(distance >= 20){
            puts("There is nobody, so the fun is OFF");
            gpio_clear(pin_fan);
            gpio_clear(pin_led);
            puts("~~~~~~~~~~~~~~~~~~~~~~");
            xtimer_sleep(10);
            continue;
        }

    
        //check the temperature and the humidity
        if(dht_read(&dev, &temp, &hum) != DHT_OK){
            //if there is an error reading the values of the temperature.
            puts("Error reading temp values");
            puts("~~~~~~~~~~~~~~~~~~~~~~");
            xtimer_sleep(5);
            continue;
        }

        char temp_s[10];
        size_t n = fmt_s16_dfp(temp_s, temp, -1);
        temp_s[n] = '\0';

        char hum_s[10];
        n=fmt_s16_dfp(hum_s, hum, -1);
        hum_s[n]='\0';
        printf("Temp: %s°C - relative humidity: %s%%\n", temp_s, hum_s); //print the temperature and humidty values

        //if the temp is < 28° turn off
        if(temp<280){
             puts("Temperature < 28.0°C, the fan is OFF");
             gpio_clear(pin_fan);
             gpio_clear(pin_led);
        }
        //if the temperature is >= 28° turn on
        else {
            puts("Temperature >= 28°C, the fan is ON");
            gpio_set(pin_fan);
            gpio_set(pin_led);
        }

        char message[9];    
        sprintf(message,"t%sh%s", temp_s, hum_s);
        publish(MQTT_TOPIC, message); //publishing message on broker

        puts("~~~~~~~~~~~~~~~~~~~~~~");
        
        xtimer_sleep(10);
    }
    return -1;
}

 int main(void){
    //Setting memory for the subscription list
    memset(subscriptions, 0, (NUMOFSUBS * sizeof(emcute_sub_t)));
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0, emcute_thread, NULL, "emcute");
    puts("Emcute Thread Started");

    // Adding GUA to the interface 
    if(address_setup(DEFAULT_INTERFACE, DEVICE_IP_ADDR)){
        puts("Impossible to set up the interface");
        return 1;
    }

    //Connecting to broker and subscribing to topics*/
    if(connect_broker()){
        puts("Impossible to Connect Correctly with the Broker");
        return 1;
    }

    //puts("Launch start()");
    res = start();
    if(res == -1) puts("Somethings gone wrong.."); 
    return -1;
    
 }