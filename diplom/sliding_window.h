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
    uint64_t length;   // Длина окна в битах
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
    const unsigned int   d,    // Максимальная длина NZW
    const unsigned int   r,    // Минимальная длина серии нулей, необходимая для перехода из NZW к ZW
    window* resWind
);
int getWindow_CLNW(
    const unsigned char* bytes,
    const unsigned int   bitOffset,
    const unsigned int   lenBytes,
    const unsigned int   d,    // Максимальная длина NZW
    window* resWind
);
unsigned int constructWindows(
    const BIGNUM* k,    // Входное число
    const unsigned int d,    // Максимальная длина NZW
    const unsigned int r,    // Минимальная длина серии нулей, необходимая для перехода из NZW к ZW
    window** winds // Массив для записи результата
);
void deleteWindows(
    window** windowsArray,
    const unsigned int windNum
);
void printWindows(
    const window* windowsArray,
    const unsigned int windNum,
    const BIGNUM* numberOpt     // Исходное число, которое было разбито на окна. Если NULL, печататься не будет
);
void multiply_sliding_window(
    const BIGNUM* k, // Множитель
    const unsigned int d, // Параметр окна: максимальная длина NZW
    const unsigned int r, // Параметр окна: минимальная длина серии нулей, необходимая для перехода из NZW к ZW. ЕСЛИ r==0, то использовать CLNW, иначе - VLNW
    const EC_POINT* P, // Точка эллиптической кривой, которая будет умножена на k
    const EC_GROUP* G, // Группа точек эллиптической кривой
    EC_POINT** Q, // Результат
    const char* dbgIndex // Номер тестируемого числа для вывода в файл
);
void multiply_sliding_window_modified(
    const BIGNUM* k, // Множитель
    const unsigned int kMaxLen, // Максимальная длина числа k
    const unsigned int d, // Параметр окна: максимальная длина NZW
    const unsigned int r, // Параметр окна: минимальная длина серии нулей, необходимая для перехода из NZW к ZW. ЕСЛИ r==0, то использовать CLNW, иначе - VLNW
    const EC_POINT* P, // Точка эллиптической кривой, которая будет умножена на k
    const EC_GROUP* G, // Группа точек эллиптической кривой
    EC_POINT** Q, // Результат
    const char* dbgIndex // Номер тестируемого числа для вывода в файл
);



