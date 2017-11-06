#include "headers.h"

/*----------Global variables-----------*/
priority_queue <struct processData> readyQ;
queue <struct processData> roundRobinQ;
vector <struct processBlock> processTable;
struct processData topProcess, runningProcess;
struct msqid_ds buf;
key_t schedulerRcvQid;
string remainingTimeStr;
int choice= 0, quantum = 0, noProcesses = 0, noFinished = 0, sleepingTime = 0, currentTime=0, timeBeforePausing =0, pBIndex = 0;
int maxFinishTime=0, minStartTime=0;     //maximum finish time, minimium start time
int rc,num_messages;
int sumRunningTime=0;                           //sum of running time
int sumWaitTime=0;                              //sum of waiting time
vector <double> weightedTurnaround;             //weighted TA array
double sumWeightedTurnaround = 0.0;             //sum of weighted TA
double wta = 0.0f;
sigset_t set;
bool runningProcessFinished = false, afterAWhile = false;

// Creating output file: scheduler.log
ofstream schedulerLog ("scheduler.log");

/*---------Functions' Headers-----*/
void Receive(key_t schedulerRcvQid);
void runProcess();
void HPF_Algorithm();
void SRTN_Algorithm();
void RR_Algorithm();
void printSignalSet(sigset_t *set);
void setMaskedList();
void releaseBlockedSignals();
void outputStatFile ();
void outputCurrentStatus ();
void updateRemainingTime();
//Signals' Handlers
void newProcessHandler(int signum);
void finishedChildHandler(int signum);
int getIndexByID(int id);

int main(int argc, char* argv[])
{
	/*-----------Communication with other files--------*/
    signal (SIGUSR1, newProcessHandler);
    signal (SIGCHLD, finishedChildHandler);

    //Create queue for scheduler to receive messages
    schedulerRcvQid= msgget(12614, IPC_CREAT | 0644);

    choice = atoi(argv[0]);
    quantum = atoi(argv[1]);
    noProcesses = atoi(argv[2]);
    runningProcess.remainingTime=0; //testing
    runningProcess.id=0;

    initClk();
    currentTime= getClk();
    cout << " Scheduler: I recieved "<<argc<<" arguments from process generator."<<endl;
    cout << " Scheduler: Choice="<< argv[0]<<", Quantum="<< argv[1]<<", and number of processes="<< argv[2]<<"."<<endl;



    if(schedulerRcvQid == -1)
    {
        perror(" \n Error in creating process receiving queue in scheduler");
        exit(-1);
    }


    while(roundRobinQ.size() <1 && readyQ.size() < 1) {}  // wait for the first process 

	afterAWhile=true;
	minStartTime=getClk();
    switch (choice)
	{
	    case HPF:
	        cout << "@clk "<<minStartTime<<" : Scheduler: Intiating HPF Algorithm..." <<endl;
	        HPF_Algorithm();
	        maxFinishTime=getClk();
	        cout << "@clk "<<maxFinishTime<<" : Scheduler: Finished finally, HPF Algorithm running time= "<<maxFinishTime-minStartTime<<endl;
	        break;

	    case SRTN:
	        cout << "@clk "<<minStartTime<<" : Scheduler: Intiating SRTN Algorithm..." <<endl;
	        SRTN_Algorithm();
	        maxFinishTime=getClk();
			cout << "@clk "<<maxFinishTime<<" : Scheduler: Finished finally, SRTN Algorithm running time= "<<maxFinishTime-minStartTime<<endl;
	        break;

	    case RoundRobin:

	        cout << "@clk "<<minStartTime<<" : Scheduler: Intiating RoundRobin Algorithm..." <<endl;
	        RR_Algorithm();
	        maxFinishTime=getClk();
	        cout << "@clk "<<maxFinishTime<<" : Scheduler: Finished finally, RoundRobin Algorithm running time= "<<maxFinishTime-minStartTime<<endl;
	        break;
    }

    cout << "@clk "<<maxFinishTime<<" : Scheduler: I will close now."<<endl;
    if (schedulerLog.is_open()) schedulerLog.close();
    outputStatFile();
    destroyClk(true);
    exit(1);
}

/*--------------------Functions-----------------*/

