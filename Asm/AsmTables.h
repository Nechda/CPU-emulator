#pragma once
#include "Types.h"


/*
\brief —труктура, описывающа€ минимальную единицу €зыка
*/
struct Lexema
{
    char command[10];
    ui16 machineCode;
    char* validFirstOperand;
    char* validSecondOperand;
};


/*
\brief “аблицы с описанием всех команд и регистров, с которыми может работать компил€тор
*/
extern Lexema commandTable[];
extern const Lexema registerTable[];

/*
\brief –азмеры таблиц
*/
extern const ui32 COMMAND_TABLE_SIZE;
extern const ui32 REGISTER_TABLE_SIZE;