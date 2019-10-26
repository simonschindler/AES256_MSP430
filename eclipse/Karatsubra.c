/*
 ============================================================================
 Name        : Karatsubra.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ntru.h"
#include "time.h"


// The data needed to convert a number mod 3 to a number in the range: -1..1.
static short neg_mod_3_data_cpu[5] = { 1, -1, 0, 1, -1 };
// The variable used to convert a number mod 3 to a number in the range: -1..1.
static short *neg_mod_3_cpu = &neg_mod_3_data_cpu[2];
static unsigned char data_dec_data[3] = { 0, 1, 1 };    // for data encode/decode
static unsigned char *data_dec = &data_dec_data[1];


//modulo operations
//standard mod operation
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

//mod operation with neg output
int modn(int a, int b)
{
    int r = a % b;
    return (r < (-b/2 + 1) ? r + b : r > b/2 ? r-b : r);
}

//mod operation with neg output, just works when b is a number with the base 2 (2 4 8 16...)
int modn2(int a, int b)
{
    int r = a & (b-1);
    return (r < (-b/2 + 1) ? r + b : r > b/2 ? r-b : r);
}

//multiplication modq
int16_t multmodn2(int16_t a, int16_t b, uint16_t q)
{
    int32_t x = a*b;
    return (int16_t) modn2(x,q);
}





uint16_t ind(uint16_t j, uint16_t i, uint16_t deg)
{
    return (j + i) <(deg) ? j + i : j + i - deg;
}


//q has to be a number with the base 2 (2 4 8 16...)
void polyMulMod_Q_With_r(int16_t *res, int16_t *polyA, int16_t *r, uint16_t deg,
                         uint16_t q)
{
    uint16_t i=0, j=0;
    for (i = 0; i<=deg; i++)
    {
        switch(r[i]) //sparseform
        {
        case 1:
            for (j = 0; j<=deg; j++)
            {
                res[ind(j,i,deg+1)] = modn2(res[ind(j,i,deg+1)]+polyA[j],q);
            }
            break;
        case -1:
            for (j = 0; j<=deg; j++)
            {
                res[ind(j,i,deg+1)] = modn2(res[ind(j,i,deg+1)]-polyA[j],q);
            }
            break;
        default:
            break;
        }
    }
}

//q has to be a number with the base 2 (2 4 8 16...)
//flat form multipication
void simplePolyMulModQ(int16_t *res, int16_t *polyA, int16_t *polyB, uint16_t deg,
                       uint16_t q)
{
    uint16_t i=0, j=0;
    for (i = 0; i<=deg; i++)
    {
        if(polyB[i]) //sparseform
        {
            for (j = 0; j<=deg; j++)
            {
                // truncated polynomial multiplication
                if(polyA[j]) //sparseform
                {
                    res[ind(j,i,deg+1)] += polyB[i]*polyA[j];
                    res[ind(j,i,deg+1)] = modn2(res[ind(j,i,deg+1)],q);
                }
            }
        }
    }
}


//polyB must be 8-bit (msg)
//q has to be a number with the base 2 (2 4 8 16...)
void simplePolyAddModQ(int16_t *res, int16_t *polyA, int16_t *polyB, uint16_t deg,
                       uint16_t q)
{
    uint16_t i=0;
    for (i = 0; i<=deg; i++)
    {
        // polynomial addition
        res[i] = modn2(polyB[i]+polyA[i],q);
    }

}

//polyB must be 8-bit (msg)
//q has to be a number with the base 2 (2 4 8 16...)
void simplePolySubModQ(int16_t *res, int16_t *polyA, int16_t *polyB, uint16_t deg,
                       uint16_t q)
{
    uint16_t i=0;
    for (i = 0; i<=deg; i++)
    {
        // polynomial addition
        res[i] = modn2(polyA[i]-polyB[i],q);
    }

}

//8-Bit version for message
//q has to be a number with the base 2 (2 4 8 16...)
void simplePolyMulModQ8(int16_t *res, int16_t *polyA, int8_t *polyB, uint16_t deg,
                        uint16_t q)
{
    uint16_t i=0, j=0;
    for (i = 0; i<=deg; i++)
    {
        if(polyB[i])//sparseform
        {
            for (j = 0; j<=deg; j++)
            {
                // truncated polynomial multiplication
                if(polyA[j])//sparseform
                {
                    res[ind(j,i,deg+1)] += polyB[i]*polyA[j];
                    res[ind(j,i,deg+1)] = modn2(res[ind(j,i,deg+1)],q);
                }
            }
        }
    }
}


//polyB must be 8-bit (msg)
//q has to be a number with the base 2 (2 4 8 16...)
void simplePolyAddModQ8(int16_t *res, int16_t *polyA, int8_t *polyB, uint16_t deg,
                        uint16_t q)
{
    uint16_t i=0;
    for (i = 0; i<=deg; i++)
    {
        // polynomial addition
        res[i] = modn2(polyB[i]+polyA[i],q);
    }

}



//q has to be a number with the base 2 (2 4 8 16...)
//karatsubra-algorithm for polynomial multiplication modular q
void karatsubra_poly_mult_modq(int16_t *res, int16_t *polyA, int16_t *polyB, uint16_t deg,
                               uint16_t q)
{
    uint8_t i,j;
    if(deg <= 171)
    {
        for (i = 0; i<=deg; i++)
        {
            if(polyB[i])//sparseform
            {
                for (j = 0; j<=deg; j++)
                {
                    // truncated polynomial multiplication
                    if(polyA[j])//sparseform
                    {
                        res[j+i] += multmodn2(polyB[i],polyA[j],q);
                        res[j+i] = modn2(res[j+i],q);
                    }
                }
            }
        }

    }
    else
    {
        uint16_t k = (deg+1)/2;
        uint16_t r = (deg+1)%2;

        uint16_t i ;
        int16_t* a1 = (int16_t*) calloc((k+r),sizeof(int16_t));//upper part
        int16_t* a0 = (int16_t*) calloc(k+r,sizeof(int16_t));//low part
        int16_t* b1 = (int16_t*) calloc(k+r,sizeof(int16_t));//upper part
        int16_t* b0 = (int16_t*) calloc(k+r,sizeof(int16_t));//low part
        int16_t* a10 = (int16_t*) calloc(k+r,sizeof(int16_t)); ///
        int16_t* b10 = (int16_t*) calloc(k+r,sizeof(int16_t)); ///
        int16_t* p2 = (int16_t*) calloc(4*k+2*r,sizeof(int16_t)); ///
        int16_t* p1 = (int16_t*) calloc(4*k+2*r,sizeof(int16_t)); ///
        int16_t* p0 = (int16_t*) calloc(4*k+2*r,sizeof(int16_t)); ///

        int16_t* c2 = (int16_t*) calloc(4*k+2*r,sizeof(int16_t)); ///
        int16_t* c1 = (int16_t*) calloc(4*k+2*r,sizeof(int16_t)); //
        int16_t* c0 = (int16_t*) calloc(4*k+2*r,sizeof(int16_t)); ///



        //split into upper an lower parts
        for( i = k; i<=deg;i++)
        {
            a1[i-k] = polyA[i];
            b1[i-k] = polyB[i];
        }

        for(i = 0; i<k;i++)
        {
            a0[i] = polyA[i];
            b0[i] = polyB[i];
        }

        //p2 = karatsuba_mul(a1, b1);
        karatsubra_poly_mult_modq(p2,a1,b1,k+r-1,q);

        simplePolyAddModQ(a10, a1, a0, k+r-1, q);
        simplePolyAddModQ(b10, b1, b0, k+r-1, q);

        karatsubra_poly_mult_modq(p1,a10,b10,k+r-1,q);
        karatsubra_poly_mult_modq(p0,a0,b0,k-1,q);
        //p1 = karatsuba_mul(a1 + a0, b1 + b0)
        //p0 = karatsuba_mul(a0, b0)


        for( i = 2*k;i<4*k+2*r;i++)
        {
            c2[i] = p2[i-2*k];
        }

        simplePolySubModQ(c0, p1, p2, 4*k+2*r-1, q);
        simplePolySubModQ(c0, c0, p0, 4*k+2*r-1, q);

        for( i = k;i<3*k+2*r;i++)
        {
            c1[i] = c0[i-k];
        }


        simplePolyAddModQ(res, c2, c1, 4*k+2*r-1, q);
        simplePolyAddModQ(res, res, p0, 4*k+2*r-1, q);

        printf("deg %d k %d r %d------------------------------- \r\n",deg,k,r);
        printf("\r\nA ");
        for(i=0;i<=deg;i++)
        {
            printf("%d ",polyA[i]);
        }

        printf("\r\nB ");
        for(i=0;i<=deg;i++)
        {
            printf("%d ",polyB[i]);
        }
        i=0;
        printf("c2 %d %d %d %d %d %d %d %d %d %d \r\n",c2[i+0],c2[i+1],c2[i+2] ,c2[i+3],c2[i+4],c2[i+5],c2[i+6] ,c2[i+7],c2[i+8],c2[i+9]);
        printf("c1 %d %d %d %d %d %d %d %d %d %d \r\n",c1[i+0],c1[i+1],c1[i+2] ,c1[i+3],c1[i+4],c1[i+5],c1[i+6] ,c1[i+7],c1[i+8],c1[i+9]);
        printf("c0 %d %d %d %d %d %d %d %d %d %d \r\n",c0[i+0],c0[i+1],c0[i+2] ,c0[i+3],c0[i+4],c0[i+5],c0[i+6] ,c0[i+7],c0[i+8],c0[i+9]);

        printf("p2 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",p2[i+0],p2[i+1],p2[i+2] ,p2[i+3],p2[i+4],p2[i+5],p2[i+6] ,p2[i+7],p2[i+8],p2[i+9],p2[i+10],p2[i+11],p2[i+12] ,p2[i+13],p2[i+14],p2[i+15],p2[i+16] ,p2[i+17],p2[i+18],p2[i+19]);
        printf("p1 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",p1[i+0],p1[i+1],p1[i+2] ,p1[i+3],p1[i+4],p1[i+5],p1[i+6] ,p1[i+7],p1[i+8],p1[i+9],p1[i+10],p1[i+11],p1[i+12] ,p1[i+13],p1[i+14],p1[i+15],p1[i+16] ,p1[i+17],p1[i+18],p1[i+19]);
        printf("p0 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",p0[i+0],p0[i+1],p0[i+2] ,p0[i+3],p0[i+4],p0[i+5],p0[i+6] ,p0[i+7],p0[i+8],p0[i+9],p0[i+10],p0[i+11],p0[i+12] ,p0[i+13],p0[i+14],p0[i+15],p0[i+16] ,p0[i+17],p0[i+18],p0[i+19]);

        printf("a1 %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d \r\n",a1[i+0],a1[i+1],a1[i+2] ,a1[i+3],a1[i+4],a1[i+5],a1[i+6] ,a1[i+7],a1[i+8],a1[i+9],a1[i+10],a1[i+11],a1[i+12] ,a1[i+13],a1[i+14],a1[i+15],a1[i+16] ,a1[i+17],a1[i+18],a1[i+19]);
        printf("a0 %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d \r\n",a0[i+0],a0[i+1],a0[i+2] ,a0[i+3],a0[i+4],a0[i+5],a0[i+6] ,a0[i+7],a0[i+8],a0[i+9],a0[i+10],a0[i+11],a0[i+12] ,a0[i+13],a0[i+14],a0[i+15],a0[i+16] ,a0[i+17],a0[i+18],a0[i+19]);
        printf("b1 %d %d %d %d %d %d %d %d %d %d \r\n",b1[i+0],b1[i+1],b1[i+2] ,b1[i+3],b1[i+4],b1[i+5],b1[i+6] ,b1[i+7],b1[i+8],b1[i+9]);
        printf("b0 %d %d %d %d %d %d %d %d %d %d \r\n",b0[i+0],b0[i+1],b0[i+2] ,b0[i+3],b0[i+4],b0[i+5],b0[i+6] ,b0[i+7],b0[i+8],b0[i+9]);
        printf("a10 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",a10[i+0],a10[i+1],a10[i+2] ,a10[i+3],a10[i+4],a10[i+5],a10[i+6] ,a10[i+7],a10[i+8],a10[i+9],a10[i+10],a10[i+11],a10[i+12] ,a10[i+13],a10[i+14],a10[i+15],a10[i+16] ,a10[i+17],a10[i+18],a10[i+19]);
        printf("b10 %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",b10[i+0],b10[i+1],b10[i+2] ,b10[i+3],b10[i+4],b10[i+5],b10[i+6] ,b10[i+7],b10[i+8],b10[i+9],b10[i+10],b10[i+11],b10[i+12] ,b10[i+13],b10[i+14],b10[i+15],b10[i+16] ,b10[i+17],b10[i+18],b10[i+19]);


        free(a1);
        free(a0);
        free(b1);
        free(b0);
        free(p2);
        free(p1);
        free(p0);
        free(a10);
        free(b10);
        free(c2);
        free(c1);
        free(c0);
    }
}

void flat_karatsubra(int16_t *res, int16_t *polyA, int16_t *polyB, uint16_t deg,
                     uint16_t q)
{

    karatsubra_poly_mult_modq(res,polyA,polyB,deg,q);

    simplePolyAddModQ(res,res,res+(deg+1),deg+1,q);

}

void test_karatsubra()
{
    int16_t res[802] = {0}, i, deg = 400;
    clock_t start_t, end_t, total_t;

    printf("\r\nKaratsuba: \r\n");

    // int16_t polyA[] = {1,2,3,4,5,6,7,8,9,10};
    // int16_t polyB[] = {1,2,3,4,5,6,7,8,9,10};
    int16_t polyA[] = {1000,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1000};
    int16_t polyB[] = {1000,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1000};

    // simplePolyAddModQ(polyA,polyA,polyA+(deg+1),deg,2048);
    start_t = clock();
    flat_karatsubra(res,e112,f112,deg,2048);
   // simplePolyMulModQ(res,e112,f112,deg,2048);
    end_t = clock();
    total_t = end_t-start_t;
    printf("time: %ld\n",total_t);
    // simplePolyMulModQ(res,polyA,polyB,deg,2048);
    // karatsubra_poly_mult_modq(res,polyA,polyB,deg,2048);
    //  Calculate mod p to isolate the message/key.
    for (i = 0; i<NTRU_N_112; i++) res[i] = neg_mod_3_cpu[res[i]%3];

/*
    for(i=0;i<=deg;i+=8)
    {
        printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n",res[i+0],res[i+1],res[i+2] ,res[i+3],res[i+4],res[i+5],res[i+6] ,res[i+7]);
    }
*/

}


int main(void) {
    printf("!!!Hello World!!!"); /* prints !!!Hello World!!! */
    test_karatsubra();
    return EXIT_SUCCESS;
}
