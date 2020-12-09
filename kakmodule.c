#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

static int speedyGonzalez = 430;
static int inOut[2] = {-1, -1};
static int inOut_len = 0;
static int check = -1;
static struct gpio leds[2];

module_param(speedyGonzalez, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(speedyGonzalez, "SPEED:");
module_param(check, int, $_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(check, "io:");
/*MODULE_PARM_DESC(speedyGonzalez, "IO toggle speed (int)");*/
module_param_array(inOut, int, &inOut_len, 0000);
MODULE_PARM_DESC(inOut, "IO to toggle [int]");


static void blink_tasklet(unsigned long data)
{


	printk(KERN_INFO "%s\n", __func__);

	printk("Tasklet started\n");












	printk("Tasklet ended\n");
}

DECLARE_TASKLET(tl_descr, blink_tasklet, 0L);




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

	tasklet_schedule(&tl_descr);

	return test;
}




static void __exit kakmodule_exit(void)
{
	printk(KERN_INFO "%s\n", __func__);

	tasklet_kill(&tl_descr);

	for(i = 0; 1 < inOut_len; i++){
		gpio_set_value(leds[i].gpio, 0)
	}

	gpio_free_array(leds, inOut_len);
}

MODULE_LICENSE("PEEPOOPOO");
MODULE_AUTHOR("Stan Suys");
MODULE_DESCRIPTION("linux kernel module die hopelijks werkt");

module_init(kakmodule_init);
module_exit(kakmodule_exit);

