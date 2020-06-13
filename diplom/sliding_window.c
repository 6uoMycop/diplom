#include "my_utils.h"
#include "sliding_window.h"





unsigned char getBit(
    const unsigned char* arr,
    unsigned int         index
)
{
    unsigned char byte = arr[index / 8];
    unsigned int  bitOffset = index % 8;
    byte <<= 7 - bitOffset;
    byte >>= 7;
    return byte;
}

unsigned int constructBeDataFromBinArray(
    unsigned char**      output,           // must be NULL, then u free it
    const unsigned char* inputStringLeBin,
    const unsigned int   numBits
)
{
    unsigned int numBytes = (numBits + 7) / 8;
    (*output) = (unsigned char*)calloc(numBytes, sizeof(unsigned char));
    if (!(*output))
    {
        errExit("calloc error");
    }

    for (int i = numBytes - 1, counterBits = 0; i >= 0; i--)
    {
        for (int j = 0; j < 8 && counterBits < numBits; j++, counterBits++)
        {
            (*output)[i] += ((inputStringLeBin[counterBits] - '0') << j);
        }
    }

    return numBytes;
}


int getWindow_VLNW(
    const unsigned char* bytes,
    const unsigned int   bitOffset,
    const unsigned int   lenBits,
    const unsigned int   d,         // Максимальная длина NZW
    const unsigned int   r,         // Минимальная длина серии нулей, необходимая для перехода из NZW к ZW
    window* resWind
)
{
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // Временный массив для хранения текущего окна. Биты в СИМВОЛЬНОМ (ASCII) представлении - для преобразования затем в BIGNUM
    unsigned char  bit                                       = 0;
    unsigned int   lenWindow                                 = 0; // В NZW это - индекс старшей единицы, индексация с 1. Т.е. если единиц нет - будет 0, иначе - положительное число
    unsigned int   lenWindowOld                              = 0;
    int            lenCheck                                  = 0;
    int            lenRemaining                              = lenBits;
    unsigned char* resultData                                = NULL;
    unsigned int   resultNumBytes                            = 0;
    BOOL           flag                                      = FALSE;

    bit = getBit(bytes, bitOffset);
    if (!bit) // ZW
    {
        for (int i = 0; !bit && i < lenBits; i++, bit = getBit(bytes, bitOffset + i))
        {
            //bit = getBit(bytes, bitOffset + i);
            tmpBitsForCurrentWindow[i] = bit + '0';
            lenWindow++;
        }
    }
    else // NZW
    {
        lenWindow = 0;
        lenWindowOld = 0;
        for (; lenRemaining; lenRemaining-=lenWindow)
        {
            flag = FALSE;
            lenWindowOld = lenWindow;
            if (lenWindow <= d - r)
            {
                lenCheck = r;
            }
            else
            {
                lenCheck = d - lenWindow; // d - 0
            }

            if (lenBits < lenCheck)
            {
                lenCheck = lenBits;
            }

            for (int i = 0; i < lenCheck; i++)
            {
                bit = getBit(bytes, bitOffset + lenWindowOld + i);
                if (bit)
                {
                    lenWindow = lenWindowOld + i + 1; // Записываем индекс единицы. В итоге тут будет длина до СТАРШЕЙ единицы
                    flag = TRUE;
                }
            }

            if (flag) // Остались в NZW, записываем
            {
                for (int i = 0; i < lenWindow; i++)
                {
                    bit = getBit(bytes, bitOffset + lenWindowOld + i);
                    tmpBitsForCurrentWindow[lenWindowOld + i] = bit + '0';
                }

            }
            else
            {


                break;
                //// Закончили NZW. Запишем единственную первую единицу
                //bit = getBit(bytes, bitOffset);
                //tmpBitsForCurrentWindow[lenWindowOld] = bit + '0';
                //lenWindow++;
                //break;
            }
        }

    }

    (*resWind).length = lenWindow;

    //for (int i = 0; i < lenWindow; i++)
    //{
    //    printf("%c", tmpBitsForCurrentWindow[i]);
    //}
    //printf("\n");


    resultNumBytes = constructBeDataFromBinArray(&resultData, tmpBitsForCurrentWindow, lenWindow);


    BN_bin2bn(resultData, resultNumBytes, (*resWind).W);

    (*resWind).W_ui = atoll(BN_bn2dec((*resWind).W));
    //printf("W: %s\t%u\n", BN_bn2hex((*resWind).W), (*resWind).W_ui);

    free(resultData);
    return lenWindow;
}

