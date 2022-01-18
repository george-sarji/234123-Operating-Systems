#include <unistd.h>
#include <cstring>

struct MallocMetadata
{
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
};

MallocMetadata *memory = nullptr;

void *scalloc(size_t num, size_t size)
{
    // We are looking for a continous free area, num blocks, each of at least size bytes.
    if (size == 0 || size * num > 10e8)
        return NULL;
    // Check if we have memory in the first place.
    if (memory)
    {
        // Iterate through the meta data to get the required block.
        for (MallocMetadata *data = memory; data != nullptr; data = data->next)
        {
            // Check if the current block is free and is appropriate.
            if (data->is_free && data->size >= num * (size + sizeof(MallocMetadata)))
            {
                // Appropriate block. Set the data to zero.
                memset(data + sizeof(MallocMetadata), 0, size);
                return data + sizeof(MallocMetadata);
            }
        }
    }

    // If we reached this point - we don't have any blocks that are appropriate.
    // We have to use sbrk.
    MallocMetadata *new_address = (MallocMetadata *)sbrk(num * size + sizeof(MallocMetadata));
    if (new_address == (void *)(-1))
    {
        // ! sbrk failed. Return null.
        return NULL;
    }
    // We have to set the parameters as required.
    new_address->size = num * size;
    new_address->is_free = false;
    new_address->next = new_address->prev = nullptr;
    // Check if had memory to add the address to the list.
    if (memory != nullptr)
    {
        // We do have memory. We have to set the new block as the tail of the linked list.
        MallocMetadata *current = memory;
        while (current->next != nullptr)
            current = current->next;
        // Set the next of current to the new block, the previous of the new block to the current.
        current->next = new_address;
        new_address->prev = current;
    }
    else
    {
        memory = new_address;
    }

    // Return the address finally.
    return new_address + sizeof(MallocMetadata);
}

void *srealloc(void *oldp, size_t size)
{
    if (size == 0 || size > 10e8)
        return NULL;
    // We have to check if oldp is null. If so, allocate size bytes.
    if (oldp == NULL)
    {
        // TODO: Change to smalloc.
        return scalloc(1, size);
    }
    // We have a valid pointer. We can assume that it's a valid metadata block.
    MallocMetadata *old_block = (MallocMetadata *)oldp - sizeof(MallocMetadata);
    // If the size is smaller than the old block, use the old block.
    if (size < old_block->size)
    {
        old_block->is_free = false;
        return oldp;
    }

    // TODO: Change to smalloc.
    void *new_address = scalloc(1, size);
    // Check if we got a valid allocation.
    if (new_address == NULL)
        return NULL;
    // Copy the data.
    memcpy(new_address, oldp, size);
    // TODO: Free the old block.
    // sfree(oldp);
    return new_address;
}

size_t _num_free_bytes()
{
    size_t metaSize = sizeof(MallocMetadata);
    size_t current_size = 0;
    for (MallocMetadata *current = memory; current != nullptr; current = current->next)
    {
        if (current->is_free)
        {
            current_size += current->size - metaSize;
        }
    }
    return current_size;
}
size_t _num_allocated_bytes()
{
    size_t metaSize = sizeof(MallocMetadata);
    size_t current_size = 0;
    for (MallocMetadata *current = memory; current != nullptr; current = current->next)
    {
        current_size += current->size - metaSize;
    }
    return current_size;
}

