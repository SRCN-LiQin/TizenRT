

#include <network/sal/sal_nb_api.h>
#include <network/sal/sal_at_api.h>
#include <network/sal/sal_bc95.h>



int los_nb_init(const int8_t* host, const int8_t* port, sec_param_s* psk)
{
    int ret;
    int timecnt = 0;
    //if(port == NULL)
        //return -1;
    /*when used nb with agenttiny*/
    /*the following para is replaced by call nb_int()*/
    at_config at_user_conf = {
        .name = AT_MODU_NAME,
        .usart_port = AT_USART_PORT,
        .buardrate = AT_BUARDRATE,
        .linkid_num = AT_MAX_LINK_NUM,
        .user_buf_len = MAX_AT_USERDATA_LEN,
        .cmd_begin = AT_CMD_BEGIN,
        .line_end = AT_LINE_END,
        .mux_mode = 1, //support multi connection mode
        .timeout = 3*AT_CMD_TIMEOUT,   //  ms , 10s seems not long enough
    };
    
    ret = at.init(&at_user_conf);
    if (ret) {
		printf("call at.init,ret is %d\n",ret);
		return -1;
    }

    /*nb_reboot();
    sleep(2);
    if(psk != NULL)//encryption v1.9
    {
        if(psk->setpsk)
            nb_send_psk(psk->pskid, psk->psk);
        else
            nb_set_no_encrypt();
    }*/


    ret = nb_hw_detect();
    if (ret) {
		printf("call nb_hw_detect,ret is %d\n",ret);
		return -1;
    }

    sleep(3);

    //nb_get_auto_connect();
    //nb_connect(NULL, NULL, NULL);

	while(timecnt < 10)
	{
		ret = nb_get_netstat();
		sleep(1);
		nb_check_csq();
		sleep(1);
		if(ret != AT_FAILED)
		{
			break;
		}
		timecnt++;
	}
	if(ret != AT_FAILED)
	{
		nb_query_ip();
	}
	//ret = nb_set_cdpserver((char *)host, (char *)port);
    return ret;
}

int los_nb_report(const char* buf, int len)
{
    if(buf == NULL || len <= 0)
        return -1;
    return nb_send_payload(buf, len);  //this is for hw cloud
}

int los_nb_notify(char* featurestr,int cmdlen, oob_callback callback, oob_cmd_match cmd_match)
{
    if(featurestr == NULL ||cmdlen <= 0 || cmdlen >= OOB_CMD_LEN - 1)
        return -1;
    return at.oob_register(featurestr,cmdlen, callback,cmd_match);
}

int los_nb_deinit(void)
{
    nb_reboot();
	at.deinit();
	return 0;
}

