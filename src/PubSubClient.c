/*
  PubSubClient.cpp - A simple client for MQTT.
  Nick O'Leary
  http://knolleary.net
*/

#include "PubSubClient.h"
#include "x86.h"
#include <stdint.h>
#include <string.h>

/******************************************************************************
 * Private Function Prototypes
 *****************************************************************************/
static void setServerIP(uint8_t * ip, uint16_t port);
static void setServerHost(const char * domain, uint16_t port);
static void setCallback(MQTT_CALLBACK_SIGNATURE);
static void setClient(Client_t* client);
static boolean  readByte    (uint8_t* result);
static boolean  readBytePos (uint8_t* result, uint16_t * index);
static uint16_t readPacket  (uint8_t* lengthLength);

static boolean  write       (uint8_t header, uint8_t* buf, uint16_t length);
static uint16_t writeString (const char* string, uint8_t* buf, uint16_t pos);

/******************************************************************************
 * Private Variable
 *****************************************************************************/
typedef struct pubSubClientData_t
{
    Client_t* client;
    uint8_t buffer[MQTT_MAX_PACKET_SIZE];
    uint16_t nextMsgId;
    unsigned long lastOutActivity;
    unsigned long lastInActivity;
    bool pingOutstanding;
    MQTT_CALLBACK_SIGNATURE;
    IPAddress_t ip;
    const char* domain;
    uint16_t port;
    int state;
} pubSubClientData_t;

static pubSubClientData_t pubSubData;

/******************************************************************************
 * Private Function Implementation
 *****************************************************************************/
static void setServerIP(uint8_t * ip, uint16_t port)
{
    memcpy(pubSubData.ip, ip, 4);
//    this->ip = addr(ip[0],ip[1],ip[2],ip[3]);
    pubSubData.port = port;
    pubSubData.domain = NULL;
}

static void setServerHost(const char * domain, uint16_t port)
{
    pubSubData.domain = domain;
    pubSubData.port = port;
}

static void setCallback(MQTT_CALLBACK_SIGNATURE)
{
    pubSubData.callback = callback;
}

static void setClient(Client_t* client)
{
    pubSubData.client = client;
}

// reads a byte into result
boolean readByte(uint8_t * result)
{
   uint32_t previousMillis = millis();
   while(!pubSubData.client->available()) {
     uint32_t currentMillis = millis();
     if(currentMillis - previousMillis >= ((int32_t) MQTT_SOCKET_TIMEOUT * 1000)){
       return false;
     }
   }
   *result = pubSubData.client->read();
   return true;
}

// reads a byte into result[*index] and increments index
boolean readBytePos(uint8_t * result, uint16_t * index){
  uint16_t current_index = *index;
  uint8_t * write_address = &(result[current_index]);
  if(readByte(write_address)){
    *index = current_index + 1;
    return true;
  }
  return false;
}

static uint16_t readPacket(uint8_t* lengthLength)
{
    uint16_t len = 0;
    if(!readBytePos(pubSubData.buffer, &len)) return 0;
    bool isPublish = (pubSubData.buffer[0]&0xF0) == MQTTPUBLISH;
    uint32_t multiplier = 1;
    uint16_t length = 0;
    uint8_t digit = 0;
    uint16_t skip = 0;
    uint8_t start = 0;

    do {
        if(!readByte(&digit)) return 0;
        pubSubData.buffer[len++] = digit;
        length += (digit & 127) * multiplier;
        multiplier *= 128;
    } while ((digit & 128) != 0);
    *lengthLength = len-1;

    if (isPublish) {
        // Read in topic length to calculate bytes to skip over for Stream writing
        if(!readBytePos(pubSubData.buffer, &len)) return 0;
        if(!readBytePos(pubSubData.buffer, &len)) return 0;
        skip = (pubSubData.buffer[*lengthLength+1]<<8)+pubSubData.buffer[*lengthLength+2];
        start = 2;
        if (pubSubData.buffer[0]&MQTTQOS1) {
            // skip message id
            skip += 2;
        }
    }

    for (uint16_t i = start;i<length;i++)
    {
        if(!readByte(&digit))
        {
            return 0;
        }
        if (len < MQTT_MAX_PACKET_SIZE)
        {
            pubSubData.buffer[len] = digit;
        }
        len++;
    }

    if (len > MQTT_MAX_PACKET_SIZE)
    {
        len = 0; // This will cause the packet to be ignored.
    }

    return len;
}

