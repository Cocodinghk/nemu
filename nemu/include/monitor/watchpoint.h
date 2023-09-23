#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	char exp[32];
	uint32_t res;

} WP;

WP* new_wp(char* e);
bool free_wp(WP *wp);
bool change();
void info_w();
WP* find_delete(int n);
int lll;

#endif
