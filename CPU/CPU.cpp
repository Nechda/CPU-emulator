#include <cstring>
#include "Tools/Support.h"


#include "Asm/Asm.h"
#include "CPU/CPU.h"
#include "CPUInfo.h"

using namespace Assembler;
using std::memcpy;


CPU::Context CPU::context = {};
CPU::Context& context = CPU::context;

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

#ifdef CPU_PROFILER
    #define COLLECT_PROFILER_INFORMATION(code) code
#else
    #define COLLECT_PROFILER_INFORMATION(code)
#endif


/*
\brief Константы, определяющие работу процессора
@{
*/
const Mcode ASM_HLT = 0 << 6 | 0 << 4 | 0 << 2 | 0x0; ///< Именно эта команда будет завершать работу процессора


/*
\brief  Функция возвращает указатель на поле структуры CPU.Register,
в качестве аргумента принимается нормер регистра (см таблицу с регистрами).
\param  [in]  number  Номер регистра, адрес которого кужно получить
\return Указатель на поле структуры CPU.Register, в случае ошибки возвращается NULL
*/
ui32* getRegisterPtr(ui8 number)
{
    if (number > sizeof(context.Register) / sizeof(ui32))
        return nullptr;
    if (10 <= number && number <= 17)
        return (ui32*)&context.Register.lr0 + 4 * (number - 10);
    else
        return (ui32*)&context.Register + (number - 1);
}

/*
\brief  Функция возвращает поясняющую строку, по коду ошибки процессора
\param  [in]  errorCode  Код ошибки
\return Возвращается строка, поясняющее код ошибки
*/
C_string getStringByErrorCode(CPUerror errorCode)
{
    switch (errorCode)
    {
    case CPU_OK:
        return "Ok";
        break;
    case CPU_ERROR_INVALID_STRUCUTE:
        return "CPU structure has been broken";
        break;
    case CPU_ERROR_INVALID_COMMAND:
        return "CPU find command that he doesn't know";
        break;
    case CPU_ERROR_EXCEPTION:
        return "An exceptional situation has occurred";
        break;
    case CPU_ERROR_EPI_OUT_OF_RANE:
        return "EPI register is too large for CPU's RAM";
        break;
    case CPU_INVALID_INPUT_DATA:
        return "The data passed is not valid";
    default:
        return "Undefined error code";
        break;
    }
}

/*
\brief  Функция производит инициализацию структуры CPU
\param  [in]  ramSize  Размер виртуальной памяти процессора
\note   Если при инициализации структуры возникли ошибки, то полю isValid присваивается значение 0
Причина возникновения ошибки записывается в лог файл.
*/
void CPU::init(const InputParams inParam)
{
    context.isGraphMode = inParam.useGraphMode;
    context.ramSize = inParam.memorySize;
    ASSERT_DO(context.ramSize > (32 << 20), "CPU error", "I'm not sure that you really want too much memory.");

    context.RAM = (ui8*)calloc(context.ramSize, sizeof(ui8));
    ASSERT_DO(!context.RAM, "CPU error", "We can't alloc memory for virtual RAM.");

    context.isValid = 1;
    context.stepByStep = inParam.useStepByStepMode;
}


