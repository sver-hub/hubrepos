
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

int main(int argc, char** argv)
{
    unsigned long long int number, result, mask = (1 << 7) - 1;

	if(argc < 2)
    {
        printf("Not enough argumants.\nUsage: ./task1 number\n");
        return 1;
    }

    number = strtoll(argv[1], NULL, 10);

    number = 
    (number & ~(mask << 12) & ~(mask << 44)) +
    ((number & (mask << 12)) << 32) +
    ((number & (mask << 44)) >> 32);

    printf("%llu\n", number);

	return 0;
}