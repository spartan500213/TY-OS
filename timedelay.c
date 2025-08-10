#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("This program will print \"HI\" infinitely at time intervals of %s seconds. Press Ctrl+C to end.\n", argv[1]);
    while(1) {
        printf("HI\n");
        sleep(atoi(argv[1]));
    }
    return 0;
}
