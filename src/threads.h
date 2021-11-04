/* date = November 4th 2021 5:45 pm */

#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <semaphore.h>

struct Socket_Buffer
{
    Socket_Fd *buf;
    u32 size;
    u32 cnt;
    u32 front;
    u32 back;
    sem_t mutex;
    sem_t empty_slots;
    sem_t available_items;
};

struct Thread_Context;

struct Thread_Pool
{
    Thread_Context **pool;
    sem_t mutex;
    u32 capacity;
    u32 current_size;
};

struct Server_Context
{
    Thread_Pool thread_pool;
    Memory_Arena *server_arena;
    Socket_Buffer sbuf;
    Socket_Fd listen_fd;
    sem_t thread_count_mutex;
    u32 current_thread_count;
    u32 target_thread_count;
};

struct Thread_Context
{
    Memory_Arena *arena;
    Server_Context *server;
    pthread_t tid;
};

#endif //THREADS_H
