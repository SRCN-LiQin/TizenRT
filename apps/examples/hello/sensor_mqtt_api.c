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
/**
 * @file mqtt_client_pub.c
 * @brief the program for testing mqtt publisher
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "sensor_mqtt_api.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/
#define MQTT_CLIENT_PUB_COMMAND_NAME	"mqtt_pub"
#define MQTT_CLIENT_SUB_COMMAND_NAME	"mqtt_sub"
#define MQTT_ID_MAX_LENGTH				23

#define MQTT_DEBUG_PRINT(client_handle, ...) ets_printf(__VA_ARGS__);

/*#define MQTT_DEBUG_PRINT(client_handle, ...) \
		do { \
			if (client_handle && (client_handle)->config && (client_handle)->config->debug) \
				printf(__VA_ARGS__); \
		} while (0);*/

typedef enum {
	CHECK_OPTION_RESULT_CHECKED_OK,
	CHECK_OPTION_RESULT_CHECKED_ERROR,
	CHECK_OPTION_RESULT_NOTHING_TODO,
} check_option_result_e;

/****************************************************************************
 * External Function Prototype
 ****************************************************************************/
extern const unsigned char *mqtt_get_ca_certificate(void);
extern const unsigned char *mqtt_get_client_certificate(void);
extern const unsigned char *mqtt_get_client_key(void);
extern int mqtt_get_ca_certificate_size(void);
extern int mqtt_get_client_certificate_size(void);
extern int mqtt_get_client_key_size(void);

/****************************************************************************
 * Global Valiables
 ****************************************************************************/
struct mqtt_structure_pub g_mqtt_pub;
struct mqtt_structure_sub g_mqtt_sub;

/****************************************************************************
 * Static Functions
 ****************************************************************************/
static void sensor_mqtt_pub_connect_callback(void *client, int result)
{
	mqtt_client_t *mqtt_client = (mqtt_client_t *) client;
	mqtt_msg_t *mqtt_msg = NULL;

	if (mqtt_client == NULL || mqtt_client->config == NULL) {
		fprintf(stderr, "Error: >>> connect callback: %s is NULL.\n", mqtt_client == NULL ? "mqtt_client" : "mqtt_client->config");
		return;
	}

	if (result == MQTT_CONN_ACCEPTED) {
		MQTT_DEBUG_PRINT(mqtt_client, ">>> connect callback: client_id=%s, connect success!\n", mqtt_client->config->client_id);

		if (mqtt_client->config->user_data) {
			mqtt_msg = (mqtt_msg_t *) mqtt_client->config->user_data;
			if (mqtt_publish(mqtt_client, mqtt_msg->topic, mqtt_msg->payload, mqtt_msg->payload_len, mqtt_msg->qos, mqtt_msg->retain) != 0) {
				fprintf(stderr, "Error: mqtt_publish() failed.\n");
			}
		} else {
			fprintf(stderr, "Error: mqtt_client is NULL.\n");
		}
	} else {
		char reason_str[40];
		switch (result) {
		case MQTT_CONN_REFUSED_UNACCEPTABLE_PROTOCOL_VER:
			snprintf(reason_str, sizeof(reason_str), "unacceptable protocol version");
			break;
		case MQTT_CONN_REFUSED_ID_REJECTED:
			snprintf(reason_str, sizeof(reason_str), "identifier rejected");
			break;
		case MQTT_CONN_REFUSED_BAD_USER_NAME_OR_PASSWORD:
			snprintf(reason_str, sizeof(reason_str), "bad user name or password");
			break;
		case MQTT_CONN_REFUSED_NOT_AUTHORIZED:
			snprintf(reason_str, sizeof(reason_str), "not authorized");
			break;
		default:
			snprintf(reason_str, sizeof(reason_str), "unknown");
			break;
		}

		MQTT_DEBUG_PRINT(mqtt_client, ">>> connect callback: client_id=%s, connect failed (reason: %s)\n", mqtt_client->config->client_id, reason_str);
	}
}

static void sensor_mqtt_pub_disconnect_callback(void *client, int result)
{
	mqtt_client_t *mqtt_client = (mqtt_client_t *) client;

	if (mqtt_client == NULL || mqtt_client->config == NULL) {
		fprintf(stderr, "Error: >>> disconnect callback: %s is NULL.\n", mqtt_client == NULL ? "mqtt_client" : "mqtt_client->config");
		return;
	}

	if (result == 0) {
		MQTT_DEBUG_PRINT(mqtt_client, ">>> disconnect callback: client_id=%s, disconnected by mqtt_disconnect()\n", mqtt_client->config->client_id);
	} else {
		MQTT_DEBUG_PRINT(mqtt_client, ">>> disconnect callback: client_id=%s, disconnected by other reason\n", mqtt_client->config->client_id);
	}

	sem_post(&g_mqtt_pub.mqtt_sem);
}