//This function is used to run process on top of readyQueue
void runProcess()
{
    runningProcessFinished = false;
    cout << " Scheduler: Running a new process."<<endl;
    if(runningProcess.runningTime == runningProcess.remainingTime) //If this is the first time the process runs (not forked yet)
    {	//create a process block for the new starting process and initialize all its data
    	 struct processBlock pB;
        pB.id = runningProcess.id;
        pB.arrivalTime=runningProcess.arrivalTime;
        pB.remainingTime=runningProcess.remainingTime;
        pB.runningTime=runningProcess.runningTime;
        pB.priority=runningProcess.priority;
        pB.startTime=getClk();
        pB.finishTime=pB.arrivalTime;
        pB.waitTime = 0;
        pB.state="STARTED";
        processTable.push_back(pB);
        pBIndex = getIndexByID(runningProcess.id);
        printf("Index of starting process is %d\n",pBIndex);
        runningProcess.PID= fork();
        if(runningProcess.PID==0)
        {	//Send parameter remaining time to process
            remainingTimeStr = to_string(runningProcess.remainingTime);
            cout << " Process"<<runningProcess.id<<": I started with remaining time="<<runningProcess.remainingTime<<" and criteria="<<runningProcess.criteria<<endl;
            char*const processPar[] = {(char*)remainingTimeStr.c_str(), 0};
            execv("./process.out", processPar);
        }
    }
    else
    {
        pBIndex = getIndexByID(runningProcess.id);
    	//If entered here, then this process is a RESUMING process
        cout<< " Process"<<runningProcess.id<<": I resumed, with remaining time ="<< runningProcess.remainingTime<<" and my running time equals "<<runningProcess.runningTime<<endl;
        processTable[pBIndex].state = "RESUMED";
        kill(runningProcess.PID, SIGCONT); //Send a signal to process to continue running
    }
	//update process wait time 
        processTable[pBIndex].waitTime += (getClk() - processTable[pBIndex].finishTime); //finish time here is the last stored time it paused (initially arrival time)
        outputCurrentStatus();
}

//This function is used to receive processes from process generator
void Receive(key_t schedulerRcvQid)
{
    int rec_val;
    struct processMsgBuff message;
    rec_val = 0;


    rc = msgctl(schedulerRcvQid, IPC_STAT, &buf);
    num_messages = buf.msg_qnum;
	cout << "@clk "<<getClk()<<" : Scheduler: I am recieving "<<num_messages<<" processes in my buffer now!"<<endl;
    /* receive all types of messages */
    for(int j=0; j<num_messages; j++)
    {
        rec_val = msgrcv(schedulerRcvQid, &message, sizeof(message.mProcess), 0, !IPC_NOWAIT);



        if(rec_val == -1)
        {
        	cout << "@clk "<<getClk()<<" : Scheduler: I FAILED to received process #"<<message.mProcess.id<<endl;
            perror("\n  failure in recieving a process from process generator at the scheduler");
        }

        else
        {
        	cout << "@clk "<<getClk()<<" : Scheduler: I received successfully process #"<<message.mProcess.id<<" I will push it now in the ready queue."<<endl;
            //inserting process data into ready queue
            if(choice == 2) roundRobinQ.push(message.mProcess);
            else   readyQ.push(message.mProcess);
        }
    }
}


/*------------Signals' Handlers--------------*/
//Scheduler enters this handler when it receives a new signal from the process generator indicating a new process arrived
void newProcessHandler(int signum)
{
    cout << " Scheduler: I am recieving a signal that a new process is arriving."<<endl;
    if (choice==SRTN && afterAWhile)updateRemainingTime();
    Receive(schedulerRcvQid); //Scheduler goes to receive new process
}

//SIGCHLD handler
void finishedChildHandler(int signum)
{
    int pid , stat_loc;
    cout << " Scheduler: I am recieving signal #"<<signum<<" from a running process."<<endl;
    pid=wait(&stat_loc);
    if(pid==runningProcess.PID&&!(stat_loc& 0x00FF))
    {
    	runningProcessFinished = true;
    	noFinished++; //If a process terminates increment number of finished processes
	    //Update process block and process data
    	processTable[pBIndex].state = "FINISHED";
    	processTable[pBIndex].finishTime = getClk();
    	processTable[pBIndex].remainingTime = 0;
    	runningProcess.remainingTime=0;
    	outputCurrentStatus();
    	cout << "@clk "<<getClk()<<" : Scheduler: process #"<<runningProcess.id<<" terminated with exit code = "<< (stat_loc>>8) <<endl ;
        cout << " Scheduler: processes finished ="<<noFinished<<" out of "<< noProcesses <<" processes."<<endl;
	    //Update total running time,total wait time, total weighted TA, and store WTA of finished process
        sumRunningTime+=processTable[pBIndex].runningTime;
        sumWaitTime+=processTable[pBIndex].waitTime;
        wta = (double)(((double)((processTable[pBIndex].finishTime)-(processTable[pBIndex].arrivalTime)))/(processTable[pBIndex].runningTime));
        weightedTurnaround.push_back(wta);
        sumWeightedTurnaround += wta;
	 //Delete finished process data from process table 
        processTable.erase(processTable.begin() + pBIndex);
    }

    return;
}
/*------------Algorithms--------------*/
//HPF Algorithm
void HPF_Algorithm()
{
    while(noFinished < noProcesses)
    {
        if(!readyQ.empty())
        {
            runningProcess = readyQ.top();
            readyQ.pop();
            sleepingTime = runningProcess.remainingTime+1; //sleep more than remaining time -> interrupted when process exits
            setMaskedList();
            runProcess();
            sleep(sleepingTime);
            cout << "@clk "<<getClk()<<" : Scheduler: I am awake now."<<endl;
            releaseBlockedSignals();  //Go handle signals
        }
        else pause();	// to sleep in the gaps

    }  // End while

}

