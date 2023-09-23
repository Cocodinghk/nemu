/*
 * @Author: your name
 * @Date: 2021-09-03 13:39:09
 * @LastEditTime: 2021-09-03 13:48:25
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /NEMU2021/nemu/src/cpu/exec/arith/add.h
 */
#ifndef __ADD_H__
#define __ADD_H__

make_helper(add_i2a_b);
make_helper(add_i2rm_b);
make_helper(add_si2rm_b);
make_helper(add_r2rm_b);
make_helper(add_rm2r_b);

make_helper(add_i2a_v);
make_helper(add_i2rm_v);
make_helper(add_si2rm_v);
make_helper(add_r2rm_v);
make_helper(add_rm2r_v);

#endif
