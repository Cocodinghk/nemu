#include "memory/tlb.h"
#include "burst.h"
#include <time.h>
#include <stdlib.h>

void init_tlb()
{
    int i;
    for(i=0;i<TLB_SIZE;i++)
    tlb[i].valid=0;
    srand(clock());
}

int readTLB(lnaddr_t addr)
{
    int tag=addr>>12;
    int i;
    for(i=0;i<TLB_SIZE;i++)
    {
        if(tlb[i].tag==tag&&tlb[i].valid==1)
            return i;
    }
    return -1;//没在tlb里找到
}

void writeTLB(lnaddr_t addr,hwaddr_t addr_)
{
    int tag=addr>>12;
    int i;
    addr_>>=12;
    for(i=0;i<TLB_SIZE;i++)
    {
        if(tlb[i].valid==0)
        {
            tlb[i].tag=tag;
            tlb[i].data=addr_;
            tlb[i].valid=1;
            return;
        }
    }
    //置换
    i=rand()%TLB_SIZE;
    tlb[i].tag=tag;
    tlb[i].data=addr_;
    tlb[i].valid=1;
}