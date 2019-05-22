
/**@defgroup atiny_osdep Agenttiny OS Depends
 * @ingroup agent
 */

#ifndef _SAL_OSDEP_H_
#define _SAL_OSDEP_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/**
 *@ingroup atiny_adapter
 *@brief get time since the system was boot.
 *
 *@par Description:
 *This API is used to retrieves the number of milliseconds that have elapsed since the system was boot.
 *@attention none.
 *
 *@param none.
 *
 *@retval #uint64_t     the number of milliseconds.
 *@par Dependency: none.
 *@see atiny_usleep
 */
uint64_t atiny_gettime_ms(void);

/**
 *@ingroup atiny_adapter
 *@brief sleep thread itself.
 *
 *@par Description:
 *This API is used to sleep thread itself, it could be implemented in the user file.
 *@attention none.
 *
 *@param usec           [IN] the time interval for which execution is to be suspended, in microseconds.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_gettime_ms
 */
void atiny_usleep(unsigned long usec);

/**
 *@ingroup atiny_adapter
 *@brief get len bytes of entropy.
 *
 *@par Description:
 *This API is used to get len bytes of entropy, it could be implemented in the user file.
 *@attention none.
 *
 *@param output         [OUT] buffer to store the entropy
 *@param len            [IN] length of the entropy
 *
 *@retval #0            Succeed.
 *@retval #-1           Failed.
 *@par Dependency: none.
 *@see none.
 */
int atiny_random(void* output, size_t len);

/**
 *@ingroup atiny_adapter
 *@brief allocates a block of size bytes of memory
 *
 *@par Description:
 *This API is used to allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *@attention none.
 *
 *@param size           [IN] specify block size in bytes.
 *
 *@retval #pointer      pointer to the beginning of the block.
 *@par Dependency: none.
 *@see atiny_free
 */
void* atiny_malloc(size_t size);

/**
 *@ingroup atiny_adapter
 *@brief deallocate memory block.
 *
 *@par Description:
 *This API is used to deallocate memory block.
 *@attention none.
 *
 *@param size           [IN] pointer to the beginning of the block previously allocated with atiny_malloc.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_malloc
 */
void atiny_free(void* ptr);

/**
 *@ingroup atiny_adapter
 *@brief writes formatted data to string.
 *
 *@par Description:
 *This API is used to writes formatted data to string.
 *@attention none.
 *
 *@param buf            [OUT] string that holds written text.
 *@param size           [IN] maximum length of character will be written.
 *@param format         [IN] format that contains the text to be written, it can optionally contain embedded
                             format specifiers that specifies how subsequent arguments are converted for output.
 *@param ...            [IN] the variable argument list, for formatted and inserted in the resulting string
                             replacing their respective specifiers.
 *
 *@retval #int          bytes of character successfully written into string.
 *@par Dependency: none.
 *@see none.
 */
int atiny_snprintf(char* buf, unsigned int size, const char* format, ...);

/**
 *@ingroup atiny_adapter
 *@brief writes formatted data to stream.
 *
 *@par Description:
 *This API is used to writes formatted data to stream.
 *@attention none.
 *
 *@param format         [IN] string that contains the text to be written, it can optionally contain embedded
                             format specifiers that specifies how subsequent arguments are converted for output.
 *@param ...            [IN] the variable argument list, for formatted and inserted in the resulting string
                             replacing their respective specifiers.
 *
 *@retval #int          bytes of character successfully written into stream.
 *@par Dependency: none.
 *@see none.
 */
int atiny_printf(const char* format, ...);

/**
 *@ingroup atiny_strdup
 *@brief returns a pointer to a new string which is a duplicate of the string ch
 *
 *@par Description:
 *This API returns a pointer to a new string which is a duplicate of the string ch. 
   Memory for the new string is obtained with atiny_malloc, and can be freed with atiny_free
 *@attention none.
 *
 *@param format         [IN] const char pointer
 *@param ch            [IN] const char pointer points to the string to be duplicated
 *
 *@retval #char pointer          a pointer to a new string which is a duplicate of the string ch
 *@par Dependency: none.
 *@see atiny_malloc atiny_free.
 */
char *atiny_strdup(const char *ch);

/**
 *@ingroup atiny_adapter
 *@brief create a mutex.
 *
 *@par Description:
 *This API is used to create a mutex.
 *@attention none.
 *
 *@param none.
 *
 *@retval #pointer      the mutex handle.
 *@retval NULL          create mutex failed.
 *@par Dependency: none.
 *@see atiny_mutex_destroy | atiny_mutex_lock | atiny_mutex_unlock
 */
void* atiny_mutex_create(void);

/**
 *@ingroup atiny_adapter
 *@brief destroy the specified mutex object.
 *
 *@par Description:
 *This API is used to destroy the specified mutex object, it will release related resource.
 *@attention none.
 *
 *@param mutex          [IN] the specified mutex.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_mutex_create | atiny_mutex_lock | atiny_mutex_unlock
 */
void atiny_mutex_destroy(void* mutex);

/**
 *@ingroup atiny_adapter
 *@brief waits until the specified mutex is in the signaled state.
 *
 *@par Description:
 *This API is used to waits until the specified mutex is in the signaled state.
 *@attention none.
 *
 *@param mutex          [IN] the specified mutex.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_mutex_create | atiny_mutex_destroy | atiny_mutex_unlock
 */
void atiny_mutex_lock(void* mutex);

/**
 *@ingroup atiny_adapter
 *@brief releases ownership of the specified mutex object.
 *
 *@par Description:
 *This API is used to releases ownership of the specified mutex object.
 *@attention none.
 *
 *@param mutex          [IN] the specified mutex.
 *
 *@retval none.
 *@par Dependency: none.
 *@see atiny_mutex_create | atiny_mutex_destroy | atiny_mutex_lock
 */
void atiny_mutex_unlock(void* mutex);

/**
 *@ingroup atiny_adapter
 *@brief reboot.
 *
 *@par Description:
 *This API is used to reboot, it could be implemented in the user file.
 *@attention none.
 *
 *@retval none.
 *@par Dependency: none.
 *@see none.
 */
void atiny_reboot(void);

/**
 *@ingroup atiny_adapter
 *@brief task delay.
 *
 *@par Description:
 *This API is used to delay some seconds.
 *@attention none.
 *
 *@param second          [IN] the specified second.
 *
 *@retval none.
 *@par Dependency: none.
 *@see none.
 */
void atiny_delay(uint32_t second);

uint32_t LOS_QueueReadCopy(uint32_t uwQueueID, void *pBufferAddr, uint32_t puwBufferSize, uint32_t uwTimeOut);

uint32_t LOS_QueueWriteCopy(uint32_t uwQueueID, void * pBufferAddr, uint32_t uwBufferSize, uint32_t uwTimeOut);

uint32_t LOS_QueueCreate(char *pcQueueName, uint16_t usLen, uint32_t *puwQueueID, uint32_t uwFlags, uint16_t usMaxMsgSize);

uint32_t LOS_QueueDelete(uint32_t uwQueueID);

#if 0
typedef struct atiny_task_mutex_tag_s
{
    UINT32 mutex;
    UINT32 valid;
}atiny_task_mutex_s;

int atiny_task_mutex_create(atiny_task_mutex_s *mutex);
int atiny_task_mutex_delete(atiny_task_mutex_s *mutex);
int atiny_task_mutex_lock(atiny_task_mutex_s *mutex);
int atiny_task_mutex_unlock(atiny_task_mutex_s *mutex);
#endif

#if defined(__cplusplus)
}
#endif

#endif

