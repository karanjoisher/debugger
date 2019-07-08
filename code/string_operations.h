#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

bool AreStringsSame(char *str1, char *str2)
{
    while(*str1 != 0 && *str2 != 0)
    {
        if(*str1++ != *str2++)
        {
            return false;
        }
    }
    
    return *str1 == *str2;
}

bool IsSubstring(char *testSubstring, char *str)
{
    while(*testSubstring != 0)
    {
        if(*str == 0)
        {
            INVALID_CODE_PATH;
        }
        
        if(*testSubstring++ != *str++)
        {
            return false;
        }
    }
    
    return true;
}

inline u32 GetStringLength(char *str)
{
    u32 result = 0;
    while(*str++ != 0)
    {
        result++;
    }
    return result;
}

void ConcateToPath(char *dir, char* filename, char* result)
{
    while(*dir)
    {
        *result++ = *dir++;
    }
    
    if(*(result - 1) != '/' || *(result - 1) != '\\')
    {
        *result++ = '/';
    }
    
    while(*filename)
    {
        *result++ = *filename++;
    }
    *result = 0;
}

s64 GetFirstIndex(char *str, char val)
{
    s64 result = -1;
    for(s32 i = 0; *str != 0; str++, i++)
    {
        if(*str == val)
        {
            result = i;
            break;
        }
    }
    return result;
}

u64 InterpretStringAsUint64(char *str, u32 length = 0)
{
    u64 value = 0;
    if(length == 0)
    {
        length = GetStringLength(str);
    }
    u32 originalLength = length;
    
    
    u64 base = 10;
    if(length > 0 && str[0] == '0')
    {
        base = 8;
        length--;
        if(length > 0)
        {
            if(str[1] == 'b')
            {
                base = 2;
                length--;
            }
            else if(str[1] == 'x')
            {
                base = 16;
                length--;
            }
        }
    }
    str = str + (originalLength - 1);
    u64 multiplier = 1;
    for(u32 i = 0; i < length; i++)
    {
        u64 digit = 0;
        if(*str >= '0' && *str <= '9')
        {
            digit = *str - '0';
        }
        else
        {
            char offsetFrom = 'a';
            if(*str >= 'A' && *str <= 'F')
            {
                offsetFrom = 'A';
            }
            digit = (*str - offsetFrom) + 10;
        }
        
        str--;
        value += digit * multiplier;
        multiplier *= base;
    }
    
    return value;
}


#endif //STRING_OPERATIONS_H