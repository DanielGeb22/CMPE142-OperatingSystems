#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hash.h>
#include <linux/gcd.h>
#include <asm/param.h>
#include <linux/jiffies.h>

/* This function is called when the module is loaded. */
static int simple_init(void)
{
    printk(KERN_INFO "Loading Kernel Module\n");
    printk(KERN_INFO "%lu\n", GOLDEN_RATIO_PRIME);
    printk(KERN_INFO "The values of jiffies and HZ are: %lu, %d\n", jiffies, HZ);

    printk(KERN_DEBUG "Flushing printk buffer...\n");
    
    return 0;
}

/* This function is called when the module is removed. */
static void simple_exit(void)
{
    unsigned long a = 3300, b = 24;
    unsigned long result = gcd(a, b);

    printk(KERN_INFO "Removing Kernel Module\n");
    printk(KERN_INFO "GCD of %lu and %lu is %lu\n", a, b, result);
    printk(KERN_INFO "The value of jiffies is now: %lu", jiffies);

    printk(KERN_DEBUG "Flushing printk buffer...\n");
}

/* Macros for registering module entry and exit points. */
module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");