/*
 * @Author: your name
 * @Date: 2021-09-01 19:04:15
 * @LastEditTime: 2021-09-03 18:22:56
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/data-mov/pop-template.h
 */
#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute(){
	//一字节的情况我确实不懂，先空着，毕竟pop也涉及不到一字节
	OPERAND_W(op_src,MEM_R(reg_l(R_ESP)));
	//REG(R_ESP)是栈顶地址，MEM_R后将栈顶的内容取出来
	swaddr_write(reg_l(R_ESP),4,0);
	reg_l(R_ESP)+=4;
	print_asm_template1();
}

#if DATA_BYTE == 2||DATA_BYTE==4
make_instr_helper(r)
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"