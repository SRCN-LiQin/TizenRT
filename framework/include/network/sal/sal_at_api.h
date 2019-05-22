
#ifndef __SAL_AT_API_H__
#define __SAL_AT_API_H__

#include <stdint.h>
#include <stdio.h>

typedef struct {
	int32_t  (*init)(void);
	int8_t (*get_localmac)(int8_t *mac);
	int8_t (*get_localip)(int8_t *ip, int8_t * gw, int8_t * mask);

    int32_t  (*bind)(const int8_t * host, const int8_t *port, int32_t proto);
	int32_t  (*connect)(const int8_t * host, const int8_t *port, int32_t proto);

	int32_t  (*send)(int32_t id , const uint8_t  *buf, uint32_t len);
    int32_t  (*sendto)(int32_t id , const uint8_t  *buf, uint32_t len,char* ipaddr,int port);
    int32_t  (*recv_timeout)(int32_t id , uint8_t  *buf, uint32_t len,char* ipaddr,int* port, int32_t timeout);
	int32_t  (*recv)(int32_t id , uint8_t *buf, uint32_t len);
    int32_t  (*recvfrom)(int32_t id , uint8_t  *buf, uint32_t len,char* ipaddr,int* port);

	int32_t  (*close)(int32_t id);
	int32_t  (*recv_cb)(int32_t id);
	int32_t  (*deinit)(void);
}at_adaptor_api;

int32_t at_api_register(at_adaptor_api *api);
int32_t at_api_bind(const char *host, const char *port, int proto);
int32_t at_api_connect(const char* host, const char* port, int proto);
int32_t at_api_send(int32_t id , const unsigned char* buf, uint32_t len);
int32_t at_api_sendto(int32_t id , uint8_t  *buf, uint32_t len,char* ipaddr,int port);
int32_t at_api_recv(int32_t id, unsigned char* buf, size_t len);
int32_t at_api_recv_timeout(int32_t id , uint8_t  *buf, uint32_t len,char* ipaddr,int* port, int32_t timeout);
int32_t at_api_close(int32_t fd);

#endif
