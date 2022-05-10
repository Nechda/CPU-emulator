#pragma once
#include "Asm/Asm.h"
#include "Tools/Argparser.h"
#include "Profiler/Profiler.h"


/*
\brief Коды ошибок, возвращаемые процессором
*/
enum CPUerror
{
    CPU_OK = 0,
    CPU_ERROR_INVALID_STRUCUTE,
    CPU_ERROR_INVALID_COMMAND,
    CPU_ERROR_EXCEPTION,
    CPU_ERROR_PC_OUT_OF_RANE,
    CPU_INVALID_INPUT_DATA,
    CPU_STATUS_COUNT
};

//#define CPU_GRAPH_MODE
//#define CPU_PROFILER
//#define DUMP_PRINT_MEMORY
//#define CPU_SMART_PRINT_MEMORY

class CPU
{
    public:
        static CPU& Instance()
        {
            static CPU theInstance;
            return theInstance;
        }
        void init(InputParams inParam);
        CPUerror run(const char* bytes, ui32 size, ui32 insert_point);
        void dump(Stream outStream);
        ~CPU();
    private:
        CPU() {};
        CPU(const CPU&) = delete;
        CPU& operator=(const CPU) = delete;
        CPUerror evaluate();
};


C_string getStringByErrorCode(CPUerror errorCode);

