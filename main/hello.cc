#include <stdio.h>

#include "pil.h"

int main
(
    int    argc, 
    char **argv
)
{
    (void) argc;
    (void) argv;
    printf("Hello, world!\n");
    printf("Linked against PIL version %s\n", PIL_VERSION_STRING);
    return 0;
}