/*
\brief  Функция делает cleanUp структуры CPU
*/
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
    #if 0
    if (outStream ? ferror(outStream) : 1)
        outStream = stdout;

    fprintf(outStream, "CPU{\n");
    fprintf(outStream, "    Registers{\n");

    #define printRegInfo(regName)\
        fprintf(outStream, "        " #regName ":0x%08X  (int: %011d) \t(float: %f)\n", context.Register.##regName,context.Register.##regName,*((float*)&context.Register.##regName))
    printRegInfo(eax);printRegInfo(ebx);printRegInfo(ecx);printRegInfo(edx);
    printRegInfo(esi);printRegInfo(edi);printRegInfo(ebp);printRegInfo(esp);
    #undef printRegInfo

    #define printLongRegInfo(regName)\
        fprintf(outStream, "%8s"#regName":","");\
        for(ui8 i = 0; i < 4; i++)\
            fprintf(outStream, "%08X\'", context.Register.##regName.b[i]);\
        fprintf(outStream, "\n");
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
    #endif
}

static void command_prepare(Instruction& cmd) {
    for(int i = 0; i < cmd.bits.nOperands; i++) {
        switch (cmd.get_operand_type(i))
        {
        case OPERAND_REGISTER:
            cmd.ops[i] = getRegisterPtr(cmd.operand[i]);
            break;
        case OPERAND_NUMBER:
            cmd.ops[i] = &cmd.operand[i];
            break;
        case OPERAND_MEMORY: {
            cmd.ops[i] = &context.RAM[cmd.operand[i]];
            break;
        }
        case OPERAND_MEM_BY_REG: {
            auto reg_location = getRegisterPtr(cmd.operand[i]);
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
CPU::PtrToFunction CPU::runFunction[] =
{
    #define DEF(name, mCode, vStr1, vStr2, vStr3, code) run_##name,
    #include "Extend.h"
    #undef DEF
};
const ui32 CPU::FUNCTION_TABLE_SIZE = sizeof(CPU::runFunction) / sizeof(CPU::PtrToFunction);



/*
\brief  Функция, запускает выполнение программы, начиная с текущего значение CPU.pc
\param  [in]  writeResultInLog  Флаг, отвечающий за то, хотим ли мы увидеть результат работы программы в логе
\return Возвращается код ошибки или CPU_OK
*/
CPUerror CPU::evaluate()
{
    using Holder = PairHolder<Pointer, ui32>;
    Pointer ptr(&context.RAM[context.pc]);
    Instruction cmd;
    
    while (ptr.DrefAs<Mcode>() != ASM_HLT)
    {
        ptr = &context.RAM[context.pc];
        cmd.bits.marchCode = ptr.DrefAs<Mcode>();
        Holder(ptr, context.pc) += sizeof(Mcode);

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

        COLLECT_PROFILER_INFORMATION(profiler.pushCommand(cmd, context.pc));
        command_prepare(cmd);
        runFunction[indexCalledFunc](&cmd);
        ptr = &context.RAM[context.pc];

        //Disassembler::disasmCommand(cmd, logger.getStream());

        ASSERT_DO(context.interruptCode, "CPU error", "Catch exception after execution command.");
        ASSERT_DO(context.pc >= context.ramSize, "CPU error", "Register epi quite big for RAM.");
    }
    return CPU_OK;
}


CPUerror CPU::run(ui8* bytes, ui32 size, ui32 ptrStart)
{
    ASSERT_DO(context.isValid, "CPU error", "You try to evaluate program on broken CPU.");
    ASSERT_DO(bytes, "CPU error", "You try execute program, located by NULL pointer.");
    ASSERT_DO(size <= 0, "CPU error", "You try execute program, that have incorrect size (= 0).");
    ASSERT_DO(ptrStart + size + 1 >= context.ramSize, "CPU error", "Your program doesn't fit in RAM. Try to change ptrStart or write small program.");

    memcpy(&context.RAM[ptrStart], bytes, size);
    ///на всякий случай поставим код остановки, после всей программы
    Pointer(&context.RAM[ptrStart + size]).DrefAs<Mcode>() = ASM_HLT;
    context.pc = ptrStart;
    context.Register.esp = size + 1; /// стек будет лежать за кодом

    CPUerror errorCode = evaluate();

    COLLECT_PROFILER_INFORMATION(
        profiler.getScore();
        profiler.makeReport("profiler reports/command_usage_frequency.txt", Profiler::Report::COMMAND_USAGE);
        profiler.makeReport("profiler reports/sequence_usage.txt", Profiler::Report::COMMAND_SEQUENCE_USAGE);
        profiler.makeReport("profiler reports/temperature.txt", Profiler::Report::REGION_TEMPERATURE);
        profiler.makeReport("profiler reports/longest_repeated_string.txt", Profiler::Report::LONGEST_REPEATED_STRING);
    )
    return errorCode;
}
