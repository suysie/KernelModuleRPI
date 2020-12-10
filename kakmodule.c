#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/timer.h>
#include <linux/sched.h>

static int speedyGonzalez = 430;
static int inOut[2] = {-1, -1};
static int inOut_len = 0;
static int check = -1;
static struct gpio leds[2];
static struct timer_list blink_timer;

module_param(speedyGonzalez, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(speedyGonzalez, "SPEED:");
module_param(check, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(check, "io:");
module_param_array(inOut, int, &inOut_len, 0000);
MODULE_PARM_DESC(inOut, "IO to toggle [int]");


static void blink_tasklet(struct timer_list* t)
{
	int i;

	printk(KERN_INFO "%s\n", __func__);

	
	for (i = 0; i < inOut_len; i++){
		gpio_set_value(leds[i].gpio, 0);
		mdelay(500);
		gpio_set_value(leds[i].gpio, 1);
		mdelay(500);
	}
	printk("blink startn");

	printk(KERN_INFO "%s\n", __func__);
	/* schedule next execution */
	//blink_timer.data = !data;						// makes the LED toggle 
	blink_timer.expires = jiffies + (1*HZ); 		// 1 sec.
	add_timer(&blink_timer);
	printk("blink end\n");
}

/*DECLARE_TASKLET(tl_descr, blink_tasklet, 0L);*/




static int __init kakmodule_init(void)
{
	int i;
	int test = 0;

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

	return test;
}


static void __exit kakmodule_exit(void)
{
	int i;

	printk(KERN_INFO "%s\n", __func__);

	/*tasklet_kill(&tl_descr);*/
	// deactivate timer if running
	del_timer_sync(&blink_timer);

	for(i = 0; 1 < inOut_len; i++){
		gpio_set_value(leds[i].gpio, 0);
	}

	gpio_free_array(leds, inOut_len);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stan Suys");
MODULE_DESCRIPTION("linux kernel module die hopelijks werkt");

module_init(kakmodule_init);
module_exit(kakmodule_exit);

