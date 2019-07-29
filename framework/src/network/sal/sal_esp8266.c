

#include <network/sal/sal_esp8266.h>

extern at_task at;

QUEUE_BUFF g_rcv_buff[AT_MAX_LINK_NUM];
uint8_t g_peer_closed[AT_MAX_LINK_NUM];

int esp8266_cmd(int8_t *cmd, int32_t len, const char *suffix, char *resp_buf, int* resp_len)
{
	return at.cmd(cmd, len, suffix, resp_buf, resp_len);
}

int32_t esp8266_echo_off(void)
{
    return esp8266_cmd((int8_t *)AT_CMD_ECHO_OFF, strlen(AT_CMD_ECHO_OFF), "OK\r\n", NULL, NULL);
}

int32_t esp8266_reset(void)
{
    return esp8266_cmd((int8_t *)AT_CMD_RST, strlen(AT_CMD_RST), "ready\r\n", NULL, NULL);
}

int32_t esp8266_choose_net_mode(enum_net_mode m)
{
    char cmd[64] = {0};
    snprintf(cmd, 64, "%s=%d", AT_CMD_CWMODE, (int)m);
    return esp8266_cmd((int8_t *)cmd, strlen(cmd), "OK\r\n", NULL, NULL);
}

int32_t esp8266_set_mux_mode(int32_t m)
{
    char cmd[64] = {0};
    snprintf(cmd, 64, "%s=%d", AT_CMD_MUX, (int)m);
    return esp8266_cmd((int8_t *)cmd, strlen(cmd), "OK\r\n", NULL, NULL);
}
int32_t esp8266_joinap(char *pssid, char *ppasswd)
{
    char cmd[64] = {0};
    snprintf(cmd, 64, "%s=\"%s\",\"%s\"", AT_CMD_JOINAP, pssid, ppasswd);
    return esp8266_cmd((int8_t *)cmd, strlen(cmd), "OK\r\n", NULL, NULL);
}

int32_t esp8266_connect(const int8_t *host, const int8_t *port, int32_t proto)
{
    int32_t ret = AT_FAILED;
    int32_t id = 0;
    char cmd[64] = {0};

    AT_LOG("host:%s, port:%s", host, port);

    if (AT_MUXMODE_SINGLE == at.mux_mode)
    {
        snprintf(cmd, 64, "%s=\"%s\",\"%s\",%s", AT_CMD_CONN, proto == ATINY_PROTO_UDP ? "UDP" : "TCP", host, port);
    }
    else
    {
        id = at.get_id();
        if (id < 0 || id >= AT_MAX_LINK_NUM)
        {
            AT_LOG("no vailed linkid for use(id = %ld)", id);
            return AT_FAILED;
        }
        snprintf(cmd, 64, "%s=%ld,\"%s\",\"%s\",%s", AT_CMD_CONN, id, proto == ATINY_PROTO_UDP ? "UDP" : "TCP", host, port);
    }

    //init at_link
    memcpy(at.linkid[id].remote_ip, host, sizeof(at.linkid[id].remote_ip));
    (void)sscanf((char*)port, "%d", &at.linkid[id].remote_port);

    ret = LOS_QueueCreate("espdataQueue", 16, &at.linkid[id].qid, 0, sizeof(QUEUE_BUFF));
    if (ret != 0)
    {
        AT_LOG("init espdataQueue failed!");
        at.linkid[id].usable = AT_LINK_UNUSE;
        return AT_FAILED;
    }
    ret = esp8266_cmd((int8_t *)cmd, strlen(cmd), "OK\r\n", NULL, NULL);
    if (AT_FAILED == ret)
    {
        AT_LOG("at.cmd return failed!");
       (void)LOS_QueueDelete(at.linkid[id].qid);
        at.linkid[id].usable = AT_LINK_UNUSE;
        return AT_FAILED;
    }
    return id;
}

int32_t esp8266_send(int32_t id , const uint8_t  *buf, uint32_t len)
{
    int32_t ret = AT_FAILED;
    char cmd[64] = {0};
    if (AT_MUXMODE_SINGLE == at.mux_mode)
    {
        //snprintf(cmd, 64, "%s=%lu,\"%s\",%d", AT_CMD_SEND, len,at.linkid[0].remote_ip, at.linkid[0].remote_port);
    }
    else
    {
        //snprintf(cmd, 64, "%s=%ld,%lu,\"%s\",%d", AT_CMD_SEND, id, len,at.linkid[id].remote_ip, at.linkid[id].remote_port);
    }
	snprintf(cmd, 64, "%s=%ld,%lu", AT_CMD_SEND, id, len);

    //   at.cmd(cmd, strlen(cmd), ">", NULL);
    ret = at.write((int8_t *)cmd, (int8_t *)"SEND OK\r\n", (int8_t *)buf, len);

    return ret;
}

