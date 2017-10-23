#include "headers.h"
#include <errno.h>

//Signals' Handlers
void newProcessHandler(int signum);
void finishedChildHandler(int signum);

//Global Variables
priority_queue <struct processData> readyQ;
queue <struct processData> roundRobinQ;
vector <struct processBlock> processTable;
struct processData topProcess, runningProcess;
int algorithmChosen;
//Create queue for scheduler to receive messages
key_t schedulerRcvQid;


/*---------Functions' Headers-----*/
void Receive(key_t schedulerRcvQid);
void runProcess();

int main(int argc, char* argv[]) {
	//cout for testing
    cout<< "\n Hello. I am the scheduler!!!" << getpid();
   
   initClk();

   schedulerRcvQid= msgget(12614, IPC_CREAT | 0644);
	
    //Signals
   signal (SIGUSR1, newProcessHandler);
   signal (SIGCHLD, finishedChildHandler);

	//While loop then run process for testing
   while(readyQ.size() < 10) {

      }
   runningProcess = readyQ.top();
   runProcess();

    //Variables received from process generator   
    algorithmChosen = argv[1];	
    int quantum = argv[2];
    //nProcesses;
	
	//for testing variables received
    for(int i=0; i<argc; i++){
        cout<<argv[i] << "  ";
    }
    //Local Variables
  /*  bool processFinished = false;
    int currentClk;
    int tempPerProcess; //Used in case of round robin
    struct processData runningProcess;



    if(schedulerRcvQid == -1)
    {
    perror("Error in creating process receiving queue in scheduler");
    exit(-1);
    }

    Receive(schedulerRcvQid);

    //TODO: implement the scheduler :)
   while(readyQ.size() > 0) {
        //if received process from process generator insert into queue
        //write here..


	currentClk=getClk();
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
                 topProcess = roundRobinQ.front();
                 roundRobinQ.pop();
                tempPerProcess = quantum;

            //while process didn't finish executing
            while((runningProcess.remainingTime > 0) && tempPerProcess > 0) {
                if(pulse(currentClk)) {
                    tempPerProcess--;
                    (runningProcess.remainingTime)--;
                    Receive(schedulerRcvQid);
                }
             }

            //if running process finished its quantum time, stop it
               kill(runningProcess.PID, SIGSTOP);


            break;

            default:
                break;
   	 }
    }
    */
    //upon termination release clock
  //  destroyClk(true);
}

/*--------------------Functions-----------------*/
char remainingTimePar[2];
//This function is used to run process on top of readyQueue
void runProcess() {
    //runningProcess = topProcess;
    printf("In the run process");
    if(runningProcess.runningTime == runningProcess.remainingTime) {

        processTable[runningProcess.id-1].state = "started";
        runningProcess.PID= fork();

        if(runningProcess.PID==0){
            printf("I am the child. my process number = %d", runningProcess.id);
            sprintf(remainingTimePar, "%d", runningProcess.remainingTime);
            char *processPar[] = { "./process.out",remainingTimePar, 0 };
            execve(processPar[0], &processPar[0], NULL);
        }
    }
    else processTable[runningProcess.id-1].state = "resumed";
}

//This function is used to receive processes from process generator
void Receive(key_t schedulerRcvQid)
{
  int rec_val;
  struct processMsgBuff message;
    cout<< nProcesses;
  /* receive all types of messages */

      rec_val = msgrcv(schedulerRcvQid, &message, sizeof(message.mProcess), 0, !IPC_NOWAIT);
    printf("Received Successfully ID: %d", message.mProcess.id);
      if(rec_val == -1)
            {cout<< "\n error no"<<errno;
            perror("/n fail");}

      else {
          printf("Received Successfully ID: %d, readyQ size: %d", message.mProcess.id, readyQ.size());
          //insert process data into ready queue
            if(algorithmChosen == RoundRobin) roundRobinQ.push(message.mProcess);
            else readyQ.push(message.mProcess);}

}


/*------------Signals' Handlers--------------*/
void newProcessHandler(int signum)
{
    cout<<"\nIn the handler!!!!\n";
    Receive(schedulerRcvQid);

}

void finishedChildHandler(int signum)
{


}
