#define __LIBTHECORE__
#include "stdafx.h"

void socket_lingeron(int32_t s);
void socket_lingeroff(int32_t s);
void socket_timeout(int32_t s, int32_t sec, int32_t usec);
void socket_reuse(int32_t s);
void socket_keepalive(int32_t s);

int32_t socket_udp_read(int32_t desc, char * read_point, size_t space_left, struct sockaddr * from, socklen_t * fromlen)
{
    ssize_t ret;
    ret = recvfrom(desc, read_point, space_left, 0, from, fromlen);
    return (ret);
}

int32_t socket_read(int32_t desc, char* read_point, size_t space_left)
{
    int32_t	ret;

    ret = recv(desc, read_point, space_left, 0);

    if (ret > 0)
	return ret;

    if (ret == 0)
	return -1;

#ifdef EINTR
    if (errno == EINTR)
	return (0);
#endif

#ifdef EAGAIN
    if (errno == EAGAIN)
	return (0);
#endif

#ifdef EWOULDBLOCK
    if (errno == EWOULDBLOCK)
	return (0);
#endif

#ifdef EDEADLK
    if (errno == EDEADLK)
	return (0);
#endif

#ifdef _WIN32
	int32_t wsa_error = WSAGetLastError();
	if (wsa_error == WSAEWOULDBLOCK) {
		return 0;
	}
	sys_log(0, "socket_read: WSAGetLastError returned %d", wsa_error);
#endif

    sys_err("about to lose connection");
    return -1;
}


int32_t socket_write_tcp(int32_t desc, const char *txt, int32_t length)
{
    int32_t bytes_written = send(desc, txt, length, 0);

    if (bytes_written > 0)
	return (bytes_written);

    if (bytes_written == 0)
	return -1;

#ifdef EAGAIN
    if (errno == EAGAIN)
	return 0;
#endif

#ifdef EWOULDBLOCK
    if (errno == EWOULDBLOCK)
	return 0;
#endif

#ifdef EDEADLK
    if (errno == EDEADLK)
	return 0;
#endif

#ifdef _WIN32
	int32_t wsa_error = WSAGetLastError();
	if (wsa_error == WSAEWOULDBLOCK) {
		return 0;
	}
	sys_log(0, "socket_write_tcp: WSAGetLastError returned %d", wsa_error);
#endif

    return -1;
}


int32_t socket_write(int32_t desc, const char *data, size_t length)
{
    size_t	total;
    int32_t		bytes_written;

    total = length;

    do
    {
	if ((bytes_written = socket_write_tcp(desc, data, total)) < 0)
	{
#ifdef EWOULDBLOCK
	    if (errno == EWOULDBLOCK)
		errno = EAGAIN;
#endif
	    if (errno == EAGAIN)
		sys_err("socket write would block, about to close!");
	    else
		sys_err("write to desc error");

	    return -1;
	}
	else
	{
	    data        += bytes_written;
	    total       -= bytes_written;
	}
    }
    while (total > 0);

    return 0;
}

int32_t socket_bind(const char * ip, int32_t port, int32_t protocol)
{
    int32_t                 s;
#ifdef __WIN32
    SOCKADDR_IN			sa;
#else
    struct sockaddr_in  sa;
#endif

    if ((s = socket(AF_INET, protocol, 0)) < 0) 
    {
	sys_err("socket: %s", strerror(errno));
	return -1;
    }

    socket_reuse(s);
#ifndef _WIN32
    socket_lingeroff(s);
#else
	if (protocol == SOCK_STREAM) {
		socket_lingeroff(s);
	}
#endif

    memset(&sa, 0, sizeof(sa));
    sa.sin_family	= AF_INET;
#ifndef _WIN32
    sa.sin_addr.s_addr	= inet_addr(ip);
#else
	sa.sin_addr.s_addr	= INADDR_ANY;
#endif 
    sa.sin_port		= htons((uint16_t) port);

    if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0)
    {
	sys_err("bind: %s", strerror(errno));
	return -1;
    }

    socket_nonblock(s);

    if (protocol == SOCK_STREAM)
    {
	sys_log(0, "SYSTEM: BINDING TCP PORT ON [%d] (fd %d)", port, s);
	listen(s, SOMAXCONN);
    }
    else
	sys_log(0, "SYSTEM: BINDING UDP PORT ON [%d] (fd %d)", port, s);

    return s;
}

