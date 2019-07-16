#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H


struct Cursor
{
    char *at;
};


inline bool IsEndOfLine(char c)
{
    bool result = ((c == '\n') ||
                   (c == '\r'));
    
    return(result);
}

inline bool IsWhitespace(char c)
{
    bool result = ((c == ' ') ||
                   (c == '\t') ||
                   (c == '\v') ||
                   (c == '\f') ||
                   IsEndOfLine(c));
    
    return(result);
}

inline bool IsAlpha(char c)
{
    bool result = ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
    return result;
}

inline bool IsDigit(char c)
{
    bool result = ((c >= '0') && (c <= '9'));
    return result;
}

inline void  EatAllWhitespace(Cursor *cursor)
{
    while((*(cursor->at)!= 0) && IsWhitespace(*(cursor->at))) ++(cursor->at);
}


inline void  EatUntilWhitespace(Cursor *cursor)
{
    while((*(cursor->at)!= 0) && !IsWhitespace(*(cursor->at))) ++(cursor->at);
}


inline void  EatUntilEndOfLine(Cursor *cursor)
{
    while((*(cursor->at)!= 0) && !IsEndOfLine(*(cursor->at))) ++(cursor->at);
}



bool MatchAndAdvance(Cursor *cursor, char *match)
{
    Cursor original = *cursor;
    while(*match != 0)
    {
        if(*(cursor->at)++ != *match++)
        {
            *cursor = original;
            return false;
        }
    }
    
    return true;
}

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

