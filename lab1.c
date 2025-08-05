#include<unistd.h>
#include<stdio.h>

int main(int argc, char **argv) {
    char s[100];
    for(int i = 0; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    
    getcwd(s, 100);
    printf("Before change: %s\n", s);
    chdir("./..");
    getcwd(s, 100);
    printf("After change: %s\n", s);
    return 0;
}
