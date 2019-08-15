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

#include <network/sal/sal_nb_api.h>
#include <network/sal/sal_bc95.h>
#include <network/sal/sal_at_api.h>
#include <network/sal/sal_socket.h>
#include <string.h>
#include <stdlib.h>
#include <iotbus/iotbus_gpio.h>

#include <tinyara/gpio.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define SAL_NBIOT 0
#define SAL_WIFI  1

int para;
sem_t task_sem;

#if SAL_NBIOT
extern at_adaptor_api bc95_interface;


static int msg_cnt = 0;
void *ctx = NULL;
char rcv_buff[1024];

#define DEVICE1 0
#define DEVICE2 1

#if DEVICE1
//device 1, get the device IMEI, and create device on cloud
#define NBIOT_DEVICE_IMEI "460042337505289"
#endif
//device 2
#if DEVICE2
#define NBIOT_DEVICE_IMEI "869405035794842"
//gpio pir
static iotbus_gpio_context_h g_pir;
#endif

char nbiot_msg[64];


#if DEVICE1
static int gpio_set_direction(int port, gpio_direciton_t dir)
{
	char devpath[16];
	snprintf(devpath, 16, "/dev/gpio%d", port);
	int fd = open(devpath, O_RDWR);
	if (fd < 0) {
		lldbg("fd open %s fail\n",devpath);
		return -1;
	}

	ioctl(fd, GPIOIOC_SET_DIRECTION, dir);

	close(fd);
	return 0;
}

static int gpio_set_level(int port, int value)
{
	char buf[4];
	char devpath[16];
	int fd;
	
	snprintf(devpath, 16, "/dev/gpio%d", port);
	fd = open(devpath, O_RDWR);
	if (fd < 0) {
		lldbg("fd open fail\n");
		return -1;
	}

	ioctl(fd, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);
	if (write(fd, buf, snprintf(buf, sizeof(buf), "%d", !!value)) < 0) {
		lldbg("write error\n");
	}
	
	close(fd);
	return 0;
}

#endif