static boolean write(uint8_t header, uint8_t* buf, uint16_t length)
{
    uint8_t lenBuf[4];
    uint8_t llen = 0;
    uint8_t digit;
    uint8_t pos = 0;
    uint16_t rc;
    uint16_t len = length;
    do {
        digit = len % 128;
        len = len / 128;
        if (len > 0) {
            digit |= 0x80;
        }
        lenBuf[pos++] = digit;
        llen++;
    } while(len>0);

    buf[4-llen] = header;
    for (int i=0;i<llen;i++) {
        buf[5-llen+i] = lenBuf[i];
    }

#ifdef MQTT_MAX_TRANSFER_SIZE
    uint8_t* writeBuf = buf+(4-llen);
    uint16_t bytesRemaining = length+1+llen;  //Match the length type
    uint8_t bytesToWrite;
    boolean result = true;
    while((bytesRemaining > 0) && result) {
        bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE)?MQTT_MAX_TRANSFER_SIZE:bytesRemaining;
        rc = pubSubData.client->writeMulti(writeBuf,bytesToWrite);
        result = (rc == bytesToWrite);
        bytesRemaining -= rc;
        writeBuf += rc;
    }
    return result;
#else
    rc = pubSubData.client->writeMulti(buf+(4-llen),length+1+llen);
    pubSubData.lastOutActivity = millis();
    return (rc == 1+llen+length);
#endif
}

static uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos)
{
    const char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp)
    {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos-i-2] = (i >> 8);
    buf[pos-i-1] = (i & 0xFF);
    return pos;
}
/******************************************************************************
 * Function implementation
 *****************************************************************************/
void PubSubClient_init(Client_t* client)
{
    pubSubData.state = MQTT_DISCONNECTED;
    setClient(client);
}

void PubSubClient_initIP(Client_t* client, uint8_t *ip, uint16_t port)
{
    PubSubClient_initIPCallback(client, ip, port, NULL);
}

void PubSubClient_initIPCallback(Client_t* client, uint8_t *ip, uint16_t port, MQTT_CALLBACK_SIGNATURE)
{
    pubSubData.state = MQTT_DISCONNECTED;
    setServerIP(ip, port);
    setCallback(callback);
    setClient(client);
}

void PubSubClient_initHost(Client_t* client, const char* domain, uint16_t port)
{
    PubSubClient_initHostCallback(client, domain, port, NULL);
}

void PubSubClient_initHostCallback(Client_t* client, const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE)
{
    pubSubData.state = MQTT_DISCONNECTED;
    setServerHost(domain,port);
    setCallback(callback);
    setClient(client);
}

boolean PubSubClient_connectId(const char *id)
{
    return PubSubClient_connect(id,NULL,NULL,0,0,0,0);
}

boolean PubSubClient_connectIdUserPass(const char *id, const char *user, const char *pass)
{
    return PubSubClient_connect(id,user,pass,0,0,0,0);
}

/*boolean PubSubClient::connect(const char *id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage) {
    return connect(id,NULL,NULL,willTopic,willQos,willRetain,willMessage);
}*/

