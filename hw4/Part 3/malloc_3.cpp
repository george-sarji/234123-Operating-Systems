#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>

#define MAX_SIZE 1e8
#define HIST_SIZE 128
#define KILOBYTE 1024
#define MAP_MIN 128 * KILOBYTE

struct MallocMetadata
{
    size_t size;
    bool is_free;
    MallocMetadata *next;
    MallocMetadata *prev;
    void *allocated_addr;

    // Histogram pointers (buckets)
    MallocMetadata *hist_prev;
    MallocMetadata *hist_next;
};

MallocMetadata *metadata = nullptr;
MallocMetadata *mmap_list = nullptr;
MallocMetadata *histogram[HIST_SIZE];

/* Histogram functions */

int histogramIndex(size_t size)
{
    // Iterate through the buckets and go per size.
    for (int i = 0; i < HIST_SIZE; i++)
    {
        // Does the data belong in the current bucket, according to the size?
        if (size >= i * KILOBYTE && size < (i + 1) * KILOBYTE)
        {
            return i;
        }
    }
    return HIST_SIZE - 1;
}

void histogramRemove(MallocMetadata *data)
{
    // We need to remove the data from its bucket in the histogram.
    // Check if we have a previous node in the histogram.
    if (data->hist_prev != nullptr)
    {
        // We have a previous entry. Skip the current entry.
        data->hist_prev->hist_next = data->hist_next;
    }
    else
    {
        // First entry in the bucket.
        // Get the bucket's index.
        int bucket_index = histogramIndex(data->size);
        // Assign the bucket's entry to the next entry.
        histogram[bucket_index] = data->hist_next;
    }
    // Check if we have another entry after this.
    if (data->hist_next != nullptr)
    {
        // We do. Set it's previous to the current's previous.
        data->hist_next->hist_prev = data->hist_prev;
    }
    // Remove from the bucket by removing next and previous from node.
    data->hist_prev = data->hist_next = nullptr;
}

void histogramInsert(MallocMetadata *data)
{
    // Get the required bucket index.
    int index = histogramIndex(data->size);
    // Check if the current bucket is valid.
    MallocMetadata *current = histogram[index];
    if (current != nullptr)
    {
        bool valid_addition = false;
        MallocMetadata *previous = current;
        // Valid bucket. Iterate through the entries and assign.
        while (current != nullptr)
        {
            // Check if the size is appropriate.
            if (current->size >= data->size)
            {
                valid_addition = true;
                // We found an appropriate spot to add the data, before current.
                // Do we have an entry before the current bucket?
                if (current->hist_prev == nullptr)
                {
                    // No previous node, bucket entry.
                    current->hist_prev = data;
                    data->hist_next = current;
                    // Update the bucket entry.
                    histogram[index] = data;
                    break;
                }
                else
                {
                    // We have a previous entry.
                    // Update our new entry.
                    data->hist_prev = current->hist_prev;
                    data->hist_next = current;

                    // Update previous entry.
                    current->hist_prev->hist_next = data;

                    // Update current entry.
                    current->hist_prev = data;
                }
                // We found an appropriate bucket. Exit.
                break;
            }
            previous = current;
            current = current->next;
        }
        // Check if we actually added the item.
        if (!valid_addition)
        {
            // We didn't add it to the bucket because we reached an end. That means it should be the last in the list.
            previous->hist_next = data;
            data->hist_prev = previous;
            data->hist_next = nullptr;
        }
    }
    else
    {
        // We have to initiate the histogram.
        histogram[index] = data;
    }
}

/* Helper functions */

void uniteBlock(MallocMetadata *block)
{
    // We need to check if we have blocks that are adjacent to unite.
    // Check the next block.
    MallocMetadata *next = block->next;
    if (next != nullptr && next->is_free)
    {
        // Merge the two blocks into current.
        // Remove the blocks from the histogram.
        histogramRemove(block);
        histogramRemove(next);
        // Update the size of the current block.
        block->size += next->size + sizeof(MallocMetadata);
        // Update the pointers to skip over next.
        block->next = next->next;
        // Update the other pointer to correctly point at block.
        if (next->next != nullptr)
        {
            next->next->prev = block;
        }
        // Re-insert the block into the histogram.
        histogramInsert(block);
    }

    // Check the previous.
    MallocMetadata *previous = block->prev;
    if (previous != nullptr && previous->is_free)
    {
        // Merge the blocks into previous.
        // Remove the blocks from the histogram
        histogramRemove(block);
        histogramRemove(previous);
        // Update the size of the previous block.
        previous->size += block->size + sizeof(MallocMetadata);
        // Update the next block.
        previous->next = block->next;
        // Update the other pointer to correctly point at previous.
        if (block->next != nullptr)
        {
            block->next->prev = previous;
        }
        // Re-add the previous block to the histogram.
        histogramInsert(previous);
    }
}

