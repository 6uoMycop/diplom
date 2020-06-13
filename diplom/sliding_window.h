#pragma once
#include <stdint.h>
#include <malloc.h>
#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <openssl/bn.h>



#define CONST_MAX_WINDOW_LEN 128
#define NTESTS 1

typedef struct window {
    BIGNUM*  W;
    uint64_t W_ui;
    uint64_t length;   // ����� ���� � �����
} window;


unsigned char getBit(
    const unsigned char* arr,
    unsigned int         index
);
unsigned int constructBeDataFromBinArray(
    unsigned char** output, // must be NULL, then u free it
    const unsigned char* inputStringLeBin,
    const unsigned int   numBits
);
int getWindow_VLNW(
    const unsigned char* bytes,
    const unsigned int   bitOffset,
    const unsigned int   lenBytes,
    const unsigned int   d,    // ������������ ����� NZW
    const unsigned int   r,    // ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW
    window* resWind
);
int getWindow_CLNW(
    const unsigned char* bytes,
    const unsigned int   bitOffset,
    const unsigned int   lenBytes,
    const unsigned int   d,    // ������������ ����� NZW
    window* resWind
);
unsigned int constructWindows(
    const BIGNUM* k,    // ������� �����
    const unsigned int d,    // ������������ ����� NZW
    const unsigned int r,    // ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW
    window** winds // ������ ��� ������ ����������
);
void deleteWindows(
    window** windowsArray,
    const unsigned int windNum
);
void printWindows(
    const window* windowsArray,
    const unsigned int windNum,
    const BIGNUM* numberOpt     // �������� �����, ������� ���� ������� �� ����. ���� NULL, ���������� �� �����
);
void multiply_sliding_window(
    const BIGNUM* k, // ���������
    const unsigned int d, // �������� ����: ������������ ����� NZW
    const unsigned int r, // �������� ����: ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW. ���� r==0, �� ������������ CLNW, ����� - VLNW
    const EC_POINT* P, // ����� ������������� ������, ������� ����� �������� �� k
    const EC_GROUP* G, // ������ ����� ������������� ������
    EC_POINT** Q, // ���������
    const char* dbgIndex // ����� ������������ ����� ��� ������ � ����
);
void multiply_sliding_window_modified(
    const BIGNUM* k, // ���������
    const unsigned int kMaxLen, // ������������ ����� ����� k
    const unsigned int d, // �������� ����: ������������ ����� NZW
    const unsigned int r, // �������� ����: ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW. ���� r==0, �� ������������ CLNW, ����� - VLNW
    const EC_POINT* P, // ����� ������������� ������, ������� ����� �������� �� k
    const EC_GROUP* G, // ������ ����� ������������� ������
    EC_POINT** Q, // ���������
    const char* dbgIndex // ����� ������������ ����� ��� ������ � ����
);



