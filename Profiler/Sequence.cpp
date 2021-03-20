#include "Profiler.h"

using namespace std;
using namespace Assembler;


void Profiler::makeSequenceUsageReport(const std::string& filename)
{
    const i8 SEQUENCE_LENGTH = 4;


    std::ofstream reportFile;
    if (!filename.size())
    {
        std::cout << "You should set output file for report." << std::endl;
        return;
    }
    if (m_commandOrderedMap.size() <= SEQUENCE_LENGTH)
    {
        std::cout << "Your ptogram too smal for sequence analys." << std::endl;
        return;
    }

    reportFile.open(filename);

    cout << setw(SPACE_SIZE_FOR_EXPLANATORY_LINE) << "Measuring sequence usage ... ";

    vector<Command> commandVector;
    commandVector.reserve(m_commandOrderedMap.size());
    for (const auto& it : m_commandOrderedMap)
        commandVector.push_back(it.second);

    vector<pair<ui32, ui64>> sequenceTable;
    sequenceTable.reserve(commandVector.size() - SEQUENCE_LENGTH);


    #define IS_JUMP(cmd) cmd.code.bits.opCode == 8
    #define IS_RET(cmd) cmd.code.bits.opCode == 17

    ui64 hash = 0;
    ui8 isThereJmpOrRet = 0;
    hash |= commandVector[0].code.marchCode; hash <<= 16;
    isThereJmpOrRet |= IS_JUMP(commandVector[0]) || IS_RET(commandVector[0]);
    isThereJmpOrRet <<= 2;
    hash |= commandVector[1].code.marchCode; hash <<= 16;
    isThereJmpOrRet |= IS_JUMP(commandVector[1]) || IS_RET(commandVector[1]);
    isThereJmpOrRet <<= 2;
    hash |= commandVector[2].code.marchCode; hash <<= 16;
    isThereJmpOrRet |= IS_JUMP(commandVector[2]) || IS_RET(commandVector[2]);
    isThereJmpOrRet <<= 2;
    hash |= commandVector[3].code.marchCode;
    isThereJmpOrRet |= IS_JUMP(commandVector[3]) || IS_RET(commandVector[3]);


    for (ui32 i = SEQUENCE_LENGTH; i < commandVector.size(); i++)
    {
        sequenceTable.push_back({ isThereJmpOrRet ? 0 : commandVector[i-4].extend[0] , hash });
        hash <<= 16;
        isThereJmpOrRet <<= 2;
        hash |= commandVector[i].code.marchCode;
        isThereJmpOrRet |= IS_JUMP(commandVector[i]) || IS_RET(commandVector[i]);
    }

    #undef IS_JMP
    #undef IS_RET

    using pairType = pair<ui32, ui64>;
    sort(sequenceTable.begin(), sequenceTable.end(), 
        [](const pairType& a, const pairType& b) {return a.second < b.second; }
    );


    for (ui32 i = sequenceTable.size() - 1; i >= 1; i--)
    {
        if (sequenceTable[i].second == sequenceTable[i - 1].second)
        {
            sequenceTable[i - 1].first += sequenceTable[i].first;
            sequenceTable.erase(sequenceTable.begin() + i);
        }
    }

    sort(sequenceTable.begin(), sequenceTable.end(),
        [](const pairType& a, const pairType& b) {return a.first > b.first; }
    );


    ui64 check = 0;
    for (const auto& seq : sequenceTable)
    {
        ui64 hash = seq.second;
        if (!seq.first) continue;
        check += seq.first;
        reportFile << "Total usage : " << std::setprecision(4) << (double(seq.first * 4)/ m_totalCommandExecuted) << endl;
        reportFile << "    " << makeCommandProtoString(ui16((hash >> 48) & 0xFFFF)) << endl;
        reportFile << "    " << makeCommandProtoString(ui16((hash >> 32) & 0xFFFF)) << endl;
        reportFile << "    " << makeCommandProtoString(ui16((hash >> 16) & 0xFFFF)) << endl;
        reportFile << "    " << makeCommandProtoString(ui16((hash >> 0) & 0xFFFF)) << endl;
    }
    reportFile.close();

    cout << filename << endl;
}