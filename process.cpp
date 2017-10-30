#include "headers.h"

//Signals' Handlers
void ResumeHandler(int signum);

/* Global Variables*/
int remainingtime = 0;
int schID = 0;
int quantum = 0;

int main(int agrc, char* argv[]) {
    //if you need to use the emulated clock uncomment the following line
   // initClk();

    //It gets remaining time from the argument list
   // remainingtime=atoi(argv[1]);
    //int schID= argv[2]; //or getppid();?

    //Signals
	remainingtime=atoi(argv[0]);
	cout<<"I am alive!!! my remaining time is "<<remainingtime<<endl;
   // signal (SIGCONT, ResumeHandler);

    while(remainingtime>0) {
       sleep(1);
       remainingtime--;
    printf("\n I am process child no %d , my remaining time %d \n",getpid() , remainingtime); 
    }
   printf("\n I am process child no %d , I will exit now..Bye \n",getpid()); 
   exit(1); //signal to indicate that process finished
    //if you need to use the emulated clock uncomment the following line
    //destroyClk(false);
    return 0;
}


void ResumeHandler(int signum) {}

