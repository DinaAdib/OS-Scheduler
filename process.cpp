#include "headers.h"

/*----------Global variables-----------*/
int remainingtime = 0;
bool pauseNow=true;

/*---------Functions' Headers-----*/
void resumeHandler(int signum);
void pauseHandler(int signum);


int main(int agrc, char* argv[]) 
{
    remainingtime=atoi(argv[0]);
    cout<<" Process: I am alive. My remaining time is "<<remainingtime<<endl;

    //Signals
    signal (SIGCONT, resumeHandler); 
    signal( SIGUSR2 ,pauseHandler);

    while(remainingtime>0) 
    {
       pauseNow = true;
       cout<<" Process: I am running now, and my remaining time="<<remainingtime<<endl;
       sleep(1);
       remainingtime--;
    }

   cout<<" Process: I finished and I will exit now."<<endl;
   exit(1); //signal to indicate that process finished

    return 0;
}


void resumeHandler(int signum) 
{
  cout<<" Process: I was paused, and now I will resume. My remaining time="<<remainingtime<<endl;
  pauseNow = false;
}


void pauseHandler(int signum)
{

    cout<<" Process: The scheduler sent a signal, and I will pause now."<<endl;
    if(pauseNow){ pause(); }
}
