

#include <network/sal/sal_osdep.h>

#include <sys/types.h>
#include <sys/time.h>
#include <tinyara/clock.h>
#include <tinyara/semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>

#define ATINY_CNT_MAX_WAITTIME 0xFFFFFFFF
#define LOG_BUF_SIZE (256)

#ifndef OK
#define OK 0
#endif

#ifndef ERR
#define ERR -1
#endif

typedef struct {
    bool valid;
    uint32_t mq_item_size;
    mqd_t mqd_fd_send;
    mqd_t mqd_fd_recv;
}mq_info_t;
#define MAX_QUEUE_INFO 10
static mq_info_t sal_mq_info[MAX_QUEUE_INFO];

static uint64_t osKernelGetTickCount (void)
{
    uint64_t ticks = clock_systimer();

    return ticks;
}

uint64_t atiny_gettime_ms(void)
{
    return TICK2MSEC(osKernelGetTickCount());
}

void *atiny_malloc(size_t size)
{
    return malloc(size);
}

void atiny_free(void *ptr)
{
    (void)free(ptr);
    ptr = NULL;
}

int atiny_snprintf(char *buf, unsigned int size, const char *format, ...)
{
    int     ret;
    va_list args;

    va_start(args, format);
    ret = vsnprintf(buf, size, format, args);
    va_end(args);

    return ret;
}

int atiny_printf(const char *format, ...)
{
    int ret;
    char str_buf[LOG_BUF_SIZE] = {0};
    va_list list;

    memset(str_buf, 0, LOG_BUF_SIZE);
    va_start(list, format);
    ret = vsnprintf(str_buf, LOG_BUF_SIZE, format, list);
    va_end(list);

    printf("%s", str_buf);

    return ret;
}

char *atiny_strdup(const char *ch)
{
    char *copy;
    size_t length;

    if(NULL == ch)
        return NULL;

    length = strlen(ch);
    copy = (char *)atiny_malloc(length + 1);
    if(NULL == copy)
        return NULL;
    strncpy(copy, ch, length);
    copy[length] = '\0';

    return copy;
}

void atiny_delay(uint32_t second)
{
    (void)sleep(second);
}

void *atiny_mutex_create(void)
{
	sem_t *ns = atiny_malloc(sizeof(*ns));
	sem_init(ns, 0, 1);
	return ns;
}

void atiny_mutex_destroy(void *mutex)
{

    if (mutex == NULL)
    {
        return;
    }

    (void)sem_destroy((sem_t*)mutex);
}

void atiny_mutex_lock(void *mutex)
{
    if (mutex == NULL)
    {
        return;
    }

	(void)sem_wait((sem_t*)mutex);
}

void atiny_mutex_unlock(void *mutex)
{
    if (mutex == NULL)
    {
        return;
    }

    (void)sem_post((sem_t*)mutex);
}

static int sal_calc_abs_time(struct timespec *abs_time, int delayticks)
{
    int ret; 
    time_t sec; 
    uint32_t nsec;
    int offset = TICK2MSEC(delayticks);
    sec = offset / MSEC_PER_SEC;
    nsec = (offset - MSEC_PER_SEC * sec) * NSEC_PER_MSEC;

    ret = clock_gettime(CLOCK_REALTIME, abs_time);
    if (ret != 0) { 
        return ERROR;
    }    
    abs_time->tv_sec += sec; 
    abs_time->tv_nsec += nsec;
    if (abs_time->tv_nsec >= NSEC_PER_SEC) {
        abs_time->tv_sec++;
        abs_time->tv_nsec -= NSEC_PER_SEC;
    }    
    return OK;
}

