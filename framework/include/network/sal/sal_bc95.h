

#ifndef __NB_NEUL_BC95_H__
#define __NB_NEUL_BC95_H__


#include <network/sal/sal_at_main.h>

#define AT_MODU_NAME            "nb_neul95"

#define AT_NB_LINE_END 			"\r\n"
#define AT_NB_reboot    		"AT+NRB\r"
#define AT_NB_hw_detect    		"AT+CFUN?\r"
#define AT_NB_get_auto_connect  "AT+NCONFIG?\r"
#define AT_CMD_PREFIX           "\r\n+NNMI:"
#define AT_DATAF_PREFIX         "+NSONMI:"
#define CGATT                   "AT+CGATT?\r"
#define CGATT_ATTACH            "AT+CGATT=1\r"
#define CGATT_DEATTACH          "AT+CGATT=0\r"

#define AT_LINE_END 		    "\r\n"
#define AT_CMD_BEGIN		    "\r\n"



#define AT_USART_PORT       3
#define AT_BUARDRATE        9600
#define AT_CMD_TIMEOUT      10000    //ms
#define AT_MAX_LINK_NUM     4

#define NB_STAT_LOCALPORT   56


#define MAX_SOCK_NUM        5
#define UDP_PROTO           17




#if defined STM32F103xE
#define MAX_AT_USERDATA_LEN (1024*2)
#else
#define MAX_AT_USERDATA_LEN (1024*5)
#endif

#define AT_MAX_PAYLOADLEN     512

#define IP_LEN 16
typedef struct _socket_info_t
{
    int socket;
    short localport;
    char localip[IP_LEN];
    short remoteport;
    char remoteip[IP_LEN];
    bool used_flag;
}socket_info;//struct to save socket info

int str_to_hex(const char *bufin, int len, char *bufout);
int32_t nb_set_cdpserver(char* host, char* port);
int32_t nb_hw_detect(void);
int32_t nb_get_netstat(void);
int32_t nb_check_cloud_access(void);
int nb_query_ip(void);
int32_t nb_send_payload(const char* buf, int len);
int32_t nb_check_csq(void);
int32_t nb_query_IMEI(void);
int32_t nb_send_psk(char* pskid, char* psk);
int32_t nb_set_no_encrypt(void);
int32_t nb_reboot(void);
int32_t nb_recv_timeout(int32_t id , uint8_t  *buf, uint32_t len,char* ipaddr,int* port, int32_t timeout);
int nb_cmd_match(void* arg, char* buf, int buflen);
void nb_step(void);
void nb_reattach(void);
#endif
