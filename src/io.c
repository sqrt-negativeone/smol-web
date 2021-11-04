internal_function void
io_init_buffer(io_t *io, File_Fd fd)
{
    io->fd = fd;
    io->available_bytes = 0;
    io->buffer_position = io->buf;
}

internal_function
i32 io_write(File_Fd file_fd, void *buf, u32 bytes_to_write)
{
    u32 bytes_left = bytes_to_write;
    i32 bytes_written;
    char *buffer_position = (char *)buf;
    while (bytes_left)
    {
        if ((bytes_written = write(file_fd, buffer_position, bytes_left)) <= 0)
        {
            if (errno == EINTR)
            {
                bytes_written = 0;
            }
            else return -1;
        }
        bytes_left -= bytes_written;
        buffer_position += bytes_written;
    }
    return bytes_to_write;
}

internal_function
i32 io_read(File_Fd file_fd, void *buf, i32 bytes_to_read)
{
    u32 bytes_left = bytes_to_read;
    i32 bytes_read;
    char *buffer_position = (char *)buf;
    while(bytes_left > 0)
    {
        if ((bytes_read = read(file_fd, buffer_position, bytes_left)) < 0)
        {
            if (errno == EINTR)
            {
                bytes_read = 0;
            }
            else return -1;
        }
        else if (bytes_left == 0) 
            break;
        bytes_left -= bytes_read;
        buffer_position += bytes_read;
    }
    return bytes_to_read;
}

internal_function i32
io_buffered_read(io_t *io, void *buf, u32 bytes_to_read)
{
    i32 bytes_read;
    if (!io) return -1;
    if(io->available_bytes <= 0)
    {
        // NOTE(fakhri): refill buffer
        io->available_bytes = read(io->fd, io->buf, sizeof(io->buf));
        if (io->available_bytes < 0) 
        {
            return -1;
        }
        else if (io->available_bytes == 0) 
        {
            // NOTE(fakhri): EOF
            return 0;
        }
        io->buffer_position = io->buf;
    }
    bytes_read = bytes_to_read;
    if (io->available_bytes < bytes_read)
        bytes_read = io->available_bytes;
    Memory_Copy((char *)buf, io->buffer_position, bytes_read);
    io->available_bytes -= bytes_read;
    io->buffer_position += bytes_read;
    return bytes_read;
}

internal_function i32
io_buffered_readline(io_t *io, Memory_Arena *arena, s8 *line)
{
    i32 i = 1;
    if (arena)
    {
        u32 maxline = arena->capacity - arena->bytes_allocated;
        line->str = (char *)Arena_Next_Address(arena);
        char *buffer_position = line->str;
        char current_char;
        for (; i < maxline; ++i)
        {
            i32 characters_read = io_buffered_read(io, &current_char, 1);
            if (characters_read == 1)
            {
                *buffer_position = current_char;
                ++buffer_position;
                if (current_char == '\n')
                {
                    i++;
                    break;
                }
            }
            else if (characters_read == 0)
            {
                break;
            }
            else 
            {
                return -1;
            }
        }
        *buffer_position = 0;
        line->len = i - 1;
        Push_Arena(arena, line->len + 1);
    }
    return i - 1;
}
