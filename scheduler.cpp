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
int rc;
struct msqid_ds buf;
int num_messages;
bool runningPFinished = false;

/*---------Functions' Headers-----*/
void Receive(key_t schedulerRcvQid);
void runProcess();
void HPF_Algorithm();
void SRTN_Algorithm();
void RR_Algorithm();
void printSignalSet(sigset_t *set);
void setMaskedList();
void releaseBlockedSignals();

int main(int argc, char* argv[])
{
    //Signals
    signal (SIGUSR1, newProcessHandler);
    signal (SIGCHLD, finishedChildHandler);

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


// while(readyQ.size() < noProcesses && roundRobinQ.size()<noProcesses) {}
    while(roundRobinQ.size() <1 && readyQ.size() < 1) {}  // wait for the first process ---> WILL BE CHANGED
    cout<< "RRQ Queue Size = \n " << roundRobinQ.size();

    switch (choice)
    {
    case HPF:
        cout<< "\n In HPF" <<endl;
        startTime=getClk();
        HPF_Algorithm();
        endTime=getClk();

        printf("\n Total Running Time %d \n",endTime-startTime);

        break;

    case SRTN:

        cout<< "\n In SRTN" <<endl;
        startTime=getClk();
        SRTN_Algorithm();
        endTime=getClk();

        printf("\n Total Running Time %d \n",endTime-startTime);
        break;

    case RoundRobin:

        cout<< "\n In Round Robin" <<endl;
        startTime=getClk();
        RR_Algorithm();
        endTime=getClk();

        printf("\n Total Running Time %d \n",endTime-startTime);
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
    runningPFinished = false;
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
        cout<< "Process running time = "<<runningProcess.runningTime << "is not equal to its remaining time ="<< runningProcess.remainingTime<<endl;
        processTable[runningProcess.id-1].state = "resumed";
        kill(runningProcess.PID, SIGCONT); //Send signal to process to continue running

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
    /* receive all types of messages */
    for(int j=0; j<num_messages; j++)
    {
        rec_val = msgrcv(schedulerRcvQid, &message, sizeof(message.mProcess), 0, !IPC_NOWAIT);
        printf("\n Received Successfully ID: %d  at time %d \n", message.mProcess.id ,getClk());
        if(rec_val == -1)
        {
            perror("\n  fail");
        }

        else
        {
            cout<<"\n Pushing Process of ID"<<message.mProcess.id<<" at time: "<<getClk()<<endl;
            //insert process data into ready queue
            if(choice == 2)
            {
                roundRobinQ.push(message.mProcess);
            }
            else   readyQ.push(message.mProcess);
            if(running)
            cout<<"\n Received Successfully ID: "<< message.mProcess.id<<"readyQ size:  \n" << readyQ.size() <<endl;
        }
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
    printf("\n SCHEDULER: Received signal %d from child", signum );
    pid=wait(&stat_loc);
    printf("\n pid= %d",pid);
    if(pid==runningProcess.PID&&!(stat_loc& 0x00FF))
    {
        printf("\n SCHEDULER: Child Process no %d terminated at time %d with exit code %d\n", runningProcess.PID, getClk(), stat_loc>>8);
        noFinished++;
        printf("\n noFinished: %d ",noFinished);
        runningPFinished = true;
    }
    else
    {
        printf("\n Child process has been paused or continued %d\n",pid);
    }

    return;
}

/*----------Functions-----------*/

//HPF Algorithm
void HPF_Algorithm()
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
            printf("\n I am awake now at time %d\n",getClk());
            releaseBlockedSignals();

        }

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
            printf("\n I am the top process, my id= %d and readyQ size = %d\n",runningProcess.id, readyQ.size());
            readyQ.pop();
            runProcess();
            int tBeforeSleeping=getClk();
            pause();
            kill(runningProcess.PID, SIGUSR2);
            int tAfterSleeping=getClk();
            int slept=tAfterSleeping-tBeforeSleeping;
            runningProcess.remainingTime-=slept;
            runningProcess.criteria-=slept;

            printf("\n I am awake now at time %d I slept for %d\n",getClk(), slept);

            if(!runningPFinished){
                cout<< "pushing the running process with remaining time "<< runningProcess.remainingTime <<endl;
                readyQ.push(runningProcess);
            }

        }  // End while
    }
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

            if (runningProcess.remainingTime>quantum)  // process hasn't finished
            {
                sleepingTime = quantum;
                sleep(sleepingTime);
                printf("\n I am awake now  at time: %d \n",getClk());
                releaseBlockedSignals();
                // update process data
                runningProcess.remainingTime-=quantum;
                kill(runningProcess.PID, SIGUSR2);
                cout<<"\n Pushing Process of ID"<<runningProcess.id<<" at time: "<<getClk()<<endl;
                roundRobinQ.push(runningProcess);
                // update process block

                // pause signal

                // meen yt3mlo push elawl?
            }
            else   // matet f msh 3wzeenha :)
            {

                sleepingTime = quantum+1;
                sleep(sleepingTime);
                printf("\n I am awake now at time: %d \n",getClk());
                releaseBlockedSignals();
                // update process block --> log

            }

        }

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
