# name of your application
APPLICATION = individual_project

# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f401re

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += xtimer
USEMODULE += periph_gpio_irq

USEMODULE += dht
USEMODULE += fmt

USEMODULE += periph_gpio

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6
USEMODULE += gnrc_ipv6_default
# Include MQTT-SN
USEMODULE += emcute
# For testing we also include the ping command and some stats
USEMODULE += gnrc_icmpv6_echo
# Optimize network stack to for use with a single network interface
USEMODULE += gnrc_netif_single

USEMODULE += stdio_ethos gnrc_uhcpc
USEMODULE += sock_udp
USEMODULE += emcute 

CFLAGS += -DCONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF=3 #allow us to specify the     number of IP addressed of the interface (2 are generated automatically and the third one will be added by us)

USE_DHCPV6 ?= 0
IPV6_PREFIX ?= 2000:2::/64

STATIC_ROUTES ?= 1 

UPLINK ?= ethos #specifying that the uplink will be reached using ethos

ETHOS_BAUDRATE ?= 115200 #parameter needed by ethos

CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE) 

TAP ?= tap0
host-tools:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)
TERMDEPS += host-tools
TERMPROG ?= sudo sh $(RIOTTOOLS)/ethos/start_network.sh
TERMFLAGS ?= $(FLAGS_EXTRAS) $(PORT) $(TAP) $(IPV6_PREFIX) $(ETHOS_BAUDRATE)

# Allow for env-var-based override of the nodes name (EMCUTE_ID)
ifneq (,$(EMCUTE_ID))
  CFLAGS += -DEMCUTE_ID=\"$(EMCUTE_ID)\"
endif

include $(RIOTBASE)/Makefile.include

