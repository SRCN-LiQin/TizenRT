

/**@defgroup nbiot
 * @ingroup nbiot
 */

#ifndef __SAL_NB_IOT_H__
#define __SAL_NB_IOT_H__

#include <network/sal/sal_at_main.h>

typedef struct sec_param{
char* psk;
char* pskid;
uint8_t setpsk;
}sec_param_s;

extern at_task at;

/*
Func Name: los_nb_init

@par Description
    This API is used to init nb module and connect to cloud.
@param[in]  host  cloud ip
@param[in]  port  cloud port
@param[in]  psk   if not null,the security param
@par Return value
*  0:on success
*  negative value: on failure
*/
int los_nb_init(const int8_t* host, const int8_t* port, sec_param_s* psk);
/*
Func Name: los_nb_report

@par Description
    This API is used for nb module to report data to cloud.
@param[in] buf point to data to be reported
@param[in] buflen data length
@par Return value
*  0:on success
*  negative value: on failure
*/
int los_nb_report(const char* buf, int buflen);
/*
Func Name: los_nb_notify

@par Description
    This API is used to regist callback when receive the cmd from cloud.
@param[in] featurestr feature string that in cmd
@param[in] cmdlen length of feature string
@param[in] callback callback of device
@par Return value
*  0:on success
*  negative value: on failure
*/

int los_nb_notify(char* featurestr,int cmdlen, oob_callback callback, oob_cmd_match cmd_match);
/*
Func Name: los_nb_deinit

@par Description
    This API is used to deinit the nb module.
@param[in] NULL
@par Return value
*  0:on success
*  negative value: on failure
*/

int los_nb_deinit(void);
#endif
