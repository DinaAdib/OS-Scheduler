#include "headers.h"


//Signals' Handlers
void newProcessHandler(int signum);
void finishedChildHandler(int signum);

//Global Variables
priority_queue <struct processData> readyQ;
queue <struct processData> roundRobinQ;
vector <struct processBlock> processTable;
struct processData topProcess, runningProcess;
int algorithmChosen=1;
key_t schedulerRcvQid;
string remainingTimeStr;
string choice, quantum;
//int number;
//const char* nProcessesStr;

/*---------Functions' Headers-----*/
void Receive(key_t schedulerRcvQid);
void runProcess();

int main(int argc, char* argv[]) {

cout<< "count of arg: "<< argc <<endl;
    choice =argv[0];
    cout<< "arg 1: "<< argv[0]<<endl;
    cout<< "arg 2: "<< argv[1]<<endl;
     cout<< "arg 3: "<< argv[2]<<endl;
   // nProcessesStr = argv[2];
   // int x;
     string s=argv[2];
     int number = 0;
        int neg = s[0] == '-';
        int i = neg ? 1 : 0;
        while ( s[i] >= '0' && s[i] <= '9' )
        {
          number *= 10;             // multiply number by 10
          number += s[i] - '0'; // convet ASCII '0'..'9' to digit 0..9 and add it to number
          i ++;                     // step one digit forward
        }
        if ( neg )
           number *= -1;



        printf( "\n  number %d", number );
  //int number=stoi(s);

//for( x=0 ; x<(unsigned char)*nProcessesStr-'0'<10;nProcessesStr++)
  //1   x=10*x+(*nProcessesStr-'0');
  /*
    quantum = argv[2];
   // nProcessesStr = argv[2];
    printf("\n nProcesses = %d \n", nProcesses);
    cout<< "nProcesses = "<< nProcesses <<endl;
     cout<< "\n Hello. I am the scheduler!!! my algorithm= " << choice <<endl;
      cout<< "\n Hello. I am the scheduler!!! my quantum= " << quantum <<endl;*/
      // printf("\n Hello. I am the scheduler!!! my nProcesses= %d" , number);
    //   printf("\n Hello. I am the scheduler!!! my x= %d" , x);
   initClk();

   //Create queue for scheduler to receive messages
   schedulerRcvQid= msgget(12614, IPC_CREAT | 0644);

   if(schedulerRcvQid == -1)
   {
   perror(" \n Error in creating process receiving queue in scheduler");
   exit(-1);
   }
   //Signals
   signal (SIGUSR1, newProcessHandler);
   signal (SIGINT, finishedChildHandler);

  while(readyQ.size() < number && roundRobinQ.size()<number) {}
 cout<< "Ready Queue Size = \n " << readyQ.size();
 runningProcess.arrivalTime= 7;
 runningProcess.remainingTime= 7;
 runningProcess.runningTime= 7;
 runningProcess.priority= 1;
  readyQ.push(runningProcess);
  roundRobinQ.push(runningProcess);
 runProcess();
while(1){}
    //Variables received from process generator


    //Local Variables
    struct processData runningProcess;
    bool processFinished = false;
    int currentClk;
    int tempPerProcess; //Used in case of round robin

    //Local Variables
  /*

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
    destroyClk(true);

    exit(1);
}

/*--------------------Functions-----------------*/

//This function is used to run process on top of readyQueue
void runProcess() {
    //runningProcess = topProcess;
    printf("\n In the run process");
    if(runningProcess.runningTime == runningProcess.remainingTime) {
        struct processBlock pB;
        pB.arrivalTime=runningProcess.arrivalTime;
        pB.remainingTime=runningProcess.remainingTime;
        pB.runningTime=runningProcess.runningTime;
        pB.priority=runningProcess.priority;
        pB.startTime=getClk();
        pB.state="started";
        processTable.push_back(pB);
        runningProcess.PID= fork();

        if(runningProcess.PID==0){
            remainingTimeStr = to_string(runningProcess.remainingTime);

           printf("\n I am the child. my process number = %d, remaining time= %d, converted= \n ", runningProcess.id, runningProcess.remainingTime);
           cout << remainingTimeStr <<endl;

           char*const processPar[] = {(char*)remainingTimeStr.c_str(), 0};
           execv("./process.out", processPar);
        }
    }
    else {
      processTable[runningProcess.id-1].state = "resumed";
      kill(runningProcess.PID, SIGCONT); //Send signal to process to continue running
    }
}

//This function is used to receive processes from process generator
void Receive(key_t schedulerRcvQid)
{
  int rec_val;
  struct processMsgBuff message;
  /* receive all types of messages */
      rec_val = msgrcv(schedulerRcvQid, &message, sizeof(message.mProcess), 0, !IPC_NOWAIT);
    printf("\n Received Successfully ID: %d \n", message.mProcess.id);
      if(rec_val == -1)
            {perror("\n  fail");}

      else {
          //insert process data into ready queue
          if(!choice.compare("2")) roundRobinQ.push(message.mProcess);
          else   readyQ.push(message.mProcess);
               cout<<"\n Received Successfully ID: "<< message.mProcess.id<<"readyQ size:  \n" << readyQ.size() <<endl;
        }
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
