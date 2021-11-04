/* date = November 2nd 2021 11:53 pm */

#ifndef MEMORY_H
#define MEMORY_H

struct Memory_Arena
{
    void *base;
    u32 capacity;
    u32 bytes_allocated;
};

#endif //MEMORY_H
