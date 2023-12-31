/*
 * @Author: your name
 * @Date: 2021-09-01 16:21:31
 * @LastEditTime: 2021-09-03 18:21:53
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/data-mov/push-template.h
 */
#include "cpu/exec/template-start.h"

#define instr push

//为了保险起见我们开栈都以4字节为单位
static void do_execute(){
	op_src->val=op_src->val;
	reg_l(R_ESP)-=4;
	swaddr_write(reg_l(R_ESP),4,op_src->val,R_SS);
	print_asm_template1();
}

make_instr_helper(i)

#if DATA_BYTE==2 || DATA_BYTE==4
make_instr_helper(r)
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"
