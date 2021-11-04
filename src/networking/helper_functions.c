
#define LISTENQ 1024

internal_function
Socket_Fd open_clientfd(char *hostname, char *port)
{
    Socket_Fd clientfd;
    struct addrinfo hints, *addrinfo_result, *p;
    hints = {};
    
    // NOTE(fakhri): we want streaming socket, with a numerical port
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
    
    Getaddrinfo(hostname, port, &hints, &addrinfo_result);
    for (p = addrinfo_result; p; p = p->ai_next)
    {
        // NOTE(fakhri): open client socket
        clientfd = Socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (clientfd < 0) continue;
        // NOTE(fakhri): connect to the socket
        if (Connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) break;
        // NOTE(fakhri): close the client socket to avoid memory leak
        Close(clientfd);
    }
    freeaddrinfo(addrinfo_result);
    if (!p) return -1;
    return clientfd;
}

internal_function
Socket_Fd open_listenfd(char *port)
{
    Socket_Fd listenfd;
    i32 optionval = 1;
    struct addrinfo hints = {}, *addrinfo_result, *p;
    
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG | AI_PASSIVE;
    Getaddrinfo(0, port, &hints, &addrinfo_result);
    
    for (p = addrinfo_result; p; p = p->ai_next)
    {
        listenfd = Socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listenfd < 0) continue;
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optionval, sizeof(i32));
        if (Bind(listenfd, p->ai_addr, p->ai_addrlen) != -1) break;
        Close(listenfd);
    }
    
    freeaddrinfo(addrinfo_result);
    if (!p) return -1;
    
    if (listen(listenfd, LISTENQ) < 0)
    {
        Close(listenfd);
        listenfd = -1;
    }
    return listenfd;
}