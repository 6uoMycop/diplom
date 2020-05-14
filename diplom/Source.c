#include <stdint.h>
#include <malloc.h>
#include <Windows.h>
#include <stdio.h>

typedef uint64_t _uinteger;
typedef  int64_t  _integer;

const _integer __A = 4; // Коэффициенты уравнения эллиптической кривой:
const _integer __B = 8;  // Z*Y^2 = X^3 + A*X*Z^2 + B*Z^3


void errExit(const char* message)
{
    printf("ERROR: %s\n", message);
    system("pause");
    exit(1);
}

// Точка в проективных координатах
typedef struct pointProjective
{
    _integer X;
    _integer Y;
    _integer Z;
} pointProjective;

pointProjective createPointProjective(
    _integer X,
    _integer Y,
    _integer Z
)
{
    pointProjective P;
    P.X = X;
    P.Y = Y;
    P.Z = Z;
    return P;
}

// Взятие обратной точки эллиптической кривой в проективных координатах
pointProjective inverseProjective(const pointProjective P)
{
    pointProjective Q;
    Q.X = P.X;
    Q.Y = -P.Y;
    Q.Z = P.Z;
    return Q;
}

// Удвоение точки эллиптической кривой в проективных координатах
pointProjective doublingProjective(
    const pointProjective P
)
{
    _integer A1 = 0, B1 = 0;
    pointProjective Q;

    A1 = 3 * P.X * P.X + __A * P.Z * P.Z;
    B1 = 2 * P.Y * P.Z;

    Q.X = B1 * (A1 * A1 - 4 * P.X * P.Y * B1);
    Q.Y = A1 * (6 * P.X * P.Y * B1 - A1 * A1) - 4 * P.Y * P.Y * B1 * B1;
    Q.Z = B1 * B1 * B1;

    return Q;
}

// Сложение двух точек эллиптической кривой в проективных координатах
pointProjective additionProjective(
    const pointProjective P1,
    const pointProjective P2
)
{
    _integer A1 = 0, B1 = 0, C = 0, D = 0, E = 0;
    pointProjective Q;

    A1 = P2.Y * P1.Z - P1.Y * P2.Z;
    B1 = P2.X * P1.Z - P1.X * P2.Z;
    C  = P2.X * P1.Z - P1.X * P2.Z;
    D  = 2 * P1.X * P2.Z + P2.X * P1.Z;
    E  = P1.Z * P2.Z;

    Q.X = B1 * (A1 * A1 * E - B1 * B1 * C);
    Q.Y = A1 * (B1 * B1 * D - A1 * A1 * E) - P1.Y * P2.Z * B1 * B1 * B1;
    Q.Z = B1 * B1 * B1 * E;

    return Q;
}


_integer kmods2w(
    const _integer  k,
    const _uinteger w
) // u = kmods2w(k,w) = k (mod 2^{w}); -2^{w-1} <= u < 2^{w-1}
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

// Вычисление NAFw(k)
void wNAF(
    const _integer  k,   // входное число    -- IN
    const _uinteger w,   // длина окна       -- IN
    _integer**      naf, // NAF_{w}(k)       -- OUT
    unsigned int*   l    // длина NAF_{w}(k) -- OUT
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

// Вычисление кратной точки kP оконным методом NAFw(k)
pointProjective pointMultiplication_wNAF(
    const _uinteger       w, // длина окна                 -- IN
    const _integer        k, // множитель                  -- IN
    const pointProjective P  // точка, которую умножаем    -- IN
)
{
    _integer* naf = NULL;
    unsigned int l = 0;
    pointProjective* P_i = NULL;
    pointProjective Q;

    /// Предварительный этап

    // 1. Вычислить NAFw(k)
    wNAF(k, w, &naf, &l);

    // 2. Вычислить P_{i} = iP, i \in {1,3,5,...,2^{w-1}-1}
    P_i = (pointProjective*)calloc((1 << w - 1) / 2, sizeof(pointProjective));
    if (!P_i)
    {
        errExit("calloc error");
    }

    pointProjective _2P = doublingProjective(P); // чтобы с ней складывать в цикле
    P_i[0] = P;
    for (int i = 1; i < (1 << w - 1) / 2; i++)
    {
        P_i[i] = additionProjective(P_i[i - 1], _2P);
    }

    // 3. Q <- Qinf
    Q = createPointProjective(0, 1, 0);

    /// Непосредственно вычисление
    for (int i = l - 1; i >= 0; i--)
    {
        Q = additionProjective(Q, Q);
        if (naf[i] > 0)
        {
            Q = additionProjective(
                Q, 
                P_i[naf[i]]);
        }
        else if (naf[i] < 0)
        {
            Q = additionProjective(
                Q, 
                inverseProjective(P_i[-naf[i]]));
        }
    }

    free(naf);
    free(P_i);
    return Q;
}



int main()
{
    //_integer* naf = NULL;
    //unsigned int l = 0;
    //wNAF(1122334455, 6, &naf, &l);
    //
    //_integer res = 0;
    //
    //for (int i = 0; i < l; i++)
    //{
    //    res += naf[i] * (1 << i);
    //}

    pointProjective kP;

    kP = pointMultiplication_wNAF(
        3,
        1122334455,
        createPointProjective(64, 8, 16)
    );
    
    printf("(%i, %i, %i)\n", kP.X, kP.Y, kP.Z);

    return 0;
}

