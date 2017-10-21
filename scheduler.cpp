#include "headers.h"

//Global Variables
priority_queue <struct processData> readyQ;
queue <struct processData> roundRobinQ;
vector <struct processBlock> processTable;
struct processData topProcess, runningProcess;
bool forkRunning = false; //flag set when a process starts to make the scheduler fork a starting process
//run process on top of readyQueue
void runProcess() {
    forkRunning = false;
    runningProcess = topProcess;
    if(runningProcess.runningTime == runningProcess.remainingTime) {
	    processTable[runningProcess.id-1].state = "started";
	    forkRunning = true;
    }
    else processTable[runningProcess.id].state = "resumed";
}

int main(int argc, char* argv[]) {
    
    initClk();
    //assume received from process generator
    int quantum;
    //nProcesses;
    int algorithmChosen;

    //Local Variables
    bool processFinished = false;
    int currentClk;
    int tempPerProcess; //Used in case of round robin
   struct processData currentProcess;

    //TODO: implement the scheduler :)
    while(readyQ.size() > 0) {
        //if received process from process generator insert into queue
        //write here..
	    
        //make topProcess = process at the top of the queue (not necessarily the one currently running in HPF and SRTN)

	currentClk=getClk();
        switch (algorithmChosen)
       {  case HPF:
             topProcess = readyQ.top();
             readyQ.pop();
            break;

        case SRTN:
              topProcess = readyQ.top();
              readyQ.pop();
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
                }
            }

            //if running process finished its quantum time, stop it
            kill(runningProcess.PID, SIGSTOP);

            
            break;

        default:
            break;
   	 }
	}

    //upon termination release clock
    destroyClk(true);
    
}