int32_t socket_tcp_bind(const char * ip, int32_t port)
{
    return socket_bind(ip, port, SOCK_STREAM);
}

int32_t socket_udp_bind(const char * ip, int32_t port)
{
    return socket_bind(ip, port, SOCK_DGRAM);
}

void socket_close(int32_t s)
{
#ifdef _WIN32
    closesocket(s);
#else
    close(s);
#endif
}

int32_t socket_accept(int32_t s, struct sockaddr_in *peer)
{
    int32_t desc;
    socklen_t i;

    i = sizeof(*peer);

    if ((desc = accept(s, (struct sockaddr *) peer, &i)) == -1)
    {
	sys_err("accept: %s (fd %d)", strerror(errno), s);
	return -1;
    }

    if (desc >= 65500)
    {
	sys_err("SOCKET FD 65500 LIMIT! %d", desc);
	socket_close(s);
	return -1;
    }

    socket_nonblock(desc);
    socket_lingeroff(desc);
    return (desc);
}

int32_t socket_connect(const char* host, uint16_t port)
{
    int32_t            s = 0;
    struct sockaddr_in  server_addr;
    int32_t                 rslt;

    memset(&server_addr, 0, sizeof(server_addr));

    if (isdigit(*host))
	server_addr.sin_addr.s_addr = inet_addr(host);
    else
    {
	struct hostent *hp;

	if ((hp = gethostbyname(host)) == nullptr)
	{
	    sys_err("socket_connect(): can not connect to %s:%d", host, port);
	    return -1;
	}

	memcpy((char* ) &server_addr.sin_addr, hp->h_addr, sizeof(server_addr.sin_addr));
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("socket");
	return -1;
    }

    socket_keepalive(s);
    socket_sndbuf(s, 233016);
    socket_rcvbuf(s, 233016);
    socket_timeout(s, 10, 0);
    socket_lingeron(s);

    if ((rslt = connect(s, (struct sockaddr *) &server_addr, sizeof(server_addr))) < 0)
    {
	socket_close(s);

#ifdef _WIN32
	switch (WSAGetLastError())
#else
	    switch (rslt)
#endif
	    {
#ifdef _WIN32
		case WSAETIMEDOUT:
#else
		case EINTR:
#endif
		    sys_err("HOST %s:%d connection timeout.", host, port);
		    break;
#ifdef _WIN32
		case WSAECONNREFUSED:
#else
		case ECONNREFUSED:
#endif
		    sys_err("HOST %s:%d port is not opened. connection refused.", host, port);
		    break;
#ifdef _WIN32
		case WSAENETUNREACH:
#else
		case ENETUNREACH:
#endif
		    sys_err("HOST %s:%d is not reachable from this host.", host, port);
		    break;

		default:
		    sys_err("HOST %s:%d, could not connect.", host, port);
		    break;
	    }

	perror("connect");
	return (-1);
    }

    return (s);
}

#ifndef _WIN32

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void socket_nonblock(int32_t s)
{
    int32_t flags;

    flags = fcntl(s, F_GETFL, 0);
    flags |= O_NONBLOCK;

    if (fcntl(s, F_SETFL, flags) < 0) 
    {
	sys_err("fcntl: nonblock: %s", strerror(errno));
	return;
    }
}

void socket_block(int32_t s)
{
    int32_t flags;

    flags = fcntl(s, F_GETFL, 0);
    flags &= ~O_NONBLOCK;

    if (fcntl(s, F_SETFL, flags) < 0)
    {
	sys_err("fcntl: nonblock: %s", strerror(errno));
	return;
    }
}
#else
void socket_nonblock(int32_t s)
{
    uint32_t val = 1;
    ioctlsocket(s, FIONBIO, &val);
}

void socket_block(int32_t s)
{
    uint32_t val = 0;
    ioctlsocket(s, FIONBIO, &val);
}
#endif

void socket_dontroute(int32_t s)
{
    int32_t set;

    if (setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (const char *) &set, sizeof(int32_t)) < 0)
    {
	sys_err("setsockopt: dontroute: %s", strerror(errno));
	socket_close(s);
	return;
    }
}