static void sensor_mqtt_publish_callback(void *client, int msg_id)
{
	mqtt_client_t *mqtt_client = (mqtt_client_t *) client;

	if (mqtt_client == NULL || mqtt_client->config == NULL) {
		fprintf(stderr, "Error: >>> publish callback: %s is NULL.\n", mqtt_client == NULL ? "mqtt_client" : "mqtt_client->config");
		return;
	}

	MQTT_DEBUG_PRINT(mqtt_client, ">>> publish callback: client_id=%s, msg_id = %d\n", mqtt_client->config->client_id, msg_id);

	MQTT_DEBUG_PRINT(mqtt_client, "disconnect from a MQTT broker before stopping MQTT client.\n");
	if (mqtt_disconnect(mqtt_client) != 0) {
		fprintf(stderr, "Error: mqtt_disconnect() failed.\n");
	}
}

static void sensor_mqtt_sub_connect_callback(void *client, int result)
{
	mqtt_client_t *mqtt_client = (mqtt_client_t *) client;
	mqtt_msg_t *mqtt_msg = NULL;

	if (mqtt_client == NULL || mqtt_client->config == NULL) {
		fprintf(stderr, "Error: >>> connect callback: %s is NULL.\n", mqtt_client == NULL ? "mqtt_client" : "mqtt_client->config");
		return;
	}

	if (result == MQTT_CONN_ACCEPTED) {

		MQTT_DEBUG_PRINT(mqtt_client, ">>> connect callback: client_id=%s, connect success!\n", mqtt_client->config->client_id);

		if (mqtt_client->config->user_data) {
			mqtt_msg = (mqtt_msg_t *) mqtt_client->config->user_data;
			if (mqtt_subscribe(mqtt_client, mqtt_msg->topic, mqtt_msg->qos) != 0) {
				fprintf(stderr, "Error: mqtt_subscribe() failed.\n");
			}
		} else {
			fprintf(stderr, "Error: mqtt_client is NULL.\n");
		}
	} else {
		char reason_str[40];
		switch (result) {
		case MQTT_CONN_REFUSED_UNACCEPTABLE_PROTOCOL_VER:
			snprintf(reason_str, sizeof(reason_str), "unacceptable protocol version");
			break;
		case MQTT_CONN_REFUSED_ID_REJECTED:
			snprintf(reason_str, sizeof(reason_str), "identifier rejected");
			break;
		case MQTT_CONN_REFUSED_BAD_USER_NAME_OR_PASSWORD:
			snprintf(reason_str, sizeof(reason_str), "bad user name or password");
			break;
		case MQTT_CONN_REFUSED_NOT_AUTHORIZED:
			snprintf(reason_str, sizeof(reason_str), "not authorized");
			break;
		default:
			snprintf(reason_str, sizeof(reason_str), "unknown");
			break;
		}

		MQTT_DEBUG_PRINT(mqtt_client, ">>> connect callback: client_id=%s, connect failed (reason: %s)\n", mqtt_client->config->client_id, reason_str);
	}
}

static void sensor_mqtt_sub_disconnect_callback(void *client, int result)
{
	mqtt_client_t *mqtt_client = (mqtt_client_t *) client;

	if (mqtt_client == NULL || mqtt_client->config == NULL) {
		fprintf(stderr, "Error: >>> disconnect callback: %s is NULL.\n", mqtt_client == NULL ? "mqtt_client" : "mqtt_client->config");
		return;
	}

	if (result == 0) {
		MQTT_DEBUG_PRINT(mqtt_client, ">>> disconnect callback: client_id=%s, disconnected by mqtt_disconnect()\n", mqtt_client->config->client_id);
	} else {
		MQTT_DEBUG_PRINT(mqtt_client, ">>> disconnect callback: client_id=%s, disconnected by other reason\n", mqtt_client->config->client_id);
	}
}

static void sensor_mqtt_subscribe_callback(void *client, int msg_id, int qos_count, const int *granted_qos)
{
	mqtt_client_t *mqtt_client = (mqtt_client_t *) client;

	if (mqtt_client == NULL || mqtt_client->config == NULL) {
		fprintf(stderr, "Error: >>> subscribe callback: %s is NULL.\n", mqtt_client == NULL ? "mqtt_client" : "mqtt_client->config");
		return;
	}

	MQTT_DEBUG_PRINT(mqtt_client, ">>> subscribe callback: client_id=%s, msg_id=%d, qos_count=%d, granted_qos=%d\n", mqtt_client->config->client_id, msg_id, qos_count, *granted_qos);
}

