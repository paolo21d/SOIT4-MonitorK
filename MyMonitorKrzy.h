#include <iostream>
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include "MonitorUnix.h"
using namespace std;
#define BUFSIZE 10
#define QUEUE_QUANTITY 3

class Queue {
    int buf[BUFSIZE];
    int head, tail, length;
    public:
    Queue(){
        head=0;
        tail=0;
        length=0;
        for (unsigned i=0; i<BUFSIZE; ++i)
            buf[i]=0;
    }
    void putToBuff(int val){
        buf[tail] = val;
        tail = (tail+1)%BUFSIZE;
        length++;
        printf("Wsadzam %d, size: %d\n", val, length);
        printfQueue();
        //cout<<"Wsadzam "<<val<<", size: "<<length<<endl;
    }
    int getFromBuf(){
        int ret = buf[head];
        head = (head+1)%BUFSIZE;
        length--;
        //printf("\tWyjmuje %d\n", ret);
        printfQueue();
        //cout<<"Wyjmuje "<<ret<<endl;
        return ret;
    }
    int size(){
        return length;
    }
    void printfQueue(/*int nr*/){
    //printf("Queue %d size %d: ", nr, length);
    printf("\tQueue size %d :", length);
    int index = head;
    for(int i=0; i<length; ++i, ++index)
        printf("%d ", buf[index%BUFSIZE]);

    printf("\n");
    }
};

class MyMonitor:public Monitor{
    Queue buffer[QUEUE_QUANTITY];
    Condition groupEmpty;
    Condition blockCon1, blockCon2, blockCon3, blockCon4, blockCon5;
    //int emptyQuantity;
    public:
    //MyMonitor(): emptyQuantity(BUFSIZE * QUEUE_QUANTITY){};

    void add(const int *tab){
        int putTo;
        enter();
        if(buffer[0].size()==BUFSIZE && buffer[1].size()==BUFSIZE && buffer[2].size()==BUFSIZE) //all buffers are full
            wait(groupEmpty);
        for(int i=0; i<QUEUE_QUANTITY; ++i){
            int id = tab[i];
            if(buffer[id].size()<BUFSIZE){
                //cout<<"Wstawiam do: "<<id<<endl;
                buffer[id].putToBuff(id);
                putTo = id;
                break;
            }
        }
        
        if((buffer[0].size()==1&&buffer[1].size()>=2&&buffer[2].size()>=3&&putTo==0) || (buffer[0].size()>=1&&buffer[1].size()==2&&buffer[2].size()>=3&&putTo==1) || (buffer[0].size()>=1&&buffer[1].size()>=2&&buffer[2].size()==3&&putTo==2)){
            //cout<<"*********SIGNAL Con1"<<endl;
            signal(blockCon1);
        }
        if((buffer[0].size()==3&&buffer[1].size()>=2&&buffer[2].size()>=1&&putTo==0) || (buffer[0].size()>=3&&buffer[1].size()==2&&buffer[2].size()>=1&&putTo==1) || (buffer[0].size()>=3&&buffer[1].size()>=2&&buffer[2].size()==1&&putTo==2)){
            //cout<<"*********SIGNAL Con2"<<endl;
            signal(blockCon2);
        }
        if(buffer[0].size()==1 && putTo==0) {
            signal(blockCon3);
        }
        if(buffer[1].size()==1 && putTo==1){
            signal(blockCon4);
        }
        if(buffer[2].size()==1 && putTo==2){
            signal(blockCon5);
        }
        
        leave();
    }
    void remove(int consumerId){
        if(consumerId == 0){
            enter();
            if(buffer[0].size()<1 || buffer[1].size()<2 || buffer[2].size()<3) //cannot getFromBuffer
                wait(blockCon1);
            cout<<"Konsument 1 wyjmuje: 1A, 2B, 3C"<<endl;
            for(int i=0; i<1; ++i) buffer[0].getFromBuf();
            for(int i=0; i<2; ++i) buffer[1].getFromBuf();
            for(int i=0; i<3; ++i) buffer[2].getFromBuf();
            if(buffer[0].size()==BUFSIZE-1 && buffer[1].size()==BUFSIZE-2 && buffer[2].size()==BUFSIZE-3)
                signal(groupEmpty);
            leave();
        }else if(consumerId==1){
            enter();
            if(buffer[0].size()<3 || buffer[1].size()<2 || buffer[2].size()<1)
                wait(blockCon2);
            cout<<"Konsument 2 wyjmuje: 3A, 2B, 1C"<<endl;
            for(int i=0; i<3; ++i) buffer[0].getFromBuf();
            for(int i=0; i<2; ++i) buffer[1].getFromBuf();
            for(int i=0; i<1; ++i) buffer[2].getFromBuf();
            if(buffer[0].size()==BUFSIZE-3 && buffer[1].size()==BUFSIZE-2 && buffer[2].size()==BUFSIZE-1)
                signal(groupEmpty);
            leave();
        }else if(consumerId==2){
            enter();
            if(buffer[0].size()<1)
                wait(blockCon3);
            cout<<"Konsument 3 wyjmuje: 1A, 0B, 0C"<<endl;
            buffer[0].getFromBuf();
            if(buffer[0].size()==BUFSIZE-1)
                signal(groupEmpty);
            leave();
        }else if(consumerId==3){
            enter();
            if(buffer[1].size()<1)
                wait(blockCon4);
            cout<<"Konsument 4 wyjmuje: 0A, 1B, 0C"<<endl;
            buffer[1].getFromBuf();
            if(buffer[1].size()==BUFSIZE-1)
                signal(groupEmpty);
            leave();
        }else if(consumerId==4){
            enter();
            if(buffer[2].size()<1)
                wait(blockCon5);
            cout<<"Konsument 5 wyjmuje: 0A, 0B, 1C"<<endl;
            buffer[2].getFromBuf();
            if(buffer[2].size()==BUFSIZE-1)
                signal(groupEmpty);
            leave();
        }
    }
};