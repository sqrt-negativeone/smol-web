/* date = November 3rd 2021 3:06 pm */

#ifndef IO_H
#define IO_H

#define BUFFER_SIZE KB(8)
struct io_t
{
    File_Fd fd;
    u32 available_bytes;
    char *buffer_position;
    char buf[BUFFER_SIZE];
};

#endif //IO_H
