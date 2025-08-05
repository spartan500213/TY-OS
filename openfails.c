#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int main() {
    int fd1, fd2, fd3;
    fd1 = open("hi", O_RDWR); perror("First error");
    //fd2 = open("", O_CREAT); 
    errno = ENAMETOOLONG; perror("Second error");
    errno = EISDIR; perror("Third error");
    errno = 0; perror("");
    return 0;
}
