/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * examples/hello/hello_main.c
 *
 *   Copyright (C) 2008, 2011-2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <stdio.h>

/****************************************************************************
 * hello_main
 ****************************************************************************/
#include "sensor_mqtt_api.h"

char utopic[64];
static int regsister_mqtt_server(mqtt_client_type_e type,char *addr, char *topic)
{
	if (!topic || !addr)
	{
		printf("Input null pointer!\n");
		return -1;
	}
	//sent the event to cloud server
	init_mqtt_variables(type);
	set_mqtt_host_addr(type,addr);
	set_mqtt_topic(type,topic);
	set_mqtt_debug_mode(type, true);
	return 0;
}


//static int mqtt_pub_send_msg(struct rule_engine_event *evt)
static int mqtt_pub_send_msg(char *evt)
{
	int i;

	if (!evt)
	{
		printf("Input null pionter!\n");
		return -1;
	}
/*
	//Create event data
	struct mqtt_sensor_event senEvt;
	memset(&senEvt, 0,sizeof(struct mqtt_sensor_event));
	senEvt.event_type = evt->event_type;
	senEvt.data_cnt = evt->data_cnt;
	senEvt.reserved[0] = evt->reserved[0];
	senEvt.reserved[1] = evt->reserved[1];
	senEvt.timestamp = evt->timestamp;

	for(i=0;i<evt->data_cnt;i++)
	{
		senEvt.data[i] = evt->data[i];
	}
*/
	set_mqtt_pub_payload(evt, strlen(evt)+1);
	mqtt_client_pub_task();
	return 0;
}

void sensor_message_callback(void *client, mqtt_msg_t *msg)
{
	mqtt_client_t *mqtt_client = (mqtt_client_t *)client;

	printf("\n=============mqtt_message_callback==============\n");

	if (mqtt_client == NULL || mqtt_client->config == NULL) {
		fprintf(stderr, "Error: >>> message callback: %s is NULL.\n", mqtt_client == NULL ? "mqtt_client" : "mqtt_client->config");
		return;
	}

	if (msg->payload_len) {
		printf("[Topic]: %s\n", msg->topic);
		printf("[Message]: %s\n", msg->payload);
	}
}

void enable_mqtt_sub()
{
printf("subscribe topic %s\n",utopic);
	 regsister_mqtt_server(SENSOR_MQTT_SUB, "109.123.100.115", utopic);
	 mqtt_client_sub_task();
	
	unsigned int count = 10000;
	 while(count)
	 {
		 sleep(2);
		 count--;
	 }
	
	 mqtt_destroy_client_sub_task();

}
void sensor_mqtt_demo(void)
{
	int count = 0;
	char msg[64] = {0,};
	printf("sensor_mqtt_demo\n");
	
	while (count++ < 10) {
		printf("start regsister_mqtt_server\n");
		regsister_mqtt_server(SENSOR_MQTT_PUB, "109.123.100.115", utopic);
		printf("Send msg : TizenRT message[%d] via mqtt.\n", count);
		snprintf(msg, 64, "TizenRT message[%d] via mqtt.", count);
		mqtt_pub_send_msg(msg);
		sleep(3);
	}

}

struct options {
	void* func;
	uint16_t channel;
	char *ssid;
	char *bad_ssid;
	char *password;
	char *bad_password;
	int    auth_type;
	int  crypto_type;
	char *softap_ssid;
	char *softap_password;
};
extern void wm_start(void *arg);
extern void wm_connect(void *arg);
int wifi_connected = 0;

int first = 0;
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int hello_main(int argc, char *argv[])
#endif
{
	printf("Hello, World!! \n");
	if (!first) {
		first =1 ;
		return 0;
	}

	if (argc > 1) {
		strcpy(utopic, argv[1]);
	} else {
		strcpy(utopic, "lqmqtt");
	}

	if (!wifi_connected) {
		//task_create("wifi connect", 100, 1024 * 10, wm_process, NULL);
		wifi_connected = 1;
		wm_start(NULL);
		sleep(2);
		struct options op = {
			.ssid = "SRCN-2.4G119",
			.auth_type = get_auth_type("wpa2_aes"),
			.crypto_type = get_crypto_type("wpa2_aes"), //WIFI_MANAGER_CRYPTO_AES,
			.password = "wc6rn9v3"
		};
		wm_connect(&op);
		sleep(7);
	}

	//sensor_mqtt_demo(); //for publish
	enable_mqtt_sub(); //for subscribe
	return 0;
}
