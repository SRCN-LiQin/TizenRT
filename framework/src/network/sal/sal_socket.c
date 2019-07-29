
#include "network/sal/sal_socket.h"
#include "network/sal/sal_osdep.h"
#include "network/sal/sal_at_api.h"

#include <sys/socket.h>
#include <tinyara/config.h>

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

typedef struct {
	int at_fd;
	char host[16];
	int  port;
	uint8_t loopback;
	uint8_t used;
	uint8_t proto;
	uint8_t shutdown;
} vsocket_mgr_t;
#define VSOCKET_NUM 100
static vsocket_mgr_t g_vsocket[VSOCKET_NUM];

static sem_t g_atselect;
static int g_atseminit = 0;
static int g_localhostsend = 0;

#define SAL_SETID(id) (CONFIG_NFILE_DESCRIPTORS + (id))
#define SAL_GETID(id) ((id) - CONFIG_NFILE_DESCRIPTORS)
int atiny_sal_socket(int protocol)
{
	if (g_atseminit == 0) {
		printf("sem_init->\n");
		sem_init(&g_atselect, 1, 0);
		g_atseminit = 1;
	}

	int id = 0;
	for(; id < VSOCKET_NUM; ++id) {
		if(!g_vsocket[id].used) {
			memset(&g_vsocket[id], 0, sizeof(vsocket_mgr_t));
			g_vsocket[id].at_fd = -1;
			g_vsocket[id].used = 1;
			g_vsocket[id].proto = (uint8_t)protocol;
			SOCKET_LOG("new s %d\n", id);
			return SAL_SETID(id);
		}
	}
}

int atiny_sal_bind(int s, const struct sockaddr *name, socklen_t namelen)
{
	s = SAL_GETID(s);
	if (!name || !g_vsocket[s].used) {
		return -1;
	}

	g_vsocket[s].port = name->sa_family;
	memcpy(g_vsocket[s].host, name->sa_data, sizeof(name->sa_data));
	if (name->sa_data[2] == 127) {
	SOCKET_LOG("s %d loopback\n", s);
		g_vsocket[s].loopback = 1;
	}
	return 0;
}

int atiny_sal_accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	//s is listening , accept addr, create new socket to communicate with addr

	//for current requirement, it's for socket pair of localhost, we may create two sockets for it.
	s = SAL_GETID(s);
	int ns = atiny_sal_socket(g_vsocket[s].proto);
	g_vsocket[ns] = g_vsocket[s];
	
	return SAL_SETID(ns);
}

//how : SHUT_RD, SHUT_RD, SHUT_RD
int atiny_sal_shutdown(int s, int how)
{
	s = SAL_GETID(s);
	SOCKET_LOG("shut s %d\n", s);

	g_vsocket[s].shutdown = how;
	return 0;
}

int atiny_sal_closesocket(int s)
{
	s = SAL_GETID(s);
	SOCKET_LOG("close s %d\n", s);

	if (!g_vsocket[s].used) {
		return 0;
	}

	g_vsocket[s].used = 0;
	if (g_vsocket[s].loopback) {
		return 0;
	}

	at_api_close(g_vsocket[s].at_fd);
	return 0;
}

int atiny_sal_connect(int s, const struct sockaddr *name, socklen_t namelen)
{
	s = SAL_GETID(s);

	char port[6] = {0,};
	char address[16] = {0,};

	if (!name || !g_vsocket[s].used) {
		return -1;
	}
	
	if (name->sa_data[2] == 127 || g_vsocket[s].loopback) { //localhost, no handle.
		g_vsocket[s].loopback = 1;
		SOCKET_LOG("s %d loopback\n", s);
		return 0;
	}

	//snprintf(port, 6, "%d%d", name->sa_data[0], name->sa_data[1]);
	snprintf(port, 6, "%s", "1883");
	snprintf(address, 16, "%d.%d.%d.%d", name->sa_data[2], name->sa_data[3], name->sa_data[4], name->sa_data[5]);

	SOCKET_LOG("atiny_sal_connect : port %s , remote %s\n", port, address);

	//snprintf(port, 6, "%s", "1883");

	int at_fd = at_api_connect(address, port, g_vsocket[s].proto);
	if (at_fd < 0) {
		SOCKET_LOG("atiny_sal_connect : port %s , remote %s FAILED!\n", port, address);
		return -1;
	}
	SOCKET_LOG("connect : remote %s OK, fd %d, s %d!\n", address, at_fd, s);
	g_vsocket[s].at_fd = at_fd;

	return 0;
}

int atiny_sal_getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
	//currently mqtt doesn't rely on this to work, do nothing 
	// socket proto/remote ip/port -> name
	
	return 0;
}

int atiny_sal_listen(int s, int backlog)
{
	//listen to the socket event - TODO
	return 0;
}

ssize_t atiny_sal_recv(int s, void *mem, size_t len, int flags)
{
	s = SAL_GETID(s);

	int ret = -1;
	
	if (g_vsocket[s].loopback) { //localhost, no handle.
		if (g_localhostsend) {
			SOCKET_LOG("lb rcv\n");
			g_localhostsend = 0;
			*((uint8_t*)mem) = 0;
			return 1;
		}
		return 0;
	}

	ret = at_api_recv(g_vsocket[s].at_fd, mem, len);
	SOCKET_LOG("atiny_sal_recv, s %d , recv size %d\n", s, ret);
	return (ssize_t)ret;
}

ssize_t atiny_sal_recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	s = SAL_GETID(s);

	//no requirement of this api for mqtt client. TODO
	return atiny_sal_recv(s, mem, len, flags);
}

void atiny_sal_notify_rcv(int atqid)
{
	if (g_atseminit == 0) {
		SOCKET_LOG("noti sem_init\n");
		sem_init(&g_atselect, 1, 0);
		g_atseminit = 1;
	}

	SOCKET_LOG("at rcv, post g_atselect\n");
	sem_post(&g_atselect);
}


int atiny_sal_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
	SOCKET_LOG("select@ \n");

	if (sem_wait(&g_atselect)) {
		SOCKET_LOG("sem_wait failed, errno %d\n",get_errno());
		return 0;
	}
	SOCKET_LOG("g_atselect take\n");
	return 1;
}

ssize_t atiny_sal_send(int s, const void *data, size_t size, int flags)
{
	s = SAL_GETID(s);

	int ret = -1;
	if (g_vsocket[s].loopback) { //localhost, no handle.
		g_localhostsend = 1;
		return 1;
	}

	ret = at_api_send(g_vsocket[s].at_fd, data, size);
	SOCKET_LOG("atiny_sal_send, s %d, at_fd %d , send size %d\n",s, g_vsocket[s].at_fd, ret);

	return ret;
}

ssize_t atiny_sal_sendto(int s, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
	s = SAL_GETID(s);

	int ret = -1;
	char port[6] = {0,};
	char address[16] = {0,};

	//snprintf(port, 2, "%d%d", to->sa_data[0], to->sa_data[1]);
	snprintf(port, 6, "%s", "1883");
	snprintf(address, 16, "%d.%d.%d.%d", to->sa_data[2], to->sa_data[3], to->sa_data[4], to->sa_data[5]);

	ret = at_api_sendto(g_vsocket[s].at_fd, data, size, address, port);

    return ret;
}

//====================================================================================//
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

