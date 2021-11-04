#include <stdio.h>
#include <signal.h>

#include "defines.h"
#include "wrappers.c"

#include "memory.h"
#include "memory.c"

#include "string.h"
#include "string.c"

#include "io.h"
#include "io.c"

#include "threads.h"
#include "threads.c"

#include "networking/helper_functions.c"

#include "web_server.c"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage : smol <PORT>\n");
        return 1;
    }
    
    signal(SIGPIPE, SIG_IGN);
    Server_Context server_ctx = {};
    server_ctx.listen_fd = open_listenfd(argv[1]);
    server_ctx.server_arena = Create_Arena(SERVER_MEMORY_USAGE);
    Init_Socket_Buffer(server_ctx.server_arena, &server_ctx.sbuf, SOCKET_BUFFER_SIZE);
    
    // NOTE(fakhri): init thread count stuff
    {
        server_ctx.current_thread_count = 0;
        server_ctx.target_thread_count = INITIAL_WORKER_THREADS_COUNT;
        sem_init(&server_ctx.thread_count_mutex, 0, 1);
    }
    
    // NOTE(fakhri): init thread pool
    {
        server_ctx.thread_pool = {};
        server_ctx.thread_pool.capacity = MAX_WORKER_THRADS_COUNT; 
        server_ctx.thread_pool.pool = (Thread_Context **)Push_Arena(server_ctx.server_arena, server_ctx.thread_pool.capacity * sizeof(Thread_Context *));
        sem_init(&server_ctx.thread_pool.mutex, 0, 1);
    }
    
    Start_Worker_Threads(&server_ctx);
    
    b32 should_stop = 0;
    while(!should_stop)
    {
        Socket_Fd client_fd = Accept(server_ctx.listen_fd, 0, 0);
        Insert_Socketfd(&server_ctx, &server_ctx.sbuf, client_fd);
    }
    return 0;
}
