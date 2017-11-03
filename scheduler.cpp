#include "headers.h"

/*----------Global variables-----------*/
priority_queue <struct processData> readyQ;
queue <struct processData> roundRobinQ;
vector <struct processBlock> processTable;
struct processData topProcess, runningProcess;
key_t schedulerRcvQid;
string remainingTimeStr;
int choice= 0;
int quantum = 0;
int noProcesses = 0;
int noFinished = 0;
int sleepingTime = 0;
int currentTime=0;
int runningProcessStartTime =0;
int startTime;
int endTime;
sigset_t set;
int rc;
struct msqid_ds buf;
int num_messages;
bool runningProcessFinished = false;

bool afterAWhile = false;
// Creating output file: scheduler.log
ofstream schedulerLog ("scheduler.log");

//if (schedulerLog.is_open()) schedulerLog<<"#At time x process y state arr w total z remain y wait k"<<endl;
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

int main(int argc, char* argv[])
{
	/*-----------Communication with other files--------*/
    signal (SIGUSR1, newProcessHandler);
    signal (SIGCHLD, finishedChildHandler);
    //kill(getppid(),SIGCONT);

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


	// while(readyQ.size() < noProcesses && roundRobinQ.size()<noProcesses) {}
    while(roundRobinQ.size() <1 && readyQ.size() < 1) {}  // wait for the first process ---> WILL BE CHANGED
    
    //cout<< "RRQ Queue Size = \n " << roundRobinQ.size();
    currentTime= getClk();
	afterAWhile=true;
    switch (choice)
	{
	    case HPF:
	        cout << "@clk "<<currentTime<<" : Scheduler: Intiating HPF Algorithm..." <<endl;
	        startTime=getClk();
	        HPF_Algorithm();
	        endTime=getClk();
	        cout << "@clk "<<endTime<<" : Scheduler: Finished finally, HPF Algorithm running time= "<<endTime-startTime<<endl;
	        break;

	    case SRTN:
	        cout << "@clk "<<currentTime<<" : Scheduler: Intiating SRTN Algorithm..." <<endl;
	        startTime=getClk();
	        SRTN_Algorithm();
	        endTime=getClk();
			cout << "@clk "<<endTime<<" : Scheduler: Finished finally, SRTN Algorithm running time= "<<endTime-startTime<<endl;
	        break;

	    case RoundRobin:

	        cout << "@clk "<<currentTime<<" : Scheduler: Intiating RoundRobin Algorithm..." <<endl;
	        startTime=getClk();
	        RR_Algorithm();
	        endTime=getClk();
	        cout << "@clk "<<endTime<<" : Scheduler: Finished finally, RoundRobin Algorithm running time= "<<endTime-startTime<<endl;
	        break;
    }

    cout << "@clk "<<getClk()<<" : Scheduler: I will close now."<<endl;
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
    if(runningProcess.runningTime == runningProcess.remainingTime)
    {
    	//If entered here, then this process is a NEW process
        struct processBlock pB;
        pB.arrivalTime=runningProcess.arrivalTime;
        pB.remainingTime=runningProcess.remainingTime;
        pB.runningTime=runningProcess.runningTime;
        pB.priority=runningProcess.priority;
        pB.startTime=getClk();
        pB.state="STARTED";
        processTable.push_back(pB);
        outputCurrentStatus();
        runningProcess.PID= fork();
        if(runningProcess.PID==0)
        {
            remainingTimeStr = to_string(runningProcess.remainingTime);
            cout << " Process"<<runningProcess.id<<": I started with remaining time="<<runningProcess.remainingTime<<" and criteria="<<runningProcess.criteria<<endl;
            //cout << remainingTimeStr <<endl;
            char*const processPar[] = {(char*)remainingTimeStr.c_str(), 0};
            execv("./process.out", processPar);
        }
    }
    else
    {
    	//If entered here, then this process is a RESUMING process
        cout<< " Process"<<runningProcess.id<<": I resumed, with remaining time ="<< runningProcess.remainingTime<<" and my running time equals "<<runningProcess.runningTime<<endl;
        processTable[runningProcess.id-1].state = "RESUMED";
        processTable[runningProcess.id-1].waitTime += (getClk() - processTable[runningProcess.id-1].finishTime);
        kill(runningProcess.PID, SIGCONT); //Send a signal to process to continue running
        outputCurrentStatus();
    }
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
void newProcessHandler(int signum)
{
    cout << " Scheduler: I am recieving a signal that a new process is arriving."<<endl;
    if (choice==SRTN && afterAWhile)updateRemainingTime();
    Receive(schedulerRcvQid);
}

void finishedChildHandler(int signum)
{
    int pid , stat_loc;
    cout << " Scheduler: I am recieving signal #"<<signum<<" from a running process."<<endl;
    pid=wait(&stat_loc);
    if(pid==runningProcess.PID&&!(stat_loc& 0x00FF))
    {
    	
    	runningProcessFinished = true;
    	noFinished++;
    	processTable[runningProcess.id-1].state = "FINISHED";
    	processTable[runningProcess.id-1].finishTime = getClk();
    	processTable[runningProcess.id-1].remainingTime = 0;
    	runningProcess.remainingTime=0;
    	//processTable[runningProcess.id-1].waitTime = processTable[runningProcess.id-1].finishTime - processTable[runningProcess.id-1].startTime - processTable[runningProcess.id-1].runningTime;
    	outputCurrentStatus();
    	cout << "@clk "<<getClk()<<" : Scheduler: process #"<<runningProcess.id<<" terminated with exit code = "<< (stat_loc>>8) <<endl ;
        cout << " Scheduler: processes finished ="<<noFinished<<" out of "<< noProcesses <<" processes."<<endl ;
    }
    
    //else printf("\n Child process has been paused or continued %d\n",pid);
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

            runningProcessStartTime=getClk();
            do {pause();}
            while(!(readyQ.top().remainingTime<processTable[runningProcess.id-1].remainingTime) && !runningProcessFinished);
            //while((processTable[runningProcess.id-1].remainingTime>0) && (readyQ.empty() || readyQ.top().remainingTime > processTable[runningProcess.id-1].remainingTime));
             
            if(!runningProcessFinished) // if the process is not finished yet
            {
            	kill(runningProcess.PID, SIGUSR2);  //Send process signal to pause 
            	processTable[runningProcess.id-1].state = "STOPPED";
            	processTable[runningProcess.id-1].remainingTime = runningProcess.remainingTime;
            	processTable[runningProcess.id-1].finishTime = getClk(); //needed to get instantinous wait time, we can check if remaining time>0 then this finish time is not final
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

                //Updating process data
                runningProcess.remainingTime-=quantum;

                //Sending a signal to pause the process
                kill(runningProcess.PID, SIGUSR2);  
                processTable[runningProcess.id-1].state = "STOPPED";
                processTable[runningProcess.id-1].finishTime = getClk(); //needed to get instantinous wait time, we can check if remaining time>0 then this finish time is not final
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
    /* Now, SIGINT will be "blocked". */
    //  printf("--- NEW signal mask for this process: ---\n");
    //  printSignalSet(&set);

}
void outputStatFile ()
{
    cout<<" Scheduler: Outputing scheduler.perf file..."<<endl;
    int i=0;                                        //index
    int maxFinishTime=processTable[0].finishTime;   //maximium finish time
    int minStartTime=processTable[0].startTime;     //minimium start time
    int sumRunningTime=0;                           //sum of running time
    int sumWaitTime=0;                              //sum of waiting time
    double utilization;                             //utilization
    double sumWeightedTurnaround = 0.0;             //sum of weighted TA
    double avgWeightedTurnaround = 0.0;             //average weighted TA
    double avgWaitTime;                             //average waiting time
    double accum = 0.0;                             //accumilating for Standard dev
    double stdDev;                                   //standard deviation of weighted TA
    vector <double> weightedTurnaround;             //weighted TA array


    for (i=0;i<processTable.size();i++)
        {
            //calculating weighted turnaround
            double wta=0.0;                                     //weighted turn around
            wta = ((processTable[i].finishTime)-(processTable[i].arrivalTime))/(processTable[i].runningTime);
            weightedTurnaround.push_back(wta);
            sumWeightedTurnaround += wta;
            sumWaitTime += processTable[i].waitTime;
            //getting parameters to calculate utilization   
            sumRunningTime += processTable[i].runningTime;
            if(maxFinishTime < processTable[i].finishTime) maxFinishTime = (processTable[i].finishTime);
            if(minStartTime > processTable[i].startTime) minStartTime = (processTable[i].startTime);
        }

    //utilization =  sum running time / (max finish - min start)
    //utilization *100
    utilization= (sumRunningTime/(maxFinishTime-minStartTime)) *100;

    avgWeightedTurnaround= sumWeightedTurnaround/processTable.size();
    avgWaitTime = sumWaitTime/processTable.size();

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
void outputCurrentStatus ()
{
    if (schedulerLog.is_open())
    {
       	schedulerLog << "At time "<<getClk()<< " process "<< runningProcess.id;
       	schedulerLog << " "<<processTable[runningProcess.id-1].state;
       	schedulerLog << " arr " <<processTable[runningProcess.id-1].arrivalTime;
       	schedulerLog << " total " <<processTable[runningProcess.id-1].runningTime;
       	schedulerLog << " remain " <<processTable[runningProcess.id-1].remainingTime;
       	schedulerLog << " wait " <<processTable[runningProcess.id-1].waitTime;
       	if(processTable[runningProcess.id-1].state == "FINISHED")
       	{
       	double x;
       	x=((processTable[runningProcess.id-1].finishTime)-(processTable[runningProcess.id-1].arrivalTime));
       	schedulerLog << " TA " <<x;
       	schedulerLog << " WTA " <<x/(processTable[runningProcess.id-1].runningTime);
       	}

       	schedulerLog <<endl;
        
    }
    else cout << " Scheduler: unable to open scheduler.log file."<<endl;
}
void updateRemainingTime()
{
	int slept=getClk()-runningProcessStartTime;
	runningProcess.remainingTime-=slept;
    runningProcess.criteria-=slept;
	processTable[runningProcess.id-1].remainingTime = runningProcess.remainingTime;
	cout << "@clk "<<getClk()<<" : Scheduler: I am awake now, I slept for  "<<slept<<" ticks."<<endl;
}
