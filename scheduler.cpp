#include "headers.h"
#include <errno.h>

/*----------Global Variables-----------*/
//Containers for multi processes's structs
priority_queue <struct processData> readyQ;	//Ready Queue for HDP and SRTN
queue <struct processData> roundRobinQ;		//Ready Queue for Round Robin
vector <struct processBlock> processTable;	//Processes Data Block Sorted by ID

struct processData topProcess, runningProcess;	//one process container
struct processBlock pB;                         //To save process in process block entry
Algorithm algorithmChosen=SRTN; 		//Algorithm choice
key_t schedulerRcvQid; 				//Queue ID used by Scheduler to recieve data
int  schID;					//Scheduler PID
char schIDPar[2];				//Scheduler to be passed as an argument
char remainingTimePar[2];			//Remaining time to be passed as an argument for each process

//Round robin algorithm specific variables
int quantum=3;	//Received from process generator
int tempPerProcess=3;

//Signals' Handlers
void newProcessHandler(int signum);
void finishedChildHandler(int signum);


/*---------Functions' Headers-----*/
//Signals' Handlers
void newProcessHandler(int signum);
void finishedChildHandler(int signum);

//Data Communication
void Receive(key_t schedulerRcvQid);
//running (forking) a process
void runProcess();
void switchProcess(struct processData runningProcess);
void updateProcessBlock(struct processData runningProcess, string status);

int main(int argc, char* argv[]) {

    schID = getpid();
////////////////////////////////////////////// FOR TESTING /////////////////////////////////////////////
    cout<< "\n Hello. I am the scheduler!!! and My PID=\n" << schID;
///////////////////////////////////////////////////////////////////////////////////////////////////////


   initClk();

   /*-----------Creating a queue for the scheduler to receive messages--------*/
       schedulerRcvQid= msgget(12614, IPC_CREAT | 0644); //Get The ID
       if(schedulerRcvQid == -1)
       {
               perror(" \n Error in creating process receiving queue in scheduler");
               exit(-1);
       }

   /*-----------Handeling different signals--------*/
       signal (SIGUSR1, newProcessHandler);	//New process Handling (triggered by process generator)
       signal (SIGINT, finishedChildHandler);	//Child (process) finish (triggered by current process running)

    /*-----------Local Variables--------*/
       bool processFinished = false;
       struct processData runningProcess;
       int currentClk;


       while(readyQ.size() < 3) {}
 cout<< "Ready Queue Size = \n " << readyQ.size();
  // runningProcess = readyQ.top();
 runProcess();
    while(1){}



  /*


        while (1)
        {
                //currentClk=getClk();
        if(readyQ.size() > 0)
        {
                switch (algorithmChosen)
                {
                   case HPF:

                    //make topProcess = process at the top of the queue (not necessarily the one currently running in HPF and SRTN)
                     topProcess = readyQ.top();
                     readyQ.pop();

                    break;

                    case SRTN:

                      runningProcess=readyQ.top();
                      while(nFinished!=nProcesses){
                            runProcess();
                            int start= getclk();
                            sleep();
                            int wake = getclk();
                            runningProcess.remainingTime = runningProcess.remainingTime - (start-wake);

                            if(runningProcess.remainingTime==0) {
                             //delete process
                            }
                            else { if(runningProcess.remainingTime > (readyQ.top()).remainingTime) {
                            //context switching

                            }
                        }

                    break;

                    case RoundRobin:
                        processFinished=false;
                        runningProcess=readyQ.front();
                        readyQ.pop();
                        tempPerProcess=runningProcess.runningTime;
                        runProcess();
                        sleep(quantum);		//to try later: sleep(min(Q,x))


                        if(!processFinished || ((x-Q)!=0)) switchProcess(); x=x-Q; -> save the remTime;
                        else { save the remTime=0; Set "FINISHED" }

                        #2 (sigchld handler function)
                                if (the signal from the process child) check (x<Q){//the process is killed-> raise flag} else {Error}

                Must do:
                1- Edit: CHLD handler




                        break;

                default: break;
                } //End of switch case
        }//End of if condition

        else raise(SIGSTOP);

        } //End of while(1)
                //The scheduler kills itself
                /* The scheduler should wake by any of the following signals:
                        1- If the Process generator sends a signal.
                        2- If a process sends a signal.
                        3- At every clock pulse it wakes and then kills itself again.
                        (Suggestion: a better way for round robin is to ignore clock pulses and "sleep(Q).")
                */


    //upon termination release clock
    destroyClk(true);

    exit(1);
}

