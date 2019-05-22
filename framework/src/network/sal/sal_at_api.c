

#include <network/sal/sal_at_api.h>

static at_adaptor_api  *gp_at_adaptor_api = NULL;

int32_t at_api_register(at_adaptor_api *api)
{
    if (NULL == gp_at_adaptor_api)
    {
        gp_at_adaptor_api = api;
        if (gp_at_adaptor_api && gp_at_adaptor_api->init)
        {
            return gp_at_adaptor_api->init();
        }
    }

    return 0;
}

int32_t at_api_bind(const char *host, const char *port, int proto)
{
    int32_t ret = -1;

    if (gp_at_adaptor_api && gp_at_adaptor_api->bind)
    {
        ret = gp_at_adaptor_api->bind((int8_t *)host, (int8_t *)port, proto);
    }
    return ret;
}

int32_t at_api_connect(const char *host, const char *port, int proto)
{
    int32_t ret = -1;

    if (gp_at_adaptor_api && gp_at_adaptor_api->connect)
    {
        ret = gp_at_adaptor_api->connect((int8_t *)host, (int8_t *)port, proto);
    }
    return ret;
}

int32_t at_api_send(int32_t id , const unsigned char *buf, uint32_t len)
{
    if (gp_at_adaptor_api && gp_at_adaptor_api->send)
    {
        return gp_at_adaptor_api->send(id, buf, len);
    }
    return -1;
}

int32_t at_api_sendto(int32_t id , uint8_t  *buf, uint32_t len,char* ipaddr,int port)
{
    if (gp_at_adaptor_api && gp_at_adaptor_api->sendto)
    {
        return gp_at_adaptor_api->sendto(id, buf, len,ipaddr, port);
    }
    return -1;
}

int32_t at_api_recv(int32_t id, unsigned char *buf, size_t len)
{
    if (gp_at_adaptor_api && gp_at_adaptor_api->recv)
    {
        return gp_at_adaptor_api->recv(id, buf, len);
    }
    return -1;
}

int32_t at_api_recv_timeout(int32_t id , uint8_t  *buf, uint32_t len,char* ipaddr,int* port, int32_t timeout)
{
    if (gp_at_adaptor_api && gp_at_adaptor_api->recv_timeout)
    {
        return gp_at_adaptor_api->recv_timeout(id , buf, len, ipaddr, port, timeout);
    }
    return -1;
}

int32_t at_api_close(int32_t fd)
{
    if (gp_at_adaptor_api && gp_at_adaptor_api->close)
    {
        return gp_at_adaptor_api->close(fd);
    }
    return -1;
}

