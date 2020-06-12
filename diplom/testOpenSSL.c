#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <malloc.h>
#include "vlnw.h"

//#include <openssl/obj_mac.h>
//#include <openssl/ec.h>
//#include <openssl/bn.h>

//#define CONST_ARRAY_SIZE     4096
//#define CONST_MAX_WINDOW_LEN 1024


int main()
{
    ////unsigned char* binary = NULL;
    ////constructBeDataFromBinArray(&binary, "101100111101", 12);
    ////free(binary);
    //
    //
    //BIGNUM* k_tmp = NULL;
    //window* windowsArray = NULL;
    //
    ////BN_dec2bn(&k_tmp, "269");
    ////BN_dec2bn(&k_tmp, "37");
    //BN_dec2bn(&k_tmp, "137830");
    ////BN_dec2bn(&k_tmp, "81");
    ////BN_dec2bn(&k_tmp, "399974");
    ////BN_dec2bn(&k_tmp, "10");
    //
    ////printf("%s\n", BN_bn2hex(k_tmp));
    //
    //unsigned char bin[1024] = { 0 };
    //
    //BN_bn2bin(k_tmp, bin, 1024);
    //
    ////int test = BN_num_bits(k_tmp);
    //
    //unsigned int windNum = constructWindows(
    //    k_tmp,
    //    4, // d
    //    2, // r
    //    &windowsArray
    //);
    //
    //printWindows(windowsArray, windNum, k_tmp);
    //
    //deleteWindows(&windowsArray, windNum);
    //
    ////return 0;


    EC_GROUP* G = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_POINT* P = EC_POINT_new(G);
    //EC_POINT* Q_check = EC_POINT_new(G);
    BIGNUM* x = BN_new(), *y = BN_new(), *z = BN_new();
    BIGNUM* k1 = BN_new();
    BIGNUM* k2 = BN_new();
    BIGNUM* k3 = BN_new();

    //BN_dec2bn(&k, "137830");
    //BN_dec2bn(&k, "9362");
    //BN_dec2bn(&k, "40210710958665");
    //BN_dec2bn(&k, "70368744177663");
    BN_dec2bn(&k1, "57896044618658097711785492504343953926634992332820282019728792003956564819968"); //2^255
    BN_dec2bn(&k2, "792089237316195423456485008687907853269984665640564039457584007913129639936"); // |k|=249
    BN_dec2bn(&k3, "66166908135609254527754848576393090201868562666080322308261476575950359794249");  // 100100100100...1001
    //BN_dec2bn(&k, "1");
    //BN_dec2bn(&k1, "9223372036854775808");  // 2^63
    //BN_dec2bn(&k2, "1468841314162263496");  // |k|=61
    //BN_dec2bn(&k3, "10540996613548315209"); // 100100100100...1001


    //EC_POINT_set_to_infinity(G, P);
    //createPoint(G, &Q, "10", "7", "3");


    //printf("On curve %s\nIs inf %s\n",
    //    EC_POINT_is_on_curve(G, P, NULL) ? "yes" : "no",
    //    EC_POINT_is_at_infinity(G, P, NULL) ? "yes" : "no");


    //BIGNUM* tmp = BN_new();
    //BN_dec2bn(&tmp, "10");
    //EC_POINT_mul(G, P, tmp, NULL, NULL, NULL);
    //
    //EC_POINT_get_Jprojective_coordinates_GFp(G, P, x, y, z, NULL);
    //printf("P:\nx %s\ny %s\nz %s\nOn curve %s\nIs inf %s\n\n",
    //    BN_bn2hex(x), BN_bn2hex(y), BN_bn2hex(z),
    //    EC_POINT_is_on_curve(G, P, NULL) ? "yes" : "no",
    //    EC_POINT_is_at_infinity(G, P, NULL) ? "yes" : "no");



    //time_t start = clock();
    //EC_POINT_mul(
    //    G, 
    //    Q_check, 
    //    NULL, 
    //    P, 
    //    k, 
    //    NULL
    //);
    //printf("EC_POINT_mul: %i\n", clock() - start);

    unsigned int d = 4;
    unsigned int r = 2;
    unsigned int kMaxLen = 256;
    //unsigned int kMaxLen = 64;
    EC_POINT* Q = NULL;
    for (int i = 0; i < 1000; i++)
    {
        multiply_VLNW(
            k1,
            d,
            r,
            P,
            G,
            &Q
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < 1000; i++)
    {
        multiply_VLNW(
            k2,
            d,
            r,
            P,
            G,
            &Q
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < 1000; i++)
    {
        multiply_VLNW(
            k3,
            d,
            r,
            P,
            G,
            &Q
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    printf("\n\n---------\n\n");
    printf("\n\n---------\n\n");
    printf("\n\n---------\n\n");
    for (int i = 0; i < 1000; i++)
    {
        multiply_VLNW_modified(
            k1,
            kMaxLen, // max(|k|)
            d,
            r,
            P,
            G,
            &Q,
            1
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < 1000; i++)
    {
        multiply_VLNW_modified(
            k2,
            kMaxLen, // max(|k|)
            d,
            r,
            P,
            G,
            &Q,
            1
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < 1000; i++)
    {
        multiply_VLNW_modified(
            k3,
            kMaxLen, // max(|k|)
            d,
            r,
            P,
            G,
            &Q,
            1
        );
        EC_POINT_clear_free(Q);
    }
    //printf("\n\n---------\n\n");
    //for (int i = 0; i < 1000; i++)
    //{
    //    multiply_VLNW_modified2(
    //        k,
    //        256, // max(|k|)
    //        d,
    //        r,
    //        P,
    //        G,
    //        &Q
    //    );
    //    EC_POINT_clear_free(Q);
    //}

    //EC_POINT_get_Jprojective_coordinates_GFp(G, Q, x, y, z, NULL);
    //printf("Q:\nx %s\ny %s\nz %s\nOn curve %s\nIs inf %s\n\n",
    //    BN_bn2hex(x), BN_bn2hex(y), BN_bn2hex(z),
    //    EC_POINT_is_on_curve(G, Q, NULL) ? "yes" : "no",
    //    EC_POINT_is_at_infinity(G, Q, NULL) ? "yes" : "no");
    //
    //EC_POINT_get_Jprojective_coordinates_GFp(G, Q_check, x, y, z, NULL);
    //printf("Q_check:\nx %s\ny %s\nz %s\nOn curve %s\nIs inf %s\n\n",
    //    BN_bn2hex(x), BN_bn2hex(y), BN_bn2hex(z),
    //    EC_POINT_is_on_curve(G, Q_check, NULL) ? "yes" : "no",
    //    EC_POINT_is_at_infinity(G, Q_check, NULL) ? "yes" : "no");
    //
    //printf("Q equal Q_check: %s\n",
    //    EC_POINT_cmp(G, Q, Q_check, NULL) ? "no" : "yes");


    BN_clear_free(k1);
    BN_clear_free(k2);
    BN_clear_free(k3);
    EC_POINT_clear_free(P);
    //EC_POINT_clear_free(Q);
    //EC_POINT_clear_free(Q_check);
    EC_GROUP_clear_free(G);

    system("pause");
    return 0;
}