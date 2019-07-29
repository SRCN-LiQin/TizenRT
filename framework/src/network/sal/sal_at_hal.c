



#include "network/sal/sal_at_hal.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>

extern at_task at;

int at_usart;
/*
static USART_TypeDef *s_pUSART = USART2;
static uint32_t s_uwIRQn = USART2_IRQn;
*/
//uint32_t list_mux;
uint8_t buff_full = 0;
static uint32_t g_disscard_cnt = 0;

uint32_t wi       = 0;
uint32_t pre_ri   = 0;/*only save cur msg start*/
uint32_t ri       = 0;

#define UART_BYTE_SIZE 8
#define UART_DEVPATH "/dev/ttyS1"
//as reference, MAX_AT_USERDATA_LEN is 5k
#define UART_MAX_PAYLOAD 1100
char uart_tmp_buf[UART_MAX_PAYLOAD];

#if 0
static void at_usart_adapter(uint32_t port)
{
    switch ( port )
    {
    case 1 :
        s_pUSART = USART1;
        s_uwIRQn = USART1_IRQn;
        break;
    case 2 :
        s_pUSART = USART2;
        s_uwIRQn = USART2_IRQn;
        break;
    case 3 :
        s_pUSART = USART3;
        s_uwIRQn = USART3_IRQn;
        break;
    default:
        break;
    }
}


void at_irq_handler(void)
{
    recv_buff recv_buf;
    at_config *at_user_conf = at_get_config();

    if(__HAL_UART_GET_FLAG(&at_usart, UART_FLAG_RXNE) != RESET)
    {
        at.recv_buf[wi++] = (uint8_t)(at_usart.Instance->DR & 0x00FF);
        if(wi == ri)buff_full = 1;
        if (wi >= at_user_conf->user_buf_len)wi = 0;
    }
    else if (__HAL_UART_GET_FLAG(&at_usart, UART_FLAG_IDLE) != RESET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&at_usart);
        /*
        Ring Buffer ri------------------------>wi

         __________________________________________________
         |      msg0           |  msg1        |   msg2    |           
         ri(pre_ri0)        pre_ri1         pre_ri2     wi(pre_ri3)
         __________________________________________________ 

         read_resp ---->ri= pre_ri1----------->---------->ri=wi=pre_ri3(end)  
        */
        recv_buf.ori = pre_ri;
        recv_buf.end = wi;

        pre_ri = recv_buf.end;
        recv_buf.msg_type = AT_USART_RX;

        if(LOS_QueueWriteCopy(at.rid, &recv_buf, sizeof(recv_buff), 0) != LOS_OK)
        {
            g_disscard_cnt++;
        }
    }
}
#endif

int at_read_task(int argc, char *argv[])
{
	int ret;
	int retry;
	int tmp_len;
	recv_buff recv_buf;
	at_config *at_user_conf = at_get_config();
	
	printf("at_read_task start\n");
	/*
	Ring Buffer ri------------------------>wi
	
	 __________________________________________________
	 |		msg0		   |  msg1		  |   msg2	  | 		  
	 ri(pre_ri0)		pre_ri1 		pre_ri2 	wi(pre_ri3)
	 __________________________________________________ 
	
	 read_resp ---->ri= pre_ri1----------->---------->ri=wi=pre_ri3(end)  
	*/
	retry = 0;

	while (1) {
		ret = read(at_usart, uart_tmp_buf, UART_MAX_PAYLOAD);
		if (ret < 0) {
			printf("read uart data failed, ret %d\n",ret);
			retry++;
			if (retry > 5) break;
			
			continue; //try to read for next time
		}

		if (ret == 1) {  //there is len 1 
			printf("read length 1, ignored.\n");
			continue;
		}

		retry = 0; //clear retry count
		if ((wi + ret) <= at_user_conf->user_buf_len) //not reach the end
		{
			memcpy(&at.recv_buf[wi], uart_tmp_buf, ret);
			wi += ret;
		}
		else
		{
			tmp_len = at_user_conf->user_buf_len - wi;
			if (tmp_len) {
				memcpy(&at.recv_buf[wi], uart_tmp_buf, tmp_len);
			}
			memcpy(&at.recv_buf[0], uart_tmp_buf + tmp_len, ret - tmp_len);
			
			wi = ret - tmp_len;
			if(wi >= ri) { //overlap
				buff_full = 1;
			}
		}

        recv_buf.ori = pre_ri;
        recv_buf.end = wi;

        pre_ri = recv_buf.end;
        recv_buf.msg_type = AT_USART_RX;

		AT_LOG("receive message length %d\n", ret);
        if(LOS_QueueWriteCopy(at.rid, &recv_buf, sizeof(recv_buff), 0) != 0)
        {
            g_disscard_cnt++;
        }
	}

	return 0;
}

