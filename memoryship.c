/*
 * @author: Hunter Chambers
 *
 * This is Memoryship based on battleship
 * Each player will have a 5x5 grid area
 * Each ship is a single cell
 * Each side has 5 ships
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>



 #define SIZE sizeof(long long int)
 #define NUMBER_OF_PLAYERS 2
 #define GRID_SIZE 25
  #define NLOOPS (GRID_SIZE * 2)

 int main() {
   int fd, i, counter;
   pid_t pid;
   long long int *area;
   char *cells;

   cells = malloc(sizeof(char) * NUMBER_OF_PLAYERS * GRID_SIZE);

   fd = open("/dev/zero", O_RDWR);
   area = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   close(fd);

   /* Initialize the grid to all "." */
   for(i = 0; i < (NUMBER_OF_PLAYERS * GRID_SIZE); i++)
   {
     cells = ".";
     printf("%s\n", cells);
     cells++;
   }

   *area = 0;

   pid = fork();

   if (pid > 0) {
     /* Parent process */
     for (i = 0; i < NLOOPS; i += 2) {
       while (*area != i); /* empty loop */
       /* won't print unless *area is even */
       printf("<P> sees value <%lld>\n", *area);
       fflush(stdout);
       *area += 1;
     }
   }
   else {
     /* Child process */
     for (i = 1; i < NLOOPS + 1; i += 2) {
       while (*area != i); // empty loop
       /* won't print unless *area is odd */
       printf("<C> sees value <%lld>\n", *area);
       fflush(stdout);
       *area += 1;
     }
   }
   return 0;
 }
