#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define UINT32_BITS     32
#define LOOPS           100

typedef struct Pair32
{
    uint32_t A;
    uint32_t B;

} Pair32;

static void Pair32_init(Pair32 *Pair) __attribute__ ((noinline));
static bool CoinFlip(void);
static bool BitTest32(uint32_t Mask, uint8_t Index);
static void BitSet32(uint32_t *pMask, uint8_t Index);
static void BitReset32(uint32_t *pMask, uint8_t Index);

int main(void)
{
    int i;
    uint8_t Index;
    Pair32 Pair;
    
    srand(time(NULL));
    
    (void) Pair32_init(&Pair);
    
    for (i = 0; i < LOOPS; ++i)
    {
        Index = rand() % UINT32_BITS;
        if (CoinFlip())
        {
            if (!BitTest32(Pair.B, Index))
            {
                BitSet32(&Pair.A, Index);
            }
            else if (!BitTest32(Pair.A, Index))
            {
                BitReset32(&Pair.B, Index);
            }
        }
        else
        {
            if (BitTest32(Pair.B, Index))
            {
                BitReset32(&Pair.A, Index);
            }
            else if (BitTest32(Pair.A, Index))
            {
                BitSet32(&Pair.B, Index);
            }
            else
            {
                BitSet32(&Pair.A, Index);
                BitSet32(&Pair.B, Index);
            }
        }
    }
    
    printf("A: ");
    for (i = 0; i < UINT32_BITS; ++i)
    {
        printf("%c", BitTest32(Pair.A, i) ? '1' : '0');
    }
    printf("\n");
    printf("B: ");
    for (i = 0; i < UINT32_BITS; ++i)
    {
        printf("%c", BitTest32(Pair.B, i) ? '1' : '0');
    }
    printf("\n");
    
    if (Pair.A == Pair.B || Pair.A == 0 || Pair.B == 0)
    {
        printf("You win!!!\n");
    }
    
    return EXIT_SUCCESS;
}

static void Pair32_init(Pair32 *Pair)
{
    Pair->A = 0;
    Pair->B = 0;
}

static bool CoinFlip(void)
{
    return rand() % 2 == 0;
}

static bool BitTest32(uint32_t Mask, uint8_t Index)
{
    return (Mask & 1 << Index) != 0;
}

static void BitSet32(uint32_t *pMask, uint8_t Index)
{
    *pMask |= (1 << Index);
}

static void BitReset32(uint32_t *pMask, uint8_t Index)
{
    *pMask &= ~(1 << Index);
}

