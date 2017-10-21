//
// Created by rana-afifi on 10/21/17.
//

#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <string>

using namespace std;


#define SHKEY 300

//Defining Structs

//Struct for process data in process table block
struct processBlock {
    int id;
    int arrivalTime;
    int runningTime;
    int priority;
    int remainingTime;
    int startTime;
    int finishTime;
    int waitTime; //derived from other attributes?
    string status;
}processBlock;

//Struct for process data
struct processData {
    int id;
    int arrivalTime;
    int runningTime;
    int priority;
    int criteria;

    processData(){}
    processData(int i , int a,int r , int p)
    {id=i; arrivalTime=a; runningTime=r; priority=p;}
    /* overload the less-than operator so priority queues know how to compare two processData objects */
    bool operator>(const processData& right) const
    {
        return criteria < right.criteria;
    }
    bool operator<(const processData& right) const
    {
        return criteria > right.criteria;
    }

    void print() {
        printf("\n Process Data: id %d \t arrivaltime %d runningtime %d priority %d", id, arrivalTime, runningTime,
               priority);
    }


}processData;

///==============================
//don't mess with this variable//
int* shmaddr;                  //
//===============================

int nProcesses=0;

int getClk()
{

    int clk=*shmaddr;
    return clk;
}


/* All process call this function at the begining to establish communication
between them and the clock module
Again, Remember that the clock is only emulation
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while((int)shmid == -1)
    {
        //Make sure that the Clock exists
        printf("wait, Clock not initialized yet\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }

    shmaddr = (int*) shmat(shmid, (void *)0, 0);
}


/* All process call this function at the end to release the  communication
resources between them and the clock module
Again, Remember that the clock is only emulation
input: terminateAll : is a flag to indicate whether that
this is the end of simulation it terminates all the system and release resources
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if(terminateAll)
        killpg(getpgrp(),SIGINT);
}
