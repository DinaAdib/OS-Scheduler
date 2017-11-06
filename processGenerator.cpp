#include "headers.h"

/*----------Global variables-----------*/
queue <struct processData> processes;
int clkID, schdID;
int choice;
int quantum=0;
int currentTime=0;

/*----------Functions' headers-----------*/
void clearResources(int);
void loadInputFile();
void Send(key_t pGeneratorSendQid, struct processData processToSend);
void readUserInput();
//Signals' Handlers
void signalChildHandler(int);

int main()
{
    /*-----------Local Variables--------*/
    int indexProcesses=0;
    string choiceStr, quantumStr, nProcessesStr;
    struct processData topProcess;
    key_t pGeneratorSendQid;

    /*-----------Communication with other files--------*/
    signal(SIGINT,clearResources);
    signal(SIGCHLD,signalChildHandler);

    pGeneratorSendQid= msgget(12614, IPC_CREAT | 0644);
    if(pGeneratorSendQid == -1) {perror("Error in creating process generator send Queue"); exit(-1);}

    /*-----------Reading input--------*/
    readUserInput();
    loadInputFile();
    if (nProcesses<1) {cout << "@clk "<<currentTime<<" : ProcessGenerator: The processes file is apparently empty. I'll close for now."<<endl; clearResources(1);}

    cout << "==============================================================================" <<endl;

    /*-----------Forking the scheduler--------*/
    schdID=fork();
    if (schdID==0) //child executes itself
    {
        quantumStr = to_string(quantum);
        choiceStr = to_string(choice);
        nProcessesStr = to_string(nProcesses);
        char*const schPar[] = {(char*)choiceStr.c_str(), (char*)quantumStr.c_str(), (char*)nProcessesStr.c_str(), 0 };
        execv("./sch.out",schPar);
    }
    /*-----------Forking the clock--------*/
    clkID=fork();
    if (clkID==0) //child executes itself
    {
        cout<<"@clk 0: clock is now starting..."<<endl;
        char*const clkPar[] = { 0 };
        execv("./clk.out", clkPar);
    }

    initClk();

    /*-----------Sending processes to the scheduler--------*/
    topProcess=processes.front();

    currentTime= getClk();
    cout<<"@clk "<<currentTime<<" : ProcessGenerator: I have a number of processes equals: "<<nProcesses<<endl;

    //while not all processes are sent, stay in while loop (finishes only when all processes arrive)
    while(indexProcesses <= nProcesses)
    {
        currentTime=getClk();
        if(topProcess.arrivalTime == currentTime)
        {
        	//sleep(1);
            Send(pGeneratorSendQid, topProcess);    //Send message to scheduler containing this process data
            kill(schdID, SIGUSR1);                  //Send signal to scheduler to wake it up to receive arriving process
            processes.pop();
            topProcess=processes.front();
            indexProcesses++;
        }
    }
    while(1) {sleep(1);}
}


/*------------Functions------------*/
// This function loads the input file into a queue
void loadInputFile()
{
    char Data[500];
    ifstream inp("processes.txt");

    //Load processes info
    struct processData p;
    cout<<"ProcessGenerator: Note we take up to 500 charachters from the .txt file."<<endl;
    while( inp.getline(Data,500))
    {

        //printf("\nData= %s \n ", Data);
        if(isdigit(Data[0]))
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

//This function is to output and read data from the user
void readUserInput()
{
    cout<<"\n Choose the scheduling algorithm: \n 1-Non Pre-emptive HPF \n 2-Pre-emptive SRTN \n 3-Round Robin."<<endl;
    do {scanf("%d",&choice);}
    while (choice<1 || choice>3);

    choice--;
    if(choice == RoundRobin)
    {
        printf("\n Please enter the value of one quantum: ");
        do {scanf("%d",&quantum);}
        while (quantum<1);
    }
    cout<<"ProcessGenerator: You chose number "<<choice+1<<", and quantum = "<<quantum<<". Good Luck :)"<<endl;
}

//This function is used to send process data to scheduler when current time = arrival time of process
void Send(key_t pGeneratorSendQid, struct processData processToSend)
{
    int send_val;
    struct processMsgBuff message;
    message.mProcess = processToSend;
    message.mtype = processToSend.arrivalTime;
    currentTime=getClk();
    cout<<"@clk "<<currentTime<<" : ProcessGenerator: Process to be sent is of ID = " << message.mProcess.id<<endl;
    send_val = msgsnd(pGeneratorSendQid, &message, sizeof(message.mProcess), !IPC_NOWAIT);

    if(send_val == -1)
        perror("Error in send \n ");
    else cout<<"@clk "<<currentTime<<" : ProcessGenerator: I sent the process successfully."<<endl;

}

// This function is signal child handler. It is called when a child sends a this signal
void signalChildHandler(int signum)
{
    int pid , stat_loc;
    currentTime= getClk();
    cout<<"@clk "<<currentTime<<" : ProcessGenerator: I received a signal"<<signum<<" from a child."<<endl;
    pid=wait(&stat_loc);
    cout<<"@clk "<<currentTime<<" : ProcessGenerator: pid="<<pid<<endl;
    if(!(stat_loc& 0x00FF)) cout<<"@clk "<<currentTime<<" : ProcessGenerator: The scheduler terminated with exit code "<< (stat_loc>>8) <<endl;
    else {cout<<"@clk "<<currentTime<<" : ProcessGenerator: I think Rana will go nuts."<<endl; }
    clearResources(1);
    return;
}

//This function is called upon exiting to clear the resources and exit
void clearResources(int)
{   currentTime= getClk();
    cout<<"@clk "<<currentTime<<" : ProcessGenerator: Clearing resources. Bye :)"<<endl;
    destroyClk(true);
    exit(1);
}

