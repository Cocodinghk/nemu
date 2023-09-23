/*
 * @Author: your name
 * @Date: 2021-09-03 10:40:58
 * @LastEditTime: 2021-09-03 13:06:34
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/data-mov/leave-template.h
 */
#include "cpu/exec/template-start.h"

#define instr leave

static void do_execute()
{
    //先把ESP置为EBP的值，即移动栈顶指针
    //所以要先把从ESP到EBP的所有栈的内容都清零
    //注意EBP自己不要清零，后面还要用呢
    swaddr_t i;
    for(i=cpu.esp;i<cpu.ebp;i+=DATA_BYTE)
        MEM_W(i,0,2);
    cpu.esp=cpu.ebp;
    REG(R_EBP)=swaddr_read(REG(R_ESP),DATA_BYTE,2);
    MEM_W(REG(R_ESP),0,2);
    reg_l(R_ESP)+=DATA_BYTE;
    print_asm("leave");
}

make_instr_helper(r)

#include "cpu/exec/template-end.h"
