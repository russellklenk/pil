#include <Windows.h>

void*
PIL_AllocGeneral
(
    size_t nbytes
)
{
    return malloc(nbytes);
}

void
PIL_FreeGeneral
(
    void *ptr
)
{
    free(ptr);
}