int getWindow_CLNW(
    const unsigned char* bytes,
    const unsigned int   bitOffset,
    const unsigned int   lenBits,
    const unsigned int   d,         // Максимальная длина NZW
    window* resWind
)
{
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // Временный массив для хранения текущего окна. Биты в СИМВОЛЬНОМ (ASCII) представлении - для преобразования затем в BIGNUM
    unsigned char  bit                                       = 0;
    unsigned int   lenWindow                                 = 0;
    unsigned char* resultData                                = NULL;
    unsigned int   resultNumBytes                            = 0;

    bit = getBit(bytes, bitOffset);
    if (!bit) // ZW
    {
        for (int i = 0; !bit && i < lenBits; i++, bit = getBit(bytes, bitOffset + i))
        {
            tmpBitsForCurrentWindow[i] = bit + '0';
            lenWindow++;
        }
    }
    else // NZW
    {
        for (int i = 0; i < d && i < lenBits; i++, bit = getBit(bytes, bitOffset + i))
        {
            tmpBitsForCurrentWindow[i] = bit + '0';
            lenWindow++;
        }
    }

    (*resWind).length = lenWindow;
    //for (int i = 0; i < lenWindow; i++)
    //{
    //    printf("%c", tmpBitsForCurrentWindow[i]);
    //}
    //printf("\n");
    resultNumBytes = constructBeDataFromBinArray(&resultData, tmpBitsForCurrentWindow, lenWindow);
    BN_bin2bn(resultData, resultNumBytes, (*resWind).W);
    (*resWind).W_ui = atoll(BN_bn2dec((*resWind).W));
    //printf("W: %s\t%u\n", BN_bn2hex((*resWind).W), (*resWind).W_ui);

    free(resultData);
    return lenWindow;
}

unsigned int constructWindows(
    const BIGNUM*      k,    // Входное число
    const unsigned int d,    // Максимальная длина NZW
    const unsigned int r,    // Минимальная длина серии нулей, необходимая для перехода из NZW к ZW. ЕСЛИ r==0, то использовать CLNW, иначе - VLNW
    window**           winds // Массив для записи результата
)
{
    window*        tmpRealloc                                = NULL;  // Для проверок выделения пмяти realloc'ом
    unsigned char* bytes                                     = NULL;  // Байты входного числа
    unsigned char  bit                                       = 0;
    int            num_bits                                  = 0;
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // Временный массив для хранения текущего окна. Биты в СИМВОЛЬНОМ (ASCII) представлении - для преобразования затем в BIGNUM
    BOOL           windowType                                = FALSE; // ZW - FALSE // NZW - TRUE
    unsigned int   windNum                                   = 1;

    bytes = (unsigned char*)calloc(BN_num_bytes(k), sizeof(unsigned char));
    if (!bytes)
    {
        errExit("calloc error");
    }

    BN_bn2lebinpad(k, bytes, BN_num_bytes(k));

    (*winds) = (window*)calloc(1, sizeof(window));
    if (!(*winds))
    {
        errExit("calloc error");
    }
    (*winds)[0].W = BN_new();
    num_bits = BN_num_bits(k);

    for (int offset = 0; offset < num_bits; windNum++)
    {
        /// v init
        // Выделяем новое окно
        tmpRealloc = realloc(*winds, windNum * sizeof(window));
        if (!tmpRealloc)
        {
            errExit("realloc error");
        }
        *winds = tmpRealloc;
        tmpRealloc = NULL;
        (*winds)[windNum - 1].W = BN_new();
        (*winds)[windNum - 1].W_ui = 0;
        /// ^ init

        offset +=
            (r)
            ?
            getWindow_VLNW(
                &(bytes[offset / 8]),
                offset % 8,
                num_bits - offset,
                d,
                r,
                &((*winds)[windNum - 1])
            )
            :
            getWindow_CLNW(
                &(bytes[offset / 8]),
                offset % 8,
                num_bits - offset,
                d,
                &((*winds)[windNum - 1])
            );
    }

    free(bytes);

    return windNum - 1;
}

void deleteWindows(
    window**           windowsArray, // Сами окна
    const unsigned int windNum       // Количество окон
)
{
    for (int i = 0; i < windNum; i++)
    {
        BN_clear_free((*windowsArray)[i].W);
    }
    free(*windowsArray);
}

