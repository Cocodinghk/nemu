/*
 * @Author: your name
 * @Date: 2021-09-02 19:01:44
 * @LastEditTime: 2021-09-03 16:27:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/control/ret-template.h
 */
#include "cpu/exec/template-start.h"

#define instr ret

/*here DATA_BYTE and SUFFIX means how many bytes I need to get for eip from
the top of the stack, not the byte of operand.
The operand has either no byte or 2 bytes, it is unchangable.*/
make_helper(concat(ret_n_,SUFFIX))
{
	cpu.eip=MEM_R(reg_l(R_ESP));
	if(DATA_BYTE==2)
		cpu.eip&=0xffff;
	reg_l(R_ESP)+=DATA_BYTE;

	print_asm("ret");
	return 1;
}

make_helper(concat(ret_i_,SUFFIX))
{
	int val=instr_fetch(cpu.eip+1,2);
	cpu.eip=MEM_R(reg_l(R_ESP));
	if(DATA_BYTE==2)
		cpu.eip&=0xffff;
	reg_l(R_ESP)+=DATA_BYTE;
	int i;
	for(i=0;i<val;i+=DATA_BYTE)
		MEM_W(REG(R_ESP)+i,0);
	REG(R_ESP)+=val;
	print_asm("ret $0x%x",val);
	return 1;
}

#include "cpu/exec/template-end.h"
