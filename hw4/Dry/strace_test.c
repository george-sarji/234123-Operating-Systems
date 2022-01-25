#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    // Get the number from input.
    int bytes = 0;
    scanf("%d", &bytes);
    // Allocate bytes.
    usleep(1);
    void *address = malloc(bytes);
    usleep(2);
    // Free and exit.
    // free(address);
    return 1;
}