// NOTE(fakhri): forward declaration
internal_function void *
Thread_Main(void *ctx_ptr);

internal_function b32
Push_Thread_Context(Thread_Pool *thread_pool, Thread_Context *thrd_ctx)
{
    b32 result = 0;
    if (thread_pool)
    {
        sem_wait(&thread_pool->mutex);
        if (thread_pool->current_size < thread_pool->capacity)
        {
            result = 1;
            thread_pool->pool[thread_pool->current_size++] = thrd_ctx;
        }
        else
        {
            // NOTE(fakhri): should not arrive here
            assert_break;
        }
        sem_post(&thread_pool->mutex);
    }
    return result;
}

internal_function Thread_Context*
Pop_Thread_Context(Thread_Pool *thread_pool)
{
    Thread_Context *result = 0;
    if (thread_pool)
    {
        sem_wait(&thread_pool->mutex);
        if (thread_pool->current_size != 0)
        {
            result = thread_pool->pool[--(thread_pool->current_size)];
        }
        sem_post(&thread_pool->mutex);
    }
    return result;
}

internal_function void
Start_Worker_Threads(Server_Context *server_ctx)
{
    sem_wait(&server_ctx->thread_count_mutex);
    if (server_ctx->target_thread_count > server_ctx->current_thread_count)
    {
        u32 threads_to_start = server_ctx->target_thread_count - server_ctx->current_thread_count;
        
        for (u32 i = 0; i < threads_to_start; ++i)
        {
            // NOTE(fakhri): get thread context from threads pool if available, else create one
            Thread_Context *thrd_ctx = Pop_Thread_Context(&server_ctx->thread_pool);
            if (!thrd_ctx)
            {
                thrd_ctx = (Thread_Context *)Push_Arena(server_ctx->server_arena, sizeof(Thread_Context));
                thrd_ctx->arena = Create_Sub_Arena(server_ctx->server_arena, KB(64));
                thrd_ctx->server = server_ctx;
            }
            pthread_create(&thrd_ctx->tid, 0, Thread_Main, (void *) thrd_ctx);
            ++server_ctx->current_thread_count;
        }
        
    }
    sem_post(&server_ctx->thread_count_mutex);
}

internal_function void
Init_Socket_Buffer(Memory_Arena *arena, Socket_Buffer *sbuf, u32 count)
{
    sbuf->buf = (Socket_Fd *)Push_Arena(arena, count * sizeof(Socket_Fd));
    sbuf->size = count;
    sbuf->front = sbuf->back = 0;
    sem_init(&sbuf->mutex, 0, 1);
    sem_init(&sbuf->empty_slots, 0, count);
    sem_init(&sbuf->available_items, 0, 0);
}

internal_function void
Insert_Socketfd(Server_Context *server_ctx, Socket_Buffer *sbuf, Socket_Fd sockfd)
{
    sem_wait(&sbuf->empty_slots);
    sem_wait(&sbuf->mutex);
    sbuf->buf[(++sbuf->back) % sbuf->size] = sockfd;
    ++(sbuf->cnt);
    if (sbuf->cnt == sbuf->size)
    {
        // NOTE(fakhri): buffer becomes full, we should double the number of worker threads
        if (server_ctx->target_thread_count < MAX_WORKER_THRADS_COUNT)
        {
            server_ctx->target_thread_count <<= 1;
            printf("buffer full, doubling thread count, new thread count is : %d\n", server_ctx->target_thread_count);
            Start_Worker_Threads(server_ctx);
        }
    }
    sem_post(&sbuf->mutex);
    sem_post(&sbuf->available_items);
}

internal_function Socket_Fd
Remove_Socketfd(Server_Context *server_ctx, Socket_Buffer *sbuf)
{
    Socket_Fd result = 0;
    
    sem_wait(&sbuf->available_items);
    sem_wait(&sbuf->mutex);
    result = sbuf->buf[(++sbuf->front) % sbuf->size];
    --(sbuf->cnt);
    if (sbuf->cnt == 0)
    {
        // NOTE(fakhri): buffer becomes empty, we should double the number of worker threads
        if (server_ctx->target_thread_count > 1)
        {
            server_ctx->target_thread_count >>= 1;
            printf("buffer empty, halving thread count, new thread count is : %d\n", server_ctx->target_thread_count);
        }
    }
    sem_post(&sbuf->mutex);
    sem_post(&sbuf->empty_slots);
    
    return result;
}