static void sensor_mqtt_unsubscribe_callback(void *client, int msg_id)
{
	mqtt_client_t *mqtt_client = (mqtt_client_t *) client;

	if (mqtt_client == NULL || mqtt_client->config == NULL) {
		fprintf(stderr, "Error: >>> unsubscribe callback: %s is NULL.\n", mqtt_client == NULL ? "mqtt_client" : "mqtt_client->config");
		return;
	}

	MQTT_DEBUG_PRINT(mqtt_client, ">>> unsubscribe callback: client_id=%s, msg_id=%d\n", mqtt_client->config->client_id, msg_id);
}

void mqtt_set_srand(void)
{
	static int initialzed = 0;
	if (!initialzed) {
		srand(time(NULL));
		initialzed = 1;
	}
}

char *mqtt_generate_client_id(const char *id_base)
{
	int len;
	char *client_id = NULL;

	len = strlen(id_base) + strlen("/") + 5 + 1;
	client_id = malloc(len);
	if (!client_id) {
		fprintf(stderr, "Error: Out of memory.\n");
		return NULL;
	}
	snprintf(client_id, len, "%s/%05d", id_base, rand() % 100000);
	if (strlen(client_id) > MQTT_ID_MAX_LENGTH) {
		/* Enforce maximum client id length of 23 characters */
		client_id[MQTT_ID_MAX_LENGTH] = '\0';
	}

	return client_id;
}

void init_mqtt_variables(mqtt_client_type_e type)
{
	if (SENSOR_MQTT_PUB == type) {
		memset(&g_mqtt_pub, 0, sizeof(struct mqtt_structure_pub));
		g_mqtt_pub.port = MQTT_DEFAULT_BROKER_PORT;
		g_mqtt_pub.keepalive = MQTT_DEFAULT_KEEP_ALIVE_TIME;
		g_mqtt_pub.protocol_version = MQTT_PROTOCOL_VERSION_31;
	} else if (SENSOR_MQTT_SUB == type) {
		memset(&g_mqtt_sub, 0, sizeof(struct mqtt_structure_sub));
		g_mqtt_sub.port = MQTT_DEFAULT_BROKER_PORT;
		g_mqtt_sub.keepalive = MQTT_DEFAULT_KEEP_ALIVE_TIME;
		g_mqtt_sub.protocol_version = MQTT_PROTOCOL_VERSION_31;
		g_mqtt_sub.clean_session = true;
		g_mqtt_sub.debug = 1;
	}
}

void deinit_mqtt_variables(mqtt_client_type_e type)
{
	if (SENSOR_MQTT_PUB == type) {
		if (g_mqtt_pub.id) {
			free(g_mqtt_pub.id);
			g_mqtt_pub.id = NULL;
		}

		if (g_mqtt_pub.host_addr) {
			free(g_mqtt_pub.host_addr);
			g_mqtt_pub.host_addr = NULL;
		}

		if (g_mqtt_pub.topic) {
			free(g_mqtt_pub.topic);
			g_mqtt_pub.topic = NULL;
		}

		if (g_mqtt_pub.message) {
			free(g_mqtt_pub.message);
			g_mqtt_pub.message = NULL;
		}

		if (g_mqtt_pub.username) {
			free(g_mqtt_pub.username);
			g_mqtt_pub.username = NULL;
		}

		if (g_mqtt_pub.password) {
			free(g_mqtt_pub.password);
			g_mqtt_pub.password = NULL;
		}
	} else if (SENSOR_MQTT_SUB == type) {
		if (g_mqtt_sub.id) {
			free(g_mqtt_sub.id);
			g_mqtt_sub.id = NULL;
		}

		if (g_mqtt_sub.host_addr) {
			free(g_mqtt_sub.host_addr);
			g_mqtt_sub.host_addr = NULL;
		}

		if (g_mqtt_sub.topic) {
			free(g_mqtt_sub.topic);
			g_mqtt_sub.topic = NULL;
		}

		if (g_mqtt_sub.username) {
			free(g_mqtt_sub.username);
			g_mqtt_sub.username = NULL;
		}

		if (g_mqtt_sub.password) {
			free(g_mqtt_sub.password);
			g_mqtt_sub.password = NULL;
		}

		if (g_mqtt_sub.unsub_topic) {
			free(g_mqtt_sub.unsub_topic);
			g_mqtt_sub.unsub_topic = NULL;
		}

		if (g_mqtt_sub.sub_topic) {
			free(g_mqtt_sub.sub_topic);
			g_mqtt_sub.sub_topic = NULL;
		}
	}
}

