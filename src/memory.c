internal_function
void *Memory_Allocate(u32 size)
{
    // TODO(fakhri): custom allocator?
    void *result = malloc(size);
    return result;
}

internal_function inline void *
Arena_Next_Address(Memory_Arena *arena)
{
    void *result = (void *)((u8 *)arena->base + arena->bytes_allocated);
    return result;
}

internal_function void
Memory_Copy(char *dest, char *src, u32 src_cnt)
{
    for (u32 i = 0; i < src_cnt; ++i)
    {
        dest[i] = src[i];
    }
}

internal_function void *
Push_Arena(Memory_Arena *arena, u32 bytes_to_push)
{
    void *result=0;
    if (arena)
    {
        u32 remaining_bytes = arena->capacity - arena->bytes_allocated;
        if (bytes_to_push <= remaining_bytes)
        {
            result = Arena_Next_Address(arena);
            arena->bytes_allocated += bytes_to_push;
        }
    }
    return result;
}

internal_function void *
Pop_Arena(Memory_Arena *arena, u32 bytes_to_pop)
{
    void *result = 0;
    if (arena)
    {
        if (arena->bytes_allocated >= bytes_to_pop)
        {
            arena->bytes_allocated -= bytes_to_pop;
            result = Arena_Next_Address(arena);
        }
    }
    return result;
}


internal_function
void Flush_Arena(Memory_Arena *arena)
{
    if (arena)
    {
        arena->bytes_allocated = 0;
    }
}

internal_function Memory_Arena *
Create_Arena(u32 capacity)
{
    Memory_Arena *arena = 0;
    arena = (Memory_Arena *)Memory_Allocate(sizeof(Memory_Arena));
    
    // NOTE(fakhri): initialize memory arena
    {
        arena->base = Memory_Allocate(capacity);
        arena->capacity = capacity;
        arena->bytes_allocated = 0;
    }
    
    return arena;
}

internal_function Memory_Arena *
Create_Sub_Arena(Memory_Arena *arena, u32 capacity)
{
    // TODO(fakhri): implment this
    Memory_Arena *result = 0;
    result = (Memory_Arena *)Push_Arena(arena, sizeof(Memory_Arena));
    
    
    // NOTE(fakhri): initialize memory arena
    {
        result->base = Push_Arena(arena, capacity);
        result->capacity = capacity;
        result->bytes_allocated = 0;
    }
    
    return result;
}