int32_t esp8266_recv_timeout(int32_t id, uint8_t *buf, uint32_t len, char * host, int * port, int32_t timeout)
{
    uint32_t qlen = sizeof(QUEUE_BUFF);
    uint32_t rxlen = 0;

	if (g_rcv_buff[id].len) {
		if (len < g_rcv_buff[id].len) {
			rxlen = len;
			memcpy(buf, g_rcv_buff[id].addr, rxlen);

			g_rcv_buff[id].len -= len;
			memcpy(g_rcv_buff[id].addr, (g_rcv_buff[id].addr + rxlen), g_rcv_buff[id].len);
		} else {
			rxlen = g_rcv_buff[id].len;
			memcpy(buf, g_rcv_buff[id].addr, rxlen);

			g_rcv_buff[id].len = 0;
			at_free(g_rcv_buff[id].addr);
		}
		
		return rxlen;
	}

    QUEUE_BUFF  qbuf = {0, NULL};
    int ret = LOS_QueueReadCopy(at.linkid[id].qid, (void *)&qbuf, (UINT32 *)&qlen, timeout);
//    AT_LOG("ret = %x, len = %ld, id = %ld, timeout = %d", ret, qbuf.len, id, timeout);
    if (ret != 0)
    {
        return AT_FAILED;
    }

    if (qbuf.len)
    {
		if (len < qbuf.len) {
			rxlen = len;
			memcpy(buf, qbuf.addr, rxlen);

			g_rcv_buff[id] = qbuf;
			g_rcv_buff[id].len -= len;
			memcpy(qbuf.addr, (qbuf.addr + rxlen), g_rcv_buff[id].len);
		} else {
			rxlen = qbuf.len;
			memcpy(buf, qbuf.addr, rxlen);
			at_free(qbuf.addr);
		}
    }
    return rxlen;
}

int32_t esp8266_recv(int32_t id, uint8_t *buf, uint32_t len)
{
    return esp8266_recv_timeout(id, buf, len, NULL, NULL, LOS_WAIT_FOREVER);
}

int32_t esp8266_close(int32_t id)
{
    char cmd[64] = {0};

    if(at.linkid[id].usable == AT_LINK_UNUSE)
        return 0;

    if (AT_MUXMODE_SINGLE == at.mux_mode)
    {
        snprintf(cmd, 64, "%s", AT_CMD_CLOSE);
    }
    else
    {
        uint32_t qlen = sizeof(QUEUE_BUFF);
        QUEUE_BUFF  qbuf = {0, NULL};
        while(0 == LOS_QueueReadCopy(at.linkid[id].qid, (void *)&qbuf, (UINT32 *)&qlen, 10))
        {
            if (qbuf.len)
            {
                at_free(qbuf.addr);
                memset(&qbuf, 0, sizeof(QUEUE_BUFF)); // don't use qlen
            }
        }
        (void)LOS_QueueDelete(at.linkid[id].qid);
        memset(&at.linkid[id], 0, sizeof(at_link));
        snprintf(cmd, 64, "%s=%ld", AT_CMD_CLOSE, id);
    }
    
    if (g_peer_closed[id]) {
		g_peer_closed[id] = 0;
		return 0;
    }
    return esp8266_cmd((int8_t *)cmd, strlen(cmd), "OK\r\n", NULL, NULL);
}

int32_t esp8266_closed_handler(void *arg, int8_t *buf, int32_t len)
{
	char *p1;
	int linkid;
    if (NULL == buf || len <= 0)
    {
        AT_LOG("param invailed!");
        return AT_FAILED;
    }

	//"0,CLOSED"
	p1 = (char *)(buf - 2);
	linkid = *p1 - '0';
	if (linkid < AT_MAX_LINK_NUM) {
		g_peer_closed[linkid] = 1;
	}
}