/*--------------------Functions-----------------*/

//This function is used to run process on top of readyQueue
void runProcess() {
    //Running process will be the one on top or front of the queue
    if(algorithmChosen == RoundRobin) runningProcess = roundRobinQ.front();
    else runningProcess = readyQ.top();

    ////////////////////////////////////////////// FOR TESTING /////////////////////////////////////////////
        printf("\n In the run process function: \n");
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    if(runningProcess.runningTime == runningProcess.remainingTime) {
 /*       pB.arrivalTime=runningProcess.arrivalTime;
        pB.remainingTime=runningProcess.remainingTime;
        pB.runningTime=runningProcess.runningTime;
        pB.priority=runningProcess.priority;
        pB.startTime=getClk();
        pB.state="started";
        processTable.push_back(pB);*/

        runningProcess.PID= fork(); //if process is just starting, fork new process

        if(runningProcess.PID==0){
            printf("\n I am the child. my process number = %d  and my remaining Time= %d\n ", runningProcess.id ,runningProcess.remainingTime);
           //Convert remaining time and scheuler ID to parameters of type char*
            sprintf(remainingTimePar, "%d", runningProcess.remainingTime);
            sprintf(schIDPar, "%d", schID);
            char *processPar[] = { "./process.out",remainingTimePar, schIDPar, 0};
            execve("./process.out", &processPar[0], NULL);
        }
    }
    else {
      processTable[runningProcess.id-1].state = "resumed";
      kill(runningProcess.PID, SIGCONT); //Send signal to process to continue running
    }
}


//--- switchProcess(processData) ---//
//This function is used to switch to the process on the top of readyQueue
//{send pause; -> push();}
void switchProcess(struct processData runningProcess)
{
        // Pause the running process
        kill(runningProcess.PID, SIGSTOP);

        // Push again to the ready queue
        switch (algorithmChosen)
        {
                case SRTN:
                        readyQ.push(runningProcess);
                        break;
                case RoundRobin:
                        roundRobinQ.push(runningProcess);
                        break;
                default:
                        printf("\n Error in switching the proccesses.\n");
                        break;
        }
}

//--- updateProcessBlock(processData, status) ---//
//This function is to update the process Block of the running process
void updateProcessBlock(struct processData runningProcess, string status)
{
        processTable[runningProcess.id-1].runningTime = runningProcess.runningTime;
        processTable[runningProcess.id-1].remainingTime = runningProcess.remainingTime;
        processTable[runningProcess.id-1].state = status;
}

//--- Recieve (QueueID) ---//
//This function is used to receive processes from process generator
void Receive(key_t schedulerRcvQid)
{
  int rec_val;
  struct processMsgBuff message;
  /* receive all types of messages */
      rec_val = msgrcv(schedulerRcvQid, &message, sizeof(message.mProcess), 0, !IPC_NOWAIT);

      if(rec_val == -1)
            {cout<< "\n  error no"<<errno;
            perror("\n  fail");}

      else {
          //insert process data into ready queue
            if(algorithmChosen == RoundRobin) roundRobinQ.push(message.mProcess);
            else readyQ.push(message.mProcess);}
      printf("\n Received Successfully ID: %d, readyQ size: %d \n", message.mProcess.id, readyQ.size());

      /*
                      //Update process Table
                      struct processBlock pBlock(message.mProcess);
                      processTable[message.mProcess.id-1]=pBlock;
      */

}


/*------------Signals' Handlers--------------*/
void newProcessHandler(int signum)
{
    cout<<"\nIn the new process handler!!\n";
    Receive(schedulerRcvQid);

}

void finishedChildHandler(int signum)
{
    /*if(runningProcess.runningTime == 0){
        //switch
        processTable[runningProcess.id].state = "Finished";
        processTable[runningProcess.id].finishTime= getClk();
        kill(runningProcess.PID, SIGKILL);
    }
    else {
    //Process hasn't finished yet (probably round robin)
      //push this process into queue then runProcess again

    }
    */

}
