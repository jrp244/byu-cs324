/* Part 1:
1. Page 1 for executable programs, 2 for system calls, 3 for library calls
2. Pages 1, 2
3. Pages 2, 3 
4. 2
5. #include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
6. 2, 7
7. SO_ACCEPTCONN
8. 3 pages
9. null-terminated strings
10. returns an integer that is greater than zero
*/
// Part 2: 
// I completed the TMUX exerciseS for part 2
// 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
    int pid = getpid();
    fprintf(stderr, "\n%d\n\n" , pid);
    FILE *fp; // declaration of file pointer
    fp = fopen(argv[1], "r"); // opening of file
    if (!fp){
      printf("Error\n");
      return 1;
    }

    char file_c[1000];
    while (fgets (file_c, 1000, fp)!=NULL ) {
      /* writing content to stdout */
      if (getenv("CATMATCH_PATTERN") == NULL) {
        printf("%c ", '0');
      } else {
        if ((strstr(file_c ,getenv("CATMATCH_PATTERN")) == NULL )) {
          printf("%c ", '0');
        } else {
          printf("%c ", '1');
        }
    }
      puts(file_c);
    }
    fclose(fp);
    return  0;
}