#include "pil.h"

PIL_API(void)
PIL_GetVersion
(
    int32_t *o_major, 
    int32_t *o_minor, 
    int32_t *o_bugfix
)
{
    if (o_major ) *o_major  = PIL_VERSION_MAJOR;
    if (o_minor ) *o_minor  = PIL_VERSION_MINOR;
    if (o_bugfix) *o_bugfix = PIL_VERSION_BUGFIX;
}

PIL_API(char const*)
PIL_GetVersionString
(
    void
)
{
    return PIL_VERSION_STRING;
}

