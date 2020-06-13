#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <malloc.h>
#include "sliding_window.h"

//#include <openssl/obj_mac.h>
//#include <openssl/ec.h>
//#include <openssl/bn.h>

//#define CONST_ARRAY_SIZE     4096
//#define CONST_MAX_WINDOW_LEN 1024


int main()
{
    EC_GROUP* G  = EC_GROUP_new_by_curve_name(NID_secp256k1);
    EC_POINT* P  = EC_POINT_new(G);
    BIGNUM*   k1 = BN_new();
    BIGNUM*   k2 = BN_new();
    BIGNUM*   k3 = BN_new();
    BIGNUM*   k3_1 = BN_new();
    int numTests = 1;

    BN_dec2bn(&k1, "57896044618658097711785492504343953926634992332820282019728792003956564819968"); //2^255
    BN_dec2bn(&k2, "792089237316195423456485008687907853269984665640564039457584007913129639936"); // |k|=249
    BN_dec2bn(&k3, "66166908135609254527754848576393090201868562666080322308261476575950359794249");  // 100100100100...1001
    BN_dec2bn(&k3_1, "69475253542389717254142591005212744711961990799384338423674550404747877783961");  // 100110011001100...11001 - худший случай для CLNW

    //BN_dec2bn(&k1, "9223372036854775808");  // 2^63
    //BN_dec2bn(&k2, "1468841314162263496");  // |k|=61
    //BN_dec2bn(&k3, "10540996613548315209"); // 100100100100...1001


    unsigned int d = 31;
    unsigned int r = 31;
    unsigned int kMaxLen = 256;
    //unsigned int kMaxLen = 64;
    EC_POINT* Q = NULL;

    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window(
            k1,
            d,
            r,
            P,
            G,
            &Q,
            "_k1"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window(
            k2,
            d,
            r,
            P,
            G,
            &Q,
            "_k2"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window(
            k3,
            d,
            r,
            P,
            G,
            &Q,
            "_k3"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window(
            k1,
            d,
            0,
            P,
            G,
            &Q,
            "_k1"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window(
            k2,
            d,
            0,
            P,
            G,
            &Q,
            "_k2"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window(
            k3_1,
            d,
            0,
            P,
            G,
            &Q,
            "_k31"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window_modified(
            k1,
            kMaxLen, // max(|k|)
            d,
            r,
            P,
            G,
            &Q,
            "_m_k1"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window_modified(
            k2,
            kMaxLen, // max(|k|)
            d,
            r,
            P,
            G,
            &Q,
            "_m_k2"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window_modified(
            k3,
            kMaxLen, // max(|k|)
            d,
            r,
            P,
            G,
            &Q,
            "_m_k3"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window_modified(
            k1,
            kMaxLen, // max(|k|)
            d,
            0,
            P,
            G,
            &Q,
            "_m_k1"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window_modified(
            k2,
            kMaxLen, // max(|k|)
            d,
            0,
            P,
            G,
            &Q,
            "_m_k2"
        );
        EC_POINT_clear_free(Q);
    }
    printf("\n\n---------\n\n");
    for (int i = 0; i < numTests; i++)
    {
        multiply_sliding_window_modified(
            k3_1,
            kMaxLen, // max(|k|)
            d,
            0,
            P,
            G,
            &Q,
            "_m_k31"
        );
        EC_POINT_clear_free(Q);
    }





    BN_clear_free(k1);
    BN_clear_free(k2);
    BN_clear_free(k3);
    EC_POINT_clear_free(P);
    EC_GROUP_clear_free(G);

    system("pause");
    return 0;
}