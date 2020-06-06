#pragma once

#include <stdio.h>
#include <Windows.h>

#define CONST_ARRAY_SIZE 4096

void StartCounter();
double GetCounter();
void errExit(const char*);