//SRTN Algorithm
void SRTN_Algorithm()
{
    while(noFinished < noProcesses)
    {
        if(!readyQ.empty())
        {
            runningProcess = readyQ.top();
            readyQ.pop();
            cout << "@clk "<<getClk()<<" : Scheduler: I will run process #"<< runningProcess.id<< " now, and ready queue size ="<<readyQ.size()<<endl;

           	runProcess();

            timeBeforePausing=getClk();
            do {
            pause();
            }
            while(!(readyQ.top().remainingTime<processTable[pBIndex].remainingTime) && !runningProcessFinished);

             printf("\nOUT OF WHILE LOOP and process finished is %d\n", runningProcessFinished);
            if(!runningProcessFinished) // if the process is not finished yet
            {
            	kill(runningProcess.PID, SIGUSR2);  //Send process signal to pause
		    //Update process block 
            	processTable[pBIndex].state = "STOPPED";
            	processTable[pBIndex].remainingTime = runningProcess.remainingTime;
            	processTable[pBIndex].finishTime = getClk(); //needed to get instantinous wait time, we can check if remaining time>0 then this finish time is not final
                cout<< " Scheduler: Pushing the running process #"<<runningProcess.id<<" in the ready queue again and its remaining time="<< runningProcess.remainingTime <<endl;
                outputCurrentStatus();
                readyQ.push(runningProcess);
            }

        }
        else pause(); // to sleep in the gaps
    }// End while
}

//Round Robin Algorithm
void RR_Algorithm()
{
    while(noFinished < noProcesses)
    {
        if(!roundRobinQ.empty())
        {
            runningProcess = roundRobinQ.front();
            roundRobinQ.pop();
            setMaskedList();
            runProcess();

            if (runningProcess.remainingTime>quantum)  // The process will not finish this loop
            {
                sleepingTime = quantum;
                sleep(sleepingTime);
                cout << "@clk "<<getClk()<<" : Scheduler: I am awake now."<<endl;
                releaseBlockedSignals();


                //Sending a signal to pause the process
                kill(runningProcess.PID, SIGUSR2);

                //Updating process data and process block
                runningProcess.remainingTime-=quantum;
                processTable[pBIndex].state = "STOPPED";
                processTable[pBIndex].finishTime = getClk(); //needed to get instantaneous wait time, we can check if remaining time>0 then this finish time is not final
                outputCurrentStatus();
                cout << "@clk "<<getClk()<<" : Scheduler:  Pushing Process #"<<runningProcess.id<<endl;
                roundRobinQ.push(runningProcess);
                // update process block --> log
            }
            else
            {
                sleepingTime = runningProcess.remainingTime +1;  //if the process remaining time = quantum, I want to ensure that the process finishs and sends a signal
                sleep(sleepingTime);
                cout << "@clk "<<getClk()<<" : Scheduler: I am awake now."<<endl;
                releaseBlockedSignals();
                // update process block --> log

            }

        }
        else pause(); // to sleep in the gaps

    }  // End while
}

/* Iterates through a list of signals and prints out which signals are in a signal set. */
void printSignalSet(sigset_t *set)
{
    /* This listing of signals may be incomplete. */
    const int sigList[] = {  SIGINT, SIGKILL,  SIGUSR1,
                             SIGUSR2, SIGCHLD, SIGCONT, SIGSTOP
                          };
    const char *sigNames[] = {  "SIGINT","SIGKILL", "SIGUSR1",
                                "SIGUSR2", "SIGCHLD", "SIGCONT", "SIGSTOP"
                             };
    const int sigLen = 7;

    for(int i=0; i<sigLen; i++)
    {
        int ret = sigismember(set, sigList[i]);
        if(ret == -1)
        {
            perror("sigismember:");
            exit(EXIT_FAILURE);
        }
        else if(ret == 1)
        {
            printf("Signal %s=%d IS in the set.\n", sigNames[i], sigList[i]);
        }
        else
        {
            printf("Signal %s=%d is not in the set.\n", sigNames[i], sigList[i]);
        }
    }
}

