#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include "ioctlDeelB.h"


#define FIRST_MINOR 0
#define MINOR_CNT 1

static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
//static int speedyGonzalez = 1, edgeCcdount = 0;

static int speedyGonzalez = 430;
static int inOut[2] = {-1, -1};
static int inOut_len = 0;
static int check = -1;
static struct gpio leds[2];
static struct timer_list blink_timer;
static int Edge;
static int edgeCount = 0;

static struct gpio ledsCheck;

module_param(speedyGonzalez, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(speedyGonzalez, "SPEED:");
module_param(check, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(check, "io:");
module_param_array(inOut, int, &inOut_len, 0000);
MODULE_PARM_DESC(inOut, "IO to toggle [int]");

static int my_open(struct inode *i, struct file *f)
{
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int my_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif

{
    query_arg_t q;
 
    switch (cmd)
    {
        case QUERY_GET_VARIABLES:
            q.speed = speedyGonzalez;
            q.count = edgeCount;
            if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            break;
        case QUERY_CLR_VARIABLES:
            speedyGonzalez = 500;
            edgeCount = 0;
            break;
        case QUERY_SET_VARIABLES:
            if (copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            speedyGonzalez = q.speed;
            break;
        default:
            return -EINVAL;
    }
 
    return 0;
}

static struct file_operations query_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = my_ioctl
#else
    .unlocked_ioctl = my_ioctl
#endif
};

static irqreturn_t button_isr(int irq, void *data)
{
	
	edgeCount ++;
	printk(KERN_INFO "edges: %d\n",edgeCount);
	return IRQ_HANDLED;

}

static void blink_tasklet(struct timer_list* t)
{
	int i;

	//printk(KERN_INFO "%s\n", __func__);

	
	for (i = 0; i < inOut_len; i++){
		gpio_set_value(leds[i].gpio, 0);
		mdelay(speedyGonzalez);
		gpio_set_value(leds[i].gpio, 1);
		mdelay(speedyGonzalez);
	}

	//printk(KERN_INFO "%s\n", __func__);
	/* schedule next execution */
	//blink_timer.data = !data;						// makes the LED toggle 
	blink_timer.expires = jiffies + (1*HZ); 		// 1 sec.
	add_timer(&blink_timer);
}

/*DECLARE_TASKLET(tl_descr, blink_tasklet, 0L);*/




static int __init kakmodule_init(void)
{
	int i;
	int test = 0;
	struct device *dev_ret;

	printk(KERN_INFO "%s\n", __func__);
	printk(KERN_INFO "Speed: %d\n" , speedyGonzalez);
	printk(KERN_INFO "togglen de volgende in outs");

	for (i = 0; i < inOut_len; i++)
	{
		struct gpio led;
		char label[10];
		printk(KERN_INFO "inOut[%d] = %d\n", i, inOut[i]);
		led.gpio = inOut[i];
		led.flags = GPIOF_OUT_INIT_HIGH;
		sprintf(label,"led-%d", inOut[i]);
		led.label = label;
		leds[i] = led;
		if(check == inOut[i]){
			ledsCheck = led;
		}
	}

	test = gpio_request_array(leds, inOut_len);

	if (test) {
		printk(KERN_ERR "Unable to request GPIOs: %d\n", test);
		return test;
	}

	timer_setup(&blink_timer, blink_tasklet, 0);

	blink_timer.function = blink_tasklet;
	//blink_timer.data = 1L;							// initially turn LED on
	blink_timer.expires = jiffies + (1*HZ); 		// 1 sec.
	add_timer(&blink_timer);

	/*tasklet_schedule(&tl_descr);*/



	test = gpio_to_irq(ledsCheck.gpio);

	if(test < 0) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", test);
	}

	Edge = test;

	printk(KERN_INFO "Successfully requested led IRQ # %d\n", Edge);

	test = request_irq(Edge, button_isr, IRQF_TRIGGER_RISING /*| IRQF_DISABLED*/, "gpiomod#edgeLed", NULL);

	if(test) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", test);
	}

	if ((test = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "query_ioctl")) < 0)
    {
        return test;
    }
 
    cdev_init(&c_dev, &query_fops);
 
    if ((test = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        return test;
    }
     
    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "query")))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }


	return test;
}


static void __exit kakmodule_exit(void)
{
	int i;

	printk(KERN_INFO "%s\n", __func__);

	/*tasklet_kill(&tl_descr);*/
	// deactivate timer if running
	del_timer_sync(&blink_timer);

	free_irq(Edge, NULL);

	for(i = 0; 1 < inOut_len; i++){
		gpio_set_value(leds[i].gpio, 0);
	}
	device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);

	gpio_free_array(leds, inOut_len);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stan Suys");
MODULE_DESCRIPTION("linux kernel module die hopelijks werkt");

module_init(kakmodule_init);
module_exit(kakmodule_exit);

