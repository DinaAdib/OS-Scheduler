#include "headers.h"


void ClearResources(int);

int main() {
    
     signal(SIGINT,ClearResources);
    //TODO: 
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
    // 2-Initiate and create Scheduler and Clock processes.
    short choice; 
    printf("\n Choose the scheduling algorithm: \n 1-Non Pre-emptive HPF \n 2-Pre-emptive SRTN \n 3-Round Robin"); 
    scanf("%d",&choice); 
    
    int schdID=fork()
    if (schID==0) //child executes itself 
    {  char* const par[]={(char*)0};
       execv("./scheduler.out",par) 
    }
    // 3-use this function after creating clock process to initialize clock
    initClk();
    /////Toget time use the following function
    int x= getClk();
    printf("current time is %d\n",x);
    //TODO:  Generation Main Loop
    //4-Creating a data structure for process  and  provide it with its parameters 
    //5-Send the information to  the scheduler at the appropriate time 
    //(only when a process arrives) so that it will be put it in its turn.

    //6-clear clock resources
    destroyClk(true);
    
}

void ClearResources(int)
{
    //TODO: it clears all resources in case of interruption
}




