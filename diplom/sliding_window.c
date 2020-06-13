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
    const unsigned int   d,         // ������������ ����� NZW
    const unsigned int   r,         // ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW
    window* resWind
)
{
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // ��������� ������ ��� �������� �������� ����. ���� � ���������� (ASCII) ������������� - ��� �������������� ����� � BIGNUM
    unsigned char  bit                                       = 0;
    unsigned int   lenWindow                                 = 0; // � NZW ��� - ������ ������� �������, ���������� � 1. �.�. ���� ������ ��� - ����� 0, ����� - ������������� �����
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
                    lenWindow = lenWindowOld + i + 1; // ���������� ������ �������. � ����� ��� ����� ����� �� ������� �������
                    flag = TRUE;
                }
            }

            if (flag) // �������� � NZW, ����������
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
                //// ��������� NZW. ������� ������������ ������ �������
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
    const unsigned int   d,         // ������������ ����� NZW
    window* resWind
)
{
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // ��������� ������ ��� �������� �������� ����. ���� � ���������� (ASCII) ������������� - ��� �������������� ����� � BIGNUM
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
    const BIGNUM*      k,    // ������� �����
    const unsigned int d,    // ������������ ����� NZW
    const unsigned int r,    // ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW. ���� r==0, �� ������������ CLNW, ����� - VLNW
    window**           winds // ������ ��� ������ ����������
)
{
    window*        tmpRealloc                                = NULL;  // ��� �������� ��������� ����� realloc'��
    unsigned char* bytes                                     = NULL;  // ����� �������� �����
    unsigned char  bit                                       = 0;
    int            num_bits                                  = 0;
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // ��������� ������ ��� �������� �������� ����. ���� � ���������� (ASCII) ������������� - ��� �������������� ����� � BIGNUM
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
        // �������� ����� ����
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
    window**           windowsArray, // ���� ����
    const unsigned int windNum       // ���������� ����
)
{
    for (int i = 0; i < windNum; i++)
    {
        BN_clear_free((*windowsArray)[i].W);
    }
    free(*windowsArray);
}

