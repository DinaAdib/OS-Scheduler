
/* This file is done for you,
Probably you will not need to change anything
This file represents an emulated clock for the simulation purpose only
it is not a real part of operating system */

#include "headers.h"
int shmid;
/*clear the resources before exit*/
void cleanup(int x)
{
  shmctl( shmid,IPC_RMID,NULL);
  printf("Clock terminating \n");
  raise(9);
}

/* this file represents the system clock for ease of calculations*/
int main() {
  printf("Clock Starting\n");
  signal(SIGINT,cleanup);
  int clk=0;

 //Create shared memory for one integer variable 4 bytes
 shmid = shmget(SHKEY, 4, IPC_CREAT|0644);
 if((long)shmid == -1)
  	{
  	  perror("Error in create shm");
  	  exit(-1);
  	}

 int * shmaddr = (int*) shmat(shmid, (void *)0, 0);
  if((long)shmaddr == -1)
  {	
  	perror("Error in attach in parent");
  	exit(-1);
  }
  else
  {	
   	*shmaddr = clk;		/* initialize shared memory */
  }
   while(1)
   {
       sleep(1);
       (*shmaddr)++;
   }

}