void printWindows(
    const window*      windowsArray, // Сами окна
    const unsigned int windNum,      // Количество окон
    const BIGNUM*      numberOpt     // Исходное число, которое было разбито на окна. Если NULL, печататься не будет
)
{
    unsigned char bytes[CONST_ARRAY_SIZE] = { 0 };

    printf("\n--- WINDOWS OUTPUT ---\n");

    if (numberOpt)
    {
        BN_bn2lebinpad(numberOpt, bytes, CONST_ARRAY_SIZE);
        for (int j = BN_num_bits(numberOpt) - 1; j >= 0; j--)
        {
            printf("%c", getBit(bytes, j) + '0');
        }
        printf("\n");
        memset(bytes, 0, CONST_ARRAY_SIZE);
    }

    for (int i = 0; i < windNum; i++)
    {
        printf("%lld\t%s\t%i\t:\t", windowsArray[i].length, ((BN_is_zero(windowsArray[i].W)) ? "ZW" : "NZW"), i);
        if (BN_is_zero(windowsArray[i].W))
        {
            for (int j = 0; j < windowsArray[i].length; j++)
            {
                printf("0");
            }
        }
        else
        {
            memset(bytes, 0, CONST_ARRAY_SIZE);
            BN_bn2lebinpad(windowsArray[i].W, bytes, CONST_ARRAY_SIZE);
            for (int j = BN_num_bits(windowsArray[i].W) - 1; j >= 0; j--)
            {
                printf("%c", getBit(bytes, j) + '0');
            }
            //char tmp[256] = { 0 };
            //_itoa_s(windowsArray[i].W_ui, tmp, 256, 16);
            //printf("\t%s", tmp);
        }
        printf("\n");
    }
    printf("--- ^^^^^^^^^^^^^^ ---\n\n");
}

void multiply_sliding_window_operative(
    const unsigned int r,
    const unsigned int d,
    const unsigned int s,
    const window* f,
    const EC_GROUP* G,
    const EC_POINT** iP,
    EC_POINT** Q,
    const char* dbgIndex
)
{
    int i, j, test;
    double finish;

    /// Оперативный этап ///
    StartCounter();

    for (test = 0; test < NTESTS; test++)
    {

        /// 4 /// Для i от s-1 до 0 выполнять:
        for (i = s - 1; i >= 0; i--)
        {
            /// 4.1 /// Q <- 2^{len(f_i)}Q
            //for (int j = 0; j < pow2[f[i].length - 1]; j++)
            for (j = 0; j < f[i].length; j++)
            {
                printf("D");
                EC_POINT_dbl(
                    G,
                    *Q,
                    *Q,
                    NULL
                );
            }

            /// 4.2 /// Если f_i != 0, то Q <- Q + f_{i}P
            if (f[i].W_ui)
            {
                printf("A");
                EC_POINT_add(
                    G,
                    *Q,
                    *Q,
                    iP[f[i].W_ui],
                    NULL
                );
            }
        }

    }

    finish = GetCounter();
    printTimeToFile(finish, (r) ? "VLNW" : "CLNW", dbgIndex, d, r);
}

