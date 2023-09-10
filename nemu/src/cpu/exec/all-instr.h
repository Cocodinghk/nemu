/*
 * @Author: your name
 * @Date: 2021-09-02 19:54:18
 * @LastEditTime: 2021-09-03 18:40:00
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/all-instr.h
 */
#include "prefix/prefix.h"

#include "data-mov/mov.h"
#include "data-mov/xchg.h"
#include "data-mov/movext.h"
#include "data-mov/cltd.h"
#include "data-mov/push.h"
#include "data-mov/pop.h"
#include "data-mov/leave.h"

#include "arith/adc.h"
#include "arith/dec.h"
#include "arith/inc.h"
#include "arith/neg.h"
#include "arith/imul.h"
#include "arith/mul.h"
#include "arith/idiv.h"
#include "arith/div.h"
#include "arith/sbb.h"
#include "arith/sub.h"
#include "arith/add.h"

#include "control/jmp.h"
#include "control/je.h"
#include "control/ret.h"
#include "control/call.h"
#include "control/jbe.h"
#include "control/jne.h"
#include "control/jle.h"
#include "control/jg.h"
#include "control/jl.h"
#include "control/jge.h"
#include "control/ja.h"
#include "control/js.h"
#include "control/jns.h"

#include "logic/and.h"
#include "logic/or.h"
#include "logic/not.h"
#include "logic/xor.h"
#include "logic/sar.h"
#include "logic/shl.h"
#include "logic/shr.h"
#include "logic/shrd.h"
#include "logic/test.h"
#include "logic/cmp.h"
#include "logic/setne.h"
#include "logic/sete.h"

#include "string/rep.h"
#include "string/scas.h"
#include "string/stos.h"
#include "string/movs.h"
#include "string/lods.h"

#include "misc/misc.h"

#include "special/special.h"

