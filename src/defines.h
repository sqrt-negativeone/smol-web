/* date = November 1st 2021 9:36 pm */

#ifndef DEFINES_H
#define DEFINES_H

#include <cinttypes>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef i32 b32;

typedef i32 Socket_Fd;
typedef i32 File_Fd;

#define internal_function static
#define global_variable static
#define local_persist static

#define KB(x) ((x) << 10)
#define MB(x) (KB(x) << 10)
#define GB(x) (MB(x) << 10)

#define assert_break (*((int *)0) = 0)
#define assert_true(expr) if (!(expr)) { assert_break;}

#define MAX_WORKER_THRADS_COUNT 16
#define INITIAL_WORKER_THREADS_COUNT 4
#define SOCKET_BUFFER_SIZE 512
#define SERVER_MEMORY_USAGE MB(128)

#endif //DEFINES_H