boolean PubSubClient_connect(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage) {
    if (!PubSubClient_connected())
    {
        int result = 0;

        if (pubSubData.domain != NULL) {
            result = pubSubData.client->connectHost(pubSubData.domain, pubSubData.port);
        } else {
            result = pubSubData.client->connectIP(pubSubData.ip, pubSubData.port);
        }
        if (result == 1) {
            pubSubData.nextMsgId = 1;
            // Leave room in the buffer for header and variable length field
            uint16_t length = 5;
            unsigned int j;

#if MQTT_VERSION == MQTT_VERSION_3_1
            uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p', MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
            uint8_t d[7] = {0x00,0x04,'M','Q','T','T',MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 7
#endif
            for (j = 0;j<MQTT_HEADER_VERSION_LENGTH;j++) {
                pubSubData.buffer[length++] = d[j];
            }

            uint8_t v;
            if (willTopic) {
                v = 0x06|(willQos<<3)|(willRetain<<5);
            } else {
                v = 0x02;
            }

            if(user != NULL) {
                v = v|0x80;

                if(pass != NULL) {
                    v = v|(0x80>>1);
                }
            }

            pubSubData.buffer[length++] = v;

            pubSubData.buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
            pubSubData.buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
            length = writeString(id,pubSubData.buffer,length);
            if (willTopic) {
                length = writeString(willTopic,pubSubData.buffer,length);
                length = writeString(willMessage,pubSubData.buffer,length);
            }

            if(user != NULL) {
                length = writeString(user,pubSubData.buffer,length);
                if(pass != NULL) {
                    length = writeString(pass,pubSubData.buffer,length);
                }
            }

            write(MQTTCONNECT,pubSubData.buffer,length-5);

            pubSubData.lastInActivity = pubSubData.lastOutActivity = millis();

            while (!pubSubData.client->available()) {
                unsigned long t = millis();
                if (t-pubSubData.lastInActivity >= ((int32_t) MQTT_SOCKET_TIMEOUT*1000UL)) {
                    pubSubData.state = MQTT_CONNECTION_TIMEOUT;
                    pubSubData.client->stop();
                    return false;
                }
            }
            uint8_t llen;
            uint16_t len = readPacket(&llen);

            if (len == 4)
            {
                if (pubSubData.buffer[3] == 0)
                {
                    pubSubData.lastInActivity = millis();
                    pubSubData.pingOutstanding = false;
                    pubSubData.state = MQTT_CONNECTED;
                    return true;
                } else {
                    pubSubData.state = pubSubData.buffer[3];
                }
            }
            pubSubData.client->stop();
        } else {
            pubSubData.state = MQTT_CONNECT_FAILED;
        }
        return false;
    }
    return true;
}


boolean PubSubClient_loop()
{
    if (PubSubClient_connected())
    {
        unsigned long t = millis();
        if( (t - pubSubData.lastInActivity > MQTT_KEEPALIVE*1000UL) ||
            (t - pubSubData.lastOutActivity > MQTT_KEEPALIVE*1000UL))
        {
            if (pubSubData.pingOutstanding) {
                pubSubData.state = MQTT_CONNECTION_TIMEOUT;
                pubSubData.client->stop();
                return false;
            } else {
                pubSubData.buffer[0] = MQTTPINGREQ;
                pubSubData.buffer[1] = 0;
                pubSubData.client->writeMulti(pubSubData.buffer,2);
                pubSubData.lastOutActivity = t;
                pubSubData.lastInActivity = t;
                pubSubData.pingOutstanding = true;
            }
        }
        if (pubSubData.client->available())
        {
            uint8_t llen;
            uint16_t len = readPacket(&llen);
            uint16_t msgId = 0;
            uint8_t *payload;
            if (len > 0) {
                pubSubData.lastInActivity = t;
                uint8_t type = pubSubData.buffer[0]&0xF0;
                if (type == MQTTPUBLISH)
                {
                    uint16_t tl = (pubSubData.buffer[llen+1]<<8)+pubSubData.buffer[llen+2];
                    char topic[tl+1];
                    for (uint16_t i=0;i<tl;i++)
                    {
                        topic[i] = pubSubData.buffer[llen+3+i];
                    }
                    topic[tl] = 0;
                    // msgId only present for QOS>0
                    if ((pubSubData.buffer[0]&0x06) == MQTTQOS1)
                    {
                        msgId = (pubSubData.buffer[llen+3+tl]<<8)+pubSubData.buffer[llen+3+tl+1];
                        payload = pubSubData.buffer+llen+3+tl+2;
                        pubSubData.callback(topic,payload,len-llen-3-tl-2);

                        pubSubData.buffer[0] = MQTTPUBACK;
                        pubSubData.buffer[1] = 2;
                        pubSubData.buffer[2] = (msgId >> 8);
                        pubSubData.buffer[3] = (msgId & 0xFF);
                        pubSubData.client->writeMulti(pubSubData.buffer,4);
                        pubSubData.lastOutActivity = t;

                    }
                    else
                    {
                        payload = pubSubData.buffer+llen+3+tl;
                        pubSubData.callback(topic,payload,len-llen-3-tl);
                    }
                }
                else if (type == MQTTPINGREQ)
                {
                    pubSubData.buffer[0] = MQTTPINGRESP;
                    pubSubData.buffer[1] = 0;
                    pubSubData.client->writeMulti(pubSubData.buffer,2);
                }
                else if (type == MQTTPINGRESP)
                {
                    pubSubData.pingOutstanding = false;
                }
            }
        }
        return true;
    }
    return false;
}

boolean PubSubClient_publish(const char* topic, const uint8_t* payload, unsigned int plength)
{
    return PubSubClient_publishRetained(topic, payload, plength, false);
}

boolean PubSubClient_publishRetained(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained)
{
    if (PubSubClient_connected()) {
        if (MQTT_MAX_PACKET_SIZE < 5 + 2+strlen(topic) + plength) {
            // Too long
            return false;
        }
        // Leave room in the buffer for header and variable length field
        uint16_t length = 5;
        length = writeString(topic,pubSubData.buffer,length);
        uint16_t i;
        for (i=0;i<plength;i++) {
            pubSubData.buffer[length++] = payload[i];
        }
        uint8_t header = MQTTPUBLISH;
        if (retained) {
            header |= 1;
        }
        return write(header,pubSubData.buffer,length-5);
    }
    return false;
}


boolean PubSubClient_subscribe(const char* topic)
{
    return PubSubClient_subscribeQOS(topic, 0);
}

boolean PubSubClient_subscribeQOS(const char* topic, uint8_t qos)
{
    if (qos < 0 || qos > 1)
    {
        return false;
    }
    if (MQTT_MAX_PACKET_SIZE < 9 + strlen(topic))
    {
        // Too long
        return false;
    }
    if (PubSubClient_connected())
    {
        // Leave room in the buffer for header and variable length field
        uint16_t length = 5;
        pubSubData.nextMsgId++;
        if (pubSubData.nextMsgId == 0)
        {
            pubSubData.nextMsgId = 1;
        }
        pubSubData.buffer[length++] = (pubSubData.nextMsgId >> 8);
        pubSubData.buffer[length++] = (pubSubData.nextMsgId & 0xFF);
        length = writeString((char*)topic, pubSubData.buffer,length);
        pubSubData.buffer[length++] = qos;
        return write(MQTTSUBSCRIBE|MQTTQOS1,pubSubData.buffer,length-5);
    }
    return false;
}

boolean PubSubClient_unsubscribe(const char* topic)
{
    if (MQTT_MAX_PACKET_SIZE < 9 + strlen(topic)) {
        // Too long
        return false;
    }
    if (PubSubClient_connected()) {
        uint16_t length = 5;
        pubSubData.nextMsgId++;
        if (pubSubData.nextMsgId == 0) {
            pubSubData.nextMsgId = 1;
        }
        pubSubData.buffer[length++] = (pubSubData.nextMsgId >> 8);
        pubSubData.buffer[length++] = (pubSubData.nextMsgId & 0xFF);
        length = writeString(topic, pubSubData.buffer,length);
        return write(MQTTUNSUBSCRIBE|MQTTQOS1,pubSubData.buffer,length-5);
    }
    return false;
}

void PubSubClient_disconnect()
{
    pubSubData.buffer[0] = MQTTDISCONNECT;
    pubSubData.buffer[1] = 0;
    pubSubData.client->writeMulti(pubSubData.buffer,2);
    pubSubData.state = MQTT_DISCONNECTED;
    pubSubData.client->stop();
    pubSubData.lastInActivity = pubSubData.lastOutActivity = millis();
}

boolean PubSubClient_connected()
{
    boolean rc;
    if (pubSubData.client == NULL ) {
        rc = false;
    } else {
        rc = (int)pubSubData.client->connected();
        if (!rc) {
            if (pubSubData.state == MQTT_CONNECTED) {
                pubSubData.state = MQTT_CONNECTION_LOST;
                pubSubData.client->flush();
                pubSubData.client->stop();
            }
        }
    }
    return rc;
}

