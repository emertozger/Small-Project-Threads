#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#define LED_BLINK_TIME 500

#define STACKSIZE_THREAD1 512
#define STACKSIZE_THREAD2 512
#define PRIORITY_THREAD1 6
#define PRIORITY_THREAD2 6

#define LED0_NODE      DT_NODELABEL(led0)
#define LED1_NODE      DT_NODELABEL(led1)
#define BUTTON0_NODE   DT_NODELABEL(button0)

static const struct gpio_dt_spec led0_spec = GPIO_DT_SPEC_GET(LED0_NODE,gpios);
static const struct gpio_dt_spec led1_spec = GPIO_DT_SPEC_GET(LED1_NODE,gpios);
static const struct gpio_dt_spec button0_spec = GPIO_DT_SPEC_GET(BUTTON0_NODE,gpios);
static struct gpio_callback button0_cb;

K_SEM_DEFINE(instance_monitor_sem,0,1);

void button_pressed_callback(const struct device *gpiob, struct gpio_callback *cb, gpio_port_pins_t pins)
{
    printk("Pressed to the button\n");
	k_sem_give(&instance_monitor_sem);
}

void main(void)
{
gpio_pin_configure_dt(&led0_spec,GPIO_OUTPUT);
gpio_pin_configure_dt(&led1_spec,GPIO_OUTPUT);
gpio_pin_configure_dt(&button0_spec,GPIO_INPUT);

gpio_pin_interrupt_configure_dt(&button0_spec, GPIO_INT_EDGE_TO_ACTIVE);
gpio_init_callback(&button0_cb, button_pressed_callback, BIT(button0_spec.pin));
gpio_add_callback(button0_spec.port,&button0_cb);
}


void thread1()
{
		while(1)
	{
		printk("You are at thread 1\n");

		gpio_pin_set_dt(&led0_spec,0);
        k_msleep(LED_BLINK_TIME);
		gpio_pin_set_dt(&led0_spec,1);
        k_msleep(LED_BLINK_TIME);
	}
}

void thread2()
{

		while(1)
	{
		k_sem_take(&instance_monitor_sem,K_FOREVER);

		printk("You are at Thread 2\n");
		gpio_pin_set_dt(&led1_spec,0);
        k_msleep(LED_BLINK_TIME);
		gpio_pin_set_dt(&led1_spec,1);
        k_msleep(LED_BLINK_TIME);
	}
}

K_THREAD_DEFINE(thread1_id,STACKSIZE_THREAD1,thread1,NULL,NULL,NULL,PRIORITY_THREAD1,0,0);
K_THREAD_DEFINE(thread2_id,STACKSIZE_THREAD2,thread2,NULL,NULL,NULL,PRIORITY_THREAD2,0,0);
