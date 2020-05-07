#include <stdint.h>
#include <malloc.h>
#include <Windows.h>
#include <stdio.h>

//typedef uint32_t _uinteger;
typedef  int32_t  _integer;

void errExit(const char* message)
{
    printf("ERROR: %s\n", message);
    system("pause");
    exit(1);
}



_integer kmods2w(const _integer k, const unsigned int w) // u = kmods2w(k,w) = k (mod 2^{w}); -2^{w-1} <= u < 2^{w-1}
{
    _integer u = k % (1 << w);
    while (1)
    {
        if (u >= (1 << (w - 1)))
        {
            u -= (1 << w);
        }
        else if (u < -(1 << (w - 1)))
        {
            u += (1 << w);
        }
        else
        {
            return u;
        }
    }
}

void NAFw(
    const _integer     k,   // входное число    -- IN
    const unsigned int w,   // длина окна       -- IN
    _integer**         naf, // NAF_{w}(k)       -- OUT
    unsigned int*      l    // длина NAF_{w}(k) -- OUT
)
{
    int i = 0;
    _integer K = k;
    _integer* k_i = NULL;
    k_i = (_integer*)calloc(1, sizeof(_integer));
    if (!k_i)
    {
        errExit("calloc error");
    }

    while (K >= 1)
    {
        k_i = (_integer*)realloc(k_i, (i + 1) * sizeof(_integer));
        if (!k_i)
        {
            errExit("realloc error");
        }

        if (K % 2 == 1)
        {
            k_i[i] = kmods2w(K, w);
            K -= k_i[i];
        }
        else
        {
            k_i[i] = 0;
        }
        K /= 2;
        i++;
    }

    *naf = (_integer*)malloc(i * sizeof(_integer));
    if (!*naf)
    {
        errExit("malloc error");
    }
    memcpy(*naf, k_i, i * sizeof(_integer));
    *l = i;
}


int main()
{
    _integer* naf = NULL;
    unsigned int l = 0;
    NAFw(1122334455, 6, &naf, &l);

    _integer res = 0;

    for (int i = 0; i < l; i++)
    {
        res += naf[i] * (1 << i);
    }

    return 0;
}

