#include "headers.h"


//Signals' Handlers
void newProcessHandler(int signum);
void finishedChildHandler(int signum);

//Global Variables
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
int startTime;
int endTime;
sigset_t set;

/*---------Functions' Headers-----*/
void Receive(key_t schedulerRcvQid);
void runProcess();
void setMaskedList();
void releaseBlockedSignals();
void HPFAlgorithm();
void printSignalSet(sigset_t *set);

int main(int argc, char* argv[])
{

    cout<< "count of arg: "<< argc <<endl;

    cout<< "arg 1: "<< argv[0]<<endl;
    cout<< "arg 2: "<< argv[1]<<endl;
    cout<< "arg 3: "<< argv[2]<<endl;

    choice = atoi(argv[0]);
    quantum = atoi(argv[1]);
    noProcesses = atoi(argv[2]);


    printf( "\n  number of processes: %d", noProcesses );

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
    signal (SIGCHLD, finishedChildHandler);

// while(readyQ.size() < noProcesses && roundRobinQ.size()<noProcesses) {}
    while(readyQ.size() <1) {}  // wait for the first process
    cout<< "Ready Queue Size = \n " << readyQ.size();

    switch (choice)
    {
    case HPF:
        cout<< "\n In HPF" <<endl;
        startTime=getClk();
        HPFAlgorithm();
        endTime=getClk();

        printf("\n Total Running Time %d \n",endTime-startTime);

        break;

    case SRTN:

        break;

    case RoundRobin:

        break;

    default:
        break;
    }

    //upon termination release clock

    printf("\n Scheduler Terminating \n");

    destroyClk(true);

    exit(1);
}

/*--------------------Functions-----------------*/

//This function is used to run process on top of readyQueue
void runProcess()
{
    //runningProcess = topProcess;
    printf("\n In the run process");
    if(runningProcess.runningTime == runningProcess.remainingTime)
    {
        struct processBlock pB;
        pB.arrivalTime=runningProcess.arrivalTime;
        pB.remainingTime=runningProcess.remainingTime;
        pB.runningTime=runningProcess.runningTime;
        pB.priority=runningProcess.priority;
        pB.startTime=getClk();
        pB.state="started";
        processTable.push_back(pB);
        runningProcess.PID= fork();

        if(runningProcess.PID==0)
        {
            remainingTimeStr = to_string(runningProcess.remainingTime);

            printf("\n I am the child. my process number = %d, remaining time= %d, criteria = %d\n ", runningProcess.id, runningProcess.remainingTime, runningProcess.criteria);
            cout << remainingTimeStr <<endl;

            char*const processPar[] = {(char*)remainingTimeStr.c_str(), 0};
            execv("./process.out", processPar);
        }
    }
    else
    {
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
    {
        perror("\n  fail");
    }

    else
    {
        //insert process data into ready queue
        if(choice == 2) roundRobinQ.push(message.mProcess);
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
    int pid , stat_loc;
    printf("\n Received signal %d from child", signum );
    pid=wait(&stat_loc);
    printf("\n pid= %d",pid);
    if(pid==runningProcess.PID&&!(stat_loc& 0x00FF))
    {
        printf("\n Child Process no %d terminated with exit code %d\n", runningProcess.PID, stat_loc>>8);
        noFinished++;
        printf("\n noFinished: %d ",noFinished);
    }
    else
    {
        printf("\n Dina hatetganen :'( 3shan PID= %d\n",pid);
    }

    return;
}

/*----------Functions-----------*/

void HPFAlgorithm()
{
    while(noFinished < noProcesses)
    {
        if(!readyQ.empty())
        {
            runningProcess = readyQ.top();
            readyQ.pop();
            sleepingTime = runningProcess.remainingTime+1;
            setMaskedList();
            runProcess();
            sleep(sleepingTime);
            printf("\n I am awake now \n");
            releaseBlockedSignals();

        }

    }  // End while

}


/* Iterates through a list of signals and prints out which signals are in a signal set. */
void printSignalSet(sigset_t *set)
{
    /* This listing of signals may be incomplete. */
    const int sigList[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL,
                            SIGABRT, SIGFPE, SIGKILL, SIGSEGV,
                            SIGPIPE, SIGALRM, SIGTERM, SIGUSR1,
                            SIGUSR2, SIGCHLD, SIGCONT, SIGSTOP,
                            SIGTSTP, SIGTTIN, SIGTTOU
                          };
    const char *sigNames[] = { "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL",
                               "SIGABRT", "SIGFPE", "SIGKILL", "SIGSEGV",
                               "SIGPIPE", "SIGALRM", "SIGTERM", "SIGUSR1",
                               "SIGUSR2", "SIGCHLD", "SIGCONT", "SIGSTOP",
                               "SIGTSTP", "SIGTTIN", "SIGTTOU"
                             };
    const int sigLen = 19;

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
    printf("--- NEW signal mask for this process: ---\n");
    printSignalSet(&set);

}
