//
// Created by agml_ on 19/01/2022.
//

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>

#define MAX_SIZE 1e8
#define KB_128 131072

class MallocMetadata
{
public:
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
    void *allocated_addr;
};

void isInMemory(void *pVoid);

MallocMetadata *memory = nullptr;
MallocMetadata *mmap_list = nullptr;

static void split_free(MallocMetadata *to_split, size_t size)
{
    long split_size;
    split_size = to_split->size - size - sizeof(MallocMetadata);
    if (split_size >= 128)
    {
        MallocMetadata *un_used_split = (MallocMetadata *)(((char *)to_split->allocated_addr) + size);
        un_used_split->size = to_split->size - size - sizeof(MallocMetadata);
        un_used_split->is_free = true;
        un_used_split->allocated_addr = (void *)((char *)un_used_split + sizeof(MallocMetadata));
        un_used_split->prev = to_split;
        un_used_split->next = to_split->next;
        to_split->size = size;
        to_split->is_free = false;
        if (to_split->next != nullptr)
            to_split->next->prev = un_used_split;
        to_split->next = un_used_split;
    }
}

void *allocate128(size_t size)
{

    MallocMetadata *ptr = memory;
    MallocMetadata *meta;

    while (ptr != nullptr)
    {
        if (ptr->is_free)
        {
            if (ptr->size >= size)
            {
                split_free(ptr, size);
                ptr->is_free = false;
                return ptr->allocated_addr;
            }
        }
        ptr = ptr->next;
    }
    // here we are sure that there is no free block that we can use
    // then we should allocate an new block using sbrk
    ptr = memory;
    if (ptr != nullptr)
    {
        while (ptr->next != nullptr)
            ptr = ptr->next;
    }

    if (ptr != nullptr && ptr->next == nullptr && ptr->is_free)
    {
        if (sbrk(size - ptr->size) == (void *)-1)
            return nullptr;
        ptr->is_free = false;
        ptr->size = size;
        return ptr->allocated_addr;
    }
    meta = (MallocMetadata *)sbrk(sizeof(MallocMetadata) + size);
    if (meta == (void *)-1)
        return nullptr;

    meta->size = size;
    meta->is_free = false;
    meta->allocated_addr = ((char *)meta + sizeof(MallocMetadata));
    if (memory != nullptr)
    {
        ptr = memory;
        while (ptr->next != nullptr)
        {
            ptr = ptr->next;
        }
        ptr->next = meta;
        meta->prev = ptr;
        meta->next = nullptr;
    }
    else
    {
        memory = meta;
    }
    return meta->allocated_addr;
}

void *allocateMmap(size_t size)
{

    MallocMetadata *ptr = memory;
    MallocMetadata *meta;

    meta = ((MallocMetadata *)mmap(nullptr, size + sizeof(MallocMetadata), PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    if (meta == (void *)-1)
    {

        return nullptr;
    }
    meta->size = size;
    meta->allocated_addr = ((char *)meta) + sizeof(MallocMetadata);

    meta->is_free = false;
    if (mmap_list == nullptr)
        mmap_list = meta;
    else
    {
        ptr = mmap_list;
        while (ptr->next != nullptr)
        {
            ptr = ptr->next;
        }
        ptr->next = meta;
        meta->prev = ptr;
        meta->next = nullptr;
    }
    return meta->allocated_addr;
}

void *smalloc(size_t size)
{
    if (size == 0 || size > MAX_SIZE)
        return NULL;

    MallocMetadata *ptr = memory;
    MallocMetadata *meta;
    // checks if we can reuse an free sector
    if (size <= KB_128)
    {
        return allocate128(size);
    }

    else
    {
        return allocateMmap(size);
    }
}

void isInMmapList(void *p)
{

    MallocMetadata *ptr;
    ptr = mmap_list;

    while (ptr != nullptr)
    {
        if (ptr->allocated_addr == p)
        {
            ptr->prev->next = ptr->next;
            if (ptr->next != nullptr)
                ptr->next->prev = ptr->prev;
            if (ptr->prev == nullptr)
                mmap_list = ptr->next;
            munmap(ptr, ptr->size + sizeof(MallocMetadata));
            return;
        }
        ptr = ptr->next;
    }
}

void isInMemory(void *p)
{

    MallocMetadata *ptr = memory;
    MallocMetadata *tmp = (MallocMetadata *)p;
    char *tmp1 = (char *)tmp->allocated_addr + tmp->size;

    // check if the block is at the head of the heap
    if (tmp1 == (char *)sbrk(0))
    {
        MallocMetadata *p1 = tmp->prev;
        p1->next = nullptr;
        sbrk(-(tmp->size + sizeof(MallocMetadata)));
        return;
    }

    // the block is in the middle
    while (ptr != nullptr)
    {

        if (ptr->allocated_addr == p)
        {                        // we find the the block that we have to free
            ptr->is_free = true; // free the block
            if (ptr->next != nullptr && ptr->next->is_free)
            { // check if the next block is freed and merge in case if it is true
                ptr->size += ptr->next->size + sizeof(MallocMetadata);
                if (ptr->next->next != nullptr)
                    ptr->next->next->prev = ptr;
                ptr->next = ptr->next->next;
            }
            if (ptr->prev->is_free)
            { // check if the prev block is freed and merge in case if it is true
                ptr->prev->size += ptr->size + sizeof(MallocMetadata);
                if (ptr->next != nullptr)
                    ptr->next->prev = ptr->prev;
                ptr->prev->next = ptr->next;
            }

            return;
        }
        ptr = ptr->next;
    }
}

void sfree(void *p)
{
    if (p == nullptr)
        return;
    isInMmapList(p);
    isInMemory(p);
}

void *scalloc(size_t num, size_t size)
{
    // We are looking for a continous free area, num blocks, each of at least size bytes.
    if (size == 0 || size * num > MAX_SIZE)
        return NULL;
    // Check if we have memory in the first place.
    int i = 0;
    int new_size = size * num;
    bool zero_flag = true;
    if (memory)
    {
        // Iterate through the meta data to get the required block.
        for (MallocMetadata *data = memory; data != nullptr; data = data->next)
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
                        zero_flag = false;
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

size_t _num_free_blocks()
{
    size_t counter = 0;
    MallocMetadata *ptr = memory;
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
    MallocMetadata *ptr = memory;
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
    MallocMetadata *ptr = memory;
    while (ptr)
    {
        counter++;
        ptr = ptr->next;
    }
    ptr = mmap_list;
    while (ptr != nullptr)
    {
        counter++;
        ptr = ptr->next;
    }
    return counter;
}

size_t _num_allocated_bytes()
{

    size_t counter = 0;
    MallocMetadata *ptr = memory;
    while (ptr)
    {
        counter += ptr->size;
        ptr = ptr->next;
    }
    ptr = mmap_list;
    while (ptr != nullptr)
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