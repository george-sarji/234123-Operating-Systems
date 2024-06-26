#include <unistd.h>
#include <cstring>
#include <iostream>

#define MAX_SIZE 1e8

struct MallocMetadata
{
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
    void *allocated_addr;
};

MallocMetadata *metadata = nullptr;

void *smalloc(size_t size)
{

    if (size == 0 || size > MAX_SIZE)
    {
        return NULL;
    }
    // We need to check if the current list has any available blocks.
    MallocMetadata *current = metadata;
    while (current != nullptr)
    {
        if (current->is_free && current->size >= size)
        {
            // We found an appropriate block. We can assign here.
            current->is_free = false;
            return current->allocated_addr;
        }
        current = current->next;
    }
    // If we reached here - we don't have any appropriate blocks.
    // Allocate the block as required.
    MallocMetadata *new_address = (MallocMetadata *)sbrk(sizeof(MallocMetadata));
    // Check if successful.
    if (new_address == (void *)(-1))
    {
        return NULL;
    }
    void *start_address = (MallocMetadata *)sbrk(size);
    if (start_address == (void *)(-1))
    {
        return NULL;
    }

    // Set the required parameters for the newly allocated block.
    new_address->is_free = false;
    new_address->size = size;
    new_address->prev = nullptr;
    new_address->next = nullptr;
    new_address->allocated_addr = start_address;
    // Successful allocation. Check if we have an allocation list.
    if (metadata != nullptr)
    {
        // We have an allocation list.
        // Get the end item and add the allocation to the list.
        current = metadata;
        while (current->next != nullptr)
        {
            current = current->next;
        }
        // We have the last item in current.
        current->next = new_address;
        new_address->prev = current;
        // We can return the new address here.
    }
    else
    { // We don't have a list. Set it as the new one.
        metadata = new_address;
    }
    return new_address->allocated_addr;
}

void *scalloc(size_t num, size_t size)
{
    // We are looking for a continous free area, num blocks, each of at least size bytes.
    if (size == 0 || size * num > MAX_SIZE)
        return NULL;
    // We can use smalloc to allocate a block for us.
    void *new_address = smalloc(num * size);
    // Check if we got a valid address.
    if (new_address == NULL)
    {
        return NULL;
    }
    // Set the data to zero.
    memset(new_address, 0, size * num);
    return new_address;
}

void sfree(void *p)
{
    if (p == nullptr)
        return;
    // Assistive cast to avoid size upscale.
    char *current = (char *)p - sizeof(MallocMetadata);
    MallocMetadata *block = (MallocMetadata *)current;
    block->is_free = true;
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
    // Assistive cast to char to prevent upscaling of size.
    char *oldp_cast = (char *)oldp - sizeof(MallocMetadata);
    MallocMetadata *current = (MallocMetadata *)oldp_cast;
    // Check if the size is appropriate.
    if (current->size < size)
    {
        // Size is not appropriate. Allocate new block.
        void *new_address = smalloc(size);
        // Check for successful allocation.
        if (new_address == nullptr)
        {
            return nullptr;
        }
        // Copy the data from the old block into the new.
        memcpy(new_address, current->allocated_addr, current->size);
        // Set the current block as used.
        current->is_free = false;
        // Free the old block.
        sfree(oldp);
        return new_address;
    }
    else
    {
        // Size is appropriate. Reuse the current block.
        return current->allocated_addr;
    }
}

size_t _num_free_blocks()
{
    size_t counter = 0;
    MallocMetadata *ptr = metadata;
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
    MallocMetadata *ptr = metadata;
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
    MallocMetadata *ptr = metadata;
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
    MallocMetadata *ptr = metadata;
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
