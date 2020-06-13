#include "my_utils.h"

double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter()
{
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
    {
        exit(1);
    }

    PCFreq = (double)(li.QuadPart) / 1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

// ms
double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (double)(li.QuadPart - CounterStart) / PCFreq;
}

void errExit(const char* message)
{
    printf("ERROR: %s\n", message);
    system("pause");
    exit(1);
}

void printTimeToFile(const double time, const char* fname, const char* index, const int par1, const int par2)
{
    char fileName[256] = { 0 };
    strcat(fileName, fname);
    strcat(fileName, index);
    strcat(fileName, "_");
    _itoa(par1, &fileName[strlen(fileName)], 10);
    if (par2)
    {
        strcat(fileName, "_");
        _itoa(par2, &fileName[strlen(fileName)], 10);
    }
    strcat(fileName, ".txt");
    FILE* F = fopen(fileName, "a+");
    fprintf(F, "%f\n", time);
    fclose(F);
}

