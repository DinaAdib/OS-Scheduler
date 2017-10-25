#include "headers.h"

//Note: Clean the message queue using command (ipcrm -Q 12614)

/*-----------Global Variables--------*/
queue <struct processData> processes;
int clkID, schdID;	//clkID is PID of clock and schdID is PID of schduler
key_t pGeneratorSendQid; //Queue ID used by Process Generator to Send data

/*----------Functions' headers-----------*/
void clearResources(int);
void childSignals(int signum);
void Load();
void Send(key_t pGeneratorSendQid, struct processData processToSend);



int main() {

    /*-----------Local Variables--------*/
        int i = 0; //index incremented when current time = arrival time of a process. Use: to index next upcoming process.
        int choice, quantum; //Passes variables from Process Generator
        char choicePar[2], quantumPar[2];  //Algorithm passed parameter and Round Robin Quantum time passed parameter

    /*-----------Communication with other files--------*/
        pGeneratorSendQid= msgget(12614, IPC_CREAT | 0644); //Get The ID
        if(pGeneratorSendQid == -1)
        {
                perror("Error in creating process generator send Queue");
                exit(-1);
        }

    /*-----------Handeling different signals--------*/
        signal(SIGINT,clearResources);	//Killing the process generator
        signal(SIGCHLD ,childSignals);	//Receving a signal from a child (process)

    /*-----------User's Input--------*/
        // Asking the user about the chosen scheduling Algorithm and its parameters if exists.
        printf("\n Kindly choose one of the following scheduling algorithms: \n\t1-Non Pre-emptive High Priority First Algorithm. \n\t2-Pre-emptive SRTN Algorithm.\n\t3-Round Robin Algorithm.\n");
        scanf("%d",&choice);

        if(choice == 3) {
         printf("\nKindly enter the quantum time (in time slices' units): ");
          scanf("%d",&quantum);
        }


    /*-----------Generated File's Input--------*/

        Load();

    ////////////////////////////////////////////// FOR TESTING /////////////////////////////////////////////
            struct processData top=processes.front();
            //for testing
            printf("\n top arrival time in file= %d\n",top.arrivalTime);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////


        /*-----------Generating clock--------*/
        clkID=fork();
        if (clkID==0) //child executes itself
        {
                cout<<"\n ---- Starting Clock ----\n";
                char *clkPar[] = {"./clk.out", 0 };
                execve(clkPar[0], &clkPar[0], NULL);
        }
        initClk();

        /*-----------Getting current time--------*/
        int currentT= getClk();
        printf("Current time is %d\n",currentT);

        /*-----------Creating the scheduler and sending user's arguments--------*/
        schdID=fork();

        /*-----------Convert user inputs into a types suitable as arguments--------*/
            sprintf(choicePar, "%d", choice);
            sprintf(quantumPar, "%d", quantum);

        if (schdID==0) //child executes itself
        {
                char *schPar[] = { "./sch.out",choicePar, quantumPar, 0 };
                execve(schPar[0], &schPar[0], NULL);
        }


    ////////////////////////////////////////////// FOR TESTING /////////////////////////////////////////////
    cout << "\n number of processes: " <<nProcesses;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

     /*-----------Sending Processes to Scheduler when current time = arrival time--------*/
        while(i <= nProcesses){
        currentT=getClk();

        if(top.arrivalTime == currentT) {
            printf("\n  sending process %d \n ",i);
            Send(pGeneratorSendQid, top); //Send message to scheduler containing this process data
            kill(schdID, SIGUSR1);  //Send signal to scheduler to wake it up to receive arriving process
            processes.pop();
            top=processes.front();
            i++;
        }
    }
    while(1){}

}

/*------------Functions' implementations------------*/


void clearResources(int)
{
    destroyClk(true);
    exit(1);
    //TODO: it clears all resources in case of interruption
}


/*------------Functions------------*/

//--- Load() ---//
// This functions loads the input file into a queue
void Load()
{
        char Data[500];
        ifstream inp("processes.txt");

        //Load each process in a struct of type ProcessData
         struct processData p;

        while( inp.getline(Data,500))
        {

        ////////////////////////////////////////////// FOR TESTING /////////////////////////////////////////////
                    printf("\nData= %s \n ", Data);
        ////////////////////////////////////////////////////////////////////////////////////////////////////////

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
        return;
}

//--- Send(QueueID, Process's Struct) ---//
//This function is used to send process data to scheduler when current time = arrival time of process
void Send(key_t pGeneratorSendQid, struct processData processToSend)
{
  int send_val;
  struct processMsgBuff message;
  message.mProcess = processToSend;
  message.mtype = 7;
  ////////////////////////////////////////////// FOR TESTING /////////////////////////////////////////////
          cout << "\nSending a process of ID = " << message.mProcess.id << " from Process Generator to Scheduler\n";
  ////////////////////////////////////////////////////////////////////////////////////////////////////////
 send_val = msgsnd(pGeneratorSendQid, &message, sizeof(message.mProcess), !IPC_NOWAIT);

  if(send_val == -1)
        perror("Error in send \n ");
  else printf("\n Process sent successfully \n ");
  return;
}

//--- childSignals(Signal Key) ---//
void childSignals(int signum)
{
        int pid , stat_loc;
////////////////////////////////////////////// FOR TESTING /////////////////////////////////////////////
        printf("\n A Child of process generator has sent a SIGCHLD signal. The Signal = #%d\n",signum);
////////////////////////////////////////////////////////////////////////////////////////////////////////
        pid = wait(&stat_loc);
        printf("\n pid= %d",pid);
        if(pid==schdID&&!(stat_loc & 0x00FF)) printf("\n The scheduler terminated with exit code %d\n",  stat_loc>>8);
        else if(pid==clkID&&!(stat_loc & 0x00FF)) printf("\n The clock terminated with exit code %d\n",  stat_loc>>8);
        else printf("\n The scheduler didn't terminate but sent a signal anyway.");
        return;
}
