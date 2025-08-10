#include <stdio.h>
#include <unistd.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

int main() {
    for(int i = 0; i <= 100; i++) {
        if(i % 2 == 0) printf(RED "\r\033[12C%d" RESET, i);
        else printf(GREEN "\r\033[12C%d" RESET, i);
        fflush(stdout);
        sleep(1);
    }
    return 0;
}
