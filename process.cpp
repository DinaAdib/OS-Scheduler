#include "headers.h"

//Signals' Handlers
void resumeHandler(int signum);
void pauseHandler(int signum);

/* Global Variables*/
int remainingtime = 0;
bool pauseNow=true;

int main(int agrc, char* argv[]) {
    //if you need to use the emulated clock uncomment the following line

    //Signals
	remainingtime=atoi(argv[0]);
	cout<<"I am alive!!! my remaining time is "<<remainingtime<<endl;
     signal (SIGCONT, resumeHandler);
     signal( SIGUSR2 ,pauseHandler);

    while(remainingtime>0) {
       sleep(1);
       remainingtime--;
       pauseNow = true;
    printf("\n I am process child no %d , my remaining time %d \n",getpid() , remainingtime);
    }
   printf("\n I am process child no %d , I will exit now..Bye \n",getpid());
   exit(1); //signal to indicate that process finished

    return 0;
}


void resumeHandler(int signum) {
printf("\n I am process child no %d , I AM BACK :) \n",getpid());
pauseNow = false;
}


void pauseHandler(int signum){
printf("\n I am process child no %d , I will pause for now :) \n",getpid());
if(pauseNow)
pause();
}
