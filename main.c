#include <stdint.h>
#include <stdbool.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clientSock.h"
#include "PubSubClient.h"
#include "x86.h"
#include "Client.h"
void callback(char* topic, uint8_t* payload, unsigned int length);

void callback(char* topic, uint8_t* payload, unsigned int length)
{
    char* p = (char*)malloc(length+1);

    memcpy(p, payload, length);
    p[length]=0;
    printf(">>Received: %s::%s\r\n", topic, p);

    free(p);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    clientSocket_init();

    PubSubClient_initHostCallback((Client_t*)&clientSock, "test.mosquitto.org",   1883, callback);
//  PubSubClient_initHostCallback((Client_t*)&clientSock, "data.sparkfun.com",    1883, callback);

    millis_init();
    uint8_t interval = 0;
    while(1)
    {
        printf("\ntry to connect...\r\n");
        if (PubSubClient_connectId("mqttClient"))
        {
            printf("    * connected * \r\n");
            PubSubClient_subscribe("bla");

            while(PubSubClient_connected())
            {
                PubSubClient_loop();
                interval++;
                sleep(1);
                if(interval>10)
                {
                    interval=0;
                    printf("try to publish...\r\n");
                    char msg[]="hello world";
                    if(PubSubClient_publish("dummy", (uint8_t*)msg, sizeof(msg)))
                    {
                        printf("    * published * \r\n");
                    }
                    else
                    {
                        PubSubClient_disconnect();
                    }
                }
            }
        }
        sleep(2);
    }
    PubSubClient_disconnect();

    return 0;
}
