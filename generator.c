#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <seed> <count>\n", argv[0]);
        return 1;
    }

    // Extract seed and count from command line arguments
    int seed = atoi(argv[1]);
    int count = atoi(argv[2]);

    // Set seed for random number generator
    srand(seed);
    
    int lowerLimit = 1000000;
    int upperLimit = 2100000000;

    // Generate and output random numbers
    for (int i = 0; i < count; ++i) {
        //long long random_number = 1000000LL + (rand() % (1999999999LL - 1000000LL + 1));
        int random_number = rand() % (upperLimit - lowerLimit + 1) + lowerLimit;
        printf("%d\n", random_number);
    }

    return 0;
}
