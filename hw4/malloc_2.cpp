#include <unistd.h>

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
                // Appropriate block. Return address.
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
    if(memory != nullptr)
    {
        // We do have memory. We have to set the new block as the tail of the linked list.
        MallocMetadata* current = memory;
        while(current->next != nullptr) current = current->next;
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