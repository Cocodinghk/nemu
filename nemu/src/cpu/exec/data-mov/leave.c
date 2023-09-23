/*
 * @Author: your name
 * @Date: 2021-09-03 10:41:16
 * @LastEditTime: 2021-09-03 13:23:15
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/data-mov/leave.c
 */
#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "leave-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "leave-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "leave-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(leave_r)