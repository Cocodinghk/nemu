#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_,SUFFIX))
{
	int len=concat(decode_i_,SUFFIX)(eip+1);
	reg_l(R_ESP)-=DATA_BYTE;
	swaddr_write(reg_l(R_ESP),4,cpu.eip+len,R_SS);
	/*the third argument is not cpu.eip+len+1,though it should be, cuz that is the beginnig pos of the next command. But we need to consider ret. When ret, we need to return 1. So this length here plus 1 is the final pos. Also,you can let call return cpu.eip+len+1,let ret return 0.*/
	DATA_TYPE_S imm=op_src->val;//the displacement
	print_asm("call 0x%x",cpu.eip+1+len+imm);
	/*remember,we execute first, change eip next, so we are always one command ealier. when the eip point to call, we actually just have finished executing the command before call. But all the operation in function call is done to the commmand after call, so you need to add the length of command call to leap over it.*/
	cpu.eip+=imm;//here we don't need to leap, cuz the prog will add it for us.
	return len+1;
}
make_helper(concat(call_rm_,SUFFIX)){
	int len=concat(decode_rm_,SUFFIX) (eip+1);
	reg_l (R_ESP)-=DATA_BYTE;
	swaddr_write(reg_l (R_ESP),4,cpu.eip+len,R_SS);
	DATA_TYPE_S imm=op_src->val;
	print_asm("call 0x%x",imm);
	cpu.eip=imm-len-1;
	/*eip在call指令结束后应该指向imm，但记住在cpu_exec函数中我们执行完一条指令后会让eip加上指令长度
	eip应当是加完那个长度后再指向imm，因此考虑到call函数返回长度为len+1,所以这里我们让eip=imm-len-1*/
	return len+1;
}
#include "cpu/exec/template-end.h"