void splitBlock(MallocMetadata *block, size_t new_size)
{
    // We want to split the block into two portions; size and the rest.
    // Remove the current block from the histogram.
    histogramRemove(block);
    // We want to create a new histogram with the required size.
    size_t second_size = block->size - new_size - sizeof(MallocMetadata);
    // Resize current block.
    block->size = new_size;
    // Get the new block from the secondary data in the previous block.
    MallocMetadata *new_data = (MallocMetadata *)(block->allocated_addr + new_size);
    // Set the required parameters.
    new_data->is_free = true;
    new_data->size = second_size;
    new_data->allocated_addr = new_data + sizeof(MallocMetadata);
    // Assign new_data's neighbor
    new_data->prev = block;
    new_data->next = block->next;
    // Update block's neighbor's prev.
    if (block->next != nullptr)
    {
        block->next->prev = new_data;
    }

    // Update block's neighbor.
    block->next = new_data;
    // Insert both blocks into the histogram.
    histogramInsert(block);
    histogramInsert(new_data);
    // Unite to prevent unmerged fragmentation
    uniteBlock(new_data);
}

void *allocateNewMap(size_t size)
{

    MallocMetadata *current = metadata;
    MallocMetadata *new_block;
    // Allocate a new block using memory mapping
    new_block = ((MallocMetadata *)mmap(nullptr, size + sizeof(MallocMetadata), PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0));
    if (new_block == (void *)-1)
    {
        return nullptr;
    }
    // Set the properties of the entry.
    new_block->size = size;
    new_block->allocated_addr = ((char *)new_block) + sizeof(MallocMetadata);
    new_block->is_free = false;
    // Do we have a memory mapping list?
    if (mmap_list == nullptr)
        mmap_list = new_block;
    else
    {
        // Iterate through the memory mapping list and get the last entry to add the new entry.
        current = mmap_list;
        while (current->next != nullptr)
        {
            current = current->next;
        }
        current->next = new_block;
        new_block->prev = current;
        new_block->next = nullptr;
    }
    return new_block->allocated_addr;
}

void freeInMap(void *p)
{

    MallocMetadata *current = mmap_list;
    // Iterate through the memory mapping list.
    while (current != nullptr)
    {
        // Is this the same allocation?
        if (current->allocated_addr == p)
        {
            // Yes. Skip over block.
            current->prev->next = current->next;
            if (current->next != nullptr)
                current->next->prev = current->prev;
            if (current->prev == nullptr)
                mmap_list = current->next;
            // Unmap the entry.
            munmap(current, current->size + sizeof(MallocMetadata));
            return;
        }
        current = current->next;
    }
}

/* --------------------------------------------------- Functions --------------------------------------------------- */