void multiply_sliding_window(
    const BIGNUM*      k, // Множитель
    const unsigned int d, // Параметр окна: максимальная длина NZW
    const unsigned int r, // Параметр окна: минимальная длина серии нулей, необходимая для перехода из NZW к ZW. ЕСЛИ r==0, то использовать CLNW, иначе - VLNW
    const EC_POINT*    P, // Точка эллиптической кривой, которая будет умножена на k
    const EC_GROUP*    G, // Группа точек эллиптической кривой
    EC_POINT**         Q, // Результат
    const char*        dbgIndex // Номер тестируемого числа для вывода в файл
)
{
    window*      f                     = NULL;  // Массив скользящих окон
    unsigned int s                     = 0;     // Количество окон
    EC_POINT**   iP                    = NULL;  // Массив кратных точек iP
    char         str[CONST_ARRAY_SIZE] = { 0 }; 

    int i, j;
    double finish;

    *Q = EC_POINT_new(G); // Точка Q - результат

    // Выделение памяти для массива iP
    iP = (EC_POINT**)calloc(1 << d, sizeof(EC_POINT*));
    if (!iP)
    {
        errExit("calloc error");
    }


    /// АЛГОРИТМ /// Вычисление кратной точки методом скользящего окна (слева-направо)

    /// 1 /// Для i = [1 - P], 3, 5, 7, ..., 2^d - 1 вычислить и сохранить точки iP
    BIGNUM* bn_i = BN_new();
    BN_dec2bn(&bn_i, "1");
    str[d / 8] = 1 << d % 8;
    BIGNUM* bn_max = BN_new();
    BN_lebin2bn(str, d + 1, bn_max);
    for (i = 1; i < (1 << d); BN_add_word(bn_i, 2), i += 2)
    {
        iP[i] = EC_POINT_new(G);
        EC_POINT_mul(
            G,
            iP[i],
            NULL,
            P,
            bn_i,
            NULL
        );
    }
    BN_clear_free(bn_i);
    BN_clear_free(bn_max);
    memset(str, 0, CONST_MAX_WINDOW_LEN * sizeof(char));

    /// 2 /// Положить Q <- O_{inf}
    EC_POINT_set_to_infinity(G, *Q);

    /// 3 /// Разбить бинарное представление k на нулевые и ненулевые окна соответствующим методом: k = (f_{s-1}, f_{s-2}, ..., f_0)
    s = constructWindows(
        k, // (const) Число k
        d, // (const) Число d
        r, // (const) Число r
        &f // Массив для записи результата
    );
    printWindows(f, s, k);

    /// Оперативный этап ///
    multiply_sliding_window_operative(
        r,
        d,
        s,
        f,
        G,
        iP,
        Q,
        dbgIndex
    );
    
    //printf("\n");

    // v Освобождение памяти
    deleteWindows(&f, s);
    for (i = 0; i < (1 << d); i++)
    {
        EC_POINT_clear_free(iP[i]);
    }
    free(iP);
    // ^

    /// 5 /// Вернуть Q
}


void multiply_sliding_window_modified_operative(
    const unsigned int r,
    const unsigned int d,
    const unsigned int s,
    const unsigned int S_max,
    const unsigned int S_real,
    const unsigned int kRealLen,
    const unsigned int kMaxLen,
    const window* f,
    const EC_GROUP* G,
    const EC_POINT** iP,
    const EC_POINT* P,
    EC_POINT** Q,
    const char* dbgIndex
)
{
    //printf("smax-sreal=%i-%i=%i\n", S_max, S_real, S_max - S_real);


    EC_POINT* T = EC_POINT_new(G); // Точка T - dummy
    EC_POINT_copy(T, P);

    int i, j, test;
    double finish;

    /// Оперативный этап ///
    StartCounter();

    for (test = 0; test < NTESTS; test++)
    {

        /// 4 /// Для i от s-1 до 0 выполнять:
        for (i = s - 1; i >= 0; i--)
        {
            /// 4.1 /// Q <- 2^{len(f_i)}Q
            for (j = 0; j < f[i].length; j++)
            {
                printf("D");
                EC_POINT_dbl(
                    G,
                    *Q,
                    *Q,
                    NULL
                );
            }

            /// 4.2 /// Если f_i != 0, то Q <- Q + f_{i}P
            if (f[i].W_ui)
            {
                printf("A");
                EC_POINT_add(
                    G,
                    *Q,
                    *Q,
                    iP[f[i].W_ui],
                    NULL
                );
            }
        }
        // Дополнительные удвоения
        for (i = 0; i < kMaxLen - kRealLen; i++)
        {
            printf("d");
            EC_POINT_dbl(
                G,
                T,
                T,
                NULL
            );
        }
        ////// Пустые сложения
        for (i = S_max - S_real; i >= 0; i--)
        {
            if (i + 1)
            {
                printf("a");
                EC_POINT_add(
                    G,
                    T,
                    T,
                    P,
                    NULL
                );
            }
        }

    }

    finish = GetCounter();
    printTimeToFile(finish, (r) ? "VLNW" : "CLNW", dbgIndex, d, r);

    EC_POINT_clear_free(T);
}