void printWindows(
    const window*      windowsArray, // ���� ����
    const unsigned int windNum,      // ���������� ����
    const BIGNUM*      numberOpt     // �������� �����, ������� ���� ������� �� ����. ���� NULL, ���������� �� �����
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

    /// ����������� ���� ///
    StartCounter();

    for (test = 0; test < NTESTS; test++)
    {

        /// 4 /// ��� i �� s-1 �� 0 ���������:
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

            /// 4.2 /// ���� f_i != 0, �� Q <- Q + f_{i}P
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
    const BIGNUM*      k, // ���������
    const unsigned int d, // �������� ����: ������������ ����� NZW
    const unsigned int r, // �������� ����: ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW. ���� r==0, �� ������������ CLNW, ����� - VLNW
    const EC_POINT*    P, // ����� ������������� ������, ������� ����� �������� �� k
    const EC_GROUP*    G, // ������ ����� ������������� ������
    EC_POINT**         Q, // ���������
    const char*        dbgIndex // ����� ������������ ����� ��� ������ � ����
)
{
    window*      f                     = NULL;  // ������ ���������� ����
    unsigned int s                     = 0;     // ���������� ����
    EC_POINT**   iP                    = NULL;  // ������ ������� ����� iP
    char         str[CONST_ARRAY_SIZE] = { 0 }; 

    int i, j;
    double finish;

    *Q = EC_POINT_new(G); // ����� Q - ���������

    // ��������� ������ ��� ������� iP
    iP = (EC_POINT**)calloc(1 << d, sizeof(EC_POINT*));
    if (!iP)
    {
        errExit("calloc error");
    }


    /// �������� /// ���������� ������� ����� ������� ����������� ���� (�����-�������)

    /// 1 /// ��� i = [1 - P], 3, 5, 7, ..., 2^d - 1 ��������� � ��������� ����� iP
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

    /// 2 /// �������� Q <- O_{inf}
    EC_POINT_set_to_infinity(G, *Q);

    /// 3 /// ������� �������� ������������� k �� ������� � ��������� ���� ��������������� �������: k = (f_{s-1}, f_{s-2}, ..., f_0)
    s = constructWindows(
        k, // (const) ����� k
        d, // (const) ����� d
        r, // (const) ����� r
        &f // ������ ��� ������ ����������
    );
    printWindows(f, s, k);

    /// ����������� ���� ///
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

    // v ������������ ������
    deleteWindows(&f, s);
    for (i = 0; i < (1 << d); i++)
    {
        EC_POINT_clear_free(iP[i]);
    }
    free(iP);
    // ^

    /// 5 /// ������� Q
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


    EC_POINT* T = EC_POINT_new(G); // ����� T - dummy
    EC_POINT_copy(T, P);

    int i, j, test;
    double finish;

    /// ����������� ���� ///
    StartCounter();

    for (test = 0; test < NTESTS; test++)
    {

        /// 4 /// ��� i �� s-1 �� 0 ���������:
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

            /// 4.2 /// ���� f_i != 0, �� Q <- Q + f_{i}P
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
        // �������������� ��������
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
        ////// ������ ��������
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
    const BIGNUM*      k,       // ���������
    const unsigned int kMaxLen, // ������������ ����� ����� k
    const unsigned int d,       // �������� ����: ������������ ����� NZW
    const unsigned int r,       // �������� ����: ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW. ���� r==0, �� ������������ CLNW, ����� - VLNW
    const EC_POINT*    P,       // ����� ������������� ������, ������� ����� �������� �� k
    const EC_GROUP*    G,       // ������ ����� ������������� ������
    EC_POINT**         Q,       // ���������
    const char*        dbgIndex // ����� ������������ ����� ��� ������ � ����
)
{
    window*      f                     = NULL;            // ������ ���������� ����
    unsigned int s                     = 0;               // ���������� ����
    unsigned int S_max                 = 0;               // ������������ ���������� ��������� ����
    unsigned int S_real                = 0;               // �������������� ���������� ��������� ����
    unsigned int kRealLen              = 0;               // |k|
    EC_POINT**   iP                    = NULL;            // ������ ������� ����� iP
    char         str[CONST_ARRAY_SIZE] = { 0 };

    int i, j;
    double finish;

    *Q = EC_POINT_new(G); // ����� Q - ���������
    

    // ��������� ������ ��� ������� iP
    iP = (EC_POINT**)calloc(1 << d, sizeof(EC_POINT*));
    if (!iP)
    {
        errExit("calloc error");
    }

    // ������� |k|
    BN_bn2lebinpad(k, str, kMaxLen);
    for (kRealLen = kMaxLen; !getBit(str, kRealLen) && kRealLen >= 0; kRealLen--);
    kRealLen++;
    memset(str, 0, CONST_MAX_WINDOW_LEN * sizeof(char));
    //printf("\n|k|=%i\n", kRealLen);


    /// �������� /// ���������� ������� ����� ������� ����������� ���� (�����-�������) ����������������

    /// 1 /// ��� i = [1 - P], 3, 5, 7, ..., 2^d - 1 ��������� � ��������� ����� iP
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

    /// 2 /// �������� Q <- O_{inf}, T <- O_{inf} (� ������� ������������ ����� �����)
    EC_POINT_set_to_infinity(G, *Q);
    //EC_POINT_set_to_infinity(G, T);
    //EC_POINT_copy(T, P);

    /// 3 /// ������� �������� ������������� k �� ������� � ��������� ���� ��������������� �������: k = (f_{s-1}, f_{s-2}, ..., f_0)
    s = constructWindows(
        k, // (const) ����� k
        d, // (const) ����� d
        r, // (const) ����� r
        &f // ������ ��� ������ ����������
    );

    printWindows(f, s, k);

       
    ////// ��������� S_max
    if (r) // vlnw
    {
        S_max = (kMaxLen - 1) / (r + 1) + (((kMaxLen - 1) % (r + 1)) ? 2 : 1);
    }
    else // clnv
    {
        S_max = kMaxLen / d + ((kMaxLen % d) ? 1 : 0);
    }
    //printf("S_max=%i\n", S_max);

    ////// ��������� S_real
    for (i = s - 1; i >= 0; i--)
    {
        if (f[i].W_ui != 0)
        {
            S_real++;
        }
    }
    //printf("S_real=%i\n", S_real);

    //printWindows(f, s, k);



    /// ����������� ���� ///
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

    // v ������������ ������
    deleteWindows(&f, s);

    for (int i = 0; i < (1 << d); i++)
    {
        EC_POINT_clear_free(iP[i]);
    }
    free(iP);

    //EC_POINT_clear_free(T);

    // ^

    /// 5 /// ������� Q
    //return Q;
}