void *smalloc(size_t size)
{

    if (size == 0 || size > MAX_SIZE)
    {
        return NULL;
    }
    if (size >= MAP_MIN)
    {
        // Allocate in the memory mapping.
        return allocateNewMap(size);
    }
    // We need to check if the histogram has any appropriate blocks.
    MallocMetadata *current, *previous;
    for (int i = histogramIndex(size); i < HIST_SIZE; i++)
    {
        current = previous = histogram[i];
        while (current != nullptr)
        {
            if (current->size >= size)
            {
                current->is_free = false;
                // We found an appropriate block. We can assign here.
                // Check if the remaining size is bigger than 128 bytes.
                if (current->size - size >= 128)
                {
                    // We need to split the blocks.
                    splitBlock(current, size);
                }
                // Remove the block from the histogram.
                histogramRemove(current);
                return current->allocated_addr;
            }
            previous = current;
            current = current->hist_next;
        }
    }
    // Check if we have the wilderness chunk.
    current = metadata;
    while (current != nullptr && current->next != nullptr)
    {
        current = current->next;
    }
    if (current != nullptr && current->is_free)
    {
        // We have a free wilderness chunk. We can extend it to host the required data.
        size_t required_size = size - current->size;
        // Use sbrk.
        void *extension = sbrk(required_size);
        if (extension == (void *)(-1))
        {
            // Couldn't extend with sbrk. Exit.
            return nullptr;
        }
        // Remove the current chunk from the histogram.
        histogramRemove(current);
        // Extend the size inside the current chunk.
        current->size = size;
        // Set as used.
        current->is_free = false;
        return current->allocated_addr;
    }

    // If we reached here - we don't have any appropriate blocks in the histogram.
    // Allocate the block as required.
    MallocMetadata *new_address = (MallocMetadata *)sbrk(sizeof(MallocMetadata));
    // Check if successful.
    if (new_address == (void *)(-1))
    {
        return NULL;
    }

    void *start_address = sbrk(size);
    if (start_address == (void *)-1)
    {
        return NULL;
    }
    // Set the required parameters for the newly allocated block.
    new_address->is_free = false;
    new_address->size = size;
    new_address->prev = new_address->next = nullptr;
    new_address->hist_prev = new_address->hist_next = nullptr;
    new_address->allocated_addr = start_address;
    // Successful allocation. Check if we have an allocation list.
    if (metadata != nullptr)
    {
        // We have an allocation list.
        // Get the end item and add the allocation to the list.
        MallocMetadata *current = metadata;
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
    {
        // We don't have a list. Set it as the new one.
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
    freeInMap(p);
    char *current = (char *)p - sizeof(MallocMetadata);
    MallocMetadata *block = (MallocMetadata *)current;
    block->is_free = true;
    // Add the block to the histogram.
    histogramInsert(block);
    uniteBlock(block);
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
        if (size >= MAP_MIN)
        {
            sfree(current);
            return allocateNewMap(size);
        }
        // Size is not appropriate. Attempt merging with adjacent blocks.
        MallocMetadata *previous = current->prev, *next = current->next, *new_block;
        bool overwrite = false;
        if (previous != nullptr && previous->is_free && previous->size + current->size >= size)
        {
            // Appropriate to merge with previous.
            // Remove previous and current from histogram.
            histogramRemove(previous);
            histogramRemove(current);
            overwrite = size > previous->size;
            // We need to merge. Set the new size.
            previous->size += current->size + sizeof(MallocMetadata);
            // We need to update the pointers.
            // Previous's pointers, next goes to current's next.
            previous->next = next;
            // Set the previous of next (is relevant)
            if (next != nullptr)
            {
                next->prev = previous;
            }
            new_block = previous;
            new_block->is_free = false;
        }
        else if (next != nullptr && next->is_free && next->size + current->size >= size)
        {
            // Appropriate to merge with next.
            // Remove next and current from histogram.
            histogramRemove(next);
            histogramRemove(current);
            // We need to merge. Set the new size.
            current->size += next->size + sizeof(MallocMetadata);
            // We need to update the pointers.
            // current's pointers, next goes to next's next.
            current->next = next->next;
            // Set the next of next (is relevant)
            if (next->next != nullptr)
            {
                next->next->prev = current;
            }
            new_block = current;
            new_block->is_free = false;
        }
        else if (next != nullptr && previous != nullptr &&
                 next->is_free && previous->is_free &&
                 previous->size + current->size + next->size >= size)
        {
            // Merge with previous and next.
            // Remove previous and current and next from histogram.
            histogramRemove(previous);
            histogramRemove(current);
            histogramRemove(next);
            // We need to merge. Set the new size.
            previous->size += current->size + next->size + 2 * sizeof(MallocMetadata);
            // We need to update the pointers.
            // Previous's pointers, next goes to next's next.
            previous->next = next->next;
            // Set the previous of next's next (is relevant)
            if (next->next != nullptr)
            {
                next->next->prev = previous;
            }
            new_block = previous;
            new_block->is_free = false;
        }
        else if (current->next == nullptr)
        {
            // We have the wilderness chunk. Merging did not help.
            // We can extend it to host the required data.
            size_t required_size = size - current->size;
            // Use sbrk.
            void *extension = sbrk(required_size);
            if (extension == (void *)(-1))
            {
                // Couldn't extend with sbrk. Exit.
                return nullptr;
            }
            // Remove the previous chunk from the histogram.
            histogramRemove(current);
            // Extend the size inside the previous chunk.
            current->size = size;
            // Set as used.
            current->is_free = false;
            new_block = current;
        }
        else
        {
            // Can't merge. Allocate new.
            void *new_address = smalloc(size);
            // Check for successful allocation.
            if (new_address == nullptr)
            {
                return nullptr;
            }
            // Assistive cast.
            char *new_allocation = (char *)new_address - sizeof(MallocMetadata);
            new_block = (MallocMetadata *)new_allocation;
        }
        // Set the current block as used.
        new_block->is_free = false;
        // Check if we can perform splitting.
        if (new_block->size - size >= 128)
        {
            splitBlock(new_block, size);
        }
        
        // Copy the data from the old block into the new.
        // If we are overwriting the old data, only copy.
        memcpy(new_block->allocated_addr, current->allocated_addr, current->size);
        // Free the old block.
        if (!overwrite && current->allocated_addr != new_block->allocated_addr)
        {
            sfree(oldp);
        }
        return new_block->allocated_addr;
    }
    else
    {
        // Size is appropriate. Reuse the current block.
        current->is_free = false;
        // Check if we have to split.
        if (current->size - size >= 128)
        {
            splitBlock(current, size);
        }
        histogramRemove(current);
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
    MallocMetadata *mapped = mmap_list;
    while (mapped)
    {
        counter++;
        mapped = mapped->next;
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
    MallocMetadata *mapped = mmap_list;
    while (mapped)
    {
        counter += mapped->size;
        mapped = mapped->next;
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
