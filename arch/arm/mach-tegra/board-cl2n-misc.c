#include <linux/init.h>
#include <linux/bootmem.h>
#include "board-cl2n.h"
#include <mach/board-cl2n-misc.h>

static int hw_ramcode;

static int __init cl2n_hw_ramcode(char *code)
{
	char *p = code;

	hw_ramcode = memparse(p, &p);
	printk("hw_ramcode is %d\n", hw_ramcode);
	return 1;
}

int cl2n_get_hw_ramcode(void)
{
	return hw_ramcode;
}

__setup("androidboot.hw_ramcode=", cl2n_hw_ramcode);


static int board_strap = 0;

static int __init cl2n_board_strap(char *code)
{
	char *p = code;

	board_strap = memparse(p, &p);
	printk("board_strap is %d\n", board_strap);
	return 1;
}

int cl2n_get_board_strap(void)
{
	return board_strap;
}
//EXPORT_SYMBOL(cl2n_get_board_strap);

__setup("androidboot.board_strap=", cl2n_board_strap);