static int make_client_config_pub(void)
{
	if (g_mqtt_pub.host_addr == NULL) {
		fprintf(stderr, "Error: broker address is NULL. You can set host address with -h option.\n");
		goto errout;
	}

	if (g_mqtt_pub.topic == NULL) {
		fprintf(stderr, "Error: topic is NULL. You can set host address with -t option.\n");
		goto errout;
	}

	if ((g_mqtt_pub.message == NULL) && (g_mqtt_pub.nullmsg == false)) {
		fprintf(stderr, "Error: message is NULL. You can set host address with -m option.\n");
		goto errout;
	}

	if (g_mqtt_pub.id == NULL) {
		g_mqtt_pub.id = mqtt_generate_client_id(MQTT_CLIENT_PUB_COMMAND_NAME);
		if (g_mqtt_pub.id == NULL) {
			fprintf(stderr, "Error: fail to set a client id.\n");
			goto errout;
		}
	}

	/* set information to publish */
	memset(&g_mqtt_pub.mqtt_message, 0, sizeof(mqtt_msg_t));
	g_mqtt_pub.mqtt_message.topic = strdup(g_mqtt_pub.topic);
	if (g_mqtt_pub.nullmsg) {
		g_mqtt_pub.mqtt_message.payload = NULL;
		g_mqtt_pub.mqtt_message.payload_len = 0;
	} else {
		g_mqtt_pub.mqtt_message.payload = malloc(g_mqtt_pub.messageLen);
		if (g_mqtt_pub.mqtt_message.payload) {
			memcpy(g_mqtt_pub.mqtt_message.payload, g_mqtt_pub.message, g_mqtt_pub.messageLen);
			g_mqtt_pub.mqtt_message.payload_len = g_mqtt_pub.messageLen;
			free(g_mqtt_pub.message);
			g_mqtt_pub.message = NULL;
			g_mqtt_pub.messageLen = 0;
		}
	}

	g_mqtt_pub.mqtt_message.qos = g_mqtt_pub.qos;
	g_mqtt_pub.mqtt_message.retain = g_mqtt_pub.retain;

#if defined(CONFIG_NETUTILS_MQTT_SECURITY)
	/* set tls parameters */

	/* set ca_cert */
	g_mqtt_pub.tls.ca_cert = mqtt_get_ca_certificate();	/* the pointer of ca_cert buffer */
	g_mqtt_pub.tls.ca_cert_len = mqtt_get_ca_certificate_size();	/* the length of ca_cert buffer  */

	/* set cert */
	g_mqtt_pub.tls.cert = mqtt_get_client_certificate();	/* the pointer of cert buffer */
	g_mqtt_pub.tls.cert_len = mqtt_get_client_certificate_size();	/* the length of cert buffer */

	/* set key */
	g_mqtt_pub.tls.key = mqtt_get_client_key();	/* the pointer of key buffer */
	g_mqtt_pub.tls.key_len = mqtt_get_client_key_size();	/* the length of key buffer */

#endif
	/* set mqtt config */
	memset(&g_mqtt_pub.mqtt_client_config, 0, sizeof(mqtt_client_config_t));
	g_mqtt_pub.mqtt_client_config.client_id = strdup(g_mqtt_pub.id);
	g_mqtt_pub.mqtt_client_config.user_name = strdup(g_mqtt_pub.username);
	g_mqtt_pub.mqtt_client_config.password = strdup(g_mqtt_pub.password);
	g_mqtt_pub.mqtt_client_config.protocol_version = g_mqtt_pub.protocol_version;
	g_mqtt_pub.mqtt_client_config.debug = g_mqtt_pub.debug;
	g_mqtt_pub.mqtt_client_config.on_connect = sensor_mqtt_pub_connect_callback;
	g_mqtt_pub.mqtt_client_config.on_disconnect = sensor_mqtt_pub_disconnect_callback;
	g_mqtt_pub.mqtt_client_config.on_publish = sensor_mqtt_publish_callback;
	g_mqtt_pub.mqtt_client_config.user_data = &g_mqtt_pub.mqtt_message;

#if defined(CONFIG_NETUTILS_MQTT_SECURITY)
	if (g_port == MQTT_SECURITY_BROKER_PORT) {
		g_mqtt_pub.mqtt_client_config.tls = &g_tls;
	} else {
		g_mqtt_pub.mqtt_client_config.tls = NULL;
	}
#else
	g_mqtt_pub.mqtt_client_config.tls = NULL;
#endif

	return 0;

errout:
	return -1;
}

