#pragma once
#include <stdint.h>
#include <malloc.h>
#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <openssl/bn.h>



#define CONST_MAX_WINDOW_LEN 128

typedef struct window {
    BIGNUM* W;
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
int getWindow(
    const unsigned char* bytes,
    const unsigned int   bitOffset,
    //const unsigned int   lenBytes,
    const unsigned int   d,    // Максимальная длина NZW
    const unsigned int   r,    // Минимальная длина серии нулей, необходимая для перехода из NZW к ZW
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
void multiply_VLNW(
    const BIGNUM* k, // Множитель
    const unsigned int d, // Параметр окна: максимальная длина NZW
    const unsigned int r, // Параметр окна: минимальная длина серии нулей, необходимая для перехода из NZW к ZW
    const EC_POINT* P, // Точка эллиптической кривой, которая будет умножена на k
    const EC_GROUP* G, // Группа точек эллиптической кривой
    EC_POINT** Q
);
void multiply_VLNW_modified(
    const BIGNUM* k, // Множитель
    const unsigned int kMaxLen, // Максимальная длина числа k
    const unsigned int d, // Параметр окна: максимальная длина NZW
    const unsigned int r, // Параметр окна: минимальная длина серии нулей, необходимая для перехода из NZW к ZW
    const EC_POINT* P, // Точка эллиптической кривой, которая будет умножена на k
    const EC_GROUP* G, // Группа точек эллиптической кривой
    EC_POINT** Q,
    const int mode // 0 - CLNW, 1 - VLNW
);
void multiply_VLNW_modified2(
    const BIGNUM* k, // Множитель
    const unsigned int kMaxLen, // Максимальная длина числа k
    const unsigned int d, // Параметр окна: максимальная длина NZW
    const unsigned int r, // Параметр окна: минимальная длина серии нулей, необходимая для перехода из NZW к ZW
    const EC_POINT* P, // Точка эллиптической кривой, которая будет умножена на k
    const EC_GROUP* G, // Группа точек эллиптической кривой
    EC_POINT** Q
);


