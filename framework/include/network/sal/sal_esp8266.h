
#ifndef __SAL_ESP8266_H__
#define __SAL_ESP8266_H__

#include "sal_at_main.h"

#define WIFI_SSID      		"GalaxyC7Pro"
#define WIFI_PASSWD    		"19850928"

#define AT_MODU_NAME    	"ESP8266"
#define AT_USART_PORT   	3
#define AT_BUARDRATE   		115200
#define AT_CMD_TIMEOUT		10000    //ms
#define AT_MAX_LINK_NUM     4

#define AT_LINE_END 		"\r\n"
#define AT_CMD_BEGIN		"\r\n"


#define MAX_AT_USERDATA_LEN (1024*5)


#define AT_CMD_RST    		"AT+RST"
#define AT_CMD_ECHO_OFF 	"ATE0"
#define AT_CMD_CWMODE  		"AT+CWMODE_CUR"
#define AT_CMD_JOINAP  		"AT+CWJAP_CUR"
#define AT_CMD_MUX 			"AT+CIPMUX"
#define AT_CMD_CONN			"AT+CIPSTART"
#define AT_CMD_SEND			"AT+CIPSEND"
#define AT_CMD_CLOSE		"AT+CIPCLOSE"
#define AT_CMD_CHECK_IP		"AT+CIPSTA_CUR?"
#define AT_CMD_CHECK_MAC	"AT+CIPSTAMAC_CUR?"
#define AT_CMD_SHOW_DINFO   "AT+CIPDINFO"

#define AT_DATAF_PREFIX      "\r\n+IPD"

typedef enum {
	STA = 1,
	AP, 
	ATA_AP,
}enum_net_mode;
#endif