static void clean_client_config_pub(void)
{
	/* g_publish_msg */
	if (g_mqtt_pub.mqtt_message.topic) {
		free(g_mqtt_pub.mqtt_message.topic);
		g_mqtt_pub.mqtt_message.topic = NULL;
	}

	if (g_mqtt_pub.mqtt_message.payload) {
		free(g_mqtt_pub.mqtt_message.payload);
		g_mqtt_pub.mqtt_message.payload = NULL;
	}

	/* g_mqtt_client_config */
	if (g_mqtt_pub.mqtt_client_config.client_id) {
		free(g_mqtt_pub.mqtt_client_config.client_id);
		g_mqtt_pub.mqtt_client_config.client_id = NULL;
	}

	if (g_mqtt_pub.mqtt_client_config.user_name) {
		free(g_mqtt_pub.mqtt_client_config.user_name);
		g_mqtt_pub.mqtt_client_config.user_name = NULL;
	}

	if (g_mqtt_pub.mqtt_client_config.password) {
		free(g_mqtt_pub.mqtt_client_config.password);
		g_mqtt_pub.mqtt_client_config.password = NULL;
	}
}

static int make_client_config_sub(void)
{
	if (g_mqtt_sub.host_addr == NULL) {
		fprintf(stderr, "Error: broker address is NULL. You can set host address with -h option.\n");
		goto errout;
	}

	if (g_mqtt_sub.topic == NULL) {
		fprintf(stderr, "Error: topic is NULL. You can set host address with -t option.\n");
		goto errout;
	}

	if (g_mqtt_sub.id == NULL) {
		if (g_mqtt_sub.clean_session == false) {
			fprintf(stderr, "Error: You must provide a client id using -i option if you are using the -c option.\n");
			goto errout;
		}

		g_mqtt_sub.id = mqtt_generate_client_id(MQTT_CLIENT_SUB_COMMAND_NAME);
		if (g_mqtt_sub.id == NULL) {
			fprintf(stderr, "Error: fail to set a client id.\n");
			goto errout;
		}
	}

	/* set information to subscribe */
	memset(&g_mqtt_sub.mqtt_message, 0, sizeof(mqtt_msg_t));
	g_mqtt_sub.mqtt_message.topic = strdup(g_mqtt_sub.topic);
	g_mqtt_sub.mqtt_message.qos = g_mqtt_sub.qos;

#if defined(CONFIG_NETUTILS_MQTT_SECURITY)
	/* set tls parameters */

	/* set ca_cert */
	g_mqtt_sub.tls.ca_cert = mqtt_get_ca_certificate();	/* the pointer of ca_cert buffer */
	g_mqtt_sub.tls.ca_cert_len = mqtt_get_ca_certificate_size();	/* the length of ca_cert buffer  */

	/* set cert */
	g_mqtt_sub.tls.cert = mqtt_get_client_certificate();	/* the pointer of cert buffer */
	g_mqtt_sub.tls.cert_len = mqtt_get_client_certificate_size();	/* the length of cert buffer */

	/* set key */
	g_mqtt_sub.tls.key = mqtt_get_client_key();	/* the pointer of key buffer */
	g_mqtt_sub.tls.key_len = mqtt_get_client_key_size();	/* the length of key buffer */
#endif

	/* set mqtt config */
	memset(&g_mqtt_sub.mqtt_client_config, 0, sizeof(mqtt_client_config_t));
	g_mqtt_sub.mqtt_client_config.client_id = strdup(g_mqtt_sub.id);
	g_mqtt_sub.mqtt_client_config.user_name = strdup(g_mqtt_sub.username);
	g_mqtt_sub.mqtt_client_config.password = strdup(g_mqtt_sub.password);
	g_mqtt_sub.mqtt_client_config.clean_session = g_mqtt_sub.clean_session;
	g_mqtt_sub.mqtt_client_config.protocol_version = g_mqtt_sub.protocol_version;
	g_mqtt_sub.mqtt_client_config.debug = g_mqtt_sub.debug;
	g_mqtt_sub.mqtt_client_config.on_connect = sensor_mqtt_sub_connect_callback;
	g_mqtt_sub.mqtt_client_config.on_disconnect = sensor_mqtt_sub_disconnect_callback;
	extern void sensor_message_callback(void * client, mqtt_msg_t * msg);
	g_mqtt_sub.mqtt_client_config.on_message = sensor_message_callback;
	g_mqtt_sub.mqtt_client_config.on_subscribe = sensor_mqtt_subscribe_callback;
	g_mqtt_sub.mqtt_client_config.on_unsubscribe = sensor_mqtt_unsubscribe_callback;
	g_mqtt_sub.mqtt_client_config.user_data = &g_mqtt_sub.mqtt_message;

#if defined(CONFIG_NETUTILS_MQTT_SECURITY)
	if (g_mqtt_sub.port == MQTT_SECURITY_BROKER_PORT) {
		g_mqtt_sub.mqtt_client_config.tls = &g_mqtt_sub.tls;
	} else {
		g_mqtt_sub.mqtt_client_config.tls = NULL;
	}
#else
	g_mqtt_sub.mqtt_client_config.tls = NULL;
#endif

	return 0;

errout:
	return -1;
}

