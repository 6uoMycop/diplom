#include "my_utils.h"
#include "vlnw.h"





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
    unsigned char** output, // must be NULL, then u free it
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
    const unsigned int   lenBits,
    const unsigned int   d,    // ������������ ����� NZW
    const unsigned int   r,    // ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW
    window* resWind
)
{
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // ��������� ������ ��� �������� �������� ����. ���� � ���������� (ASCII) ������������� - ��� �������������� ����� � BIGNUM
    unsigned char  bit = 0;
    unsigned int   lenWindow = 0; // � NZW ��� - ������ ������� �������, ���������� � 1. �.�. ���� ������ ��� - ����� 0, ����� - ������������� �����
    unsigned int   lenWindowOld = 0;
    int            lenCheck = 0;
    int            lenRemaining = lenBits;
    unsigned char* resultData = NULL;
    unsigned int   resultNumBytes = 0;
    BOOL flag = FALSE;

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

unsigned int constructWindows(
    const BIGNUM* k,    // ������� �����
    const unsigned int d,    // ������������ ����� NZW
    const unsigned int r,    // ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW
    window** winds // ������ ��� ������ ����������
)
{
    window* tmpRealloc = NULL;  // ��� �������� ��������� ����� realloc'��
    unsigned char* bytes = NULL;  // ����� �������� �����
    unsigned char  bit = 0;
    int            num_bits = 0;
    unsigned char  tmpBitsForCurrentWindow[CONST_ARRAY_SIZE] = { 0 };  // ��������� ������ ��� �������� �������� ����. ���� � ���������� (ASCII) ������������� - ��� �������������� ����� � BIGNUM
    BOOL           windowType = FALSE; // ZW - FALSE // NZW - TRUE
    unsigned int   windNum = 1;

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

        offset += getWindow(
            &(bytes[offset / 8]),
            offset % 8,
            num_bits - offset,
            d,
            r,
            &((*winds)[windNum - 1])
        );
    }

    free(bytes);

    return windNum - 1;
}

void deleteWindows(
    window** windowsArray,
    const unsigned int windNum
)
{
    for (int i = 0; i < windNum; i++)
    {
        BN_clear_free((*windowsArray)[i].W);
    }
    free(*windowsArray);
}

