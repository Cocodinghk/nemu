#include "cpu/exec/template-start.h"

#define instr ret

/*here DATA_BYTE and SUFFIX means how many bytes I need to get for eip from
the top of the stack, not the byte of operand.
The operand has either no byte or 2 bytes, it is unchangable.*/
make_helper(concat(ret_n_,SUFFIX))
{
	cpu.eip=MEM_R(reg_l(R_ESP),2);//2表示SS，ret是与栈相关的
	if(DATA_BYTE==2)
		cpu.eip&=0xffff;
	reg_l(R_ESP)+=DATA_BYTE;

	print_asm("ret");
	return 1;
}

make_helper(concat(ret_i_,SUFFIX))
{
	int val=instr_fetch(eip+1,2);
	cpu.eip=MEM_R(reg_l(R_ESP),2);
	if(DATA_BYTE==2)
		cpu.eip&=0xffff;
	reg_l(R_ESP)+=DATA_BYTE;
	int i;
	for(i=0;i<val;i+=DATA_BYTE)
		MEM_W(REG(R_ESP)+i,0,2);
	REG(R_ESP)+=val;
	print_asm("ret $0x%x",val);
	return 1;
}

#include "cpu/exec/template-end.h"
