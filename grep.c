#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
    //checking whether the user has supplied 3 values 
    if(argc != 3) {
        perror("Usage: ./a.out [filepath] [word to be found]"); return 1;
    }
    
    //opening the file
    int fd = open(argv[1], O_RDONLY);
    if(fd == -1) {
        perror("Unable to open the file"); return 1;
    }

    //parsing the file
    int size, len = strlen(argv[2]); char *fword;
    long long int count = 0, line = 0, i = 0;
    fword = (char *) malloc(sizeof(char) * (len + 1));
    while((size = read(fd, &fword[i], 1)) != 0) {
        count++; //keeping the character count of currrent line
        if(count == 1) line++; // incrementing the line number
        
        //when a newline is encountered set the character count to 0 again
        if(fword[i] == '\n') {
            count = 0; continue;
        }
        //if the character matches with that of given by the user increment pos
        if(i > 0 && fword[i] != argv[2][i]) {
            i = 0;
        }
        else if(fword[i] == argv[2][i]) {
            i++;
            if(i == len) { // if pos == len of the word supplied by the user
                i = 0;
                size = lseek(fd, -count, SEEK_CUR); // going to the start of the line
                if(size == -1) {
                    perror(""); return 1;
                }
                //printing the whole line
                printf("Line %lld:", line);
                read(fd, fword, 1);
                while(fword[0] != '\n') {
                    putchar(fword[0]); read(fd, fword, 1);
                }
                printf("\n");
                count = 0; //setting the word count to zero
            }
        }
    }

    //closing the file
    if(close(fd) == -1) {
        perror(""); return 1;
    }

    return 0;
}
