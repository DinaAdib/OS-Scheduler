//
// Created by rana-afifi on 10/21/17.
//
#include "headers.h"


int main(int argc, char* argv[]) {
    printf("\n Hey I am scheduler!");
    //initClk();
    // initialize ready queue


    struct processData p1(1,2,3,5);
    struct processData p2(3,3,2,7);
    struct processData p5(7,3,2,1);
    p1.criteria=5;
    p2.criteria=7;
    p5.criteria=1;
    priority_queue <struct processData> readyQ;
    readyQ.push(p1);
    readyQ.push(p2);
    readyQ.push(p5);
//for testing purposes
    struct processData p3=readyQ.top();

    printf(" AYWAAAAAAAAAAA %d",p3.criteria);
    readyQ.pop();
    p3=readyQ.top();
    printf(" AYWAAAAAAAAAAA %d",p3.criteria);

    //receive process data from PG
    // load data to a priority queue

    //upon termination release clock
    //destroyClk(true);

}
