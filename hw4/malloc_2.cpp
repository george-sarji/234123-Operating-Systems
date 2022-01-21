#include <unistd.h>
#include <cstring>

#define MAX_SIZE 1e8

struct MallocMetadata
{
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
    void *allocated_addr;
};

MallocMetadata *malloc = nullptr;

void *smalloc(size_t size)
{

    if (size == 0 || size >= MAX_SIZE)
        return NULL;

    MallocMetadata *ptr = malloc;
    MallocMetadata *meta;
    void *start_address;
    // checks if we can reuse an free sector
    while (ptr != nullptr)
    {
        if (ptr->is_free)
        {
            if (ptr->size >= size)
            {
                ptr->is_free = false;
                return ptr->allocated_addr;
            }
        }
        ptr = ptr->next;
    }
    // here we are sure that there is no free sector that we can use
    // then we should allocate an new sector using sbrk
    ptr = malloc;
    if (ptr != nullptr)
    {
        while (ptr->next != nullptr)
            ptr = ptr->next;
    }
    meta = (MallocMetadata *)sbrk(sizeof(MallocMetadata));
    if (meta == (void *)-1)
        return nullptr;
    start_address = sbrk(size);
    if (start_address == (void *)-1)
        return nullptr;
    meta->size = size;
    meta->is_free = false;
    meta->allocated_addr = start_address;
    if (malloc != nullptr)
    {
        ptr = malloc;
        meta->next = nullptr;
        while (ptr->next != nullptr)
        {
            ptr = ptr->next;
        }
        ptr->next = meta;
        meta->prev = ptr;
    }
    else
    {
        malloc = meta;
    }
    return start_address;
}

void *scalloc(size_t num, size_t size)
{
    // We are looking for a continous free area, num blocks, each of at least size bytes.
    if (size == 0 || size * num > MAX_SIZE)
        return NULL;
    // Check if we have malloc in the first place.
    int i = 0;
    int new_size = size * num;
    bool zero_flag = true;
    if (malloc)
    {
        // Iterate through the meta data to get the required block.
        for (MallocMetadata *data = malloc; data != nullptr; data = data->next)
        {
            zero_flag = true;
            i = 0;
            char *ptr = (char *)(data->allocated_addr);
            // Check if the current block is free and is appropriate.
            if (data->is_free && data->size >= num * (size))
            {

                while (i < new_size)
                {
                    if (*(ptr + i) != '0')
                    {
                        zero_flag = false;
                        break;
                    }
                    i++;
                }
                if (zero_flag)
                {
                    return data->allocated_addr;
                }
            }
        }
    }

    // If we reached this point - we don't have any blocks that are appropriate.
    // We have to use sbrk.
    void *new_address = smalloc(size * num);
    if (new_address == nullptr)
    {
        return nullptr;
    }
    memset(new_address, 0, size * num);
    return new_address;
}

void sfree(void *p)
{

    if (p == nullptr)
        return;
    MallocMetadata *ptr = malloc;

    while (ptr)
    {
        if (ptr->allocated_addr == p)
        {
            ptr->is_free = true;
            return;
        }
        ptr = ptr->next;
    }
}

void *srealloc(void *oldp, size_t size)
{
    if (size == 0 || size > MAX_SIZE)
        return NULL;
    // We have to check if oldp is null. If so, allocate size bytes.
    if (oldp == NULL)
    {
        return smalloc(size);
    }
    // We have a valid pointer. We can assume that it's a valid metadata block.

    char *ptr1 = (char *)oldp - sizeof(MallocMetadata);
    MallocMetadata *ptr = (MallocMetadata *)ptr1;

    if (ptr->allocated_addr == oldp)
    {
        if (ptr->size < size)
        {
            void *new_address = smalloc(size);
            if (!new_address)
                return nullptr;
            memcpy(new_address, ptr->allocated_addr, ptr->size);
            ptr->is_free = true;
            sfree(oldp);
            return new_address;
        }
        else
        {
            return ptr->allocated_addr;
        }
    }

    return nullptr;
}

size_t _num_free_blocks()
{
    size_t counter = 0;
    MallocMetadata *ptr = malloc;
    while (ptr)
    {
        if (ptr->is_free)
            counter++;
        ptr = ptr->next;
    }
    return counter;
}

size_t _num_free_bytes()
{
    size_t counter = 0;
    MallocMetadata *ptr = malloc;
    while (ptr)
    {
        if (ptr->is_free)
            counter += ptr->size;
        ptr = ptr->next;
    }
    return counter;
}

size_t _num_allocated_blocks()
{
    size_t counter = 0;
    MallocMetadata *ptr = malloc;
    while (ptr)
    {
        counter++;
        ptr = ptr->next;
    }
    return counter;
}

size_t _num_allocated_bytes()
{
    size_t counter = 0;
    MallocMetadata *ptr = malloc;
    while (ptr)
    {
        counter += ptr->size;
        ptr = ptr->next;
    }
    return counter;
}

size_t _size_meta_data()
{
    return sizeof(MallocMetadata);
}

size_t _num_meta_data_bytes()
{
    return _num_allocated_blocks() * _size_meta_data();
}
