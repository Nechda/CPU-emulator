#pragma once
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "Types.h"
#include "Asm/Asm.h"

class Profiler
{
    private:
        std::map<ui32, Assembler::Instruction> m_commandOrderedMap;
        ui64 m_totalCommandExecuted = 0;
        const ui8 SPACE_SIZE_FOR_EXPLANATORY_LINE = 40;
    public:
        Profiler() {};
        ~Profiler() {};

        enum class Report
        {
            COMMAND_USAGE,
            REGION_TEMPERATURE,
            COMMAND_SEQUENCE_USAGE,
            LONGEST_REPEATED_STRING
        };

        void getScore()
        {
            std::cout << "Score:" << m_totalCommandExecuted << std::endl;
        }
        void pushCommand(Assembler::Instruction cmd, ui32 eip = 0);
        void makeReport(const std::string filename, Report report = Report::COMMAND_USAGE);
    private:
        void measureTemperature(const std::string& filename);
        void makeSequenceUsageReport(const std::string& filename);
        void makeUsageReport(const std::string& filename);
        void searchLongestRepeatedString(const std::string& filename);
        
        static std::string makeCommandProtoString(ui16 commandCode);
        friend void prntLRS(const std::string& decode, std::ostream& outStream);
};