#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    while(1) {
        sleep(1);
        printf("%s\n", argv[1]);
    }
    return 0;
}
