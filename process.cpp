#include "headers.h"

//Signals' Handlers
void ResumeHandler(int signum);

/* Global Variables*/
int remainingtime =0;
int schID = 0;
int quantum = 0;

int main(int agrc, char* argv[]) {

    //if you need to use the emulated clock uncomment the following line
    initClk();

    //It gets remaining time from the argument list
    remainingtime = argv[1];
    int schID= argv[2]; //or getppid();?

    //Signals
    signal (SIGCONT, ResumeHandler);

   printf("I am alive!!! my remaining time is %d", argv[1]);
    while(remainingtime>0) {
       sleep(1);
       remainingtime--;
    }
    kill(schID, SIGINT); //signal to indicate that process finished
    //if you need to use the emulated clock uncomment the following line
    //destroyClk(false);
    return 0;
}


