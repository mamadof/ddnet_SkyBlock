#include "values.h"

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
bool CSkyb::IsNumberString(const char *pStr)//checks if a string is standard or not
{
    while(*pStr)
    {
        if (!(*pStr >= '0' && *pStr <= '9'))
            return false;
        pStr++;
    }
    return true;
}
int CSkyb::HSLAToInt(int H, int S, int L, int Alpha)
{
        int color = 0;
        color = (color & 0xFF00FFFF) | (H << 16);
        color = (color & 0xFFFF00FF) | (S << 8);
        color = (color & 0xFFFFFF00) | L;
        color = (color & 0x00FFFFFF) | (Alpha << 24);
        return color;
}