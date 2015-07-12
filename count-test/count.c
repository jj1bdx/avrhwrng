#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int count[256];
    int i;
    int c;

    for (i = 0; i < 256; i++) {
        count[i] = 0;
    }

    while (EOF != (c = getchar())) {
        count[c] += 1;
    }

    for (i = 0; i < 256; i++) {
        if (count[i] > 0) {
            printf("count[ %d ] = %d\n", i, count[i]);
        }
    }

    exit(0);
}
