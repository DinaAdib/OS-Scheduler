#include "headers.h"
//FOR MICHAEL:
//Important Note: DON'T FORGET to clean the message queue using command (ipcrm -Q 12614)

// Global Variables
queue <struct processData> processes;

/*----------Functions' headers-----------*/
void ClearResources(int);
void Load();  //Load from input file
void Send(key_t pGeneratorSendQid, struct processData processToSend);
int clkID;

int main() {

    /*-----------Local Variables--------*/
    int i = 0; //index incremented when current time = arrival time of a process to index next upcoming process
    int choice, quantum;
    char choicePar[2];
    char quantumPar[2];

    key_t pGeneratorSendQid;

    pGeneratorSendQid= msgget(12614, IPC_CREAT | 0644);

    if(pGeneratorSendQid == -1)
    {
        perror("Error in creating process generator send Queue");
        exit(-1);
    }


     signal(SIGINT,ClearResources);
    //TODO: 
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
     printf("\n Choose the scheduling algorithm: \n 1-Non Pre-emptive HPF \n 2-Pre-emptive SRTN \n 3-Round Robin: \n");
     scanf("%d",&choice);

     if(choice == 3) {
         printf("\n Please enter quantum value: ");
          scanf("%d",&quantum);
     }
    // 2-Initiate and create Scheduler and Clock processes.


   
    Load();
    struct processData top=processes.front();
    printf("\n top priority= %d",top.arrivalTime);



    //Create clock process
    clkID=fork();
    if (clkID==0) //child executes itself
    {
        cout<<"starting";
        char *clkPar[] = {"./clk.out", 0 };
        execve(clkPar[0], &clkPar[0], NULL);
    }

    initClk();
    //convert user inputs
    sprintf(choicePar, "%d", choice);
    sprintf(quantumPar, "%d", quantum);

    //Create scheduler
    int schdID=fork();
    if (schdID==0) //child executes itself
    {
        char *schPar[] = { "./sch.out",choicePar, quantumPar, 0 };
        execve(schPar[0], &schPar[0], NULL);
    }

    // 3-use this function after creating clock process to initialize clock



    /////Toget time use the following function
   int x= getClk();
    printf("current time is %d\n",x);
    //TODO:  Generation Main Loop
     //4-Creating a data structure for process  and  provide it with its parameters

    //5-Send the information to  the scheduler at the appropriate time 
    //(only when a process arrives) so that it will be put it in its turn.
    cout << "\nnumber of processes: " <<nProcesses;
    while(i < nProcesses-1){
        x=getClk();
  //      cout<< x << "\n";
        if(top.arrivalTime == x) {
            Send(pGeneratorSendQid, top);//Send message to scheduler containing this process data
            kill(schdID, SIGUSR1);
            processes.pop();
            top=processes.front();
            i++;
        }
    }
    while(1){}

}

void ClearResources(int)
{
    destroyClk(true);
    exit(1);
    //TODO: it clears all resources in case of interruption
}


/*------------Functions------------*/
// This functions loads the input file into a queue
void Load()
{
        char Data[500];
        ifstream inp("processes.txt");

        //Load processes info
         struct processData p;

        while (!inp.eof())
	{
            inp.getline(Data,500); 
            printf("\nData= %s", Data);
            if(Data[0]!='#') {
                stringstream ss(Data); //using stringstream to split string to int
                while(ss.good()){
                ss>> p.id;
                ss>>p.arrivalTime;
                ss>>p.runningTime;
                ss>>p.priority;
                }
                //initially remaining time of process = its running time
                p.remainingTime = p.runningTime;
                processes.push(p); // insert to queue
                nProcesses++;
            }
	}
        inp.close();
}
printf("Received Successfully ID: %d", message.mProcess.id);
//This function is used to send process data to scheduler when current time = arrival time of process
void Send(key_t pGeneratorSendQid, struct processData processToSend)
{
  int send_val;
  struct processMsgBuff message;
  message.mProcess = processToSend;
  message.mtype = 7;
 cout << "\nProcess to be sent: ID = " << message.mProcess.id;
  send_val = msgsnd(pGeneratorSendQid, &message, sizeof(message.mProcess), !IPC_NOWAIT);

  if(send_val == -1)
        perror("Error in send");
  else printf("Process sent successfully");

}

