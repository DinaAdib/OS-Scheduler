#include "headers.h"

// Global Variables
queue <struct processData> processes;

/*----------Functions' headers-----------*/
void signalChild(int);
void ClearResources(int);
void Load();  //Load from input file
void Send(key_t pGeneratorSendQid, struct processData processToSend);
int clkID, schdID;
int choice;
int quantum;

int main()
{

    /*-----------Local Variables--------*/
    int i = 0; //index incremented when current time = arrival time of a process to index next upcoming process

    string choiceStr, quantumStr, nProcessesStr;

    key_t pGeneratorSendQid;

    pGeneratorSendQid= msgget(12614, IPC_CREAT | 0644);

    if(pGeneratorSendQid == -1)
    {
        perror("Error in creating process generator send Queue");
        exit(-1);
    }


    signal(SIGINT,ClearResources);
    signal(SIGCHLD,signalChild);

    //TODO:
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
    printf("\n Choose the scheduling algorithm: \n 1-Non Pre-emptive HPF \n 2-Pre-emptive SRTN \n 3-Round Robin: \n");
    scanf("%d",&choice);
    choice--;
    if(choice == 2)
    {
        printf("\n Please enter quantum value: \n");
        scanf("%d",&quantum);
    }
    else quantum = 0;
    // 2-Initiate and create Scheduler and Clock processes.



    Load();
    struct processData top=processes.front();
    //for testing
    printf("\n top arrival time= %d",top.arrivalTime);



    //Create clock process
    clkID=fork();
    if (clkID==0) //child executes itself
    {
        cout<<"starting";
        char*const clkPar[] = { 0 };
        execv("./clk.out", clkPar);
    }

    initClk();

    //Create scheduler and send chosen algorithm and quantum for RR
    schdID=fork();
    if (schdID==0) //child executes itself
    {
        quantumStr = to_string(quantum);
        choiceStr = to_string(choice);
        nProcessesStr = to_string(nProcesses);
        char*const schPar[] = {(char*)choiceStr.c_str(), (char*)quantumStr.c_str(), (char*)nProcessesStr.c_str(), 0 };
        execv("./sch.out",schPar);
    }
    // 3-use this function after creating clock process to initialize clock



    /////Toget time use the following function
    int currentT= getClk();
    printf("current time is %d\n",currentT);
    //TODO:  Generation Main Loop
    //4-Creating a data structure for process  and  provide it with its parameters

    //5-Send the information to  the scheduler at the appropriate time
    //(only when a process arrives) so that it will be put it in its turn.
    //cout for testing
    cout << "\n number of processes: " <<nProcesses;

    //while not all processes are sent, stay in while loop (Finishes when all processes arrive)
    while(i <= nProcesses)
    {
        currentT=getClk();

        if(top.arrivalTime == currentT)
        {
            Send(pGeneratorSendQid, top); //Send message to scheduler containing this process data
            kill(schdID, SIGUSR1);  //Send signal to scheduler to wake it up to receive arriving process
            processes.pop();
            top=processes.front();
            i++;
        }
    }
    while(1) {}

}


void signalChild(int signum)
{
    int pid , stat_loc;
    printf("\n Received signal %d from child", signum );
    pid=wait(&stat_loc);
    printf("\n pid= %d",pid);
    if(!(stat_loc& 0x00FF))
        printf("\n Scheduler terminated with exit code %d\n", stat_loc>>8);
    else
    {
        printf("\n Rana hatetganen\n");
    }

    return;
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

    while( inp.getline(Data,500))
    {

        printf("\nData= %s \n ", Data);
        if(Data[0]!='#')
        {
            stringstream ss(Data); //using stringstream to split string to int
            while(ss.good())
            {
                ss>> p.id;
                ss>>p.arrivalTime;
                ss>>p.runningTime;
                ss>>p.priority;
            }
            //initially remaining time of process = its running time
            p.remainingTime = p.runningTime;
            switch (choice)
            {
            case HPF:
                p.criteria = (10 - p.priority);
                break;

            case SRTN:
                p.criteria = p.remainingTime;
                break;

            default:
                p.criteria =0;
                break;
            }
            processes.push(p); // insert to queue
            nProcesses++;
        }
    }
    inp.close();
    return;
}

/*------------------Functions---------------*/

//This function is used to send process data to scheduler when current time = arrival time of process
void Send(key_t pGeneratorSendQid, struct processData processToSend)
{
    int send_val;
    struct processMsgBuff message;
    message.mProcess = processToSend;
    message.mtype = 7;
    cout << "\nProcess to be sent: ID = \n " << message.mProcess.id;
    send_val = msgsnd(pGeneratorSendQid, &message, sizeof(message.mProcess), !IPC_NOWAIT);

    if(send_val == -1)
        perror("Error in send \n ");
    else printf("\n Process sent successfully \n ");

}

