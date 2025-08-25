#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

//0 = stdin
//1 = stdout
//2 = stderr

/* this is a replica of the 'head' user command present in linux.
 * spar10head does not offer the whole functionality as that of head in linux but does the following:
 * if no argument given read 10 line and print 10 lines from stdin to stdout
 * if argument given is - do the same as above
 * if argument given is a file name, open the file and read first 10 lines and print them to stdout
 */

void head(int fd) {
    int count = 10; 
    char c;
    
    while(count > 0) {
        if(read(fd, &c, 1) != 1) {
            printf(1, "\n");
            break;
        }
        if(c == '\n') count--;
        if(write(1, &c, 1) != 1) {
            printf(2, "spar10head: write error");
            break;
        }
    }
    exit();
}

int main(int argc, char **argv) {
    int fd;

    //no input file given
    if(argc <= 1 || (argc == 2 && argv[1][0] == '-')) {
        head(0);
    }

    //the below code will be executed if input files are present
    
    for(int i = 1; i < argc; i++) {
        printf(1, "%d file: %s\n", i, argv[i]); // print the file name first
        if((fd = open(argv[i], O_RDONLY)) < 0) {
            printf(1, "spar10head: cant open file: %s\n", argv[i]);
            continue;
        }
        head(fd);
        close(fd);
    }
    exit();
}