uint32_t LOS_QueueReadCopy(uint32_t uwQueueID, void *pBufferAddr, uint32_t puwBufferSize, uint32_t uwTimeOut)
{
	mq_info_t *queue_info = NULL;
	int prio = 0;
	int32_t ret;
	struct timespec abstime;

	if (!pBufferAddr || uwQueueID >= MAX_QUEUE_INFO) {
		return -1;
	}
	
    queue_info = &sal_mq_info[uwQueueID];
	if (queue_info->valid && queue_info->mqd_fd_recv != (mqd_t)ERROR) {
		if (uwTimeOut == 0xFFFFFFFF) {
			ret = mq_receive(queue_info->mqd_fd_recv, (char*)pBufferAddr, puwBufferSize, &prio);
			if (ret == ERROR) {
				return -1;
			}
		} else {
            sal_calc_abs_time(&abstime, uwTimeOut); 
            ret = mq_timedreceive(queue_info->mqd_fd_recv, (char*)pBufferAddr, puwBufferSize, &prio, &abstime);
			if (ret == ERROR) {
				return -1;
			}
		}
	} else {
		return -1;
	}
	
	return 0;
}

uint32_t LOS_QueueWriteCopy( uint32_t uwQueueID, void *pBufferAddr, uint32_t uwBufferSize, uint32_t uwTimeOut)
{
	int32_t ret;
	mq_info_t *queue_info = NULL;
	struct timespec abstime;

	if (!pBufferAddr || uwQueueID >= MAX_QUEUE_INFO) {
		return -1;
	}
	queue_info = &sal_mq_info[uwQueueID];
	if (queue_info->valid && queue_info->mqd_fd_send != (mqd_t)ERROR) {
		if (uwTimeOut == 0xFFFFFFFF) {
			ret = mq_send(queue_info->mqd_fd_send, (char*)pBufferAddr, uwBufferSize, 0);
			if (ret == ERROR) {
				return -1;
			}
		}
		else {
			sal_calc_abs_time(&abstime, uwTimeOut);
            ret = mq_timedsend(queue_info->mqd_fd_send, (char*)pBufferAddr, uwBufferSize, 0, &abstime);
			if (ret == ERROR) {
				return -1;
			}
		}
	} else {
		return -1;
	}
	
	return 0;
}

uint32_t LOS_QueueCreate(char *pcQueueName, uint16_t usLen, uint32_t *puwQueueID, uint32_t uwFlags, uint16_t usMaxMsgSize)
{
    int i;
	bool flag = false;
	uint32_t mq_id = 0;
    
	for (i = 0; i < MAX_QUEUE_INFO; i++) {
		if (!sal_mq_info[i].valid) {
			flag = true;
			mq_id = i;
			break;
		}
	}
	if (!flag || !puwQueueID) {
		return -1;
	}

	struct mq_attr attr;
	attr.mq_maxmsg = usLen;
	attr.mq_msgsize = usMaxMsgSize;
	attr.mq_flags = 0;

	/*Invalid param */
	sal_mq_info[mq_id].mqd_fd_send = mq_open(pcQueueName, O_RDWR | O_CREAT, 0666, &attr);
	if (sal_mq_info[mq_id].mqd_fd_send == (mqd_t)ERROR) {
		return -1;
	}
	sal_mq_info[mq_id].mqd_fd_recv = mq_open(pcQueueName, O_RDWR | O_CREAT, 0666, &attr);
	if (sal_mq_info[mq_id].mqd_fd_recv == (mqd_t)ERROR) {
        mq_close(sal_mq_info[mq_id].mqd_fd_send);
		return -1;
	}
	sal_mq_info[mq_id].valid = true;
	sal_mq_info[mq_id].mq_item_size = usMaxMsgSize;
	*puwQueueID = mq_id;
	return 0;
}

uint32_t LOS_QueueDelete(uint32_t uwQueueID)
{
	mq_info_t *queue_info = NULL;

	if (uwQueueID >= MAX_QUEUE_INFO) {
		return -1;
	}
	
	queue_info = &sal_mq_info[uwQueueID];
	if (queue_info->mqd_fd_send != (mqd_t)ERROR) {
		mq_close(queue_info->mqd_fd_send);
		queue_info->mqd_fd_send = NULL;
	}
	if (queue_info->mqd_fd_recv != (mqd_t)ERROR) {
		mq_close(queue_info->mqd_fd_recv);
		queue_info->mqd_fd_recv = NULL;
	}
	queue_info->mq_item_size = 0;
	queue_info->valid = false;

	return 0;
}