int sal_app_rcv(int argc, char *argv[])
{
	/* try to rcv msg*/
	while(1) {
		if (atiny_net_recv(ctx,rcv_buff, 1024) > 0) {
			printf("socket receive data from cloud [%s]\n", rcv_buff);
#if DEVICE1
			int seton = (strcmp(rcv_buff, "ON") == 0);
			if (seton || (strcmp(rcv_buff, "OFF") == 0)) {
				int value =  seton? 1 : 0;
				printf("set value %d\n",value);
				gpio_pad_select_gpio(2);
				gpio_set_direction(2, GPIO_DIRECTION_OUT);
				gpio_set_level(2, value);
				gpio_pad_select_gpio(0);
				gpio_set_direction(0, GPIO_DIRECTION_OUT);
				gpio_set_level(0, value);
			}
#endif
		}
	}

#if DEVICE2
static int g_PIROFF;
static int g_SentOff;
struct timespec off_time;
struct timespec send_off_time;

static int validate_task(int argc, char *argv[])
{
	uint32_t dt;
	struct timespec now;
	memset(&now, 0, sizeof(struct timespec));

	while(1) {

		if (g_PIROFF) {
			clock_gettime(CLOCK_MONOTONIC, &now);
			dt = ((now.tv_sec - off_time.tv_sec) * 1000) + ((now.tv_nsec - off_time.tv_nsec) / 1000000);
			if (dt > 5000) { // check for 5s
				g_PIROFF = 0;
				g_SentOff = 1;
				printf("send OFF!\n");
				char *msg = "OFF";
				para = 3;
				snprintf(nbiot_msg, 64, "%s", msg);
				clock_gettime(CLOCK_MONOTONIC, &send_off_time);
				sem_post(&task_sem);
			}
		}

		usleep(200 * 1000);
	}
}


static void gpio_callback_event(void *user_data)
{
	uint32_t dt;
	struct timespec now;
	memset(&now, 0, sizeof(struct timespec));

    int value = iotbus_gpio_read(g_pir);
    printf("gpio_callback_event , value %d\n", value);

	if (value) {
		clock_gettime(CLOCK_MONOTONIC, &now);
		dt = ((now.tv_sec - send_off_time.tv_sec) * 1000) + ((now.tv_nsec - send_off_time.tv_nsec) / 1000000);
		if (g_SentOff && dt > 2000) { //if pir check ON within 2 seconds, do not send ON at once
			char *msg = "ON";
			printf("send ON!\n");
			para = 3;
			snprintf(nbiot_msg, 64, "%s", msg);
			sem_post(&task_sem);
			g_SentOff = 0;
		}
		g_PIROFF = 0;
	} else {
		if (g_SentOff == 0)
			g_PIROFF = 1;

		clock_gettime(CLOCK_MONOTONIC, &off_time);
	}
}
#endif

int sal_task(int argc, char *argv[])
{
	sem_init(&task_sem, 1, 0);

	while(1) {
		sem_wait(&task_sem);
		printf("get cmd sem, para %d\n", para);
		
		switch(para) {
			case 1:
			{
				if (los_nb_init(NULL, NULL, NULL) < 0) {
					printf("los_nb_init failed->\n");
					return 0;
				}
			
				printf("los_nb_init END->\n");
				los_nb_notify("\r\n+NSONMI:",strlen("\r\n+NSONMI:"),nb_cmd_match,strncmp);
				at_api_register(&bc95_interface);
			
			sleep(5);
				if (nb_check_cloud_access() < 0) {
					printf("nb_check_cloud_access failed!\n");
					return 0;
				}
				
				//create udp socket with cloud
				ctx = atiny_net_bind("115.29.240.46", "4588", 17); //actually only port is used, host is fixed for nbiot(cloud server), and proto could only be UDP
				if (!ctx) {
					printf("atiny_net_bind failed!\n");
					return 0;
				}
			
				//register on cloud
				snprintf(nbiot_msg, 64, "ep=%s&pw=123456", NBIOT_DEVICE_IMEI);

				if (atiny_net_sendto(ctx, nbiot_msg, strlen(nbiot_msg), "115.29.240.46", 6000) < 0) {
					printf("send regist msg failed.\n");
					atiny_net_close(ctx);
					return 0;
				}
				printf("registered to cloud server.\n");
				sleep(3);

				snprintf(nbiot_msg, 64, "this is a message from tizenrt %s", NBIOT_DEVICE_IMEI);
				if (atiny_net_sendto(ctx, nbiot_msg, strlen(nbiot_msg), "115.29.240.46", 6000) < 0) {
					printf("send regist msg failed.\n");
					atiny_net_close(ctx);
					return 0;
				}			
			
				while(msg_cnt < 0)
				{
					sleep(3);
					char send_msg[64];
					snprintf(send_msg, 64, "%s %d",nbiot_msg, msg_cnt++);
					if (atiny_net_sendto(ctx, send_msg, strlen(send_msg), "115.29.240.46", 6000) < 0) {
						printf("send regist msg failed.\n");
						return 0;
					}
				}

				task_create("sal_app_rcv", SAL_RCV_TASK_PRIORITY, 0x1000, sal_app_rcv, NULL);
				//atiny_net_close(ctx);
			}
			break;
			case 2:
			{
				snprintf(nbiot_msg, 64, "ep=%s&pw=123456", NBIOT_DEVICE_IMEI);

				if (atiny_net_sendto(ctx, nbiot_msg, strlen(nbiot_msg), "115.29.240.46", 6000) < 0) {
					printf("send regist msg failed.\n");
					return 0;
				}
				printf("registered to cloud server.\n");
			}
			break;
			case 3:
			{
				if (atiny_net_sendto(ctx, nbiot_msg, strlen(nbiot_msg), "115.29.240.46", 6000) < 0) {
					printf("send regist msg failed.\n");
					return 0;
				}
			}
			break;
			case 4:
			{
				atiny_net_close(ctx);
			}
			break;
			default:
				printf("!!wrong para, do nothing\n");
				break;
		}
	}

	return 0;
}
#endif

#if SAL_WIFI
#include "sensor_mqtt_api.h"

extern at_adaptor_api esp8266_interface;
char utopic[32];
char umsg[256];
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

void enable_mqtt_sub()
{
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
	char msg[256] = {0,};
	printf("sensor_mqtt_demo\n");
	
	while (count++ < 3) {
		printf("start regsister_mqtt_server\n");
		regsister_mqtt_server(SENSOR_MQTT_PUB, "109.123.100.115", utopic);
		printf("Send msg : TizenRT message[%d] via mqtt.\n", count);
		snprintf(msg, 256, umsg);
		mqtt_pub_send_msg(msg);
		sleep(1);
	}

	mqtt_destroy_client_sub_task();
}

static int esp_wifi_task(int argc, char *argv[])
{
	sem_init(&task_sem, 1, 0);

	while(1) {
		sem_wait(&task_sem);
		//printf("get cmd sem, para %d\n", para);
		at_api_register(&esp8266_interface);
		//sleep(3);

		sensor_mqtt_demo();
		/*
		void *ctx = atiny_net_connect("109.123.100.115", "1883", 0);
		int ret = atiny_net_send(ctx, "hello 115.", 11);
		printf("send ret %d\n", ret);*/
	}
	
	return 0;
}
#endif

int first_in = 1;
int task_created = 0;
#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int hello_main(int argc, char *argv[])
#endif
{	
	//printf("Hello, World!!\n");

	if (first_in) {
		first_in = 0;
		return;
	}
#if SAL_NBIOT
	int input_para = atoi(argv[1]);
	if (input_para == 1) {
		task_create("sal_api_task", SAL_RCV_TASK_PRIORITY, 0x1000, sal_task, NULL);
		printf("sal_api_task running ->\n");
		para = 1;
		sem_post(&task_sem);
	} else if (ctx && input_para == 2) {
		para = 2;
		sem_post(&task_sem);
	} else if (ctx && input_para == 3){
		para = 3;
		snprintf(nbiot_msg, 64, "this is a message from tizenrt %s", NBIOT_DEVICE_IMEI);
		sem_post(&task_sem);
	} else if (ctx && input_para == 4) {  //close current socket, so that it could create again next time
		para = 4;
		sem_post(&task_sem);
	}
	
#if DEVICE2
	iotapi_initialize();
	sleep(20);
printf("register pir cb ->\n");
	g_pir = iotbus_gpio_open(12);
	if (!g_pir) printf("gpio open failed\n");
	iotbus_gpio_register_cb(g_pir, IOTBUS_GPIO_EDGE_BOTH, gpio_callback_event, NULL);

	task_create("pir_validation", 125, 0x800, validate_task, NULL);
#endif
#endif /*SAL_NBIOT*/

#if SAL_WIFI
	if (!task_created) {
		task_create("esp_wifi_task", SAL_RCV_TASK_PRIORITY, 0x1000, esp_wifi_task, NULL);
		task_created = 1;
	}
	if (argc > 1) {
		strcpy(utopic, argv[1]);
	} else {
		strcpy(utopic, "lqmqtt");
	}

	if (argc > 2) {
		strcpy(umsg, argv[2]);
	} else {
		strcpy(umsg, "Default message, Hello");
	}
	sem_post(&task_sem);
#endif

	return 0;
}
