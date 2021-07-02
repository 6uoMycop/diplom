#include <stdlib.h>
#include "../api.h"
#include "print.h"
#include "cpucycles.h"
#include "fail.h"
#include "avr.h"
struct number {
  unsigned char v[32];
};
struct my_point {
  struct number u;
  struct number v;
};

extern char bigint_mul256( unsigned char* r, const unsigned char* a, const unsigned char* b );
extern char bigint_square256( unsigned char* r, const unsigned char* a);
extern char bigint_subp( unsigned char* r, const unsigned char* a);

extern void avrnacl_fe25519_red( struct number *r, unsigned char *C );/* reduction modulo 2^255-19 */
extern void avrnacl_fe25519_add( struct number *r, const struct number *x, const struct number *y );
extern void avrnacl_fe25519_sub( struct number *r, const struct number *x, const struct number *y );


//static unsigned char *n;
//static unsigned char *p;

/* d */
static const struct number d = {{0xA3, 0x78, 0x59, 0x13, 0xCA, 0x4D, 0xEB, 0x75, 0xAB, 0xD8, 0x41, 0x41, 0x4D, 0x0A, 0x70, 0x00, 
                                 0x98, 0xE8, 0x79, 0x77, 0x79, 0x40, 0xC7, 0x8C, 0x73, 0xFE, 0x6F, 0x2B, 0xEE, 0x6C, 0x03, 0x52}};

void my_setzero(struct number *r)
{
  unsigned char i;
  for( i = 0; i < 32; i++ )
  {
    r->v[i] = 0;
  }
}
void my_setone(struct number *r) 
{
  unsigned char i;
  r->v[0] = 1;
  for( i = 1; i < 32; i++ )
  {
    r->v[i] = 0;
  }
}

void my_neg( struct number *r, const struct number *x ) 
{
  struct number t;
  my_setzero( &t );
  avrnacl_fe25519_sub( r, &t, x );
}

void my_mul( struct number *r, const struct number *x, const struct number *y )
{
  unsigned char t[64];
  bigint_mul256( t, x->v, y->v );
  avrnacl_fe25519_red( r, t );
} 

void my_square( struct number *r, const struct number *x )
{
  unsigned char t[64];
  bigint_square256( t, x->v );
  avrnacl_fe25519_red( r, t );
}

void my_invert( struct number *r, const struct number *x )
{
	struct number z2;
	struct number z11;
	struct number z2_10_0;
	struct number z2_50_0;
	struct number z2_100_0;
	struct number t0;
	struct number t1;
	unsigned char i;

	/* 2 */ my_square(&z2,x);
	/* 4 */ my_square(&t1,&z2);
	/* 8 */ my_square(&t0,&t1);
	/* 9 */ my_mul(&z2_10_0,&t0,x);
	/* 11 */ my_mul(&z11,&z2_10_0,&z2);
	/* 22 */ my_square(&t0,&z11);
	/* 2^5 - 2^0 = 31 */ my_mul(&z2_10_0,&t0,&z2_10_0);

	/* 2^6 - 2^1 */ my_square(&t0,&z2_10_0);
	/* 2^7 - 2^2 */ my_square(&t1,&t0);
	/* 2^8 - 2^3 */ my_square(&t0,&t1);
	/* 2^9 - 2^4 */ my_square(&t1,&t0);
	/* 2^10 - 2^5 */ my_square(&t0,&t1);
	/* 2^10 - 2^0 */ my_mul(&z2_10_0,&t0,&z2_10_0);

	/* 2^11 - 2^1 */ my_square(&t0,&z2_10_0);
	/* 2^12 - 2^2 */ my_square(&t1,&t0);
	/* 2^20 - 2^10 */ for (i = 2;i < 10;i += 2) { my_square(&t0,&t1); my_square(&t1,&t0); }
	/* 2^20 - 2^0 */ my_mul(&z2_50_0,&t1,&z2_10_0);

	/* 2^21 - 2^1 */ my_square(&t0,&z2_50_0);
	/* 2^22 - 2^2 */ my_square(&t1,&t0);
	/* 2^40 - 2^20 */ for (i = 2;i < 20;i += 2) { my_square(&t0,&t1); my_square(&t1,&t0); }
	/* 2^40 - 2^0 */ my_mul(&t0,&t1,&z2_50_0);

	/* 2^41 - 2^1 */ my_square(&t1,&t0);
	/* 2^42 - 2^2 */ my_square(&t0,&t1);
	/* 2^50 - 2^10 */ for (i = 2;i < 10;i += 2) { my_square(&t1,&t0); my_square(&t0,&t1); }
	/* 2^50 - 2^0 */ my_mul(&z2_50_0,&t0,&z2_10_0);

	/* 2^51 - 2^1 */ my_square(&t0,&z2_50_0);
	/* 2^52 - 2^2 */ my_square(&t1,&t0);
	/* 2^100 - 2^50 */ for (i = 2;i < 50;i += 2) { my_square(&t0,&t1); my_square(&t1,&t0); }
	/* 2^100 - 2^0 */ my_mul(&z2_100_0,&t1,&z2_50_0);

	/* 2^101 - 2^1 */ my_square(&t1,&z2_100_0);
	/* 2^102 - 2^2 */ my_square(&t0,&t1);
	/* 2^200 - 2^100 */ for (i = 2;i < 100;i += 2) { my_square(&t1,&t0); my_square(&t0,&t1); }
	/* 2^200 - 2^0 */ my_mul(&t1,&t0,&z2_100_0);

	/* 2^201 - 2^1 */ my_square(&t0,&t1);
	/* 2^202 - 2^2 */ my_square(&t1,&t0);
	/* 2^250 - 2^50 */ for (i = 2;i < 50;i += 2) { my_square(&t0,&t1); my_square(&t1,&t0); }
	/* 2^250 - 2^0 */ my_mul(&t0,&t1,&z2_50_0);

	/* 2^251 - 2^1 */ my_square(&t1,&t0);
	/* 2^252 - 2^2 */ my_square(&t0,&t1);
	/* 2^253 - 2^3 */ my_square(&t1,&t0);
	/* 2^254 - 2^4 */ my_square(&t0,&t1);
	/* 2^255 - 2^5 */ my_square(&t1,&t0);
	/* 2^255 - 21 */ my_mul(r,&t1,&z11);
}