void multiply_sliding_window_modified(
    const BIGNUM*      k,       // Множитель
    const unsigned int kMaxLen, // Максимальная длина числа k
    const unsigned int d,       // Параметр окна: максимальная длина NZW
    const unsigned int r,       // Параметр окна: минимальная длина серии нулей, необходимая для перехода из NZW к ZW. ЕСЛИ r==0, то использовать CLNW, иначе - VLNW
    const EC_POINT*    P,       // Точка эллиптической кривой, которая будет умножена на k
    const EC_GROUP*    G,       // Группа точек эллиптической кривой
    EC_POINT**         Q,       // Результат
    const char*        dbgIndex // Номер тестируемого числа для вывода в файл
)
{
    window*      f                     = NULL;            // Массив скользящих окон
    unsigned int s                     = 0;               // Количество окон
    unsigned int S_max                 = 0;               // Максимальное количество ненулевых окон
    unsigned int S_real                = 0;               // Действительное количество ненулевых окон
    unsigned int kRealLen              = 0;               // |k|
    EC_POINT**   iP                    = NULL;            // Массив кратных точек iP
    char         str[CONST_ARRAY_SIZE] = { 0 };

    int i, j;
    double finish;

    *Q = EC_POINT_new(G); // Точка Q - результат
    

    // Выделение памяти для массива iP
    iP = (EC_POINT**)calloc(1 << d, sizeof(EC_POINT*));
    if (!iP)
    {
        errExit("calloc error");
    }

    // находим |k|
    BN_bn2lebinpad(k, str, kMaxLen);
    for (kRealLen = kMaxLen; !getBit(str, kRealLen) && kRealLen >= 0; kRealLen--);
    kRealLen++;
    memset(str, 0, CONST_MAX_WINDOW_LEN * sizeof(char));
    //printf("\n|k|=%i\n", kRealLen);


    /// АЛГОРИТМ /// Вычисление кратной точки методом скользящего окна (слева-направо) МОДИФИЦИРОВАННЫЙ

    /// 1 /// Для i = [1 - P], 3, 5, 7, ..., 2^d - 1 вычислить и сохранить точки iP
    BIGNUM* bn_i = BN_new();
    BN_dec2bn(&bn_i, "1");
    str[d / 8] = 1 << d % 8;
    BIGNUM* bn_max = BN_new();
    BN_lebin2bn(str, d + 1, bn_max);
    for (i = 1; i < (1 << d); BN_add_word(bn_i, 2), i += 2)
    {
        iP[i] = EC_POINT_new(G);
        EC_POINT_mul(
            G,
            iP[i],
            NULL,
            P,
            bn_i,
            NULL
        );
    }
    BN_clear_free(bn_i);
    BN_clear_free(bn_max);
    memset(str, 0, CONST_MAX_WINDOW_LEN * sizeof(char));

    /// 2 /// Положить Q <- O_{inf}, T <- O_{inf} (в функции оперативного этапа будет)
    EC_POINT_set_to_infinity(G, *Q);
    //EC_POINT_set_to_infinity(G, T);
    //EC_POINT_copy(T, P);

    /// 3 /// Разбить бинарное представление k на нулевые и ненулевые окна соответствующим методом: k = (f_{s-1}, f_{s-2}, ..., f_0)
    s = constructWindows(
        k, // (const) Число k
        d, // (const) Число d
        r, // (const) Число r
        &f // Массив для записи результата
    );

    printWindows(f, s, k);

       
    ////// Вычислить S_max
    if (r) // vlnw
    {
        S_max = (kMaxLen - 1) / (r + 1) + (((kMaxLen - 1) % (r + 1)) ? 2 : 1);
    }
    else // clnv
    {
        S_max = kMaxLen / d + ((kMaxLen % d) ? 1 : 0);
    }
    //printf("S_max=%i\n", S_max);

    ////// Вычислить S_real
    for (i = s - 1; i >= 0; i--)
    {
        if (f[i].W_ui != 0)
        {
            S_real++;
        }
    }
    //printf("S_real=%i\n", S_real);

    //printWindows(f, s, k);



    /// Оперативный этап ///
    multiply_sliding_window_modified_operative(
        r,
        d,
        s,
        S_max,
        S_real,
        kRealLen,
        kMaxLen,
        f,
        G,
        iP,
        P,
        Q,
        dbgIndex
    );

    //printf("\n");

    // v Освобождение памяти
    deleteWindows(&f, s);

    for (int i = 0; i < (1 << d); i++)
    {
        EC_POINT_clear_free(iP[i]);
    }
    free(iP);

    //EC_POINT_clear_free(T);

    // ^

    /// 5 /// Вернуть Q
    //return Q;
}