//Function to release any blocked signals received while the scheduler was sleeping
void releaseBlockedSignals()
{
    /* Look for any signals that are currently blocked---and would be
     * triggered once they are unmasked. */
    sigset_t pendingSignalSet;
    sigpending(&pendingSignalSet);
    printf("--- Signals which are blocked/pending: ---\n");
    printSignalSet(&pendingSignalSet);

    printf("Removing all signals from mask.\n");
    if(sigemptyset(&set) != 0)
    {
        perror("sigemptyset:");
        exit(EXIT_FAILURE);
    }
    if(sigprocmask(SIG_SETMASK, &set, NULL) != 0)
    {
        perror("sigprocmask:");
        exit(EXIT_FAILURE);
    }
}
//Function to set list of signals to be masked (blocked) when the scheduler sleeps 
void setMaskedList()
{
    // update list of masked signals
    if(sigaddset(&set, SIGUSR1) != 0) // Add SIGUSR1 to our set
    {
        perror("sigaddset:");
        exit(EXIT_FAILURE);
    }
    /* Tell OS that we want to mask our new set of signals---which now includes SIGINT. */
    if(sigprocmask(SIG_SETMASK, &set, NULL) != 0)
    {
        perror("sigprocmask:");
        exit(EXIT_FAILURE);
    }

}
//Function that outputs the final scheduler.perf file
void outputStatFile ()
{
    cout<<" Scheduler: Outputing scheduler.perf file..."<<endl;
    int i;
    double utilization;                             //utilization
    double avgWeightedTurnaround = 0.0;             //average weighted TA
    double avgWaitTime;                             //average waiting time
    double accum = 0.0;                             //accumilating for Standard dev
    double stdDev;                                   //standard deviation of weighted TA


    printf("Max finish time = %d. Min Start time = %d. Running Time %d \n", maxFinishTime, minStartTime, sumRunningTime);

    utilization= (double)(sumRunningTime/(double)(maxFinishTime-minStartTime)) *100;
    cout<< utilization <<endl;
    avgWeightedTurnaround= sumWeightedTurnaround/noProcesses;
    avgWaitTime = sumWaitTime/noProcesses;

    //Calculating the standard deviation of weighted turnaround
    accum = 0.0;
    for (i=0;i<weightedTurnaround.size();i++)
    {
        accum += ((weightedTurnaround[i]-avgWeightedTurnaround)*(weightedTurnaround[i]-avgWeightedTurnaround));
    }
    stdDev = sqrt(accum/(weightedTurnaround.size()-1));

    // Creating output file: scheduler.perf
    ofstream myfile ("scheduler.perf");
    if (myfile.is_open())
    {
        myfile << "CPU utilization=" << utilization <<"%.\n";
        myfile << "Avg WTA=" << avgWeightedTurnaround <<"\n";
        myfile << "Avg Waiting=" << avgWaitTime <<"\n";
        myfile << "Std WTA=" << stdDev <<"\n";
        myfile.close();
    }
    else cout << "Unable to open file";
}
//Function that outputs current state of process in file Scheduler.log
void outputCurrentStatus ()
{
    if (schedulerLog.is_open())
    {
       	schedulerLog << "At time "<<getClk()<< " process "<< runningProcess.id;
       	schedulerLog << " "<<processTable[pBIndex].state;
       	schedulerLog << " arr " <<processTable[pBIndex].arrivalTime;
       	schedulerLog << " total " <<processTable[pBIndex].runningTime;
       	schedulerLog << " remain " <<processTable[pBIndex].remainingTime;
       	schedulerLog << " wait " <<processTable[pBIndex].waitTime;
       	if(processTable[pBIndex].state == "FINISHED")
       	{
       	double x;
       	x=((processTable[pBIndex].finishTime)-(processTable[pBIndex].arrivalTime));
       	schedulerLog << " TA " <<x;
       	schedulerLog << " WTA " <<x/(processTable[pBIndex].runningTime);
       	}

       	schedulerLog <<endl;

    }
    else cout << " Scheduler: unable to open scheduler.log file."<<endl;
}

//Function that updates the remaining time of the running process in SRTN Algorithm to compare it later with the remaining time of the top process
void updateRemainingTime()
{
	int slept=getClk()-timeBeforePausing;
	timeBeforePausing = getClk();
	runningProcess.remainingTime-=slept;
    runningProcess.criteria-=slept;
	processTable[pBIndex].remainingTime = runningProcess.remainingTime;
	cout << "@clk "<<getClk()<<" : Scheduler: I am awake now, I slept for  "<<slept<<" ticks."<<endl;
	printf("and the running process' remaining time is %d while the top's is %d\n", runningProcess.remainingTime, readyQ.top().remainingTime);
}
//Function to get index of process in process table by its id
int getIndexByID(int id) {

    for(int i=0; i<processTable.size(); i++) {
        if(processTable[i].id == id) return i;
    }
    return -1;
}

