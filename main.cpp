#include <string>
#include <fstream>
#include <streambuf>

#include "Tools/Argparser.h"
#include "CPU/CPU.h"

/**
\brief Функция запускает вывод справки по программе
*/
void run_Helper()
{
        printf(
            "There are some flags you can run program with, here is their description:\n"
            "-h                  Shows help to the program and describes all flags that you can use.\n"
            "-c                  Calls compilator, you also need use -i flag\n"
            "-d                  Calls disassembler, you also need use -i flag\n"
            "-r                  Calls virtual CPU to execute your program, you also need use -i flag\n"
            "-e                  This is a mixture of two programs:it first calls compilator, then virtual CPU\n"
            "you also need use -i flag to select file with a program code. There is -o flag, but it's not\n"
            "necessary: by default result of compilation will save into \"a.out\" file.\n"
            "-t                  Calls test programs, that located in Tests folder.\n"
            "-l <filename>       This flag sets the log file, by default log file has name \"log.log\".\n"
            "-i <filename>       This flag sets the input file, from this file program will read infromation.\n"
            "-o <filename>       This flag sets the output file, into this file program will write result,\n"
            "by default file has name \"a.out\" file.\n"
            "-n                  This flag disable writing in log file.\n"
            "-s                  Sets the processor's program execution mode to step-by-step.\n"
            "--memorySize=<size> Sets size of virtual RAM in bytes.(Actual RAM size will be the maximum between 512 and your size)\n"
            "-g                  Enable graphic mode, you will be able to write into video memory, located at 0x400 offset in\n"
            "virtual memory.\n"
            "--config=<filename> Specifies the configuration file, in there you can describe window and font sizes in graphic mode,\n"
            "init entry point adress for your programm also set RAM size\n"
        );
}

int main(int argc, char** argv)
{
    InputParams inputParams;
    parseConsoleArguments(argc, argv, &inputParams);

    auto file_name = inputParams.inputFilename;
    std::ifstream t(file_name);
    if(!t.is_open()) {
        std::cout << "Cant open file: `" << inputParams.inputFilename << "`" << std::endl;
        return EXIT_FAILURE;
    }
    using ist = std::istreambuf_iterator<char>;
    std::string str((ist(t)),ist());
    auto buffer = str.data();
    auto n_bytess = str.size();

    auto &CPU = CPU::Instance();
    CPU.init(inputParams);
    auto ret_code = CPU.run(buffer, n_bytess, inputParams.entryPoint);
    std::cout << "CPU finished with the code: " << (int)ret_code << " ";
    std::cout << "(" << getStringByErrorCode(ret_code) << ")";
    std::cout << std::endl;
    CPU.dump(stdout);

    return EXIT_SUCCESS;
}