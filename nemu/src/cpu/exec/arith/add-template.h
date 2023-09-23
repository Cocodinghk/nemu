/*
 * @Author: your name
 * @Date: 2021-09-03 13:39:03
 * @LastEditTime: 2021-09-03 15:56:27
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/arith/add-template.h
 */
#include "cpu/exec/template-start.h"

#define instr add

static void do_execute () {
	DATA_TYPE result = op_dest->val + op_src->val;
	OPERAND_W(op_dest, result);

	update_eflags_pf_zf_sf((DATA_TYPE_S)result);
	cpu.eflags.CF = result < op_dest->val;//这里是小于号，与sub是反的，因为sub会负溢出，而add会正溢出，这里不改会出错的！
	cpu.eflags.OF = MSB((op_dest->val ^ op_src->val) & (op_dest->val ^ result));
	if(((op_dest->val&0xf)+(op_src->val&0xf))>>4)
		cpu.eflags.AF=1;
	else
		cpu.eflags.AF=0;
	print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"