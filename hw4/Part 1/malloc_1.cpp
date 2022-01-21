#include <cstddef>
#include <unistd.h>

void* smalloc(size_t size)
{
    // We have to allocate size bytes using sbrk.
    if(size == 0 || size > 10e8) return NULL;
    // Try performing sbrk.
    void* result_address = sbrk(size);
    if(result_address == (void*)-1)
    {
        return NULL;
    }
    return result_address;
}