#include <cstring>
#include <cassert>
#include "Tools/Support.h"
#include "Stack/FlatStack.h"

#include "Asm/Asm.h"
#include "CPU/CPU.h"
#include "CPUInfo.h"

using namespace Assembler;
using std::memcpy;

typedef void(*PtrToFunction)(Assembler::Instruction*);

struct Context
{
    using ui128 = struct { ui32 b[4]; };
    Context() : stack(RAM, Register.esp) {}
    bool isValid = 0;
    bool stepByStep = 0;
    ui32 ramSize = 8;
    struct
    {
        ui32 eax; ui32 ebx; ui32 ecx; ui32 edx;
        ui32 esi; ui32 edi; ui32 esp; ui32 ebp;
        ui32 _;
        ui128 lr0; ui128 lr1; ui128 lr2; ui128 lr3;
        ui128 lr4; ui128 lr5; ui128 lr6; ui128 lr7;
    }Register;
    ui32 pc;
    ui8* RAM = NULL;
    FlatStack stack;
    static constexpr auto TOTAL_REGS = sizeof(Register) / sizeof(ui32);

    ui32* get_reg_ptr(ui32 idx) {
        if (idx > TOTAL_REGS) return nullptr;
        if (10 <= idx && idx <= 17)
            return (ui32*)&Register.lr0 + 4 * (idx - 10);
        else
            return (ui32*)&Register + (idx - 1);
    }
}context;

#ifdef CPU_SMART_PRINT_MEMORY
    #include <Windows.h>

    enum ConsoleColour
    {
        BLACK, BLUE, GREEN, CYAN,
        RED, MAGENTA, BROWN, LIGHTGRAY,
        DARKGRAY, LIGHTBLUE, LIGHTGREEN,
        LIGHTCYAN, LIGHTRED, LIGHTMAGENTA,
        YELLOW, WHITE
    };

    static void setConsoleColor(int fontColour)
    {
        WORD wColor = (fontColour & 0x0F);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
    }

    #define SET_DARK_GRAY setConsoleColor(8)
    #define SET_WHITE     setConsoleColor(15)
    #define RESET_COLOUR  setConsoleColor(7)

#endif

/*
\brief  Функция производит инициализацию структуры CPU
\param  [in]  ramSize  Размер виртуальной памяти процессора
\note   Если при инициализации структуры возникли ошибки, то полю isValid присваивается значение 0
Причина возникновения ошибки записывается в лог файл.
*/
void CPU::init(const InputParams inParam)
{
    context.ramSize = inParam.memorySize;
    assert(context.ramSize < (32 << 20));

    context.RAM = (ui8*)calloc(context.ramSize, sizeof(ui8));
    assert(context.RAM != nullptr);

    context.isValid = 1;
    context.stepByStep = inParam.useStepByStepMode;
}

CPUerror CPU::run(const char* bytes, ui32 size, ui32 insert_point)
{
    assert(context.isValid);
    assert(bytes);
    assert(size > 0);
    assert(insert_point + size + 1 < context.ramSize);

    // Copy binary file into CPU's RAM
    memcpy(&context.RAM[insert_point], bytes, size);
    // Setting up program counter
    context.pc = insert_point;
    // And the top of the stack
    context.Register.esp = size + 1;

    return evaluate();
}

CPU::~CPU()
{
    free(context.RAM);
    context.isValid = 0;
}

/*
\brief Функция, производящая дамп процессора, результат закидывается в outStream
\param [in] outStream указатель на поток вывода
*/
void CPU::dump(Stream outStream)
{
    if (outStream ? ferror(outStream) : 1)
        outStream = stdout;

    fprintf(outStream, "CPU{\n");
    fprintf(outStream, "    Registers{\n");

    auto dump_reg = [&](const char* name, void* ptr) {
        Pointer reg(ptr);
        ui32 usi = reg.DrefAs<ui32>();
        i32 si = reg.DrefAs<i32>();
        float fl = reg.DrefAs<float>();
        fprintf(outStream, "%8s%s:0x%08X  (int: %011d) \t(float: %f)\n", "", name, usi, si, fl);
    };
    #define printRegInfo(regName) dump_reg(#regName, &context.Register. regName);
    printRegInfo(eax);printRegInfo(ebx);printRegInfo(ecx);printRegInfo(edx);
    printRegInfo(esi);printRegInfo(edi);printRegInfo(ebp);printRegInfo(esp);
    #undef printRegInfo

    auto dump_log_reg = [&](const char* name, void *ptr) {
        Pointer reg(ptr);
        fprintf(outStream, "%8s%s:", "", name);
        for(int i = 0; i < 4; i++) {
            fprintf(outStream, "%08X\'", reg.DrefAs<ui32>());
            reg += sizeof(ui32);
        }
        fprintf(outStream, "\n");
    };
    #define printLongRegInfo(regName) dump_log_reg(#regName, &context.Register. regName);
    printLongRegInfo(lr0);printLongRegInfo(lr1);printLongRegInfo(lr2);printLongRegInfo(lr3);
    printLongRegInfo(lr4);printLongRegInfo(lr5);printLongRegInfo(lr6);printLongRegInfo(lr7);
    #undef printLongRegInfo

    fprintf(outStream, "    }\n");
    #ifdef DUMP_PRINT_MEMORY
    fprintf(outStream, "    RAM:\n");
    fprintf(outStream, "    Segment offset |  0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F\n");
    fprintf(outStream, "    ----------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < context.ramSize ; i += 16)
    {
        fprintf(outStream, "    Segment:0x%05X|", i);
        for (int j = 0; j < 16; j++)
        {
            ui8 data = i + j < context.ramSize ? context.RAM[i + j] : 0;
            if (!data)
                SET_DARK_GRAY;
            else
                SET_WHITE;
            fprintf(outStream, "  0x%02X", data & 0xFF);
            RESET_COLOUR;
            
        }
        fprintf(outStream, "\n");
    }
    #endif
    fprintf(outStream, "}\n");
}

static void command_prepare(Instruction& cmd) {
    for(int i = 0; i < cmd.bits.nOperands; i++) {
        switch (cmd.get_operand_type(i))
        {
        case OPERAND_REGISTER:
            cmd.ops[i] = context.get_reg_ptr(cmd.operand[i]);
            break;
        case OPERAND_NUMBER:
            cmd.ops[i] = &cmd.operand[i];
            break;
        case OPERAND_MEMORY: {
            cmd.ops[i] = &context.RAM[cmd.operand[i]];
            break;
        }
        case OPERAND_MEM_BY_REG: {
            auto reg_location = context.get_reg_ptr(cmd.operand[i]);
            auto offset = static_cast<ui32>(*reg_location);
            if (cmd.bits.longCommand)
                offset += cmd.extend[i];
            cmd.ops[i] = &context.RAM[offset];
            break;
        }
        default:
            break;
        }
    }
}


/*
\brief  Объвление дополнительных функций
*/
#define DEF(name, mCode, vStr1, vStr2, vStr3, code)\
void run_##name(Instruction* cmd) {\
    auto &idst = cmd->ops[0].DrefAs<Instruction::IType>();\
    auto &iop1 = cmd->ops[1].DrefAs<Instruction::IType>();\
    auto &iop2 = cmd->ops[2].DrefAs<Instruction::IType>();\
    auto &fdst = cmd->ops[0].DrefAs<Instruction::FType>();\
    auto &fop1 = cmd->ops[1].DrefAs<Instruction::FType>();\
    auto &fop2 = cmd->ops[2].DrefAs<Instruction::FType>();\
    code\
} 
#include "Extend.h"
#undef DEF

