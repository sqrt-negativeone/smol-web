#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

internal_function void
clineterror(Socket_Fd client_fd, s8 cause, s8 errnum, s8 short_msg, s8 long_msg, Memory_Arena *arena)
{
    s8 body[] = {
        s8_lit("<html><title>Smoll Error</title>"),
        s8_lit("<body bgcolor=\"ffffff\">\r\n"),
        errnum,  s8_lit(": "), short_msg, s8_lit("\r\n"),
        s8_lit("<p>"), long_msg, s8_lit(": "), cause, s8_lit("\r\n"),
        s8_lit("<hr><em>Smoll Web Server</em>\r\n"),
    };
    s8 body_str = s8_concat_array(arena, s8_const_array(body));
    s8 response_header[] = {
        s8_lit("HTTP/1.1"), errnum, short_msg, s8_lit("\r\n"),
        s8_lit("Content-Type: text/html\r\n"),
        s8_lit("Content-length: "), s8_int_to_str(arena, body_str.len), s8_lit("\r\n"),
        s8_lit("\r\n"),
    };
    s8 response_header_str = s8_concat_array(arena, s8_const_array(response_header));
    io_write(client_fd, response_header_str.str, response_header_str.len);
    io_write(client_fd, body_str.str, body_str.len);
}

internal_function
void Serve_HTTP_Request(Socket_Fd client_fd, Memory_Arena *arena)
{
    io_t io;
    io_init_buffer(&io, client_fd);
    printf("io initialized\n");
    s8 request_line = {};
    if (io_buffered_readline(&io, arena, &request_line) != -1)
    {
        printf("request line is : %s\n", request_line.str);
        s8 method = s8_split(&request_line, ' ');
        s8 uri = s8_split(&request_line, ' ');
        s8 version = s8_split(&request_line, ' ');
        if (!s8_compare_equal(method, s8_lit("GET")))
        {
            clineterror(client_fd, method, s8_lit("501"), s8_lit("Not Implemented"),s8_lit("Server does not implement this method"), arena); 
            return;
        }
        
        // NOTE(fakhri): read and ingore request headers
        {
            u32 old_arena_load = arena->bytes_allocated;
            s8 header_line = {};
            io_buffered_readline(&io, arena, &header_line);
            while(!s8_compare_equal(header_line, s8_lit("\r\n")))
            {
                Pop_Arena(arena, header_line.len + 1);
                io_buffered_readline(&io, arena, &header_line);
            }
            Pop_Arena(arena, header_line.len + 1);
            u32 new_arena_load = arena->bytes_allocated;
            assert_true(old_arena_load == new_arena_load);
        }
        
        // NOTE(fakhri): we just serve static content for now, the root directory for our static content
        // is /static/, and the default file is index.html
        
        s8 filename = {};
        s8 filetype = {};
        // NOTE(fakhri): parse uri
        {
            s8 strings[] = {s8_lit("./static"), uri, s8_lit("index.html")}; 
            if (uri.str[uri.len - 1] == '/')
            {
                filename = s8_concat_array(arena, s8_make_array(strings, 3));
            }
            else
            {
                filename = s8_concat_array(arena, s8_make_array(strings, 2));
            }
            
            // NOTE(fakhri): extract file type
            {
                if (s8_substring(filename, s8_lit(".html")))
                {
                    filetype = s8_copy(arena, s8_lit("text/html"));
                }
                else if (s8_substring(filename, s8_lit(".gif")))
                {
                    filetype = s8_copy(arena, s8_lit("image/gif"));
                }
                else if (s8_substring(filename, s8_lit(".png")))
                {
                    filetype = s8_copy(arena, s8_lit("image/png"));
                }
                else if (s8_substring(filename, s8_lit(".jpg")))
                {
                    filetype = s8_copy(arena, s8_lit("image/jpeg"));
                }
                else
                {
                    filetype = s8_copy(arena, s8_lit("text/plain"));
                }
            }
        }
        
        // NOTE(fakhri): send response back to client
        {
            struct stat sbuf = {};
            if (stat(filename.str, &sbuf) < 0)
            {
                // NOTE(fakhri): file not found
                clineterror(client_fd, filename, s8_lit("404"), s8_lit("Not Found"),s8_lit("Server could not find this file"), arena); 
                
                return;
            }
            if ((S_ISREG(sbuf.st_mode)) && (S_IRUSR & sbuf.st_mode))
            {
                u32 filesize = sbuf.st_size;
                File_Fd filefd = open(filename.str, O_RDONLY, 0);
                s8 response_text = {};
                
                // NOTE(fakhri): construct response text
                {
                    s8 strings[] = {
                        s8_lit("HTTP/1.0 200 ok\r\n"),
                        s8_lit("Server: Tiny Web server\r\n"),
                        s8_lit("Connection: close\r\n"),
                        s8_lit("Content_length: "), s8_int_to_str(arena, filesize), s8_lit("\r\n"),
                        s8_lit("Content-type: "), filetype, s8_lit("\r\n"),
                        s8_lit("\r\n"),
                    };
                    response_text = s8_concat_array(arena, s8_const_array(strings));
                }
                
                io_write(client_fd, response_text.str, response_text.len);
                
                char *file_content = (char *)mmap(0, filesize, PROT_READ, MAP_PRIVATE, filefd, 0);
                Close(filefd);
                if (file_content)
                {
                    io_write(client_fd, file_content, filesize);
                    munmap(file_content, filesize);
                }
            }
            else
            {
                // NOTE(fakhri): permission error
                clineterror(client_fd, filename, s8_lit("403"), s8_lit("Forbidden"),s8_lit("Server could not read this file"), arena); 
                return;
            }
        }
    }
    else
    {
        printf("Couldn't read request line\n");
    }
}



internal_function void *
Thread_Main(void *ctx_ptr)
{
    Thread_Context *thrd_ctx = (Thread_Context *)ctx_ptr;
    pthread_detach(thrd_ctx->tid);
    Server_Context *server_ctx = thrd_ctx->server;
    
    b32 should_stop = 0;
    while(!should_stop)
    {
        Socket_Fd client_fd = Remove_Socketfd(server_ctx, &server_ctx->sbuf);
        printf("thread %d handles client_fd : %d\n", (u32)thrd_ctx->tid, client_fd);
        Flush_Arena(thrd_ctx->arena);
        Serve_HTTP_Request(client_fd, thrd_ctx->arena);
        Close(client_fd);
        
        sem_wait(&server_ctx->thread_count_mutex);
        if (server_ctx->target_thread_count < server_ctx->current_thread_count)
        {
            Push_Thread_Context(&server_ctx->thread_pool, thrd_ctx);
            should_stop = 1;
            --server_ctx->current_thread_count;
        }
        sem_post(&server_ctx->thread_count_mutex);
    }
    return 0;
}
