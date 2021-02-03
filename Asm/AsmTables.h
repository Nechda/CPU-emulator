#pragma once
#include "Types.h"


/*
\brief ���������, ����������� ����������� ������� �����
*/
struct Lexema
{
    char command[10];
    ui16 machineCode;
    char* validFirstOperand;
    char* validSecondOperand;
};


/*
\brief ������� � ��������� ���� ������ � ���������, � �������� ����� �������� ����������
*/
extern Lexema commandTable[];
extern const Lexema registerTable[];

/*
\brief ������� ������
*/
extern const ui32 COMMAND_TABLE_SIZE;
extern const ui32 REGISTER_TABLE_SIZE;