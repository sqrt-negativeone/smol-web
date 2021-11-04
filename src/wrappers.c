#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

typedef sockaddr SA;

internal_function
int Accept(Socket_Fd sockfd, SA *addr, socklen_t *addrlen)
{
    int result = 0;
    if ((result = accept(sockfd, addr, addrlen)) == -1)
    {
        fprintf(stderr, "Accept() errorno : %d\n", errno);
        exit(-1);
    }
    return result;
}

internal_function
int Getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
    int result = 0;
    if ((result = getaddrinfo(node, service, hints, res)) != 0)
    {
        fprintf(stderr, "Getaddrinfo() error : %s\n", gai_strerror(result));
        exit(-1);
    }
    return result;
}

internal_function
int Socket(int domain, int type, int protocol)
{
    int result = 0;
    if ((result = socket(domain, type, protocol)) == -1)
    {
        fprintf(stderr, "Socket() error : %d\n", errno);
        exit(-1);
    }
    return result;
}

internal_function
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int result = 0;
    if ((result = connect(sockfd, addr, addrlen)) == -1)
    {
        fprintf(stderr, "Connect() error : %d\n", errno);
        exit(-1);
    }
    return result;
}

internal_function
int Close(int fd)
{
    int result = 0;
    if ((result = close(fd)) == -1)
    {
        fprintf(stderr, "Close() error : %d\n", errno);
        exit(-1);
    }
    return result;
}

internal_function
int Setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    int result = 0;
    if ((result = setsockopt(sockfd, level, optname, optval, optlen)) == -1)
    {
        fprintf(stderr, "Setsockopt() error : %d\n", errno);
        exit(-1);
    }
    return result;
}

internal_function
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    int result = 0;
    if ((result = bind(sockfd, addr, addrlen)) == -1)
    {
        fprintf(stderr, "Bind() error : %d\n", errno);
        exit(-1);
    }
    return result;
}