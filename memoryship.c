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
 #define TRUE 1
 #define FALSE 0
 #define DEBUG FALSE
 #define SHOW_STEP_BY_STEP FALSE

 void printShipLocations(int *ships);
 void checkShipsForDuplicates(int *ships);
 int pickShip(int guesser);
 void printOcean(char *ocean);
 void initializeOcean(char *ocean, int ships[NUMBER_OF_SHIPS]);
 void shipGuess(char *ocean, int *counter, int guesser); /* guesser == 0 if parent and guesser == 1 for child */
 void playGame(pid_t pid, char *ocean, long long int *area);

 pid_t otherPlayer;


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
        if(DEBUG)
        {
          printf("Found Duplicate\n");
          printShipLocations(ships);
          printf("\n\n");
        }
        if(i < 5)
        {
          ships[i] = pickShip(PARENT);
        }
        else
        {
          ships[i] = pickShip(CHILD);
        }

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

int pickShip(int guesser){
  int position = rand() % GRID_SIZE;
  if (guesser == CHILD) position += GRID_SIZE;

  return position;
}

void printShipLocations(int *ships)
{
  int i;
  char buf[50];
  for(i = 0; i < 5; i++)
  {
    sprintf(buf, "Parent: %d\nChild: %d\nEND OF SHIP LOCATIONS\n", ships[i], ships[i + (NUMBER_OF_SHIPS / NUMBER_OF_PLAYERS)]);
    write(0, buf, strlen(buf));
  }
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
  int random;
  char buf[50];
  random = pickShip(guesser);

  if(ocean[random] == '.')
  { /* If they guess an empty cell */
    ocean[random] = '*';
    sprintf(buf, "MISS!\n");
    write(0, buf, strlen(buf));
  }else if(ocean[random] == 'O')
  { /* If they guess a cell with a ship in it */
    ocean[random] = 'X';
    *counter = *counter - 1;
    sprintf(buf, "HIT!\n");
    write(0, buf, strlen(buf));

    if(*counter <= 0)
    {
      if(guesser == PARENT)
      {
        sprintf(buf, "Parent Wins!!\n");
      }else
      {
        sprintf(buf, "Child Wins!!\n");
      }
      write(0, buf, strlen(buf));
      printOcean(ocean);
      kill(otherPlayer, SIGTERM);
      exit(0);
    }

    if(DEBUG)
    {
      if(guesser)
      {
        sprintf(buf, "Childcounter: %d\n", *counter);
        write(0, buf, strlen(buf));
      }else
      {
        sprintf(buf, "Parentcounter: %d\n", *counter);
        write(0, buf, strlen(buf));
      }
    }

  }else if(ocean[random] == 'X' || ocean[random] == '*')
  { /* If they guess a cell where a ship has already been destroyed, guess again */
      shipGuess(ocean, counter, guesser);
  }
}


 int main() {
   int fd, newfd, i;
   pid_t pid;
   char *ocean;
   long long int *area;
   int ships[NUMBER_OF_SHIPS];
   srand(time(NULL));

   ocean = calloc(NLOOPS, sizeof(char));

   fd = open("/dev/zero", O_RDWR);
   area = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   close(fd);

   newfd = open("/dev/zero", O_RDWR);
   ocean = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, newfd, 0);
   close(newfd);

   /* Ship Locations */
   for(i = 0; i < 5; i++)
   {
     ships[i] = pickShip(PARENT);
     ships[i + (NUMBER_OF_SHIPS / NUMBER_OF_PLAYERS)] = pickShip(CHILD);
   }

   checkShipsForDuplicates(ships);

   if(DEBUG)
   {
     printShipLocations(ships);
   }

   initializeOcean(ocean, ships);
   printOcean(ocean);

   *area = 0;

   pid = fork();
   playGame(pid, ocean, area);

   return 0;
 }

 void playGame(pid_t pid, char *ocean, long long int *area) {
   int childCounter = 5;
   int parentCounter = 5;
   char buf[50];

   int guesser;
   int startingIndex;
   if (pid > 0) {
     // PARENT
     otherPlayer = pid;
     guesser = PARENT;
     startingIndex = 0;
   } else {
     // CHILD
     otherPlayer = getppid();
     guesser = CHILD;
     startingIndex = 1;
   }

   int i;
   for (i = startingIndex; i < NLOOPS + startingIndex; i += 2) {
     while (*area != i); /* if it isn't my turn, wait for my turn */

     if(SHOW_STEP_BY_STEP)
     {
       sleep(2);
     }
     int *counter = (guesser == PARENT) ? &parentCounter : &childCounter;
     shipGuess(ocean, counter, guesser);
     printOcean(ocean);
     //fflush(stdout);
     *area += 1;
   }
 }
