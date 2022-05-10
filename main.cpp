#include <string>
#include <fstream>
#include <streambuf>

#include "CPU/CPU.h"

std::string read_binary(const char* filename) {
    if(!filename) { 
        std::cout << "Nullptr instead of filename, did you select file?" << std::endl;
        return {};
    }
    std::ifstream t(filename);
    if(!t.is_open()) {
        std::cout << "Cant open file: `" << filename << "`" << std::endl;
        return {};
    }
    using ist = std::istreambuf_iterator<char>;
    std::string str((ist(t)),ist());
    return str;
}

int main(int argc, char** argv)
{
    auto arguments = parse(argc, argv);

    auto code = read_binary(arguments.input_file);
    auto buffer = code.data();
    auto n_bytes = code.size();
    if(n_bytes == 0) return 1;

    auto &CPU = CPU::Instance();
    CPU.init(arguments);
    CPU.run(buffer, n_bytes, arguments.entry);

    auto ret_code = CPU.ret_code();
    std::cout << "CPU finished with the code: " << (int)ret_code << " ";
    std::cout << "(" << getStringByErrorCode(ret_code) << ")";
    std::cout << std::endl;
    CPU.dump(stdout);

    return 0;
}