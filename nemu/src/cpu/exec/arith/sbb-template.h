#include "cpu/exec/template-start.h"

#define instr sbb

static void do_execute () {
	DATA_TYPE result = op_dest->val - (op_src->val + cpu.eflags.CF);
	OPERAND_W(op_dest, result);

	/* TODO: Update EFLAGS. */
	update_eflags_pf_zf_sf((DATA_TYPE_S)result);
	cpu.eflags.CF = result < op_dest->val;
	cpu.eflags.OF = MSB(~(op_dest->val ^ op_src->val) & (op_dest->val ^ result));
	if(((op_dest->val&0xf)-((op_src->val&0xf)+cpu.eflags.CF))>>4)
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
