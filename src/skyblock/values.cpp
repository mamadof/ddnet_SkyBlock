#include "values.h"

#include <engine/storage.h>

bool CSkyb::IsStandardString(const char *pStr)//checks if a string is standard or not
{
    while(*pStr)
    {
        if (!((*pStr >= 'a' && *pStr <= 'z') || (*pStr >= 'A' && *pStr <= 'Z') || (*pStr >= '0' && *pStr <= '9')))
            return false;
        pStr++;
    }
    return true;
}