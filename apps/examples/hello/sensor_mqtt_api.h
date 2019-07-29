/******************************************************************
 *
 * Copyright 2018 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/
#ifndef S_SENSOR_MQTT_H_
#define S_SENSOR_MQTT_H_

#include <tinyara/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <tinyara/clock.h>
#include <errno.h>

#include <network/mqtt/mqtt_api.h>


#if defined(CONFIG_NETUTILS_MQTT_SECURITY)
#include "mbedtls/x509_crt.h"
#include "mbedtls/pem.h"
#endif




typedef enum
{
	SENSOR_MQTT_PUB =0,
	SENSOR_MQTT_SUB,
}mqtt_client_type_e;


struct mqtt_structure_pub
{
	mqtt_client_t *mqtt_client_handle;
	mqtt_client_config_t mqtt_client_config;
	mqtt_msg_t mqtt_message;
	sem_t mqtt_sem;
#if defined(CONFIG_NETUTILS_MQTT_SECURITY)
	mqtt_tls_param_t tls;
#endif

	char *id;
	char *host_addr;
	int port;
	char *topic;
	int qos;
	int retain;
	char *username;
	char *password;
	int keepalive;
	int protocol_version;
	bool debug;
	bool nullmsg;
	void *message;
	unsigned int messageLen;
};



struct mqtt_structure_sub
{
	mqtt_client_t *mqtt_client_handle;
	mqtt_client_config_t mqtt_client_config;
	mqtt_msg_t mqtt_message;
#if defined(CONFIG_NETUTILS_MQTT_SECURITY)
	mqtt_tls_param_t tls;
#endif

	char *id;
	char *host_addr;
	int port;
	char *topic;
	int qos;
	char *username;
	char *password;
	int keepalive;
	int protocol_version;
	bool debug;
	bool clean_session;
	int stop;
	char *sub_topic;
	char *unsub_topic;
};

void init_mqtt_variables(mqtt_client_type_e type);
int set_mqtt_host_addr(mqtt_client_type_e type, char *addr);
int set_mqtt_topic(mqtt_client_type_e type, char *topic);
int set_mqtt_pub_payload(void *payload, int lenth);
int mqtt_client_pub_task(void);
int mqtt_client_sub_task(void);
void mqtt_destroy_client_sub_task(void);


#endif

