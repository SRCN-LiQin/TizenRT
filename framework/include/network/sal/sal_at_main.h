

#ifndef __SAL_AT_MAIN_H__
#define __SAL_AT_MAIN_H__
#include <stdbool.h>


#include <sys/types.h>
#include <tinyara/semaphore.h>
#include <network/sal/sal_osdep.h>
#include <network/sal/sal_socket.h>
#include <network/sal/sal_at_api.h>


#ifdef __cplusplus
extern "C" {
#endif

#define UINT32 uint32_t
#define UINT8 uint8_t
#define int32_t int

#define AT_INTO
#define AT_DEBUG

/* MACRO DEFINE */
#ifdef AT_INTO
#define AT_LOG(fmt, arg...)  do {printf("[%lu][%s:%d][I]"fmt"\n", at_get_time(), __func__, __LINE__, ##arg);} while(0)
#else
static inline void __do_nothing(const char *fmt, ...) { (void)fmt; }
#define AT_LOG(fmt, arg...)  __do_nothing(fmt, ##arg)
#endif

#ifdef AT_DEBUG
#define AT_LOG_DEBUG(fmt, arg...)  do {printf("[%lu][%s:%d][D]"fmt"\n", at_get_time(), __func__, __LINE__, ##arg);} while(0)
#else
#define AT_LOG_DEBUG(fmt, arg...)
#endif


#define AT_OK    		 0
#define AT_FAILED 		-1
#define AT_TIMEOUT      -2

#define AT_LINK_UNUSE		0
#define AT_LINK_INUSE 		1

#define AT_MUXMODE_SINGLE   0
#define AT_MUXMODE_MULTI    1

#define SAL_RCV_TASK_PRIORITY    (125)
#define LOS_WAIT_FOREVER 0xFFFF

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

#ifndef array_size
#define array_size(a) (sizeof(a)/sizeof(*(a)))
#endif



/* VARIABLE DECLEAR */

/* TYPE REDEFINE */
typedef int32_t (*oob_callback)(void* arg, int8_t* buf, int32_t buflen);
typedef int32_t (*oob_cmd_match)(const char *buf, char* featurestr,int len);

#define MAXIPLEN  40
typedef struct {
    uint32_t len;
    uint8_t *addr;
    char ipaddr[MAXIPLEN];
    int port;
}QUEUE_BUFF;

enum
{
    AT_USART_RX,
    AT_TASK_QUIT,
    AT_SENT_DONE
};
typedef uint32_t at_msg_type_e;

typedef struct {
    uint32_t ori;
    uint32_t end;
    at_msg_type_e msg_type;
}recv_buff;

typedef struct {
	UINT32 fd;		//convert between socket_fd and linkid
	UINT32 qid;    // queue id
	UINT32 usable;

    UINT8 remote_ip[16];
    UINT32 remote_port;
}at_link;

typedef struct
{
    const char **suffix;
    int suffix_num;
    int match_idx;
    char *resp_buf;
    uint32_t *resp_len;
}at_cmd_info_s;

typedef struct _listner{
	struct _listner * next;
    at_cmd_info_s cmd_info;
    uint32_t expire_time;
    int32_t (*handle_data)(const int8_t *data, uint32_t len);
}at_listener;

#define OOB_MAX_NUM 5
#define OOB_CMD_LEN  40
#define AT_DATA_LEN 1024
typedef struct oob_s{
	char featurestr[OOB_CMD_LEN];
	int len;
    int runflag;
    oob_cmd_match cmd_match;
	oob_callback callback;
	void* arg;
}oob_t;

typedef struct at_oob_s{
	oob_t oob[OOB_MAX_NUM];
	int32_t oob_num;
} at_oob_t;

typedef struct __config{
	char * name;
	uint32_t usart_port;
	uint32_t buardrate;
	uint32_t linkid_num;
	uint32_t user_buf_len; /* malloc 3 block memory for intener use, len * 3 */
	char * cmd_begin;
	char * line_end;
	uint32_t  mux_mode;
	uint32_t timeout;  //command respond timeout
}at_config;

typedef struct at_task{

	uint32_t  tsk_hdl;
	sem_t recv_sem;
    uint32_t rid;
    bool     rid_flag;
	sem_t resp_sem;
	sem_t cmd_mux;
    sem_t trx_mux;
    bool     trx_mux_flag;
	uint8_t  *recv_buf;
	uint8_t  *cmdresp;/*AT cmd response,default 512 bytes*/
	uint8_t  *userdata;  /*data form servers,default 512 bytes*/
    uint8_t  *saveddata;
	uint32_t  mux_mode;
	at_link  *linkid;
	at_listener * head;
	uint32_t timeout; //command respond timeout

	void (*step_callback)();

	int32_t (*init)(at_config *config);
	int32_t (*cmd)(int8_t * cmd, int32_t len, const char * suffix, char * resp_buf, int* resp_len);
	int32_t (*write)(int8_t * cmd, int8_t * suffix, int8_t * buf, int32_t len);
	/* get unused linkid, use in multi connection mode*/
	int32_t (*get_id)();
	/* register uset msg process to the listener list */
	int32_t (*oob_register)(char *featurestr, int cmdlen, oob_callback callback, oob_cmd_match cmd_match);
	void (*deinit)();
    int32_t (*cmd_multi_suffix)(const int8_t *cmd, int  len, at_cmd_info_s *cmd_info);
} at_task;

void at_set_config(at_config *config);
at_config *at_get_config(void);


void* at_malloc(size_t size);
void at_free(void* ptr);
int chartoint(const char* port);
extern int at_update_result_send(void);
int32_t at_cmd_in_callback(const int8_t *cmd, int32_t len, int32_t (*handle_data)(const int8_t *data, uint32_t len), uint32_t timeout);
uint32_t at_get_time(void);
void at_reg_step_callback(at_task *at_tsk, void (*step_callback)(void));


extern at_task at;

extern uint16_t at_fota_timer;

#ifdef __cplusplus
}
#endif

#endif