void my_points_add( struct my_point* R, struct my_point* P, struct my_point* Q )
{
  struct number uv_1;
  struct number uv_2;
  struct number t1;
  struct number t2;
  struct number t3;
  struct number t4;
  struct number t5;
  struct number t6;
  struct number t7;
  struct number t8;
  struct number one;

  my_setzero( &(R->u) );
  my_setzero( &(R->v) );
  my_setzero( &uv_1 );
  my_setzero( &uv_2 );
  my_setzero( &t1 );
  my_setzero( &t2 );
  my_setzero( &t3 );
  my_setzero( &t4 );
  my_setzero( &t5 );
  my_setzero( &t6 );
  my_setzero( &t7 );
  my_setzero( &t8 );
  my_setone( &one );
  
  /* u1v2 */
  my_mul( &uv_1, &(P->u), &(Q->v) );
  /* u2v1 */
  my_mul( &uv_2, &(Q->u), &(P->v) );

  /* u1v2 + u2v1 */
  avrnacl_fe25519_add( &t1, &uv_1, &uv_2 );
  
  /* m = u1v2 * u2v1 */
  my_mul( &t2, &uv_1, &uv_2 );
  
  my_setzero( &uv_1 );
  my_setzero( &uv_2 );

  /* v1v2 */
  my_mul( &uv_1, &(P->v), &(P->v) );
  /* u1u2 */
  my_mul( &uv_2, &(Q->u), &(Q->u) );
  
  /* v1v2 + u1u2 */
  avrnacl_fe25519_add( &t3, &uv_1, &uv_2 );
  
  /* dm */
  my_mul( &t4, &d, &t2 );

  /* 1 + dm */
  avrnacl_fe25519_add( &t5, &one, &t4 );
  /* 1 - dm */
  avrnacl_fe25519_sub( &t6, &one, &t4 );
  
  /* (1+dm)^-1 */
  my_invert( &t7, &t5 );
  /* (1-dm)^-1 */
  my_invert( &t8, &t6 );

  /*result*/
  my_mul( &(R->u), &t1, &t7 );
  my_mul( &(R->v), &t3, &t8 );
  
}

int main( void )
{
  

  struct my_point R, P, Q;

  my_setzero( &(P.u) );
  my_setzero( &(P.v) );
  my_setzero( &(Q.u) );
  my_setzero( &(Q.v) );

  P.u.v[31] = 0xAB;
  P.v.v[31] = 0x08;
  Q.u.v[30] = 0x9D;
  Q.v.v[31] = 0x35;
  
  my_points_add( &R, &P, &Q );
  print( "u1\n" );
  print_bytes( P.u.v, 32 );
  print( "\nv1\n" );
  print_bytes( P.v.v, 32 );

  print( "\n\nu2\n" );
  print_bytes( Q.u.v, 32 );
  print( "\nv2\n" );
  print_bytes( Q.v.v, 32 );

  print( "\n\nu3\n" );
  print_bytes( R.u.v, 32 );
  print( "\nv3\n" );
  print_bytes( R.v.v, 32 );

  avr_end();
  return 0;
}