int32_t at_usart_init(void)
{
/*
    UART_HandleTypeDef *usart = &at_usart;
    at_config *at_user_conf = at_get_config();

    at_usart_adapter(at_user_conf->usart_port);

    usart->Instance = s_pUSART;
    usart->Init.BaudRate = at_user_conf->buardrate;

    usart->Init.WordLength = UART_WORDLENGTH_8B;
    usart->Init.StopBits = UART_STOPBITS_1;
    usart->Init.Parity = UART_PARITY_NONE;
    usart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    usart->Init.Mode = UART_MODE_RX | UART_MODE_TX;
    if(HAL_UART_Init(usart) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    __HAL_UART_CLEAR_FLAG(usart, UART_FLAG_TC);
    LOS_HwiCreate(s_uwIRQn, 0, 0, at_irq_handler, 0);
    __HAL_UART_ENABLE_IT(usart, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(usart, UART_IT_RXNE);
*/

	int ret;
	int fd;
	int pid;
	char path[32];
	struct termios tio;
	at_config *at_user_conf = at_get_config();
	int byteinfo[4] = { CS5, CS6, CS7, CS8 };

	snprintf(path, 32, "%s", UART_DEVPATH);
	fd = open(path, O_RDWR, 0666);
	if (fd < 0) {
		printf("open uart failed, return %d, errno %d\n",fd, get_errno());
		return AT_FAILED;
	}

	ret = tcgetattr(fd, &tio);
	if (ret) {
		printf("get uart attr failed, ret %d\n", ret);
		close(fd);
		return AT_FAILED;
	}

	//baud rate
	tio.c_speed = at_user_conf->buardrate;
	//byte length
	tio.c_cflag &= ~CSIZE;
	tio.c_cflag |= byteinfo[UART_BYTE_SIZE - 5];
	//parity none
	tio.c_cflag &= ~PARENB;
	tio.c_cflag &= ~PARODD;
	//stopbit 1
	tio.c_cflag &= ~CSTOPB;
	//flow control none
	tio.c_cflag &= ~(CRTS_IFLOW | CCTS_OFLOW);
	ret = tcsetattr(fd, TCSANOW, &tio);
	if (ret < 0) {
		printf("set uart attr failed\n");
		close(fd);
		return AT_FAILED;
	}

	pid = task_create("at_hal_read_task", SAL_RCV_TASK_PRIORITY, 0x1000, at_read_task, NULL);
	if (pid < 0) {
		printf("create task at_hal_read_task failed\n");
		close(fd);
		return AT_FAILED;
	}

	at_usart = fd;
    return AT_OK;
}

void at_usart_deinit(void)
{
 /*   UART_HandleTypeDef *husart = &at_usart;
    __HAL_UART_DISABLE(husart);
    __HAL_UART_DISABLE_IT(husart, UART_IT_IDLE);
    __HAL_UART_DISABLE_IT(husart, UART_IT_RXNE);*/

	close(at_usart);

}

void at_transmit(uint8_t *cmd, int32_t len, int flag)
{
	if (flag) {
		AT_LOG("cmd %s\n",cmd);
	} else {
		AT_LOG("data len %d\n", len);
	}

    at_config *at_user_conf = at_get_config();
    
    char *line_end = at_user_conf->line_end;
    //(void)HAL_UART_Transmit(&at_usart, (uint8_t *)cmd, len, 0xffff);
    write(at_usart, cmd, len);
    if(flag == 1)
    {
        //(void)HAL_UART_Transmit(&at_usart, (uint8_t *)line_end, strlen(at_user_conf->line_end), 0xffff);
        write(at_usart, line_end, strlen(at_user_conf->line_end));
    }
}

int read_resp(uint8_t *buf, recv_buff* recv_buf)
{
    uint32_t len = 0;

    uint32_t tmp_len = 0;

    at_config *at_user_conf = at_get_config();

    if (NULL == buf)
    {
        return -1;
    }
    if(1 == buff_full)
    {
        AT_LOG("buf maybe full,buff_full is %d",buff_full);
    }
    //AT_LOG("wi is %d, ri is %d,pre_ri is %d, end(%d),ori(%d),buff_full is %d",
    //    wi,ri,pre_ri,recv_buf->end,recv_buf->ori,buff_full);
    
    if (recv_buf->end == recv_buf->ori)
    {
        len = 0;
        return 0;
    }

    if (recv_buf->end > recv_buf->ori)
    {
        len = recv_buf->end - recv_buf->ori;
        memcpy(buf, &at.recv_buf[recv_buf->ori], len);
    }
    else
    {
        tmp_len = at_user_conf->user_buf_len - recv_buf->ori;
        memcpy(buf, &at.recv_buf[recv_buf->ori], tmp_len);
        memcpy(buf + tmp_len, at.recv_buf, recv_buf->end);
        len = recv_buf->end + tmp_len;
    }

    ri = recv_buf->end;
    
    return len;
}

void write_at_task_msg(at_msg_type_e type)
{
    recv_buff recv_buf;
    int ret;

    memset(&recv_buf,  0, sizeof(recv_buf));
    recv_buf.msg_type = type;

    ret = LOS_QueueWriteCopy(at.rid, &recv_buf, sizeof(recv_buff), 0);
    if(ret != 0)
    {
        g_disscard_cnt++;
    }
}

