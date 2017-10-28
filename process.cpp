#include "headers.h"

//Signals' Handlers
void ResumeHandler(int signum);

/* Global Variables*/
int remainingtime =19;
int schID = 0;
int quantum = 0;

int main(int agrc, char* argv[]) {
	cout<<"I am alive!!! my remaining time is "<<remainingtime<<endl;
    //if you need to use the emulated clock uncomment the following line
   // initClk();

    //It gets remaining time from the argument list
   // remainingtime=atoi(argv[1]);
    //int schID= argv[2]; //or getppid();?

    //Signals
    signal (SIGCONT, ResumeHandler);

    while(remainingtime>0) {
       sleep(1);
       remainingtime--;
    }
   exit(1); //signal to indicate that process finished
    //if you need to use the emulated clock uncomment the following line
    //destroyClk(false);
    return 0;
}


void ResumeHandler(int signum) {}

