#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>

#define CONST_ARRAY_SIZE 4096

void StartCounter();
double GetCounter();
void errExit(const char*);
void printTimeToFile(const double time, const char* fname, const char* index, const int par1, const int par2);
