/*
 * @Author: your name
 * @Date: 2021-09-01 19:03:55
 * @LastEditTime: 2021-09-03 16:10:19
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/data-mov/pop.c
 */
#include "cpu/exec/helper.h"

#define DATA_BYTE 2
#include "pop-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "pop-template.h"
#undef DATA_BYTE

make_helper_v(pop_r)
make_helper_v(pop_rm)
