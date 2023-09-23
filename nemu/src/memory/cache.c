#include "common.h"
#include "memory/cache.h"
#include <time.h>
#include <stdlib.h>
#include "burst.h"

void ddr_read3(hwaddr_t addr,void *data);
void ddr_write3(hwaddr_t addr,void *data,uint8_t *mask);

void init_cache()
{
    t1=t2=0;
    srand(clock());
    int i,j;
    for(i=0;i<c1_set_num;i++)
    {
        for(j=0;j<c1_line_num;j++)
        {
            c1_cache[i][j].valid=0;
        }
    }
    for(i=0;i<c2_set_num;i++)
    {
        for(j=0;j<c2_line_num;j++)
        {
            c2_cache[i][j].valid=c2_cache[i][j].dirty=0;
        }
    }
}

void print_time()
{
    printf("t1 = %lu\tt2 = %lu\n",t1,t2);
}

int32_t c1_read(hwaddr_t addr)
{
    int32_t tag1=(addr>>(c1_s_num+b_num));//取出对于cache1的tag
    int32_t set1=(addr>>b_num)&(c1_set_num-1);//取出对于cache1的组索引
    int i;
    for(i=0;i<c1_line_num;i++)
    {
        if(c1_cache[set1][i].valid==0) continue;
        if(c1_cache[set1][i].tag==tag1)//在cache1中hit
        {
            t1+=2;
            return i;//返回它是这组里的第几个
        }
    }
    //在cache1中miss
    for(i=0;i<c1_line_num;i++)
    {
        if(c1_cache[set1][i].valid==0)
            break;//找到空位了，就用这个位置存它
    }
    if(i==c1_line_num)//没找到空位，就随机选一个替换
        i=rand()%c1_line_num;
    //现在要往c1_cache[set1][i]里面复制块了
	c1_cache[set1][i].valid = 1;
	c1_cache[set1][i].tag = tag1;
    int set2=(addr>>b_num)&(c2_set_num-1);
    int j=c2_read(addr);//去cache2中找这个块的位置
    //把这个块复制到cache1中
    memcpy(c1_cache[set1][i].block,c2_cache[set2][j].block,block_size);
    t1+=200;
    return i;//现在它也在cache1中了，还是返回它的位置
}

void c1_write(hwaddr_t addr,size_t len,uint32_t data)
{
    int32_t tag1=(addr>>(c1_s_num+b_num));
    int32_t set1=(addr>>b_num)&(c1_set_num-1);
    int32_t start1=(addr&(block_size-1));//这里是把块内偏移取出来了，如果块内偏移加上数组长度大于64了，说明这个数据是跨了两个块的，要特殊处理
	int hit = 0;
	int i;
    for(i=0;i<c1_line_num;i++)
    {
        if(c1_cache[set1][i].valid==0) continue;
        else if(c1_cache[set1][i].tag==tag1)
        {
            hit=1;//写命中
            break;
        }
    }
    //写命中：直写，cache1和cache2都要改
    if(hit)
    {
        if(start1+len<=block_size)//不跨越块
        {
            memcpy(c1_cache[set1][i].block+start1,&data,len);//先改cache1
        }
        else//跨越块了
		{
			//先写能写下的前半部分
			memcpy(c1_cache[set1][i].block + start1, &data, block_size - start1);
			//后半部分另找一个块写，步骤和之前一样
            //等到读数据的时候也要跨块读，会在hwaddr_read里处理
			c1_write(addr + block_size - start1, len - (block_size - start1), data >>(8 * (block_size - start1)));
		}
        t1+=2;
        c2_write(addr,len,data);//再改cache2,t2在这里面改
    }
    //写未命中：非写分配，cache1不改，改下一层
	else
	{
		t1 += 200;
		c2_write(addr, len, data);
	}
}

int32_t c2_read(hwaddr_t addr)
{
    int32_t tag2=(addr>>(c2_s_num+b_num));
    int32_t set2=(addr>>b_num)&(c2_set_num-1);
    int i;
    for(i=0;i<c2_line_num;i++)
    {
        if(c2_cache[set2][i].valid==0) continue;
        if(c2_cache[set2][i].tag==tag2)
        {
            t2+=2;
            return i;//hit
        }
    }
    //miss
    for(i=0;i<c2_line_num;i++)
    {
        if(c2_cache[set2][i].valid==0)
            break;
    }
    if(i==c2_line_num)
        i=rand()%c2_line_num;
    //注意c2_cache替换后需要写回到内存
    if(c2_cache[set2][i].valid==1&&c2_cache[set2][i].dirty==1)//之前改动过，需要写回
    {
        uint8_t mask[BURST_LEN*2];
        memset(mask,1,sizeof(mask));
        int j;
        //uint32_t addr_pre=(addr>>b_num)<<b_num;
        //不是参数里的地址，是被替换的块在内存里的地址啊！！！
        uint32_t addr_pre=((c2_cache[set2][i].tag<<(c2_s_num+b_num))|(set2<<b_num));
        for(j=0;j<block_size/BURST_LEN;j++)
        {
            //从cache2写到内存中
            ddr_write3(addr_pre+BURST_LEN*j,c2_cache[set2][i].block+BURST_LEN*j,mask);
        }
    }
    c2_cache[set2][i].valid=1;
    c2_cache[set2][i].dirty=0;
    c2_cache[set2][i].tag=tag2;
    //从内存里把对应的块复制到cache2里
    int j;
    uint32_t addr_pre=((addr>>b_num)<<b_num);
    for(j=0;j<block_size/BURST_LEN;j++)
    {
        //从内存写到cache2
        ddr_read3(addr_pre+BURST_LEN*j,c2_cache[set2][i].block+BURST_LEN*j);
    }
    t2+=200;
    return i;
}

void c2_write(hwaddr_t addr,size_t len,uint32_t data)
{
    int32_t tag2=(addr>>(c2_s_num+b_num));
    int32_t set2=(addr>>b_num)&(c2_set_num-1);
    int32_t start2=(addr&(block_size-1));
    bool hit=0;
    int i;
    for(i=0;i<c2_line_num;i++)
    {
        if(c2_cache[set2][i].valid==0) continue;
        if(c2_cache[set2][i].tag==tag2)
        {
            hit=1;
            break;
        }
    }
    //命中，回写：只改cache2，不改内存，置dirty
    if(hit)
    {
        c2_cache[set2][i].dirty=1;
        if(start2+len<=block_size)
        {
            memcpy(c2_cache[set2][i].block+start2,&data,len);
        }
        else
        {
            memcpy(c2_cache[set2][i].block+start2,&data,block_size-start2);
            c2_write(addr+block_size-start2,len-(block_size-start2),data>>((block_size-start2)*8));
        }
        t2+=2;
    }
    //未命中，先把内存中对应的块移到cache2里，再按写命中处理
    else
    {
        i=c2_read(addr);//一定未命中，read就已经包括把内存中的块移到cache2里的过程了
        c2_write(addr,len,data);//这回一定命中，就按写命中接着处理
    }
}