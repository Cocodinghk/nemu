#include "FLOAT.h"
#include <string.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	//nemu_assert(0);
	long long res=(long long)a*(long long)b;
	return (FLOAT)(res>>16);//res>>16外面要加括号啊！！！
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	/* Dividing two 64-bit integers needs the support of another library
	 * `libgcc', other than newlib. It is a dirty work to port `libgcc'
	 * to NEMU. In fact, it is unnecessary to perform a "64/64" division
	 * here. A "64/32" division is enough.
	 *
	 * To perform a "64/32" division, you can use the x86 instruction
	 * `div' or `idiv' by inline assembly. We provide a template for you
	 * to prevent you from uncessary details.
	 *
	 *     asm volatile ("??? %2" : "=a"(???), "=d"(???) : "r"(???), "a"(???), "d"(???));
	 *
	 * If you want to use the template above, you should fill the "???"
	 * correctly. For more information, please read the i386 manual for
	 * division instructions, and search the Internet about "inline assembly".
	 * It is OK not to use the template above, but you should figure
	 * out another way to perform the division.
	 */


	/*计算机计算除法的过程与人类计算的过程很类似，只是选择范围变成了0或1.
	以51/3为例说明（51：110011；3:11）

	从第一位开始为1，小于11，结果位置0；余数为1
	从第二位开始，余数*2+1=11，等于11，结果位置1，余数为0；
	从第三、四位开始，余数*2+0=0<011,结果位置0，余数为0
	从第5位开始，余数*2+1=1<11，结果置0，余数为1
	从第6位开始，余数*2+1=11=11，结果置1，余数为0.
	此时将结果位相连，恰好是10001（17）
*/
	/*对于此处FLOAT类型的除法计算，依然是遵循上面的原则的
	对于FLOAT的整数部分，暂时以a/b表示（整数除法会截断小数部分），之后要进行右移
	对于FLOAT的小数部分，就可以仿照上面的过程了
	每除一次之前先右移一位，其实就相当于把小数部分先右移16位当成整数，再与b除
	最后结果就是整数结果右移16位加上小数按上面的算法得到的结果*/

	//nemu_assert(0);
	//都转化成整数来算，最后乘上符号
	int fu=1;
	if(a<0)
	{
		a=-a;
		fu*=-1;
	}
	if(b<0)
	{
		b=-b;
		fu*=-1;
	}
	FLOAT ans=a/b;
	int i;
	a=a%b;//余数部分用来算小数
	for(i=1;i<=16;i++)
	{
		a<<=1;
		ans<<=1;
		if(a>=b)
		{
			a-=b;
			ans++;
		}
	}
	return ans*fu;
}

FLOAT f2F(float a) {
	/* You should figure out how to convert `a' into FLOAT without
	 * introducing x87 floating point instructions. Else you can
	 * not run this code in NEMU before implementing x87 floating
	 * point instructions, which is contrary to our expectation.
	 *
	 * Hint: The bit representation of `a' is already on the
	 * stack. How do you retrieve it to another variable without
	 * performing arithmetic operations on it directly?
	 */

	//nemu_assert(0);

	// float : 1 sign + 8 exp + 23 frac
	// 值：	规格化：					(-1)^s * 2^(exp - 127) * (1.frac) = (-1)^s * 2^(exp - 150) * (1frac)
	// 	   非规格化：	exp == 0		(-1)^s * 2^(1-127) * 0.frac = (-1)^s * 2^(1-150) * frac

	//把a的地址强制转化为一个int变量的地址，然后取出其中的值，即为一个int类型的值
	int bb[1];
	memcpy((void *)bb,(void *)&a,4);
	int inte=bb[0];
	int sign=inte>>31;//符号位
	int exp=(inte>>23)&0xff;//阶码位
	FLOAT frac=inte&0x7fffff;
	if(exp!=0) frac+=1<<23;
	else exp=1;
	exp-=150;//???
	if(exp+16>0) frac<<=(exp+16);
	else if(exp+16<0) frac>>=-(exp+16);
	if(sign) return -frac;
	else return frac;
}

//绝对值
FLOAT Fabs(FLOAT a) {
	//nemu_assert(0);
	if(a<0) return -a;
	else return a;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