void socket_lingeroff(int32_t s)
{
#ifdef _WIN32
    int32_t linger;
    linger = 0;
#else
    struct linger linger;

    linger.l_onoff	= 0;
    linger.l_linger	= 0;
#endif
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*) &linger, sizeof(linger)) < 0)
    {
	sys_err("setsockopt: linger: %s", strerror(errno));
	socket_close(s);
	return;
    }
}

void socket_lingeron(int32_t s)
{
#ifdef _WIN32
    int32_t linger;
    linger = 0;
#else
    struct linger linger;

    linger.l_onoff	= 1;
    linger.l_linger	= 0;
#endif
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*) &linger, sizeof(linger)) < 0)
    {
	sys_err("setsockopt: linger: %s", strerror(errno));
	socket_close(s);
	return;
    }
}

void socket_rcvbuf(int32_t s, uint32_t opt)
{
    socklen_t optlen;

    optlen = sizeof(opt);

    if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char*) &opt, optlen) < 0)
    {
	sys_err("setsockopt: rcvbuf: %s", strerror(errno));
	socket_close(s);
	return;
    }

    opt         = 0;
    optlen      = sizeof(opt);

    if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*) &opt, &optlen) < 0)
    {
	sys_err("getsockopt: rcvbuf: %s", strerror(errno));
	socket_close(s);
	return;
    }

    sys_log(1, "SYSTEM: %d: receive buffer changed to %d", s, opt);
}

void socket_sndbuf(int32_t s, uint32_t opt)
{
    socklen_t optlen;

    optlen = sizeof(opt);

    if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char*) &opt, optlen) < 0)
    {
	sys_err("setsockopt: sndbuf: %s", strerror(errno));
	return;
    }

    opt         = 0;
    optlen      = sizeof(opt);

    if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*) &opt, &optlen) < 0)
    {
	sys_err("getsockopt: sndbuf: %s", strerror(errno));
	return;
    }

    sys_log(1, "SYSTEM: %d: send buffer changed to %d", s, opt);
}

void socket_timeout(int32_t s, int32_t sec, int32_t usec)
{
#ifndef _WIN32
    struct timeval      rcvopt;
    struct timeval      sndopt;
    socklen_t		optlen = sizeof(rcvopt);

    rcvopt.tv_sec = sndopt.tv_sec = sec;
    rcvopt.tv_usec = sndopt.tv_usec = usec;
#else
    socklen_t		rcvopt, sndopt;
    socklen_t		optlen = sizeof(rcvopt);
    sndopt = rcvopt = (sec * 1000) + (usec / 1000);
#endif
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*) &rcvopt, optlen) < 0)
    {
	sys_err("setsockopt: timeout: %s", strerror(errno));
	socket_close(s);
	return;
    }

    if (getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*) &rcvopt, &optlen) < 0)
    {
	sys_err("getsockopt: timeout: %s", strerror(errno));
	socket_close(s);
	return;
    }

    optlen = sizeof(sndopt);

    if (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*) &sndopt, optlen) < 0)
    {
	sys_err("setsockopt: timeout: %s", strerror(errno));
	socket_close(s);
	return;
    }

    if (getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*) &sndopt, &optlen) < 0)
    {
	sys_err("getsockopt: timeout: %s", strerror(errno));
	socket_close(s);
	return;
    }

#ifndef _WIN32
    sys_log(1, "SYSTEM: %d: TIMEOUT RCV: %d.%d, SND: %d.%d", s, rcvopt.tv_sec, rcvopt.tv_usec, sndopt.tv_sec, sndopt.tv_usec);
#endif
}

void socket_reuse(int32_t s)
{
    int32_t opt = 1;

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*) &opt, sizeof(opt)) < 0)
    {
	sys_err("setsockopt: reuse: %s", strerror(errno));
	socket_close(s);
	return;
    }
}

void socket_keepalive(int32_t s)
{
    int32_t opt = 1;

    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char*) &opt, sizeof(opt)) < 0)
    {
	perror("setsockopt: keepalive");
	socket_close(s);
	return;
    }
}