int32_t esp8266_data_handler(void *arg, int8_t *buf, int32_t len)
{
    if (NULL == buf || len <= 0)
    {
        AT_LOG("param invailed!");
        return AT_FAILED;
    }
    AT_LOG("entry!");

    //process data frame ,like +IPD,linkid,len:data
    int32_t ret = -1;
    int32_t linkid = 0, data_len = 0;
    int32_t remote_port = 0;
    char * remote_ip;
    char *p1, *p2;
    QUEUE_BUFF qbuf;
    p1 = (char *)buf;

LOOP:
    if (0 == memcmp(p1, AT_DATAF_PREFIX, strlen(AT_DATAF_PREFIX)))
    {
        p2 = strstr(p1, ",");
        if (NULL == p2)
        {
            AT_LOG("got data prefix invailed!");
            goto END;
        }

        if (AT_MUXMODE_MULTI == at.mux_mode)
        {
            linkid = 0;
            for (p2++; *p2 <= '9' && *p2 >= '0'; p2++)
            {
                linkid = linkid * 10 + (*p2 - '0');
            }
        }

        data_len = 0;
        for (p2++; *p2 <= '9' && *p2 >= '0' ; p2++)
        {
            data_len = (data_len * 10 + (*p2 - '0'));
        }

        //remote ip:str
        remote_ip = (char *)at.linkid[linkid].remote_ip;
        for (p2++; *p2 != ',' ; p2++)
        {
            *(remote_ip++) = *p2;
        }

        //remote port
        remote_port = 0;
        for (p2++; *p2 <= '9' && *p2 >= '0' ; p2++)
        {
            remote_port = (remote_port * 10 + (*p2 - '0'));
        }
        at.linkid[linkid].remote_port = remote_port;

        p2++; //over ':'

        qbuf.addr = at_malloc(data_len);
        if (NULL == qbuf.addr)
        {
            AT_LOG("malloc for qbuf failed!");
            goto END;
        }

        qbuf.len = data_len;
        memcpy(qbuf.addr, p2, data_len);

        if (0 != (ret = LOS_QueueWriteCopy(at.linkid[linkid].qid, &qbuf, sizeof(QUEUE_BUFF), 0)))
        {
            AT_LOG("LOS_QueueWriteCopy  failed! ret = %lx", ret);
            at_free(qbuf.addr);
            goto END;
        }
        p1  = (p2 + data_len);

        if ((p1 - (char*)buf) < len)
            goto LOOP;
    }
END:
	atiny_sal_notify_rcv(at.linkid[linkid].qid);
    return 0;
}

int8_t esp8266_get_localip(int8_t *ip, int8_t *gw, int8_t *mask)   /*get local IP*/
{
    char resp[512] = {0};
    int len = 512;
    esp8266_cmd((int8_t *)AT_CMD_CHECK_IP, strlen((char *)AT_CMD_CHECK_IP), "OK", resp, &len);

    AT_LOG("resp:%s", resp);
    char *p1, *p2;
    p1 = strstr(resp, "ip");
    if (ip && p1)
    {
        p1 = strstr(p1, "\"");
        p2 = strstr(p1 + 1, "\"");
        memcpy(ip, p1 + 1, p2 - p1 - 1);
    }

    p1 = strstr(resp, "gateway");
    if (gw && p1)
    {
        p1 = strstr(p1, "\"");
        p2 = strstr(p1 + 1, "\"");
        memcpy(gw, p1 + 1, p2 - p1 - 1);
    }

    p1 = strstr(resp, "netmask");
    if (mask && p1)
    {
        p1 = strstr(p1, "\"");
        p2 = strstr(p1 + 1, "\"");
        memcpy(mask, p1 + 1, p2 - p1 - 1);
    }

    printf("get ip :%s", resp);
    return AT_OK;
}

int8_t esp8266_get_localmac(int8_t *mac) /*get local mac*/
{
    char resp[512] = {0};
    char *p1, *p2;
    int len = 512;

    esp8266_cmd((int8_t *)AT_CMD_CHECK_MAC, strlen((char *)AT_CMD_CHECK_MAC), "OK", resp, &len);
    AT_LOG("resp:%s", resp);

    p1 = strstr(resp, ":");
    if (mac && p1)
    {
        p1 = strstr(p1, "\"");
        p2 = strstr(p1 + 1, "\"");
        memcpy(mac, p1 + 1, p2 - p1 - 1);
    }


    printf("get ip :%s", resp);
    return AT_OK;
}