static void clean_client_config_sub(void)
{
	/* g_subscribe_msg */
	if (g_mqtt_sub.mqtt_message.topic) {
		free(g_mqtt_sub.mqtt_message.topic);
		g_mqtt_sub.mqtt_message.topic = NULL;
	}

	if (g_mqtt_sub.mqtt_message.payload) {
		free(g_mqtt_sub.mqtt_message.payload);
		g_mqtt_sub.mqtt_message.payload = NULL;
	}

	/* g_mqtt_client_config */
	if (g_mqtt_sub.mqtt_client_config.client_id) {
		free(g_mqtt_sub.mqtt_client_config.client_id);
		g_mqtt_sub.mqtt_client_config.client_id = NULL;
	}

	if (g_mqtt_sub.mqtt_client_config.user_name) {
		free(g_mqtt_sub.mqtt_client_config.user_name);
		g_mqtt_sub.mqtt_client_config.user_name = NULL;
	}

	if (g_mqtt_sub.mqtt_client_config.password) {
		free(g_mqtt_sub.mqtt_client_config.password);
		g_mqtt_sub.mqtt_client_config.password = NULL;
	}
}

static int check_option_on_sub_client_running(void)
{
	int result = CHECK_OPTION_RESULT_CHECKED_ERROR;

	if (!g_mqtt_sub.mqtt_client_handle) {
		if (g_mqtt_sub.stop || g_mqtt_sub.sub_topic || g_mqtt_sub.unsub_topic) {
			printf("Error: MQTT client is not running.\n");
			goto done;
		}

		result = CHECK_OPTION_RESULT_NOTHING_TODO;
		goto done;
	}

	if (g_mqtt_sub.mqtt_client_handle && !(g_mqtt_sub.stop || g_mqtt_sub.sub_topic || g_mqtt_sub.unsub_topic)) {
		printf("Error: MQTT client is running. You have to stop the mqtt subscriber with --stop\n");
		printf("      in order to start new mqtt subscriber.\n");
		goto done;
	}

	if (g_mqtt_sub.stop) {
		int disconnect_try_count = 20;

		MQTT_DEBUG_PRINT(g_mqtt_sub.mqtt_client_handle, "disconnect from a MQTT broker before stopping MQTT client.\n");
		while ((mqtt_disconnect(g_mqtt_sub.mqtt_client_handle) != 0) && disconnect_try_count) {
			disconnect_try_count--;
			usleep(500 * 1000);
		}
		if (disconnect_try_count == 0) {
			fprintf(stderr, "Error: mqtt_disconnect() failed.\n");
			goto done;
		}

		MQTT_DEBUG_PRINT(g_mqtt_sub.mqtt_client_handle, "deinitialize MQTT client context.\n");
		if (mqtt_deinit_client(g_mqtt_sub.mqtt_client_handle) != 0) {
			fprintf(stderr, "Error: mqtt_deinit_client() failed.\n");
			goto done;
		}

		g_mqtt_sub.mqtt_client_handle = NULL;
		clean_client_config_sub();
	} else {
		if (g_mqtt_sub.sub_topic) {
			MQTT_DEBUG_PRINT(g_mqtt_sub.mqtt_client_handle, "subscribe the specified topic.\n");
			if (mqtt_subscribe(g_mqtt_sub.mqtt_client_handle, g_mqtt_sub.sub_topic, g_mqtt_sub.qos) != 0) {
				fprintf(stderr, "Error: mqtt_subscribe() failed.\n");
				goto done;
			}
		}

		if (g_mqtt_sub.unsub_topic) {
			MQTT_DEBUG_PRINT(g_mqtt_sub.mqtt_client_handle, "unsubscribe the specified topic.\n");
			if (mqtt_unsubscribe(g_mqtt_sub.mqtt_client_handle, g_mqtt_sub.unsub_topic) != 0) {
				fprintf(stderr, "Error: mqtt_unsubscribe() failed.\n");
				goto done;
			}
		}
	}

	/* result is success */
	result = CHECK_OPTION_RESULT_CHECKED_OK;

done:
	return result;
}

