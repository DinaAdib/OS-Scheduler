#include "headers.h"



void ClearResources(int);
void Load();  //Load from input file

struct processData {
    int id;
    int arrivaltime;
    int runningtime;
    int priority;
    int remainingtime;
    int starttime;
    int finishtime;
    int waittime; //derived from other attributes?
    string status;
};
 queue <processData> processes;


int main() {
    
     
    // signal(SIGINT,ClearResources);
    //TODO: 
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
    // 2-Initiate and create Scheduler and Clock processes.
    

   
    Load();
    processData top=processes.front();
    printf("\n top priority= %d",top.priority);
    int choice; 
    printf("\n Choose the scheduling algorithm: \n 1-Non Pre-emptive HPF \n 2-Pre-emptive SRTN \n 3-Round Robin"); 
    scanf("%d",&choice); 
    
    int schdID=fork();
    if (schdID==0) //child executes itself 
    {  char* const par[]={(char*)0};
       execv("./sch.out",par);
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

// This functions loads the input file into a queue
void Load()
{
        char Data[500];
	ifstream inp("processes.txt");
        //assert(!inp.fail());
        //Load processes info
        
        processData p;

        while (!inp.eof())
	{
            inp.getline(Data,500); 
            printf("\nData= %s", Data);
            if(Data[0]!='#') {
                stringstream ss(Data); //using stringstream to split string to int 
                while(ss.good()){
                ss>> p.id;
                ss>>p.arrivaltime;
                ss>>p.runningtime;
                ss>>p.priority;
                }
                processes.push(p); // insert to queue
                nProcesses++;
            }
	}
	inp.close();
   //     assert(!inp.fail());

}
