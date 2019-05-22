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

extern at_adaptor_api bc95_interface;
int first_in = 1;
static int msg_cnt = 0;
void *ctx = NULL;
char rcv_buff[1024];

int para;
sem_t task_sem;
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
				char *msg = "ep=460042337505289&pw=123456";
				if (atiny_net_sendto(ctx, msg, 28, "115.29.240.46", 6000) < 0) {
					printf("send regist msg failed.\n");
					atiny_net_close(ctx);
					return 0;
				}
				printf("registered to cloud server.\n");
				sleep(3);
			
				msg = "this is a message from tizenrt";
				if (atiny_net_sendto(ctx, msg, 30, "115.29.240.46", 6000) < 0) {
					printf("send regist msg failed.\n");
					atiny_net_close(ctx);
					return 0;
				}
			
			
				while(msg_cnt < 3)
				{
					sleep(3);
					char send_msg[64];
					snprintf(send_msg, 64, "this is a message from tizenrt %d",msg_cnt++);
					if (atiny_net_sendto(ctx, send_msg, strlen(send_msg), "115.29.240.46", 6000) < 0) {
						printf("send regist msg failed.\n");
						return 0;
					}
				}

				/* try to rcv msg
				while(1) {
					if (atiny_net_recv(ctx,rcv_buff,1024) > 0) {
						printf("socket receive data from cloud %s\n", rcv_buff);
					}
				}*/
			
				//atiny_net_close(ctx);

			}
			break;
			case 2:
			{
				char *reg_msg = "ep=460042337505289&pw=123456";
				if (atiny_net_sendto(ctx, reg_msg, 28, "115.29.240.46", 6000) < 0) {
					printf("send regist msg failed.\n");
					return 0;
				}
				printf("registered to cloud server.\n");
			}
			break;
			case 3:
			{
				char send_msg[64];
				snprintf(send_msg, 64, "this is a new message from tizenrt %d",msg_cnt++);
				if (atiny_net_sendto(ctx, send_msg, strlen(send_msg), "115.29.240.46", 6000) < 0) {
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

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int hello_main(int argc, char *argv[])
#endif
{	
	printf("Hello, World!!\n");
	if (first_in) {
		first_in = 0;
		return;
	}
	
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
		sem_post(&task_sem);
	} else if (ctx && input_para == 4) {  //close current socket, so that it could create again next time
		para = 4;
		sem_post(&task_sem);
	}

	return 0;
}
