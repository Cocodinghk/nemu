#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "cpu/reg.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

//就是新建一个监视点的意思，包括取一个空闲节点并给该节点赋值
WP* new_wp(char *e)
{
	bool success;
	int ans;
	ans = expr(e, &success);
	if (success == false)
	{
		return 0;	
	}
	if (free_ == NULL)
		assert(0);
	WP *p;
	p = free_;
	free_ = free_->next;//取出空闲节点p
	strcpy(p->exp,e);
	p->res = ans;//给空闲节点赋值

	//把空闲节点连接到已用节点链表上
	if (head == NULL)
	{
		head = p;
		p->next = NULL;
	}
	else
	{
		p->next = head->next;
		head->next = p;
	}
	return p;
}

bool free_wp(WP* wp)
{
	if (wp == NULL)//没有wp
	{
		return 0;
	}
	if (head == wp)//要清除的是头节点
	{
		head = head->next;
		wp->next = free_->next;
		free_->next = wp;
	}
	else//要清除的不是头节点
	{
		WP* p = head;
		while (p != NULL)
		{
			if (p->next == wp)
				break;
			else
				p = p->next;
		}
		if (p == NULL)//没有找到wp
		{
			return 0;
		}
		//找到wp，p指向wp的前一个节点
		//现在要从已用链表中清除wp，并把它连接到空闲链表里面
		p->next = wp->next;
		wp->next = free_->next;
		if (free_ == NULL)//要注意空闲链表为空的情况
			free_ = wp;
		else
			free_->next = wp;
	}
	return 1;
}

bool change()
{
	bool changed=0;
	WP* p = head;
	while (p != NULL)
	{
		bool success_wp;
		uint32_t new_val = expr(p->exp, &success_wp);
		assert(success_wp);
		if (new_val != p->res)
		{
			changed =1;
			printf("Hint watchpoint %d at address 0x%08x, expr = %s\n", p->NO, cpu.eip - lll, p->exp);
			printf("old value = 0x%08x\n", p->res);
			printf("new value = 0x%08x\n", new_val);
		}
		p = p->next;
	}
	return changed;
}

void info_w()
{
	WP* p=head;
	if(p==NULL) printf("No watchpoint is used now!\n");
	while(p!=NULL)
	{
		printf("%d   %s   0x%08x\n",p->NO,p->exp,p->res);
		p=p->next;
	}
}

WP* find_delete(int n)
{
	if(n<0||n>=NR_WP)
	{
		printf("Error, this watchpoint doesn't exist!\n");
		return NULL;
	}	
	WP* p=head;
	while(p!=NULL)
	{
		if(p->NO==n)
			break;
		p=p->next;
	}
	if(p==NULL)
	{
		printf("Error, this watchpoint is not used!\n");
		return p;
	}
	printf("This watchpoint is deleted successfully!\n");
	return p;
}
