#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <malloc.h>
#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <openssl/bn.h>

#define MY_VERBOSE
#define CONST_ARRAY_SIZE 4096

/*
void createPoint(
    const EC_GROUP* g, 
    EC_POINT**      p, 
    const char*     x_dec, 
    const char*     y_dec, 
    const char*     z_dec)
{
    BIGNUM *X = BN_new(),
           *Y = BN_new(),
           *Z = BN_new();

    BN_dec2bn(&X, x_dec);
    BN_dec2bn(&Y, y_dec);
    BN_dec2bn(&Z, z_dec);

    EC_POINT_set_Jprojective_coordinates_GFp(
        g, 
        *p, 
        X, Y, Z, 
        NULL);


#ifdef MY_VERBOSE
    printf("Created point (%s):\n\tX=%s\n\tY=%s\n\tZ=%s\n", 
        EC_POINT_is_on_curve(g, *p, NULL) ? "on curve" : "NOT on curve",
        BN_bn2dec(X), BN_bn2dec(Y), BN_bn2dec(Z));
#endif // MY_VERBOSE

    BN_clear_free(X);
    BN_clear_free(Y);
    BN_clear_free(Z);
}
*/

typedef struct window {
    BIGNUM*  W;
    uint64_t length;   // Длина окна в битах
} window;


void errExit(const char* message)
{
    printf("ERROR: %s\n", message);
    system("pause");
    exit(1);
}



unsigned char getBit(
    const unsigned char* arr,
    unsigned int         index
)
{
    unsigned char byte      = arr[index / 8];
    unsigned int  bitOffset = index % 8;
    byte <<= 7 - bitOffset;
    byte >>= 7;
    return byte;
}

unsigned int constructBeDataFromBinArray(
    unsigned char**      output, // must be NULL, then u free it
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

int getWindow(
    const unsigned char* bytes,
    const unsigned int   bitOffset,
    //const unsigned int   lenBytes,
    const unsigned int   d,    // Максимальная длина NZW
    const unsigned int   r,    // Минимальная длина серии нулей, необходимая для перехода из NZW к ZW
    window*              resWind
)
{
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // Временный массив для хранения текущего окна. Биты в СИМВОЛЬНОМ (ASCII) представлении - для преобразования затем в BIGNUM
    unsigned char  bit = 0;
    unsigned int   lenWindow = 0; // В NZW это - индекс старшей единицы, индексация с 1. Т.е. если единиц нет - будет 0, иначе - положительное число
    unsigned int   lenWindowOld = 0;
    int            lenCheck = 0;
    unsigned char* resultData = NULL;
    unsigned int   resultNumBytes = 0;
    BOOL flag = FALSE;

    bit = getBit(bytes, bitOffset);
    if (!bit) // ZW
    {
        for (int i = 0; !bit; i++, bit = getBit(bytes, bitOffset + i))
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
        for (;;)
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
    //printf("W: %s\n", BN_bn2hex((*resWind).W));

    free(resultData);
    return lenWindow;
}

unsigned int constructWindows(
    const BIGNUM*      k,    // Входное число
    const unsigned int d,    // Максимальная длина NZW
    const unsigned int r,    // Минимальная длина серии нулей, необходимая для перехода из NZW к ZW
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

    for (int offset = 0; offset < num_bits - 1; windNum++)
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
        /// ^ init

        offset += getWindow(
            &(bytes[offset / 8]),
            offset % 8,
            d,
            r,
            &((*winds)[windNum - 1])
        );



        //printf("W%i: %s\n", windNum - 1, BN_bn2hex((*winds)[windNum - 1].W));

        // TODO getWindow()




    }



    free(bytes);

    return windNum - 1;
}

void printWindows(
    const window* windowsArray,
    const unsigned int windNum,
    const BIGNUM* numberOpt     // Исходное число, которое было разбито на окна. Если NULL, печататься не будет
)
{
    unsigned char bytes[CONST_ARRAY_SIZE] = { 0 };

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
        printf("%s%i:\t", ((BN_is_zero(windowsArray[i].W)) ? "ZW" : "NZW"), i);
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
        }
        printf("\n");
    }
}

int main()
{
    //unsigned char* binary = NULL;
    //constructBeDataFromBinArray(&binary, "101100111101", 12);
    //free(binary);


    BIGNUM* k_tmp = NULL;
    window* windowsArray = NULL;

    //BN_dec2bn(&k_tmp, "269");
    //BN_dec2bn(&k_tmp, "37");
    //BN_dec2bn(&k_tmp, "137830");
    BN_dec2bn(&k_tmp, "399974");
    //BN_dec2bn(&k_tmp, "10");

    //printf("%s\n", BN_bn2hex(k_tmp));

    unsigned char bin[1024] = { 0 };
    
    BN_bn2bin(k_tmp, bin, 1024);

    //int test = BN_num_bits(k_tmp);

    unsigned int windNum = constructWindows(
        k_tmp,
        4,
        2,
        &windowsArray
    );

    printWindows(windowsArray, windNum, k_tmp);

    return 0;


    EC_GROUP* G = EC_GROUP_new_by_curve_name(NID_secp256k1);
    //EC_POINT* P = EC_POINT_new(G);
    EC_POINT* Q = EC_POINT_new(G);


    //EC_POINT_set_to_infinity(G, P);
    //createPoint(G, &Q, "10", "7", "3");

    BIGNUM *x = BN_new(), *y = BN_new(), *z = BN_new();

    EC_POINT_get_Jprojective_coordinates_GFp(G, Q, x, y, z, NULL);
    printf("x %s\ny %s\nz %s\nOn curve %s\nIs inf %s\n",
        BN_bn2hex(x), BN_bn2hex(y), BN_bn2hex(z),
        EC_POINT_is_on_curve(G, Q, NULL) ? "yes" : "no",
        EC_POINT_is_at_infinity(G, Q, NULL) ? "yes" : "no");


    BIGNUM* k = BN_new();
    BN_dec2bn(&k, "10");
    EC_POINT_mul(G, Q, k, NULL, NULL, NULL);

    EC_POINT_get_Jprojective_coordinates_GFp(G, Q, x, y, z, NULL);
    printf("x %s\ny %s\nz %s\nOn curve %s\nIs inf %s\n",
        BN_bn2hex(x), BN_bn2hex(y), BN_bn2hex(z),
        EC_POINT_is_on_curve(G, Q, NULL) ? "yes" : "no",
        EC_POINT_is_at_infinity(G, Q, NULL) ? "yes" : "no");

    BN_clear_free(x);
    BN_clear_free(y);
    BN_clear_free(z);
    BN_clear_free(k);
    //EC_POINT_clear_free(P);
    EC_POINT_clear_free(Q);
    EC_GROUP_clear_free(G);

    system("pause");
    return 0;
}