int set_mqtt_port(mqtt_client_type_e type, int port)
{
	if (port < 1 || port > 65535) {
		fprintf(stderr, "Error: Invalid port given: %d\n", port);
		return 1;
	}

	if (SENSOR_MQTT_PUB == type) {
		g_mqtt_pub.port = port;
	} else if (SENSOR_MQTT_SUB == type) {
		g_mqtt_sub.port = port;
	}

	return 0;
}

void set_mqtt_debug_mode(mqtt_client_type_e type, bool flage)
{
	if (SENSOR_MQTT_PUB == type) {
		g_mqtt_pub.debug = flage;
	} else if (SENSOR_MQTT_SUB == type) {
		g_mqtt_sub.debug = flage;
	}
}

int set_mqtt_host_addr(mqtt_client_type_e type, char *addr)
{
	if (!addr) {
		fprintf(stderr, "Error: null addr given\n");
		return -1;
	}

	if (SENSOR_MQTT_PUB == type) {
		g_mqtt_pub.host_addr = strdup(addr);
	} else if (SENSOR_MQTT_SUB == type) {
		g_mqtt_sub.host_addr = strdup(addr);
	}

	return 0;
}

int set_mqtt_keepalive(mqtt_client_type_e type, int keepalive)
{
	if (keepalive > 65535) {
		fprintf(stderr, "Error: Invalid keepalive given: %d\n", keepalive);
	}

	if (SENSOR_MQTT_PUB == type) {
		g_mqtt_pub.keepalive = keepalive;
	} else if (SENSOR_MQTT_SUB == type) {
		g_mqtt_sub.keepalive = keepalive;
	}

	return 0;
}

int set_mqtt_topic(mqtt_client_type_e type, char *topic)
{
	if (!topic) {
		fprintf(stderr, "Error: null topic given\n");
		return -1;
	}

	if (SENSOR_MQTT_PUB == type) {
		g_mqtt_pub.topic = strdup(topic);
	} else if (SENSOR_MQTT_SUB == type) {
		g_mqtt_sub.topic = strdup(topic);
	}

	return 0;
}