void printWindows(
    const window* windowsArray,
    const unsigned int windNum,
    const BIGNUM* numberOpt     // �������� �����, ������� ���� ������� �� ����. ���� NULL, ���������� �� �����
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

void multiply_VLNW(
    const BIGNUM* k, // ���������
    const unsigned int d, // �������� ����: ������������ ����� NZW
    const unsigned int r, // �������� ����: ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW
    const EC_POINT* P, // ����� ������������� ������, ������� ����� �������� �� k
    const EC_GROUP* G, // ������ ����� ������������� ������
    EC_POINT** Q
)
{
    window* f = NULL;            // ������ ���������� ����
    unsigned int s = 0;               // ���������� ����
    *Q = EC_POINT_new(G); // ����� Q - ���������
    char         str[CONST_ARRAY_SIZE] = { 0 };
    //BIGNUM** pow2 = NULL;            // ������ ����� � ������� BIGNUM*, ��� pow2[i] = 2^{i}
    EC_POINT** iP = NULL;            // ������ ������� ����� iP

    //time_t start = clock();

    // ���������� ������� pow2
    //pow2 = (BIGNUM**)calloc(CONST_MAX_WINDOW_LEN, sizeof(BIGNUM*));
    //if (!pow2)
    //{
    //    errExit("calloc error");
    //}
    //for (int i = 0; i < CONST_MAX_WINDOW_LEN; i++)
    //{
    //    pow2[i] = BN_new();
    //    memset(str, 0, CONST_MAX_WINDOW_LEN * sizeof(char));
    //    str[i / 8] = 1 << i % 8;
    //    BN_lebin2bn(str, i + 1, pow2[i]);
    //    printf("2^%i=\t%s\n", i, BN_bn2hex(pow2[i]));
    //}
    //memset(str, 0, CONST_MAX_WINDOW_LEN * sizeof(char));

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
    for (int i = 1; i < (1 << d); BN_add_word(bn_i, 2), i += 2)
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

    ///printWindows(f, s, k);///

    //printf("My (pre-operative): %i\n", clock() - start);


    StartCounter();
    /// 4 - ����������� ���� /// ��� i �� s-1 �� 0 ���������:
    for (int i = s - 1; i >= 0; i--)
    {

        /// 4.1 /// Q <- 2^{len(f_i)}Q
        //for (int j = 0; j < pow2[f[i].length - 1]; j++)
        for (int j = 0; j < f[i].length; j++)
        {
            EC_POINT_dbl(
                G,
                *Q,
                *Q,
                NULL
            );
        }
        //EC_POINT_mul(
        //    G,
        //    *Q,
        //    NULL,
        //    *Q,
        //    pow2[f[i].length],
        //    NULL
        //);


        /// 4.2 /// ���� f_i != 0, �� Q <- Q + f_{i}P
        //if (!BN_is_zero(f[i].W))
        if (f[i].W_ui)
        {
            EC_POINT_add(
                G,
                *Q,
                *Q,
                iP[f[i].W_ui],
                NULL,
                NULL,
                NULL
            );
        }
    }

    printf("%f\n", GetCounter());

    // v ������������ ������
    deleteWindows(&f, s);

    //for (int i = 0; i < CONST_MAX_WINDOW_LEN; i++)
    //{
    //    BN_clear_free(pow2[i]);
    //}
    //free(pow2);

    for (int i = 0; i < (1 << d); i++)
    {
        EC_POINT_clear_free(iP[i]);
    }
    free(iP);

    // ^

    /// 5 /// ������� Q
    //return Q;
}


void multiply_VLNW_modified(
    const BIGNUM* k, // ���������
    const unsigned int kMaxLen, // ������������ ����� ����� k
    const unsigned int d, // �������� ����: ������������ ����� NZW
    const unsigned int r, // �������� ����: ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW
    const EC_POINT* P, // ����� ������������� ������, ������� ����� �������� �� k
    const EC_GROUP* G, // ������ ����� ������������� ������
    EC_POINT** Q,
    const int mode // 0 - CLNW, 1 - VLNW
)
{
    window* f = NULL;            // ������ ���������� ����
    unsigned int s = 0;               // ���������� ����
    unsigned int S_max = 0;               // ������������ ���������� ��������� ����
    unsigned int S_real = 0;               // �������������� ���������� ��������� ����
    unsigned int kRealLen = 0;               // �������������� ���������� ��������� ����
    *Q = EC_POINT_new(G); // ����� Q - ���������
    EC_POINT* T = EC_POINT_new(G); // ����� T - dummy

    char    str[CONST_ARRAY_SIZE] = { 0 };

    EC_POINT** iP = NULL;            // ������ ������� ����� iP

    //time_t start = clock();


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
    for (int i = 1; i < (1 << d); BN_add_word(bn_i, 2), i += 2)
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




    /// 2 /// �������� Q <- O_{inf}, T <- O_{inf}
    EC_POINT_set_to_infinity(G, *Q);
    //EC_POINT_set_to_infinity(G, T);
    EC_POINT_copy(T, P);

    /// 3 /// ������� �������� ������������� k �� ������� � ��������� ���� ��������������� �������: k = (f_{s-1}, f_{s-2}, ..., f_0)
    s = constructWindows(
        k, // (const) ����� k
        d, // (const) ����� d
        r, // (const) ����� r
        &f // ������ ��� ������ ����������
    );

    //printf("My (pre-operative): %i\n", clock() - start);


    ////// ��������� S_max
    if (mode) // vlnw
    {
        S_max = (kMaxLen - 1) / (r + 1) + (((kMaxLen - 1) % (r + 1)) ? 2 : 1);
    }
    else // clnv
    {
        S_max = kMaxLen / d + ((kMaxLen % d) ? 1 : 0);
    }
    //printf("S_max=%i\n", S_max);

    ////// ��������� S_real
    for (int i = s - 1; i >= 0; i--)
    {
        if (f[i].W_ui != 0)
        {
            S_real++;
        }
    }
    //printf("S_real=%i\n", S_real);

    //printWindows(f, s, k);

    StartCounter();
    /// 4 - ����������� ���� /// ��� i �� s-1 �� 0 ���������:
    for (int i = s - 1; i >= 0; i--)
    {
        /// 4.1 /// Q <- 2^{len(f_i)}Q
        for (int j = 0; j < f[i].length; j++)
        {
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
            EC_POINT_add(
                G,
                *Q,
                *Q,
                iP[f[i].W_ui],
                NULL,
                NULL,
                NULL
            );
        }
        //else
        //{
        //    EC_POINT_add(
        //        G,
        //        T,
        //        T,
        //        P,
        //        NULL,
        //        NULL,
        //        NULL
        //    );
        //}
    }
    // �������������� ��������
    for (int j = 0; j < kMaxLen - kRealLen; j++)
    {
        EC_POINT_dbl(
            G,
            *Q,
            *Q,
            NULL
        );
    }


    ////// ������ ��������
    for (int i = S_max - S_real; i >= 0; i--)
    {
        EC_POINT_add(
            G,
            T,
            T,
            P,
            NULL,
            NULL,
            NULL
        );
    }

    printf("%f\n", GetCounter());

    // v ������������ ������
    deleteWindows(&f, s);

    for (int i = 0; i < (1 << d); i++)
    {
        EC_POINT_clear_free(iP[i]);
    }
    free(iP);

    EC_POINT_clear_free(T);

    // ^

    /// 5 /// ������� Q
    //return Q;
}

void multiply_VLNW_modified2(
    const BIGNUM* k, // ���������
    const unsigned int kMaxLen, // ������������ ����� ����� k
    const unsigned int d, // �������� ����: ������������ ����� NZW
    const unsigned int r, // �������� ����: ����������� ����� ����� �����, ����������� ��� �������� �� NZW � ZW
    const EC_POINT* P, // ����� ������������� ������, ������� ����� �������� �� k
    const EC_GROUP* G, // ������ ����� ������������� ������
    EC_POINT** Q
)
{
    window* f = NULL;            // ������ ���������� ����
    unsigned int s = 0;               // ���������� ����
    unsigned int S_max = 0;               // ������������ ���������� ��������� ����
    //unsigned int S_real = 0;               // �������������� ���������� ��������� ����
    *Q = EC_POINT_new(G); // ����� Q - ���������
    EC_POINT* T = EC_POINT_new(G); // ����� T - dummy
    EC_POINT* cP = EC_POINT_new(G); // ����� cP - ��� ��������

    char    str[CONST_ARRAY_SIZE] = { 0 };

    BIGNUM** pow2 = NULL;            // ������ ����� � ������� BIGNUM*, ��� pow2[i] = 2^{i}
    EC_POINT** iP = NULL;            // ������ ������� ����� iP

    //time_t start = clock();

    // ���������� ������� pow2
    pow2 = (BIGNUM**)calloc(CONST_MAX_WINDOW_LEN, sizeof(BIGNUM*));
    if (!pow2)
    {
        errExit("calloc error");
    }
    for (int i = 0; i < CONST_MAX_WINDOW_LEN; i++)
    {
        pow2[i] = NULL;
        memset(str, 0, sizeof(char));
        str[i] = '1';
        BN_bin2bn(str, i + 1, pow2[i]);
    }
    memset(str, 0, sizeof(char));

    // ��������� ������ ��� ������� iP
    iP = (EC_POINT**)calloc(1 << d, sizeof(EC_POINT*));
    if (!iP)
    {
        errExit("calloc error");
    }



    /// �������� /// ���������� ������� ����� ������� ����������� ���� (�����-�������) ����������������

    /// 1 /// ��� i = [1 - P], 3, 5, 7, ..., 2^d - 1 ��������� � ��������� ����� iP
    BIGNUM* bn_i = BN_new();
    BN_dec2bn(&bn_i, "1");
    str[d] = '1';
    BIGNUM* bn_max = BN_new();
    BN_bin2bn(str, d + 1, bn_max);
    for (int i = 1; i < (1 << d); BN_add_word(bn_i, 2), i += 2)
    {
        iP[i] = EC_POINT_new(G);
        //EC_POINT_copy(
        //    iP[i], // src
        //    P      // dst
        //);
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
    memset(str, 0, sizeof(char));


    ////// ��������� � ��������� ����� cP
    BIGNUM* bn_c = BN_new();
    BN_dec2bn(&bn_c, "123456789");
    EC_POINT_mul(
        G,
        cP,
        NULL,
        P,
        bn_c,
        NULL
    );
    BN_clear_free(bn_c);


    /// 2 /// �������� Q <- O_{inf}, T <- O_{inf}
    EC_POINT_set_to_infinity(G, *Q);
    EC_POINT_set_to_infinity(G, T);

    /// 3 /// ������� �������� ������������� k �� ������� � ��������� ���� ��������������� �������: k = (f_{s-1}, f_{s-2}, ..., f_0)
    s = constructWindows(
        k, // (const) ����� k
        d, // (const) ����� d
        r, // (const) ����� r
        &f // ������ ��� ������ ����������
    );

    //printWindows(f, s, k);///

    //printf("My (pre-operative): %i\n", clock() - start);



    ////// ��������� S_max
    S_max = 1 + 2 * ((kMaxLen - 1) / (r + 1)) + (((kMaxLen - 1) % (r + 1) > 0) ? 1 : 0);
    ////printf("S_max=%i\n", S_max);

    StartCounter();
    //start = clock();
    /// 4 - ����������� ���� /// ��� i �� s-1 �� 0 ���������:
    for (int i = s - 1; i >= 0; i--)
    {
        ////
        //EC_POINT_get_Jprojective_coordinates_GFp(G, Q, x, y, z, NULL);
        //printf("%i:\nx %s\ny %s\nz %s\n\n", i,
        //    BN_bn2hex(x), BN_bn2hex(y), BN_bn2hex(z));
        ////  

        /// 4.1 /// Q <- 2^{len(f_i)}Q
        EC_POINT_mul(
            G,
            *Q,
            NULL,
            *Q,
            pow2[f[i].length],
            NULL
        );
        ////
        //EC_POINT_get_Jprojective_coordinates_GFp(G, Q, x, y, z, NULL);
        //printf("%i:\nx %s\ny %s\nz %s\n\n", i,
        //    BN_bn2hex(x), BN_bn2hex(y), BN_bn2hex(z));
        ////   


        /// 4.2 /// ���� f_i != 0, �� Q <- Q + f_{i}P
        //if (!BN_is_zero(f[i].W))
        if (f[i].W_ui != 0)
        {
            EC_POINT_add(
                G,
                *Q,
                *Q,
                iP[f[i].W_ui],
                NULL,
                NULL,
                NULL
            );
            ////
            //EC_POINT_get_Jprojective_coordinates_GFp(G, Q, x, y, z, NULL);
            //printf("IF %i:\nx %s\ny %s\nz %s\n\n", i,
            //    BN_bn2hex(x), BN_bn2hex(y), BN_bn2hex(z));
            ////  
        }
        else
        {
            EC_POINT_add(
                G,
                T,
                T,
                cP,
                NULL,
                NULL,
                NULL
            );
        }
    }


    ////// ������ ��������
    for (int i = 0; i < S_max - s; i++)
    {
        EC_POINT_mul(
            G,
            T,
            NULL,
            T,
            pow2[d],
            NULL
        );
        EC_POINT_add(
            G,
            T,
            T,
            cP,
            NULL,
            NULL,
            NULL
        );
    }

    //printf("My (operative): %i\n\n", clock() - start);
    //printf("%i\n", clock() - start);
    printf("%f\n", GetCounter());

    // v ������������ ������
    deleteWindows(&f, s);

    for (int i = 0; i < CONST_MAX_WINDOW_LEN; i++)
    {
        BN_clear_free(pow2[i]);
    }
    free(pow2);

    for (int i = 0; i < (1 << d); i++)
    {
        EC_POINT_clear_free(iP[i]);
    }
    free(iP);

    EC_POINT_clear_free(T);
    EC_POINT_clear_free(cP);

    // ^

    /// 5 /// ������� Q
    //return Q;
}



