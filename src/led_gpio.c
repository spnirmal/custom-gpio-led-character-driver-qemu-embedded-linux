#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/printk.h>

#define DEVICE_NAME "led"
#define CLASS_NAME "ledctl"

static int major_number;
static struct class* led_class = NULL;
static struct device* led_device = NULL;

static ssize_t led_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    char value;
    
    if (copy_from_user(&value, buf, 1))
        return -EFAULT;
    
    if (value == '1')
        printk(KERN_INFO "LED: ON\n");
    else if (value == '0')
        printk(KERN_INFO "LED: OFF\n");
    else
        printk(KERN_WARNING "LED: Invalid command %c\n", value);
    
    return 1;
}

static struct file_operations fops = {
    .write = led_write,
};

static int __init led_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "LED: Failed to register device\n");
        return major_number;
    }
    
    led_class = class_create(CLASS_NAME);
    if (IS_ERR(led_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "LED: Failed to create class\n");
        return PTR_ERR(led_class);
    }
    
    led_device = device_create(led_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(led_device)) {
        class_destroy(led_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "LED: Failed to create device\n");
        return PTR_ERR(led_device);
    }
    
    printk(KERN_INFO "LED: Device initialized with major %d\n", major_number);
    return 0;
}

static void __exit led_exit(void)
{
    device_destroy(led_class, MKDEV(major_number, 0));
    class_destroy(led_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "LED: Device removed\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple GPIO LED Driver");
