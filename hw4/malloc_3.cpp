#include <unistd.h>
#include <cstring>

#define MAX_SIZE 1e8
#define HIST_SIZE 128
#define KILOBYTE 1024

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

MallocMetadata *malloc = nullptr;

MallocMetadata *histogram[HIST_SIZE];

/* Histogram functions */

int histogramIndex(MallocMetadata *data)
{
    // Iterate through the buckets and go per size.
    for (int i = 0; i < HIST_SIZE; i++)
    {
        // Does the data belong in the current bucket, according to the size?
        if (data->size >= i * KILOBYTE && data->size < (i + 1) * KILOBYTE)
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
        int bucket_index = histogramIndex(data);
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
    int index = histogramIndex(data);
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
}

/* Helper functions */

void splitBlock(MallocMetadata *block, size_t new_size)
{
    // We want to split the block into two portions; size and the rest.
    // Remove the current block from the histogram.
    histogramRemove(block);
    // We want to create a new histogram with the required size.
    size_t second_size = block->size - new_size;
    // Resize current block.
    block->size = new_size;
    // Get the new block from the secondary data in the previous block.
    MallocMetadata *new_data = (MallocMetadata *)(block + new_size + sizeof(MallocMetadata));
    // Set the required parameters.
    new_data->is_free = true;
    new_data->size = second_size;
    new_data->allocated_addr = new_data + sizeof(MallocMetadata);
    // Assign new_data's neighbor
    new_data->prev = block;
    new_data->next = block->next;
    // Update block's neighbor's prev.
    if (block->next->prev != nullptr)
    {
        block->next->prev = new_data;
    }

    // Update block's neighbor.
    block->next = new_data;
    // Insert both blocks into the histogram.
    histogramInsert(block);
    histogramInsert(new_data);
}

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
        block->size += next->size;
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
        previous->size += block->size;
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

/* --------------------------------------------------- Functions --------------------------------------------------- */


void *smalloc(size_t size)
{

    if (size == 0 || size > MAX_SIZE)
    {
        return NULL;
    }
    // We need to check if the current list has any available blocks.
    MallocMetadata *current = malloc;
    MallocMetadata *previous = current;
    while (current != nullptr)
    {
        if (current->is_free && current->size >= size + sizeof(MallocMetadata))
        {
            // We found an appropriate block. We can assign here.
            // Check if the remaining size is bigger than 128 bytes.
            if (current->size - size >= 128)
            {
                // We need to split the blocks.
                splitBlock(current, size);
            }
            // Remove the block from the histogram.
            histogramRemove(current);
            current->is_free = false;
            return current + sizeof(MallocMetadata);
        }
        previous = current;
        current = current->next;
    }
    // If we reached here - we don't have any appropriate blocks.
    // Check if wilderness chunk is free.
    if (previous->is_free)
    {
        // We have a free wilderness chunk. We can extend it to host the required data.
        size_t required_size = size - previous->size;
        // Use sbrk.
        void *extension = sbrk(required_size);
        if (extension == (void *)(-1))
        {
            // Couldn't extend with sbrk. Exit.
            return nullptr;
        }
        // Extend the size inside the wilderness chunk.
        previous->size = size;
        // Set as used.
        previous->is_free = false;
        // Remove the wilderness chunk from the histogram.
        histogramRemove(previous);
        return previous->allocated_addr;
    }
    // Allocate the block as required.
    MallocMetadata *new_address = (MallocMetadata *)sbrk(size + sizeof(MallocMetadata));
    // Check if successful.
    if (new_address == (void *)(-1))
    {
        return NULL;
    }
    // Set the required parameters for the newly allocated block.
    new_address->is_free = false;
    new_address->size = size;
    new_address->prev = nullptr;
    new_address->next = nullptr;
    new_address->allocated_addr = new_address + sizeof(MallocMetadata);
    // Successful allocation. Check if we have an allocation list.
    if (malloc != nullptr)
    {
        // We have an allocation list.
        // Get the end item and add the allocation to the list.
        current = malloc;
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
        malloc = new_address;
    }
    return new_address->allocated_addr;
}

void *scalloc(size_t num, size_t size)
{
    // We are looking for a continous free area, num blocks, each of at least size bytes.
    if (size == 0 || size * num > MAX_SIZE)
        return NULL;
    // Check if we have malloc in the first place.
    int new_size = size * num;
    bool zero_flag = true;
    if (malloc)
    {
        // Iterate through the meta data to get the required block.
        for (MallocMetadata *data = malloc; data != nullptr; data = data->next)
        {
            zero_flag = true;
            char *current_char = (char *)(data->allocated_addr);
            // Check if the current block is free and is appropriate.
            if (data->is_free && data->size >= num * (size))
            {
                // Iterate through the block.
                for (int i = 0; i < new_size; i++)
                {
                    if (*(current_char + i) != '0')
                    {
                        // Not a zero. Set flag and break.
                        zero_flag = false;
                        break;
                    }
                }
                // Check if we got a valid flag.
                if (zero_flag)
                {
                    // Valid zero flag = valid block. Split the block as needed.
                    splitBlock(data, num * size);
                    // Remove the block from the histogram.
                    histogramRemove(data);
                    // Set the block as used.
                    data->is_free;
                    // Return the block address.
                    return data->allocated_addr;
                }
            }
        }
    }

    // If we reached this point - we don't have any blocks that are appropriate.
    // Use smalloc to allocate a block of size num * size.
    void *new_address = smalloc(size * num);
    // Did we get a successful allocation?
    if (new_address == nullptr)
    {
        return nullptr;
    }
    // Set the memory block to zeros.
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
        // Size is not appropriate. Allocate new block.
        void *new_address = smalloc(size);
        // Check for successful allocation.
        if (!new_address == NULL)
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
