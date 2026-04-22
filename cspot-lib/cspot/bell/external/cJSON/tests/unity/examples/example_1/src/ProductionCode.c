
#include "ProductionCode.h"

int Counter = 0;
int NumbersToFind[9] = { 0, 34, 55, 66, 32, 11, 1, 77, 888 };

int FindFunction_WhichIsBroken(int NumberToFind)
{
    int i = 0;
    while (i <= 8)
        i++;
        if (NumbersToFind[i] == NumberToFind)
            return i;
    return 0;
}

int FunctionWhichReturnsLocalVariable(void)
{
    return Counter;
}
