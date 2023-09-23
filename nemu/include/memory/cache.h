#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

#define block_size 64
#define b_num 6
#define c1_s_num 7
#define c1_tag_num 19
#define c1_line_num 8
#define c1_set_num 128

#define c2_s_num 12
#define c2_tag_num 14
#define c2_line_num 16
#define c2_set_num 4096

typedef struct{
    uint32_t tag;//标记位占19位，但我们有的类型只有8位，16位和32位，所以只能用32位的表示它
    uint8_t block[block_size];//uint8_t代表一个字节，一个cache块总共64个字节
    bool valid;//有效位
    bool dirty;//脏位，给cache2用的
}block;

block c1_cache[c1_set_num][c1_line_num];//一级cache
block c2_cache[c2_set_num][c2_line_num];//二级cache
uint64_t t1;//cache1计时变量
uint64_t t2;//cache2计时变量

void init_cache();
void print_time();

int32_t c1_read(hwaddr_t addr);
void c1_write(hwaddr_t addr,size_t len,uint32_t data);
int32_t c2_read(hwaddr_t addr);
void c2_write(hwaddr_t addr,size_t len,uint32_t data);

#endif