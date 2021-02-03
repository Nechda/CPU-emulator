#pragma once
#include "Asm/Asm.h"
#include "Tools/Logger.h"
#include "Tools/CallStack.h"

/**
\brief  ����� ��������� ��������, ������� ����� ���� ������� �� ��������� ������
*/
enum ProgramType
{
    PROG_UNDEFINED = 0,
    PROG_HELPER,
    PROG_COPMILATOR,
    PROG_DISASSEMBLER,
    PROG_CPU,
    PROG_RUN,
    PROG_TESTER,
};

/**
\brief  ����� ����� ����������������� �����
*/
enum ConfigParamType
{
    CONFIG_UNDEFINED_PARAM = -1,
    CONFIG_MEMORY_SIZE,
    CONFIG_NCOLS,
    CONFIG_NLINES,
    CONFIG_ENTRY_POINT,
    CONFIG_FONT_WIDTH,
    CONFIG_FONT_HEIGHT
};



/**
\brief  ���������, ����������� ������� ������ ��������� ������
*/
struct InputParams
{
    ProgramType programName = PROG_UNDEFINED;
    char* inputFilename = NULL;
    char* outputFilename = NULL;
    char* logFilename = "log.log";
    ui32 memorySize = 0;
    ui32 entryPoint = 0;
    bool noLogFileFlag = 0;
    bool useGraphMode = 0;
    bool useStepByStepMode = 0;
    struct
    {
        ui16 nCols = 80;
        ui16 nLines = 25;
        ui16 fontWidth = 8;
        ui16 fontHeight = 8;
    }Window;
};


/**
\brief  ������� ����� ���������� ��������� ������
*/
void parseConsoleArguments(int argc, char** argv, InputParams* ptrInParams);