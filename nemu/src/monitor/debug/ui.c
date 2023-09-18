#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "memory/cache.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static int cmd_bt(char *args);

static int cmd_cache();

static int cmd_page(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "The program stops after executing N instructions in a single step. When N is not given, it defaults to 1.", cmd_si },
	{ "info", "Input 'r' to print registers, 'w' to print watchpoints.", cmd_info },
	{ "x", "Find the EXPR value of the expression. Use it as the starting memory address. Output N consecutive 4 bytes in hexadecimal form.", cmd_x },
	{ "p", "Evaluate the result of the given expression.", cmd_p },
	{ "w", "Set watchpoint.", cmd_w },
	{ "d", "Delete watchpoint.", cmd_d },
	{ "bt", "Print stack.", cmd_bt},
	{ "p_cache", "print cache time", cmd_cache},
	{ "page", "打印对应物理地址", cmd_page}
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) /sizeof(cmd_table[0]))

static int cmd_si(char *args){
	int step;
	char *arg=strtok(NULL," ");
	if(arg==NULL) step=1;
	else sscanf(arg,"%d",&step);
	cpu_exec(step);
	return 0;	
}

static int cmd_info(char *args)
{
	char *arg=strtok(NULL," ");
	if(arg==NULL)
	{
		printf("Error, arguments are not complete!\n");
		return 0;
	}
	else if(strcmp(arg,"r")==0)
	{
		int i;	
		char regs[8][4]={"eax","ecx","edx","ebx","esp","ebp","esi","edi"};	
		for(i=0;i<8;i++)	
		{
			printf("%s\t 0x%08x\t %d\n",regs[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
		}
		printf("eip\t 0x%08x\t %d\n",cpu.eip,cpu.eip);	
	}
	else if(strcmp(arg,"w")==0)
	{
		info_w();
	}
	return 0;
}

static int cmd_x(char *args)
{
	int step,i;
	int addr;
	char *arg1=strtok(NULL," ");
	char *arg2=args+strlen(args)+1;
	if(arg1==NULL||arg2==NULL)
	{
		printf("Error, arguments are not complete!\n");
		return 0;
	}
	sscanf(arg1,"%d",&step);
	bool ok=true;
	addr = expr(arg2,&ok);
	if(!ok)
	{
		printf("Error, the given address is wrong!\n");
		return 0;
	}
	for(i=0;i<step;i++){
		if(i%4==0)
			printf("0x%08x: ",addr+i*4);
		printf("0x%08x  ", swaddr_read(addr+4*i,4,R_DS));
		if((i+1)%4==0||i==step-1)
			printf("\n");
	}
	return 0;
}

static int cmd_p(char *args)
{
	if(args==NULL)
	{
		printf("Error, arguments are not complete!\n");
		return 0;
	}
	bool success_expr;
	int res;
	res=expr(args,&success_expr);
	if(success_expr==true)
	{
		printf("0x%08x(%d)\n",res,res);
		return 0;
	}
	else
	{
		printf("Error, wrong expression!\n");
		return 0;
	}
}

static int cmd_w(char *args)
{
	if(args==NULL)
	{
		printf("Error, arguments are not complete!\n");
		return 0;
	}
	WP* tmp=NULL;
	tmp=new_wp(args);
	if(tmp==0)
	{
		printf("Error, wrong expression!\n");
		return 0;
	}
	if(tmp==NULL)
	{
		printf("Error, no more watchpoints can be set!\n");
		return 0;
	}
	printf("Watchpoint set successfully!\n");
	return 0;
}

static int cmd_d(char *args)
{
	char *arg=strtok(NULL," ");
	if(arg==NULL)
	{
		printf("Error, arguments are not complete!\n");
		return 0;
	}
	int n;
	sscanf(arg,"%d",&n);
	WP* the_one=find_delete(n);
	if(the_one!=NULL)
		free_wp(the_one);
	return 0;
}	

void get_func_name(swaddr_t* ret_addr, char* name);

typedef struct{
	swaddr_t prev_ebp;
	swaddr_t ret_addr;
	uint32_t args[4];
}PartOfStackFrame;

//打印地址、函数名、以及前4个参数
//其中地址指当前栈帧中的返回地址，
/*
这是一个栈帧的结构（向下长）：
	$ebp的旧值 <-- EBP (某个时刻的esp永远指向比它旧一级的那个旧值的地址)
	非静态局部变量
	参数
	返回地址 <-- ESP(当我要在自己里调用另一个函数时，我才会在栈帧的最后放上返回地址，返回的是我自己中的位置)
*/

/*
这里还要注意的一点就是：
如果断点恰好设在了函数开始的前两句或者函数最后的leave/ret指令处，那么栈帧链输出的结果是不对的
因为栈帧链的输出是以当前ebp寄存器中的值为表头的，
由于在前两句指令中函数还没来得及把esp的值赋给ebp，在leave指令中ebp的值已经被修改为esp的值了
所以如果断点设在这两个地方时，当前ebp中的值是不准确的（少一层）
因此在输出时会少输出最下层的那个函数的栈帧

比如在add的例子中，本来栈帧应该先输出add，再输出main
但在以上两个位置设了断点后，栈帧就只会输出add了，且参数和返回值也会有问题
（如果输出的是最上层的函数，那么返回值不会有错，因为我们一开始直接给它赋了eip的值
其余情况，由于参数和返回值的读取都依赖ebp的值，因此都会有错）
*/
static int cmd_bt(char *args)
{
	char func_name[32];
	int i;
	bool first=true;
	//这个函数是在断点触发后调用的，断点一定是设定在某个函数中的
	//now指代当前这个设了断点的函数的下一个函数栈帧，它指向的就是程序停止前的最后一个函数
	//但第一次的ret_addr只能设成eip了，因为设了断点的函数可能没有调用下一个函数(或还没来得及)，那么就不知道它的返回地址是什么
	PartOfStackFrame now;
	now.prev_ebp=reg_l(R_EBP);//这是设了断点的函数的栈底位置
	now.ret_addr=cpu.eip;
	//栈底位置一定不会是0，只有第一个调用的函数的栈底位置中存的值会是0
	while(now.prev_ebp!=0)
	{
		for(i=0;i<32;i++)
			func_name[i]='\0';
		get_func_name(&(now.ret_addr),func_name);
		if(func_name[0]=='\0')
		{
			printf("Error! No function!\n");
			break;
		}
		else{
			if(first)
			{
				printf("0x%08x in function %s()\n",now.ret_addr,func_name);
				first=false;
			}
			else
			{
				printf("0x%08x in function %s()\n",now.ret_addr+1,func_name);
				//这块注意，对于所有callee函数，返回地址要再加1
				//原因是eip在的cpu_exec()函数执行完后还会加上ret指令返回的长度1
				//那个时候的eip才是正确的，所以在这里要给它加上1
			}
			printf("The first four arguments are: 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",swaddr_read(now.prev_ebp+8,4,R_SS),swaddr_read(now.prev_ebp+12,4,R_SS),swaddr_read(now.prev_ebp+16,4,R_SS),swaddr_read(now.prev_ebp+20,4,R_SS));
			//下面这两行的顺序千万别写反了！！
			now.ret_addr=swaddr_read(now.prev_ebp+4,4,R_SS);
			now.prev_ebp=swaddr_read(now.prev_ebp,4,R_SS);
		}
	}
	return 0;
}

static int cmd_cache()
{
	print_time();
	return 1;
}

hwaddr_t cmd_page_translate(lnaddr_t addr);
static int cmd_page(char* args){
	if(args == NULL) { printf("parameter invalid!\n"); return 0; }
	uint32_t addr;
	sscanf(args, "%x", &addr);
	hwaddr_t ans = cmd_page_translate(addr);
	if(ans) printf("Addr is 0x%08x\n",ans);
	return 0;
}

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