/*
\breif Массив функций, реализующих поведение процессора
*/
const PtrToFunction runFunction[] =
{
    #define DEF(name, mCode, vStr1, vStr2, vStr3, code) run_##name,
    #include "Extend.h"
    #undef DEF
};
const ui32 FUNCTION_TABLE_SIZE = sizeof(runFunction) / sizeof(PtrToFunction);

/*
\brief  Функция, запускает выполнение программы, начиная с текущего значение CPU.pc
\return Возвращается код ошибки или CPU_OK
*/
CPUerror CPU::evaluate()
{
    using Holder = PairHolder<Pointer, ui32>;
    constexpr Mcode ASM_HLT = 0 << 6 | 0 << 4 | 0 << 2 | 0x0;

    Pointer ptr(&context.RAM[context.pc]);
    Instruction cmd;
    while (ptr.DrefAs<Mcode>() != ASM_HLT)
    {
        // Extract new command from RAM
        ptr = &context.RAM[context.pc];
        cmd.bits.marchCode = ptr.DrefAs<Mcode>();
        Holder(ptr, context.pc) += sizeof(Mcode);

        // Fill command's args
        for (int index = 0; index < cmd.bits.nOperands; index++)
        {
            OperandType opType = cmd.get_operand_type(index);
            if (opType == OPERAND_REGISTER || opType == OPERAND_MEM_BY_REG)
            {
                cmd.operand[index] = ptr.DrefAs<ui8>();
                Holder(ptr, context.pc) += sizeof(ui8);
                if (cmd.bits.longCommand && opType == OPERAND_MEM_BY_REG)
                {
                    cmd.extend[index] = ptr.DrefAs<ui32>();
                    Holder(ptr, context.pc) += sizeof(ui32);
                }
            }
            if (opType == OPERAND_NUMBER || opType == OPERAND_MEMORY)
            {
                cmd.operand[index] = ptr.DrefAs<ui32>();
                Holder(ptr, context.pc) += sizeof(ui32);
            }
        }

        if (context.stepByStep)
        {
            Disassembler::disasmCommand(cmd, stdout);
            Disassembler::disasmCommand(cmd, logger.getStream());
            dump(stdout);
            dump(logger.getStream());
            system("pause");
            system("cls");
        }

        ui32 indexCalledFunc = cmd.bits.opCode;
        ASSERT_DO(indexCalledFunc >= FUNCTION_TABLE_SIZE, "CPU error", "Invalid machine code of command.");

        // Do actual command execution
        command_prepare(cmd);
        runFunction[indexCalledFunc](&cmd);
        ptr = &context.RAM[context.pc];

        ASSERT_DO(context.pc >= context.ramSize, "CPU error", "Register epi quite big for RAM.");
    }
    return CPU_OK;
}

/*
\brief  Функция возвращает поясняющую строку, по коду ошибки процессора
\param  [in]  errorCode  Код ошибки
\return Возвращается строка, поясняющее код ошибки
*/
C_string getStringByErrorCode(CPUerror errorCode)
{
    constexpr char* table[] = {
        "Ok",
        "CPU structure has been broken",
        "CPU find command that he doesn't know",
        "An exceptional situation has occurred",
        "PC register is too large for CPU's RAM",
        "The data passed is not valid",
        "Undefined error code"
    };
    if(errorCode > CPU_STATUS_COUNT)
        errorCode = CPU_STATUS_COUNT;
    return table[errorCode];
}