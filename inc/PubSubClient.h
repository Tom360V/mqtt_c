/*
 PubSubClient.h - A simple client for MQTT.
  Nick O'Leary
  http://knolleary.net
*/

#ifndef PubSubClient_h
#define PubSubClient_h

#include <stdio.h>
#include <stdbool.h>
#include "Client.h"

#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4

// MQTT_VERSION : Pick the version
//#define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 128
#endif

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

// MQTT_MAX_TRANSFER_SIZE : limit how much data is passed to the network client
//  in each write call. Needed for the Arduino Wifi Shield. Leave undefined to
//  pass the entire MQTT packet in each write call.
//#define MQTT_MAX_TRANSFER_SIZE 80

// Possible values for client.state()
#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

#ifdef ESP8266
#include <functional>
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback
#else
#define MQTT_CALLBACK_SIGNATURE void (*callback)(char*, uint8_t*, unsigned int)
#endif

#define boolean bool

typedef unsigned long (*fpMillis_t)(void);

void    PubSubClient_setMyAddress( const char* globalLocation, const char* localLocation, const char* deviceName);
void    PubSubClient_init              (Client_t* client, fpMillis_t fpMillis);
void    PubSubClient_initIP            (Client_t* client, fpMillis_t fpMillis, uint8_t *, uint16_t);
void    PubSubClient_initIPCallback    (Client_t* client, fpMillis_t fpMillis, uint8_t *, uint16_t, MQTT_CALLBACK_SIGNATURE);
void    PubSubClient_initHost          (Client_t* client, fpMillis_t fpMillis, const char*, uint16_t);
void    PubSubClient_initHostCallback  (Client_t* client, fpMillis_t fpMillis, const char*, uint16_t, MQTT_CALLBACK_SIGNATURE);

boolean PubSubClient_connectId(const char* id);
boolean PubSubClient_connectIdUserPass(const char* id, const char* user, const char* pass);
//   boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
boolean PubSubClient_connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
void    PubSubClient_disconnect();

boolean PubSubClient_publish(const char* topic, const uint8_t * payload, unsigned int plength, boolean addAddress);
boolean PubSubClient_publishRetained(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained, boolean addAddress);

boolean PubSubClient_subscribe(const char* topic);
boolean PubSubClient_subscribeQOS(const char* topic, uint8_t qos, uint8_t sendAddress);

boolean PubSubClient_unsubscribe(const char* topic);

boolean PubSubClient_loop();
boolean PubSubClient_connected();

#endif
