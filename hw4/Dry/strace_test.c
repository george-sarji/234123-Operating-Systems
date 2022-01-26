#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int bytes = atoi(argv[1]);
    sleep(1);
    malloc(bytes);
    sleep(1);
    return 1;
}