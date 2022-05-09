#pragma once
#include <stdio.h>
#include "Tools/Logger.h"
#include "Tools/Support.h"
#include "Types.h"
#include <vector>


typedef ui16 Mcode;
const int ASM_ERROR_CODE = -1;

using std::vector;


namespace Assembler
{
    /*
    \brief Коды ошибок, возвращаемые компилятором и дизасемблером
    */
    enum AsmError
    {
        ASM_OK = 0,
        ASM_ERROR_INVALID_INPUT_DATA,
        ASM_ERROR_OUT_OF_MEMORY,
        ASM_ERROR_GEN_LABLE_TABLE,
        ASM_ERROR_GEN_MACHINE_CODE,
        ASM_ERROR_CANT_WRITE_INTO_FILE,
        ASM_ERROR_INVALID_SYNTAX,
        ASM_ERROR_INVALID_MACHINE_CODE,
        ASM_ERROR_INVALID_OPERANDS_NUMBER,
        ASM_ERROR_INVALID_OPERAND_SYNTAX,
        ASM_ERROR_INVALID_OPERAND_TYPE_FOR_COMMAND,
        ASM_ERROR_CANT_READ_LEXEMA
    };
    C_string getStringByErrorCode(AsmError errorCode); 

    struct Instruction;
    enum class Opcode;

    /*
    \brief Допустимые типы операндов
    */
    enum OperandType
    {
        OPERAND_REGISTER,
        OPERAND_NUMBER,
        OPERAND_MEMORY,
        OPERAND_MEM_BY_REG
    };

    /*
    \brief Посредствам данной стуктуры реализуется "команда" процессора
    */
    struct Instruction
    {
        using IType = i32;
        using FType = float;
        union
        {
            ui16 marchCode;
            struct
            {
                ui8 nOperands  : 2;
                ui8 typeFirst  : 2;
                ui8 typeSecond : 2;
                ui8 typeThird  : 2;
                ui8 longCommand: 1;
                ui8 reserved   : 1;
                ui8 opCode     : 6;
            };
        }bits;
        ui32 operand[3] = { 0, 0, 0 };
        ui32 extend[3] = { 0, 0, 0 };
        ui8 sizeCommand = 0;
        OperandType get_operand_type(int idx);
        void set_operand_type(int opIndex, OperandType type);
        Pointer ops[3] = {};
    };

    enum class Opcode
    {
        #define DEF(name, mCode, vStr1, vSrt2, vStr3, code)\
            name,
            #include "Extend.h"
        #undef DEF
    };

    class Disassembler
    {
        public:
            static AsmError generateCommandList(vector<Instruction>& commands, vector<ui8>& bytesFromDataSection, i8* bytes, i32 nBytes);
            static AsmError disasm(const ui8* code, int size, FILE* stream);
            static void disasmCommand(Instruction cmd, FILE* stream);
        private:
            static AsmError getCode(ui8* bytes, ui32 nBytes, FILE* outStream);
            Disassembler() {};
            Disassembler(const Disassembler&) = delete;
            Disassembler& operator=(const Disassembler&) = delete;
            ~Disassembler() {};
    };

    class Compilator
    {
        public:
            static AsmError compile(const char* code, FILE* outStream);
        private:
            Compilator() {};
            Compilator(const Compilator&) = delete;
            Compilator& operator=(const Compilator&) = delete;
    };

}
