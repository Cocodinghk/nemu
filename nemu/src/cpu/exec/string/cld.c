#include "cpu/exec/helper.h"

make_helper(cld)
{
    cpu.eflags.DF=0;
    print_asm("cld\n");
    return 1;
}