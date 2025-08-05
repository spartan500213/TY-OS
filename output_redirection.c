#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int fd;
    if(close(1) == -1) {
        perror(""); return 1;
    }
    fd = open(argv[1], O_RDWR | O_CREAT);
    printf("fd = %d\n", fd);
    printf("Hello\n");
    return 0;
}
