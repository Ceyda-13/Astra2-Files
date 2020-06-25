#ifndef __INC_LIBTHECORE_SOCKET_H__
#define __INC_LIBTHECORE_SOCKET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
typedef int32_t socklen_t;
#else
#define INVALID_SOCKET -1
#endif

    extern int32_t		socket_read(int32_t desc, char* read_point, size_t space_left);
    extern int32_t		socket_write(int32_t desc, const char *data, size_t length);

    extern int32_t		socket_udp_read(int32_t desc, char * read_point, size_t space_left, struct sockaddr * from, socklen_t * fromlen);
    extern int32_t		socket_tcp_bind(const char * ip, int32_t port);
    extern int32_t		socket_udp_bind(const char * ip, int32_t port);

    extern int32_t	socket_accept(int32_t s, struct sockaddr_in *peer);
    extern void		socket_close(int32_t s);
    extern int32_t	socket_connect(const char* host, uint16_t port);

    extern void		socket_nonblock(int32_t s);
    extern void		socket_block(int32_t s);
    extern void		socket_dontroute(int32_t s);
    extern void		socket_lingeroff(int32_t s);
    extern void		socket_lingeron(int32_t s);

    extern void		socket_sndbuf(int32_t s, uint32_t opt);
    extern void		socket_rcvbuf(int32_t s, uint32_t opt);

#ifdef __cplusplus
}
#endif

#endif
