#include "Profiler.h"
#include <sstream>

using namespace std;

void Profiler::pushCommand(Assembler::Command cmd, ui32 eip)
{
    auto search = m_commandOrderedMap.find(eip);
    if (search != m_commandOrderedMap.end())
        search->second.extend[0]++;
    else
    {
        cmd.extend[0] = 1;
        m_commandOrderedMap[eip] = cmd;
    }
    m_totalCommandExecuted++;
}

void Profiler::makeReport(const std::string filename, Report report)
{
    switch (report)
    {
    case Report::COMMAND_USAGE:
        makeUsageReport(filename);
        break;
    case Report::REGION_TEMPERATURE:
        measureTemperature(filename);
        break;
    case Report::COMMAND_SEQUENCE_USAGE:
        makeSequenceUsageReport(filename);
        break;
        case Report::LONGEST_REPEATED_STRING:
        searchLongestRepeatedString(filename);
        break;
    default:
        break;
    }
}

std::string Profiler::makeCommandProtoString(ui16 commandCode)
{
    static const std::string operand[] =
    { "reg", "imm", "imm mem", "reg mem", "gen mem" };

    static const std::string m_commandsName[] =
    {
        #define DEF(name, mCode, vStr1, vStr2, code) std::string(#name),
            #include "Extend.h"
        #undef DEF
    };

    ui8 op1 = (commandCode >> 4) & 0b11;
    ui8 op2 = (commandCode >> 2) & 0b11;
    bool isLongMemoryOperapors = (commandCode >> 6) & 1;

    ui8 opCode = commandCode >> 8;
    std::string result = m_commandsName[opCode];
    if ((commandCode & 0b11) > 0)
        result += " " + ((isLongMemoryOperapors && op1 == 3) ? operand[4] : operand[op1]);
    if ((commandCode & 0b11) > 1)
        result += " , " + ((isLongMemoryOperapors && op2 == 3) ? operand[4] : operand[op2]);

    return result;
}


void Profiler::measureTemperature(const std::string& filename)
{
    ofstream reportFile;
    if (!filename.size())
    {
        std::cout << "You should set output file for report." << std::endl;
        return;
    }
    reportFile.open(filename);

    cout << setw(SPACE_SIZE_FOR_EXPLANATORY_LINE) << "Measuring code temperature ... ";

    for (const auto& it : m_commandOrderedMap)
    {
        reportFile << "Total usage : " << setprecision(4) << (double(it.second.extend[0]) / m_totalCommandExecuted) << endl;
        reportFile << "    " << makeCommandProtoString(it.second.code.marchCode) << endl;
    }

    reportFile.close();
    cout << filename << endl;
}


void Profiler::makeUsageReport(const std::string& filename)
{
    std::ofstream reportFile;
    if (!filename.size())
    {
        std::cout << "You should set output file for report." << std::endl;
        return;
    }
    reportFile.open(filename);
    cout << setw(SPACE_SIZE_FOR_EXPLANATORY_LINE) << "Measuring command usage frequency ... ";

    using pairType = std::pair<ui16, ui32>;
    std::vector<pairType> commandUsageTable;

    commandUsageTable.reserve(m_commandOrderedMap.size());
    for (const auto& it : m_commandOrderedMap)
        commandUsageTable.push_back({
            it.second.code.marchCode,
            it.second.extend[0]
        });

    //sorting by cmd code
    std::sort(commandUsageTable.begin(), commandUsageTable.end(),
        [](const pairType& a, const pairType& b) {return a.first < b.first; }
    );

    for (ui32 i = commandUsageTable.size() - 1; i >= 1; i--)
    {
        if (commandUsageTable[i].first == commandUsageTable[i - 1].first)
        {
            commandUsageTable[i - 1].second += commandUsageTable[i].second;
            commandUsageTable.erase(commandUsageTable.begin() + i);
        }
    }

    for (const auto& it : commandUsageTable)
    {
        const ui16& commandCode = it.first;
        const ui32& nExecution = it.second;
        reportFile << setprecision(4) << double(nExecution)/m_totalCommandExecuted
                   << " " <<  makeCommandProtoString(commandCode) << endl;
    }

    reportFile.close();
    cout << filename << endl;
}


namespace LRS
{
    static string lcp(const string& s,const string& t)
    {
        ui32 n = s.size() < t.size() ? s.size() : t.size();
        for (ui32 i = 0; i < n; i++)
        {
            if (s[i] != t[i])
                return s.substr(0, i);
        }
        return "";
    }

    static string lrs(string s)
    {
        ui32 N = s.size();
        vector<string> suffixes(N);
        for (ui32 i = 0; i < N; i++)
            suffixes[i] = s.substr(i, N);


        sort(suffixes.begin(), suffixes.end());

        string lrs = "";
        for (ui32 i = 0; i < N - 1; i++)
        {
            string x = lcp(suffixes[i], suffixes[i + 1]);
            if (x.size() > lrs.size())
                lrs = x;
        }

        return lrs;
    }
}


static void prntLRS(const string& decode, ostream& outStream = std::cout)
{
    stringstream ss;
    ui32 start = decode.find_first_of(' ');
    ui32 end = decode.find_last_of(' ');
    ss.str(decode.substr(start, end - start));
    while (!ss.eof())
    {
        ui16 mCode = 0;
        ss >> mCode;
        outStream << Profiler::makeCommandProtoString(mCode) << endl;
    }
}



static void eraseAllSubStr(std::string & mainStr, const std::string & toErase)
{
    size_t pos = std::string::npos;
    while ((pos = mainStr.find(toErase)) != std::string::npos)
        mainStr.erase(pos, toErase.length());
}

void Profiler::searchLongestRepeatedString(const std::string& filename)
{
    std::ofstream reportFile;
    if (!filename.size())
    {
        std::cout << "You should set output file for report." << std::endl;
        return;
    }
    reportFile.open(filename);

    cout << setw(SPACE_SIZE_FOR_EXPLANATORY_LINE) << "Finding longest repeated sequences ... ";


    stringstream ss;
    for (const auto& it : m_commandOrderedMap)
        ss << it.second.code.marchCode << " ";
    

    string s = "";
    string lrs = "";

    for (ui8 i = 0; i < 8 && (s.size() || i==0); i++)
    {
        s = ss.str();
        if(lrs.size())
        eraseAllSubStr(s, lrs);
        ss.str(s);
        lrs = LRS::lrs(ss.str());
        reportFile << "LRS " << (i + 1) << ":" << endl;
        prntLRS(lrs, reportFile);
        reportFile << "=========================" << endl;
    }
    reportFile.close();

    cout << filename << endl;
}