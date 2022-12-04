#include "base64.h"

static int base64CharacterValue(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }
    else if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 26;
    }
    else if (c >= '0' && c <= '9')
    {
        return c - '0' + 52;
    }
    else if (c == '+')
    {
        return 62;
    }
    else if (c == '/')
    {
        return 63;
    }
    return -1;
}

static int decodeBase64Chunk(const char *base64Input, char *output)
{
    int base64Values[] = {-1, -1, -1, -1};

    for (size_t i = 0; i < 4; ++i)
    {
        base64Values[i] = base64CharacterValue(base64Input[i]);
    }

    int decodedChars = 1;
    output[0] = base64Values[0] << 2 | base64Values[1] >> 4;

    if (base64Values[2] != -1)
    {
        output[1] = (base64Values[1] & 0x0F) << 4 | base64Values[2] >> 2;
        decodedChars++;
    }
    if (base64Values[3] != -1)
    {
        output[2] = base64Values[2] << 6 | base64Values[3];
        decodedChars++;
    }

    return decodedChars;
}

int decodeBase64(const char *base64Input, char *output, size_t len)
{
    int totalBytesDecoded = 0;
    while (len > 0)
    {
        int bytesDecoded = decodeBase64Chunk(base64Input, output);
        output += bytesDecoded;
        base64Input += 4;
        totalBytesDecoded += bytesDecoded;
        len -= 4;
    }
    return totalBytesDecoded;
}