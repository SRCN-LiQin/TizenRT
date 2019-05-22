
#ifndef __SAL_AT_HAL_H__
#define __SAL_AT_HAL_H__


#include <network/sal/sal_at_main.h>

void at_transmit(uint8_t * cmd, int32_t len,int flag);
int32_t at_usart_init(void);
void at_usart_deinit(void);
int read_resp(uint8_t *buf, recv_buff* recv_buf);
void write_at_task_msg(at_msg_type_e type);


#endif