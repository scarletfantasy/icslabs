/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);


/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,k,l,x0,x1,x2,x3,x4,x5,x6,x7;
    
    /*dealing with 32*32*/
    if((M==32)&&(N==32))
    {
        for(i=0;i<32;i+=8)
        {
            for(j=0;j<32;j+=8)
            {
                for(k=0;k<8;++k)
                {
                    /*use local var to avoid some miss*/
                    x0=A[i+k][j];
                    x1=A[i+k][j+1];
                    x2=A[i+k][j+2];
                    x3=A[i+k][j+3];
                    x4=A[i+k][j+4];
                    x5=A[i+k][j+5];
                    x6=A[i+k][j+6];
                    x7=A[i+k][j+7];
                    B[j][i+k]=x0;
                    B[j+1][i+k]=x1;
                    B[j+2][i+k]=x2;
                    B[j+3][i+k]=x3;
                    B[j+4][i+k]=x4;
                    B[j+5][i+k]=x5;
                    B[j+6][i+k]=x6;
                    B[j+7][i+k]=x7;

                    
                }
                    
                
                
                
            }
        }
    }
    /*dealing with 64*64*/
    else if((M==64)&&(N==64))
    {
        for(i=0;i<64;i+=8)
        {
            for(j=0;j<64;j+=8)
            {
                    for(k=0;k<4;++k)
                    {
                        /*tranpose the left up part and move the tranposed right up part to thr right up*/
                        x0=A[i+k][j];
                        x1=A[i+k][j+1];
                        x2=A[i+k][j+2];
                        x3=A[i+k][j+3];
                        x4=A[i+k][j+4];
                        x5=A[i+k][j+5];
                        x6=A[i+k][j+6];
                        x7=A[i+k][j+7];
                        B[j][i+k]=x0;
                        B[j+1][i+k]=x1;
                        B[j+2][i+k]=x2;
                        B[j+3][i+k]=x3;
                        B[j][i+k+4]=x4;
                        B[j+1][i+k+4]=x5;
                        B[j+2][i+k+4]=x6;
                        B[j+3][i+k+4]=x7;

                        
                    }
                    for(k=0;k<4;++k)
                    {
                        /*move the right up part to left downpart and tranpose the left down part*/
                        x0=A[i+4][j+k];
                        x1=A[i+5][j+k];
                        x2=A[i+6][j+k];
                        x3=A[i+7][j+k];
                        x4=B[j+k][i+4];
                        x5=B[j+k][i+5];
                        x6=B[j+k][i+6];
                        x7=B[j+k][i+7];
                        B[j+k][i+4]=x0;
                        B[j+k][i+5]=x1;
                        B[j+k][i+6]=x2;
                        B[j+k][i+7]=x3;
                        B[j+k+4][i]=x4;
                        B[j+k+4][i+1]=x5;
                        B[j+k+4][i+2]=x6;
                        B[j+k+4][i+3]=x7;
                    }
                    
                    for(k=4;k<8;++k)
                    {
                        /*tranpose the right down part*/
                        x4=A[i+k][j+4];
                        x5=A[i+k][j+5];
                        x6=A[i+k][j+6];
                        x7=A[i+k][j+7];
                        B[j+4][i+k]=x4;
                        B[j+5][i+k]=x5;
                        B[j+6][i+k]=x6;
                        B[j+7][i+k]=x7;
                    }
            }
        }
    }
    else if((M==61)&&(N==67))
    {
        /*irregular part use block to deal with it*/
        for(i=0;i<N;i+=17)
        {
            for(j=0;j<M;j+=17)
            {
                for(k=0;k<17&&(k+i<N);++k)
                {
                    for(l=0;l<17&&(j+l<M);++l)
                    {
                        B[j+l][i+k]=A[i+k][j+l];
                    }
                }
            }
        }
        


    }
    else
    {
        for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            x0 = A[i][j];
            B[j][i] = x0;
        }
    }    
    }
    
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

