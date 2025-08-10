#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

void prntDelay(char c) {
    printf("%c", c);
    fflush(stdout); //before using this code in your program uncomment this line.
    sleep(1);
    return;
}

int main() {
    //
    //setbuf(stdout, NULL); //before using this code in your program comment this line.
    char c;
    int fd = open("magic.data", O_RDONLY);
    if(fd == -1) {
        perror("Unable to open magic.data");
        return 1;
    }
    //
    while(read(fd, &c, sizeof(char))) {
        prntDelay(c);
    }
    //
    close(fd);
    return 0;
}