int set_mqtt_pub_payload(void *payload, int lenth)
{
	if (!payload) {
		fprintf(stderr, "Error: null payload given\n");
		return -1;
	}

	g_mqtt_pub.message = malloc(lenth);
	if (g_mqtt_pub.message) {
		memcpy(g_mqtt_pub.message, payload, lenth);
		g_mqtt_pub.messageLen = lenth;
		return 0;
	}
	return -1;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int mqtt_client_pub_task(void)
{
	int result = -1;
	struct timespec abstime;
	const int wait_sec = 30;
	const int wait_nsec = 0;
	int timeout = 0;

	/* initialize a semaphore for signaling */
	sem_init(&g_mqtt_pub.mqtt_sem, 0, 0);
	sem_setprotocol(&g_mqtt_pub.mqtt_sem, SEM_PRIO_NONE);

	/* set the seed of a new sequence of random values */
	mqtt_set_srand();

	/* make mqtt publisher client config */
	if (make_client_config_pub() != 0) {
		goto done;
	}

	/* create mqtt publisher client */
	if (g_mqtt_pub.debug) {
		printf("initialize MQTT client context.\n");
	}
	g_mqtt_pub.mqtt_client_handle = mqtt_init_client(&g_mqtt_pub.mqtt_client_config);
	if (g_mqtt_pub.mqtt_client_handle == NULL) {
		fprintf(stderr, "Error: mqtt_init_client() failed.\n");
		goto done;
	}

	/* connect to a mqtt broker */
	if (g_mqtt_pub.debug) {
		printf("connect to a MQTT broker (%s).\n", g_mqtt_pub.host_addr);
	}

	if (mqtt_connect(g_mqtt_pub.mqtt_client_handle, g_mqtt_pub.host_addr, g_mqtt_pub.port, g_mqtt_pub.keepalive) != 0) {
		fprintf(stderr, "Error: mqtt_connect() failed.\n");
		goto done;
	}

	/* wait for finishing mqtt publish */
	(void)clock_gettime(CLOCK_REALTIME, &abstime);
	abstime.tv_sec += wait_sec;
	abstime.tv_nsec += wait_nsec;
	if (abstime.tv_nsec >= NSEC_PER_SEC) {
		abstime.tv_sec++;
		abstime.tv_nsec -= NSEC_PER_SEC;
	}
	while (sem_timedwait(&g_mqtt_pub.mqtt_sem, &abstime) != 0) {
		int err = get_errno();
		ASSERT(err == EINTR || err == ETIMEDOUT);

		if (err == ETIMEDOUT) {
			timeout = 1;
			break;
		}
	}
	if (timeout) {
		fprintf(stderr, "Error: mqtt_pub timeout!\n");
		goto done;
	}

	/* result is success */
	result = 0;

done:
	sem_destroy(&g_mqtt_pub.mqtt_sem);
	return result;
}

void mqtt_destroy_client_pub_task(void)
{
	if (g_mqtt_pub.mqtt_client_handle) {
		MQTT_DEBUG_PRINT(g_mqtt_pub.mqtt_client_handle, "disconnect MQTT server.\n");
		mqtt_disconnect(g_mqtt_pub.mqtt_client_handle);
		MQTT_DEBUG_PRINT(g_mqtt_pub.mqtt_client_handle, "deinitialize MQTT client context.\n");
		if (mqtt_deinit_client(g_mqtt_pub.mqtt_client_handle) != 0) {
			fprintf(stderr, "Error: mqtt_deinit_client() failed.\n");
		}

		g_mqtt_pub.mqtt_client_handle = NULL;
	}

	deinit_mqtt_variables(SENSOR_MQTT_PUB);
	clean_client_config_pub();
}

int mqtt_client_sub_task(void)
{
	int result = -1;
	int ret = 0;

	/* set  the seed of a new sequence of random values */
	mqtt_set_srand();

	/* check and do options when a client is running */
	ret = check_option_on_sub_client_running();
	if (ret == CHECK_OPTION_RESULT_CHECKED_OK) {
		result = 0;
		goto done;
	} else if (ret == CHECK_OPTION_RESULT_CHECKED_ERROR) {
		goto done;
	}

	/* make mqtt subscriber client config */
	if (make_client_config_sub() != 0) {
		goto done;
	}

	/* create mqtt subscriber client */
	if (g_mqtt_sub.debug) {
		printf("initialize MQTT client context.\n");
	}
	g_mqtt_sub.mqtt_client_handle = mqtt_init_client(&g_mqtt_sub.mqtt_client_config);
	if (g_mqtt_sub.mqtt_client_handle == NULL) {
		fprintf(stderr, "Error: mqtt_init_client() failed.\n");
		clean_client_config_sub();
		goto done;
	}

	/* connect to a mqtt broker */
	if (g_mqtt_sub.debug) {
		printf("connect to a MQTT broker (%s : %d).\n", g_mqtt_sub.host_addr, g_mqtt_sub.port);
	}
	if (mqtt_connect(g_mqtt_sub.mqtt_client_handle, g_mqtt_sub.host_addr, g_mqtt_sub.port, g_mqtt_sub.keepalive) != 0) {
		fprintf(stderr, "Error: mqtt_connect() failed.\n");

		if (mqtt_deinit_client(g_mqtt_sub.mqtt_client_handle) != 0) {
			fprintf(stderr, "Error: mqtt_deinit_client() failed.\n");
		} else {
			g_mqtt_sub.mqtt_client_handle = NULL;
		}
		clean_client_config_sub();
		goto done;
	}

	if (g_mqtt_sub.debug) {
		printf("MQTT subscriber has started successfully.\n");
	}

	/* result is success */
	result = 0;
done:
	return result;
}

void mqtt_destroy_client_sub_task(void)
{
	if (g_mqtt_sub.mqtt_client_handle) {
		MQTT_DEBUG_PRINT(g_mqtt_sub.mqtt_client_handle, "disconnect MQTT server.\n");
		mqtt_disconnect(g_mqtt_sub.mqtt_client_handle);
		MQTT_DEBUG_PRINT(g_mqtt_sub.mqtt_client_handle, "deinitialize MQTT client context.\n");
		if (mqtt_deinit_client(g_mqtt_sub.mqtt_client_handle) != 0) {
			fprintf(stderr, "Error: mqtt_deinit_client() failed.\n");
		}

		g_mqtt_sub.mqtt_client_handle = NULL;
	}

	deinit_mqtt_variables(SENSOR_MQTT_SUB);
	clean_client_config_sub();
}
