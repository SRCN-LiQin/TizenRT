/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/*
 * Copyright (c) 2016 Samsung Electronics co. ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack's wrapper
 *
 * Author: Byoungtae Cho <bt.cho@samsung.com>
 *
 */

/**
 * @file
 * Sockets BSD-Like API wrapper module
 *
 */

#include <tinyara/config.h>
#include <tinyara/cancelpt.h>

#ifdef CONFIG_NET

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <net/lwip/netdb.h>
#include <net/lwip/sockets.h>
#ifdef CONFIG_NET_IPv4
#include <net/lwip/ip4_addr.h>
#endif
#ifdef CONFIG_NET_IPv6
#include <net/lwip/ip6_addr.h>
#endif
#include "../../../framework/include/network/sal/sal_socket.h"

#define EN_LWIP 0
#define EN_SAL  1
#define SAL_LOG(fmt, arg...)  

int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
SAL_LOG("[bsdsocket] bind\n");
#if EN_LWIP
	return lwip_bind(s, name, namelen);
#endif

#if EN_SAL
	return atiny_sal_bind(s, name, namelen);
#endif
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
SAL_LOG("[bsdsocket] accept\n");
#if EN_LWIP

	/* Treat as a cancellation point */
	(void)enter_cancellation_point();
	int result = lwip_accept(s, addr, addrlen);
	leave_cancellation_point();
SAL_LOG("[bsdsocket] accept end\n");

	return result;
#endif

#if EN_SAL
	return atiny_sal_accept(s, addr, addrlen);
#endif

}

int shutdown(int s, int how)
{
	SAL_LOG("[bsdsocket] shutdown\n");
#if EN_LWIP

	return lwip_shutdown(s, how);
#endif

#if EN_SAL
	return atiny_sal_shutdown(s, how);
#endif

}

int closesocket(int s)
{
	SAL_LOG("[bsdsocket] closesocket\n");
#if EN_LWIP

	/* Treat as a cancellation point */
	(void)enter_cancellation_point();
	int result = lwip_close(s);
	leave_cancellation_point();
	SAL_LOG("[bsdsocket] closesocket end\n");
	return result;
#endif

#if EN_SAL
	return atiny_sal_closesocket(s);
#endif

}

int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
	SAL_LOG("[bsdsocket] connect\n");
#if EN_LWIP

	/* Treat as a cancellation point */
	(void)enter_cancellation_point();
	int result = lwip_connect(s, name, namelen);
	leave_cancellation_point();
	printf("[bsdsocket] connect result %d\n",result);
	return result;
#endif

#if EN_SAL
	return atiny_sal_connect(s, name, namelen);
#endif

}

int getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
	SAL_LOG("[bsdsocket] getsockname\n");
#if EN_LWIP

	return lwip_getsockname(s, name, namelen);
#endif
#if EN_SAL
	return atiny_sal_getsockname(s, name, namelen);
#endif

}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
	SAL_LOG("[bsdsocket] getpeername\n");
#if EN_LWIP

	return lwip_getpeername(s, name, namelen);
#endif
	return 0;
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	SAL_LOG("[bsdsocket] setsockopt\n");
#if EN_LWIP

	return lwip_setsockopt(s, level, optname, optval, optlen);
#endif
	return 0;
}

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
	SAL_LOG("[bsdsocket] getsockopt\n");
#if EN_LWIP

	return lwip_getsockopt(s, level, optname, optval, optlen);
#endif
	return 0;
}

int listen(int s, int backlog)
{
	SAL_LOG("[bsdsocket] listen\n");
#if EN_LWIP

	return lwip_listen(s, backlog);
#endif
#if EN_SAL
	return atiny_sal_listen(s, backlog);
#endif

}

ssize_t recv(int s, void *mem, size_t len, int flags)
{
	SAL_LOG("[bsdsocket] recv\n");
#if EN_LWIP

	/* Treat as a cancellation point */
	(void)enter_cancellation_point();
	int result = lwip_recv(s, mem, len, flags);
	leave_cancellation_point();
	printf("[bsdsocket] recv end\n");

	return result;
#endif
#if EN_SAL
	return atiny_sal_recv(s, mem, len, flags);
#endif

}

ssize_t recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	SAL_LOG("[bsdsocket] recvfrom\n");
#if EN_LWIP

	/* Treat as a cancellation point */
	(void)enter_cancellation_point();
	int result = lwip_recvfrom(s, mem, len, flags, from, fromlen);
	leave_cancellation_point();
	printf("[bsdsocket] recvfrom end\n");

	return result;
#endif

#if EN_SAL
	return atiny_sal_recvfrom(s, mem, len, flags, from, fromlen);
#endif

}

ssize_t send(int s, const void *data, size_t size, int flags)
{
	SAL_LOG("[bsdsocket] send\n");
#if EN_LWIP

	/* Treat as a cancellation point */
	(void)enter_cancellation_point();
	int result = lwip_send(s, data, size, flags);
	leave_cancellation_point();
	printf("[bsdsocket] send end\n");
	return result;
#endif

#if EN_SAL
	return atiny_sal_send(s, data, size, flags);
#endif

}

ssize_t sendto(int s, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
	SAL_LOG("[bsdsocket] sendto\n");
#if EN_LWIP

	/* Treat as a cancellation point */
	(void)enter_cancellation_point();
	int result = lwip_sendto(s, data, size, flags, to, tolen);
	leave_cancellation_point();
	printf("[bsdsocket] sendto end\n");
	return result;
#endif

#if EN_SAL
	return atiny_sal_sendto(s, data, size, flags, to, tolen);
#endif

}

static int socket_argument_validation(int domain, int type, int protocol)
{
	if (domain != AF_INET && domain != AF_INET6 && domain != AF_UNSPEC) {
		return -1;
	}
	switch (protocol) {
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE:
		if (type != SOCK_DGRAM && type != SOCK_RAW) {
			return -1;
		}
		break;
	case IPPROTO_TCP:
		if (type != SOCK_STREAM) {
			return -1;
		}
		break;
	case IPPROTO_ICMP:
	case IPPROTO_IGMP:
	case IPPROTO_ICMPV6:
		if (type != SOCK_RAW) {
			return -1;
		}
		break;
	case IPPROTO_IP:
		if (type == SOCK_RAW) {
			return -1;
		}
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

int socket(int domain, int type, int protocol)
{
	SAL_LOG("[bsdsocket] socket\n");
#if EN_LWIP

	if (!socket_argument_validation(domain, type, protocol)) {
		return lwip_socket(domain, type, protocol);
	}
#endif

#if EN_SAL
		return atiny_sal_socket(domain, type, protocol);
#endif
}

#ifdef CONFIG_DISABLE_POLL
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
	SAL_LOG("[bsdsocket] select\n");
#if EN_LWIP

	/* Treat as a cancellation point */
	(void)enter_cancellation_point();
	int result = lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
	leave_cancellation_point();
	printf("[bsdsocket] select end\n");
	return result;
#endif
	return atiny_sal_select(maxfdp1, readset, writeset, exceptset, timeout);
}
#endif

#endif
