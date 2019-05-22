
#include "network/sal/sal_socket.h"
#include "network/sal/sal_osdep.h"
#include "network/sal/sal_at_api.h"




#define SOCKET_DEBUG

#if defined(SOCKET_DEBUG)
#define SOCKET_LOG(fmt, ...) \
    do \
    { \
        (void)atiny_printf("[SOCKET][%s:%d] " fmt "\r\n", \
        __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while(0)
#else
#define SOCKET_LOG(fmt, ...) ((void)0)
#endif


typedef struct
{
    int fd;
} atiny_net_context;

void *atiny_net_bind(const char *host, const char *port, int proto)
{
    atiny_net_context *ctx;
    ctx = atiny_malloc(sizeof(atiny_net_context));
    if (NULL == ctx)
    {
    	SOCKET_LOG("malloc failed for socket context");
    	return NULL;
    }

    ctx->fd = at_api_bind(host, port, proto);
    if (ctx->fd < 0)
    {
    	SOCKET_LOG("unkown host or port");
    	atiny_free(ctx);
    	ctx = NULL;
    }
	return ctx;
}

int atiny_net_accept( void *bind_ctx, void *client_ctx, void *client_ip, size_t buf_size, size_t *ip_len )
{
/*本函数从s的等待连接队列中抽取第一个连接，创建一个与s同类的新的套接口并返回句柄*/

    ((atiny_net_context*)client_ctx)->fd = ((atiny_net_context*)bind_ctx)->fd;

    return 0;
}

void *atiny_net_connect(const char *host, const char *port, int proto)
{
    atiny_net_context *ctx = NULL;

    //if (NULL == host || NULL == port ||
    if (NULL == port ||
            (proto != ATINY_PROTO_UDP && proto != ATINY_PROTO_TCP))
    {
        SOCKET_LOG("ilegal incoming parameters,(%p,%p,%d)",host,port,proto);
        return NULL;
    }

    ctx = atiny_malloc(sizeof(atiny_net_context));
    if (NULL == ctx)
    {
        SOCKET_LOG("malloc failed for socket context");
        return NULL;
    }

    ctx->fd = at_api_connect(host, port, proto);
    if (ctx->fd < 0)
    {
        SOCKET_LOG("unkown host or port");
        atiny_free(ctx);
        ctx = NULL;
    }
    return ctx;
}

int atiny_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int ret = -1;
    int fd = ((atiny_net_context *)ctx)->fd;

    ret = at_api_recv(fd, buf, len);

    return ret;
}

int atiny_net_recv_timeout(void *ctx, unsigned char *buf, size_t len,
                           uint32_t timeout)
{
    int ret = -1;
    int fd = ((atiny_net_context *)ctx)->fd;

    ret = at_api_recv_timeout(fd, buf, len, NULL,NULL,timeout);

    return ret;
}

int atiny_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int ret = -1;
    int fd = ((atiny_net_context *)ctx)->fd;

    if (fd < 0)
    {
        SOCKET_LOG("ilegal socket(%d)", fd);
        return -1;
    }

    ret = at_api_send(fd, buf, len);

    return ret;
}

int atiny_net_sendto(void *ctx, const unsigned char *buf, size_t len, char* ipaddr, int port)
{
    int ret = -1;
    int fd = ((atiny_net_context *)ctx)->fd;

    if (fd < 0)
    {
        SOCKET_LOG("ilegal socket(%d)", fd);
        return -1;
    }

    ret = at_api_sendto(fd, buf, len, ipaddr, port);

    return ret;
}

void atiny_net_close(void *ctx)
{
    int fd = ((atiny_net_context *)ctx)->fd;

    if (fd >= 0)
    {
        at_api_close(fd);
    }

    atiny_free(ctx);
}


int atiny_net_send_timeout(void *ctx, const unsigned char *buf, size_t len,
                          uint32_t timeout)
{
    int fd;
    fd = ((atiny_net_context *)ctx)->fd;
    return at_api_send(fd , buf, (uint32_t)len);

}