bool IsPrefix(char *str, char* prefix)
{
    while(*prefix != 0)
    {
        if(*prefix++ != *str++)
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


enum DataType
{
    invalid_type,
    numeric_binary,
    numeric_octal,
    numeric_hexadecimal,
    numeric_decimal_integer,
    numeric_decimal_real,
    string
};

DataType GetDataType(char *str)
{
    DataType type = invalid_type;
    
    if(*str == '\"')
    {
        type = string;
    }
    else if(*str == '0')
    {
        if(str[1] == 'b')
        {
            type = numeric_binary;
        }
        else if(str[1] == 'x')
        {
            type = numeric_hexadecimal;
        }
        else if(str[1] == '.')
        {
            type = numeric_decimal_real;
        }
        else if(IsDigit(str[1]))
        {
            type = numeric_octal;
        }
        else
        {
            type = numeric_decimal_integer;
        }
    }
    else
    {
        s64 decimalPoint = GetFirstIndex(str, '.');
        if(decimalPoint == -1)
        {
            type = numeric_decimal_integer;
        }
        else
        {
            type = numeric_decimal_real;
        }
    }
    
    return type;
}



// TODO(Karan): Test these string to numeric/string conversions!!
u64 InterpretHexadecimalAs64BitData(char *str, u32 strLength = 0)
{
    if(strLength == 0)
    {
        char *cursor = str;
        while(IsAlpha(*cursor) || IsDigit(*cursor))
        {
            strLength++;
            cursor++;
        }
    }
    
    u64 result = 0;
    u64 multiplier = 1;
    char *hexadecimal = str + strLength - 1;
    while((*hexadecimal != 'x') && (hexadecimal != (str - 1)))
    {
        u64 val = 0;
        if((*hexadecimal >= 'a') && (*hexadecimal <= 'f'))
        {
            val = (*hexadecimal - 'a') + 10;
        }
        else if((*hexadecimal >= 'A') && (*hexadecimal <= 'F'))
        {
            val = (*hexadecimal - 'A') + 10;
        }
        else
        {
            val = *hexadecimal - '0';
        }
        result += val * multiplier;
        multiplier *= 16;
        hexadecimal--;
    }
    
    return result;
}

u64 InterpretOctalAs64BitData(char *str, u32 strLength = 0)
{
    if(strLength == 0)
    {
        char *cursor = str;
        while(IsDigit(*cursor))
        {
            strLength++;
            cursor++;
        }
    }
    
    u64 result = 0;
    u64 multiplier = 1;
    char *octal = str + strLength - 1;
    while(octal != str)
    {
        result += (*octal-- - '0') * multiplier;
        multiplier *= 8;
    }
    
    return result;
}


u64 InterpretBinaryAs64BitData(char *str, u32 strLength = 0)
{
    if(strLength == 0)
    {
        char *cursor = str;
        if(cursor[0] == '0' && cursor[1] == 'b')
        {
            cursor += 2;
            strLength += 2;
        }
        
        while(IsDigit(*cursor))
        {
            strLength++;
            cursor++;
        }
    }
    
    
    u64 result = 0;
    u64 multiplier = 1;
    char *binary = str + strLength - 1;
    while((*binary != 'b') && (binary != (str - 1)))
    {
        result += (*binary-- - '0') * multiplier;
        multiplier *= 2;
    }
    
    return result;
}

u64 InterpretDecimalIntegerAs64BitData(char *str, u32 strLength = 0)
{
    if(strLength == 0)
    {
        char *cursor = str;
        while(IsDigit(*cursor))
        {
            strLength++;
            cursor++;
        }
    }
    
    u64 result = 0;
    u64 multiplier = 1;
    char *number = str + strLength - 1;
    while((*number != '-') && (number != (str - 1)))
    {
        result += (*number-- - '0') * multiplier;
        multiplier *= 10;
    }
    
    if(*str == '-')
    {
        s64 negated = -1 * ((s64)result);
        ArrayCopy(&negated, &result, sizeof(result));
    }
    
    return result;
}


u64 InterpretRealAs64BitData(char *str, u32 strLength = 0)
{
    if(strLength == 0)
    {
        char *cursor = str;
        while(IsDigit(*cursor) || *cursor == '.' || *cursor == 'f' || *cursor == 'F')
        {
            strLength++;
            cursor++;
        }
    }
    
    f64 data = 0.0;
    f64 multiplier = 1.0;
    
    s64 decimalPoint = GetFirstIndex(str, '.');
    char *integerPart;
    char *fractionPart;
    if(decimalPoint == -1)
    {
        integerPart = str + strLength - 1;
        fractionPart = str + strLength;
    }
    else
    {
        integerPart = str + (decimalPoint - 1);
        fractionPart = str + (decimalPoint + 1);
    }
    
    while((*integerPart != '-') && (integerPart != str - 1))
    {
        f64 val = (f64)(*integerPart-- - '0');
        data += val * multiplier;
        multiplier *= 10.0;
    }
    
    multiplier = 0.1;
    while((fractionPart != (str + strLength)) && ((*fractionPart != 'f') && (*fractionPart != 'F')))
    {
        f64 val = (f64)(*fractionPart++ - '0');
        data += val * multiplier;
        multiplier *= 0.1;
    }
    if(*str == '-') data *= -1.0;
    
    u64 result = 0;
    if((*fractionPart == 'f') || (*fractionPart == 'F'))
    {
        f32 temp = (f32)data;
        ArrayCopy(&temp, &result, sizeof(temp));
    }
    else
    {
        ArrayCopy(&data, &result, sizeof(result));
    }
    return result;
}

void IdentifyStringifiedDataTypeAndWrite64BitData(char *str, u64 *dest, u32 strLength = 0)
{
    DataType type = GetDataType(str);
    switch(type)
    {
        case numeric_hexadecimal:
        {
            *dest = InterpretHexadecimalAs64BitData(str, strLength);
        }break;
        case numeric_octal:
        {
            *dest = InterpretOctalAs64BitData(str, strLength);
        }break;
        case numeric_binary:
        {
            *dest = InterpretBinaryAs64BitData(str, strLength);
        }break;
        case numeric_decimal_integer:
        {
            *dest = InterpretDecimalIntegerAs64BitData(str, strLength);
        }break;
        case numeric_decimal_real:
        {
            *dest = InterpretRealAs64BitData(str, strLength);
        }break;
        case string:
        {
            ArrayCopy(str + 1, dest, (u32)MIN(strLength - 2, sizeof(*dest)));
        }break;
        INVALID_DEFAULT_CASE;
    }
}



// NOTE(Karan): 32bit versions
u32 InterpretHexadecimalAs32BitData(char *str, u32 strLength = 0)
{
    u64 result64 = InterpretHexadecimalAs64BitData(str, strLength);
    u32 result = 0;
    ArrayCopy(&result64, &result, sizeof(result));
    return result;
}

u32 InterpretOctalAs32BitData(char *str, u32 strLength = 0)
{
    u64 result64 = InterpretOctalAs64BitData(str, strLength);
    u32 result = 0;
    ArrayCopy(&result64, &result, sizeof(result));
    return result;
}


u32 InterpretBinaryAs32BitData(char *str, u32 strLength = 0)
{
    u64 result64 = InterpretBinaryAs64BitData(str, strLength);
    u32 result = 0;
    ArrayCopy(&result64, &result, sizeof(result));
    return result;
}

u32 InterpretDecimalIntegerAs32BitData(char *str, u32 strLength = 0)
{
    u64 result64 = InterpretDecimalIntegerAs64BitData(str, strLength);
    u32 result = 0;
    ArrayCopy(&result64, &result, sizeof(result));
    return result;
}


u32 InterpretRealAs32BitData(char *str, u32 strLength = 0)
{
    u64 result64 = InterpretRealAs64BitData(str, strLength);
    u32 result = 0;
    ArrayCopy(&result64, &result, sizeof(result));
    return result;
}


void IdentifyStringifiedDataTypeAndWrite32BitData(char *str, u32 *dest, u32 strLength = 0)
{
    u64 result64 = 0;
    IdentifyStringifiedDataTypeAndWrite64BitData(str, &result64, strLength);
    ArrayCopy(&result64, dest, sizeof(*dest));
}

void InterpretHexadecimalAs80BitData(char *str, void *dest, u32 strLength = 0)
{
    if(strLength > 16)
    {
        u32 lowQuadStrLength = 16;
        u32 highQuadStrLength = strLength - lowQuadStrLength;
        
        char *highQuadStr = str;
        char *lowQuadStr = str + highQuadStrLength;
        
        u64 highQuad = InterpretHexadecimalAs64BitData(highQuadStr, highQuadStrLength);
        u64 lowQuad = InterpretHexadecimalAs64BitData(lowQuadStr, lowQuadStrLength); 
        
        ArrayCopy(&lowQuad, dest, sizeof(u64));
        dest = (((u8*)dest) + sizeof(u64));
        ArrayCopy(&highQuad, dest, sizeof(u16));
    }
    else
    {
        u64 *dest64  = (u64*)dest;
        *dest64 = InterpretHexadecimalAs64BitData(str, strLength);
    }
}

void InterpretBinaryAs80BitData(char *str, void *dest, u32 strLength = 0)
{
    if(strLength > 64)
    {
        u32 lowQuadStrLength = 64;
        u32 highQuadStrLength = strLength - lowQuadStrLength;
        
        char *highQuadStr = str;
        char *lowQuadStr = str + highQuadStrLength;
        
        u64 highQuad = InterpretBinaryAs64BitData(highQuadStr, highQuadStrLength);
        u64 lowQuad = InterpretBinaryAs64BitData(lowQuadStr, lowQuadStrLength); 
        
        ArrayCopy(&lowQuad, dest, sizeof(u64));
        dest = (((u8*)dest) + sizeof(u64));
        ArrayCopy(&highQuad, dest, sizeof(u16));
    }
    else
    {
        u64 *dest64  = (u64*)dest;
        *dest64 = InterpretBinaryAs64BitData(str, strLength);
    }
}
#endif //STRING_OPERATIONS_H