int32_t esp8266_bind(const int8_t *host, const int8_t *port, int32_t proto)
{
	int ret = AT_FAILED;
	int port_i = 0;
	char cmd[64] = {0};

	(void)sscanf((char *)port, "%d", &port_i);
	AT_LOG("get port = %d\r\n", port_i);

	if (at.mux_mode != AT_MUXMODE_MULTI)
	{
		AT_LOG("Only support in multi mode!\r\n");
		return -1;
	}

	int id = at.get_id();
    ret = LOS_QueueCreate("espdataQueue", 16, &at.linkid[id].qid, 0, sizeof(QUEUE_BUFF));
    if (ret != 0)
    {
        AT_LOG("init espdataQueue failed!");
        at.linkid[id].usable = AT_LINK_UNUSE;
        return AT_FAILED;
    }

	snprintf(cmd, 64, "%s=%d,\"%s\",\"0.0.0.0\",0,%d,0", AT_CMD_CONN, id, proto == ATINY_PROTO_UDP ? "UDP" : "TCP", port_i);

	esp8266_cmd((int8_t *)cmd, strlen(cmd), "OK\r\n", NULL, NULL);
	return id;
}
int32_t esp8266_recv_cb(int32_t id)
{
    return AT_FAILED;
}

int32_t esp8266_deinit(void)
{
    int id = 0;

    if(NULL != at.linkid)
    {
        for(id = 0; id < AT_MAX_LINK_NUM; id++)
        {
            if(AT_LINK_INUSE == at.linkid[id].usable)
            {
                if(AT_OK != esp8266_close(id))
                {
                    AT_LOG("esp8266_close(%d) failed", id);
                }
            }
        }
    }

    at.deinit();
    return AT_OK;
}

int32_t esp8266_show_dinfo(int32_t s)
{
    char cmd[64] = {0};
    snprintf(cmd, 64, "%s=%ld", AT_CMD_SHOW_DINFO, s);
    return esp8266_cmd((int8_t *)cmd, strlen(cmd), "OK\r\n", NULL, NULL);
}

int32_t esp8266_cmd_match(const char *buf, char* featurestr,int len)
{
	char *p = strstr(buf, featurestr);
	if (p == NULL) {
		return -1;
	} else {
		return (int32_t)(p - buf);
	}

    //return memcmp(buf,featurestr,len);
}

int32_t esp8266_init()
{
    at_config at_user_conf =
    {
        .name = AT_MODU_NAME,
        .usart_port = AT_USART_PORT, //actually not used
        .buardrate = AT_BUARDRATE,
        .linkid_num = AT_MAX_LINK_NUM,
        .user_buf_len = MAX_AT_USERDATA_LEN,
        .cmd_begin = AT_CMD_BEGIN,
        .line_end = AT_LINE_END,
        .mux_mode = 1, //support multi connection mode
        .timeout = AT_CMD_TIMEOUT,   //  ms

    };
    at.init(&at_user_conf);
    //at.add_listener((int8_t*)AT_DATAF_PREFIX, NULL, esp8266_data_handler);
    at.oob_register(AT_DATAF_PREFIX, strlen(AT_DATAF_PREFIX), esp8266_data_handler, esp8266_cmd_match);
    at.oob_register("CLOSED", strlen("CLOSED"), esp8266_closed_handler, esp8266_cmd_match);

    esp8266_reset();
    esp8266_echo_off();
    esp8266_show_dinfo(1);

    esp8266_choose_net_mode(STA);
    while(AT_FAILED == esp8266_joinap(WIFI_SSID, WIFI_PASSWD))
    {
        AT_LOG("connect ap failed, repeat...");
    };
    esp8266_set_mux_mode(at.mux_mode);

    static int8_t ip[32];
    static int8_t gw[32];
    static int8_t mac[32];
    esp8266_get_localip(ip, gw, NULL);
    esp8266_get_localmac(mac);
    AT_LOG_DEBUG("get ip:%s, gw:%s mac:%s", ip, gw, mac);
    return AT_OK;
}



at_adaptor_api esp8266_interface =
{

    .init = esp8266_init,
    .get_localmac = esp8266_get_localmac, /*get local MAC*/
    .get_localip = esp8266_get_localip,/*get local IP*/
    /*build TCP or UDP connection*/
    .connect = esp8266_connect,
    .bind = esp8266_bind,
    .send = esp8266_send,

    .recv_timeout = esp8266_recv_timeout,
    .recv = esp8266_recv,

    .close = esp8266_close,/*close connection*/
    .recv_cb = esp8266_recv_cb,/* operation for events, not implements yet */

    .deinit = esp8266_deinit,
};

