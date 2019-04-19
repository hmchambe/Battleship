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
#include <time.h>
#include <string.h>
#include <signal.h>



 #define SIZE sizeof(long long int)
 #define NUMBER_OF_PLAYERS 2
 #define GRID_SIZE 25
 #define NLOOPS (GRID_SIZE * NUMBER_OF_PLAYERS)
 #define NUMBER_OF_SHIPS 10
 #define PARENT 0
 #define CHILD 1

 void printShipLocations(int *ships);
 void checkShipsForDuplicates(int *ships);
 int pickChildShip();
 int pickParentShip();
 void printOcean(char *ocean);
 void initializeOcean(char *ocean, int ships[NUMBER_OF_SHIPS]);
 void shipGuess(char *ocean, int *counter, int guesser); /* guesser == 0 if parent and guesser == 1 for child */



void checkShipsForDuplicates(int *ships)
{
  int i, j, temp;
  for(i = 0; i < NUMBER_OF_SHIPS; i++)
  {
    for(j = 0; j < NUMBER_OF_SHIPS; j++)
    {
      /* Skip Current Ship */
      if(i == j){j++;}

      /* Generate new ship location, if two are equal */
      while(ships[i] == ships[j])
      {
        if(i < 25) { ships[i] = pickParentShip(); }
        else{ ships[i] = pickChildShip(); }
      }
    }
  }

  /* Sorts ships */
  for (i = 0; i < (10 - 1); ++i)
  {
       for (j = 0; j < 10 - 1 - i; ++j )
       {
            if (ships[j] > ships[j+1])
            {
                 temp = ships[j+1];
                 ships[j+1] = ships[j];
                 ships[j] = temp;
            }
       }
  }
}

void printOcean(char *ocean)
{
  int i;
  for(i = 0; i < NLOOPS; i++)
  {
    if(i % 5 == 0)
    {
      printf("\n");
    }
    if(i == 25)
    {
      printf("= = = = =\n");
    }
    printf("%c ", ocean[i]);
  }
  printf("\n");
}

int pickChildShip(){return (rand() % 25 + 24);}
int pickParentShip(){return (rand() % 25);}
void printShipLocations(int *ships)
{
  int i;
  for(i = 0; i < 5; i++)
  {
    printf("Parent: %d\nChild: %d\n", ships[i], ships[i + (NUMBER_OF_SHIPS / NUMBER_OF_PLAYERS)]);
  }
    printf("END OF SHIP LOCATIONS\n");
}

void initializeOcean(char *ocean, int ships[NUMBER_OF_SHIPS])
{
  int i, j;
  j = 0;
  for(i = 0; i < NLOOPS; i++)
  {
    if(i == ships[j])
    {
      ocean[i] = 'O';
      j++;
    }
    else
    {
      ocean[i] = '.';
    }
  }
}

/* Guesser is 0 for parent, 1 for child */
void shipGuess(char *ocean, int *counter, int guesser)
{
  int random, i;
  if(guesser)
  { /* Parent */
    random = pickChildShip();
  }else
  {
    random = pickParentShip();
  }

  if(ocean[random] == '.')
  { /* If they guess an empty cell */
    ocean[random] = '*';
    printf("MISS!\n");
  }else if(ocean[random] == 'O')
  { /* If they guess a cell with a ship in it */
    ocean[random] = 'X';
    counter[0] = counter[0] - 1;

    if(DEBUG)
    {
      printf("HIT!\n");
      if(guesser)
      {
        printf("ChildCounter: %d\n", *counter);
      }else
      {
        printf("ParentCounter: %d\n", *counter);
      }

    }
  }else if(ocean[random] == 'X' || ocean[random] == '*')
  { /* If they guess a cell where a ship has already been destroyed, guess again */
      shipGuess(ocean, counter, guesser);
  }
}


 int main() {
   int fd, newfd, i, j;
   int parentCounter[1];
   int childCounter[1];
   pid_t pid;
   long long int *area;
   char *ocean;
   int ships[NUMBER_OF_SHIPS];
   srand(time(NULL));

   ocean = calloc(NLOOPS, sizeof(char));
   parentCounter[0] = 5;
   childCounter[0] = 5;

   fd = open("/dev/zero", O_RDWR);
   area = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   close(fd);

   newfd = open("/dev/zero", O_RDWR);
   ocean = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, newfd, 0);

   close(newfd);

   /* Ship Locations */
   for(i = 0; i < 5; i++)
   {
     ships[i] = pickParentShip();
     ships[i + (NUMBER_OF_SHIPS / NUMBER_OF_PLAYERS)] = pickChildShip();
   }

   //printShipLocations(ships);
   checkShipsForDuplicates(ships);
   printShipLocations(ships);
   initializeOcean(ocean, ships);

   printOcean(ocean);

   *area = 0;

   pid = fork();

   if (pid > 0) {
     /* Parent process */
     for (i = 0; i < NLOOPS; i += 2) {
       while (*area != i); /* empty loop */
       if(*childCounter <= 0 || *parentCounter <= 0)
       {
         kill(pid, SIGTERM);
         kill(getpid(), SIGTERM);
       }
       //sleep(2);
       shipGuess(ocean, parentCounter, PARENT);
       printOcean(ocean);
       fflush(stdout);
       *area += 1;
     }
   }
   else {
     /* Child process */
     for (i = 1; i < NLOOPS + 1; i += 2) {
       while (*area != i); // empty loop

       if(*childCounter <= 0 || *parentCounter <= 0)
       {
         kill(getppid(), SIGTERM);
         kill(pid, SIGTERM);
       }
       //sleep(2);
       shipGuess(ocean, childCounter, CHILD);
       printOcean(ocean);
       fflush(stdout);
       *area += 1;
     }
   }
   return 0;
 }
