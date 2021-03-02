/*
 * rwfm.c - kernel module for RTFM model
 *
 * Rewritten by Saurav Yadav <sauravyad@protonmail.com> Mar 2021
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "rwfm.h"

static int syscall_replace(void) {
	
}

static int __init rwfm_init(void)
{
        pr_info("Initializing system call interceptor\n");
        return 0;
}

static void __exit rwfm_exit(void)
{
        pr_info("Removing system call interceptor\n");
}

module_init(rwfm_init);
module_exit(rwfm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Saurav Yadav");
MODULE_DESCRIPTION("A system call interceptor module for RWFM model");
MODULE_VERSION("0.1");