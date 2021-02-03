#include "LibStr.h"
#include "Logger.h"
#include "CallStack.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>


/**
\brief  ������� ��������� ��������� ����
\param  [in]      filename  ��� ������������ �����
\param  [in,out]  outString ��������� �� ��������� ������
\param  [in]      readBytesPtr  ��������� �� unsigned, � ������� ����� ��������� ���������� ��������� ������
\return � ������ ������ ������������ ���������� ����������� ����.
���� ��������� ������, �� ������������ ��������� -1.
*/
int readFullFile(const char* filename, char** outString)
{$
    assert(filename);
    assert(outString);
    if (!filename || !outString)
    {
        $$$("NULL ptr in filename or outString");
        return STANDART_ERROR_CODE;
    }

    FILE* inputFile = fopen(filename, "rb");
    assert(inputFile);
    if (!inputFile)
    {
        $$$("Can't read file");
        return STANDART_ERROR_CODE;
    }
    if (ferror(inputFile))
    {
        $$$("Stream related to the file has an error");
        return STANDART_ERROR_CODE;
    }

    fseek(inputFile, 0, SEEK_END);
    long fsize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    char* string = (char*)calloc(fsize + 8, sizeof(char));
    assert(string);
    if (!string)
    {
        $$$("Can't allocate memory for out string");
        return STANDART_ERROR_CODE;
    }

    unsigned nReadBytes = fread(string, sizeof(char), fsize, inputFile);
    fclose(inputFile);
    string[fsize] = 0;

    *outString = string;

    $$
    return nReadBytes;
}


/**
\brief  ������� ������� �� ������ ��� ���� �������, ����� ���, ������� ������� � ������ dontDelChar
\param  [in,out]      ptrStr  ��������� �� ������ � ������� ����� ����������� ��������
\param  [in,out]      dontDelChar ������, �������� ����� ��������, ������� �� ������� �������
\return � ������ ������ ������������ ����� ����� ������.
���� ��������� ������, �� ������������ ��������� ASM_ERROR_CODE.
\note   � ����� ������ ����� ������ �����,����� � ������� �� ������ dontDelChar
*/
int removeExtraChar(char** ptrStr, const char* dontDelChar)
{
    $
    Assert_c(ptrStr);
    Assert_c(dontDelChar);
    if (!ptrStr || !dontDelChar)
    {
        $$$("NULL in ptrStr or dontDelChar");
        return STANDART_ERROR_CODE;
    }
    char* str = *ptrStr;
    Assert_c(str);
    if (!str)
    {
        $$$("NULL in *ptrStr");
        return STANDART_ERROR_CODE;
    }

    unsigned i = 0;
    unsigned j = 0;

    bool isStrChar = 0;
#define isValid(x) (isalpha(x) || isdigit(x) || strchr(dontDelChar , x) )
    while (str[j])
    {
        if (str[j] == '\"')
        {
            isStrChar ^= 1;
            j++;
            continue;
        }
        if (isValid(str[j]) || isStrChar)
        {
            str[i] = str[j];
            i++;
        }
        j++;
    }
    str[i++] = 0;
    i++;
#undef isValid
    str = (char*)realloc(str, i);
    Assert_c(str);
    if (!str)
    {
        $$$("Can't allocate memeory for new string");
        return STANDART_ERROR_CODE;
    }

    *ptrStr = str;

    $$
    return i;
}


