#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/decompress/mm.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");  //声明本驱动遵循GPL协议
#define BUTTON_MINOR 100

#define GPNCON 0x7f008830
#define GPNDAT 0x7f008834
unsigned int *gpio_data;
unsigned int key_num = 0;

struct work_struct *work; //定义工作

struct timer_list  timer;//定义定时器变量

wait_queue_head_t  button_queue;

/**
 *  Interrupt handler
 */
static irqreturn_t button_interrupt(int irq, void *dev_id)
{
    schedule_work(work);
         
    return IRQ_HANDLED;
}


int button_irq_init(void)
{
    unsigned int *gpio_config;
    unsigned short data;
    	
    gpio_config = ioremap(GPNCON,4);
    data = readw(gpio_config);
    data &= ~0b111111;
    data |= 0b111111;

    writew(data,gpio_config);
    
    gpio_data = ioremap(GPNDAT,4);
     
    request_irq(IRQ_EINT(0), button_interrupt,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"REWIND",(void *)2); //快退 s2
    request_irq(IRQ_EINT(1), button_interrupt,IRQF_TRIGGER_FALLING,"PREV",(void *)3);   //上一首 s3
    request_irq(IRQ_EINT(2), button_interrupt,IRQF_TRIGGER_FALLING,"PLAY&STOP",(void *)4); //播放/停止 s4
    request_irq(IRQ_EINT(3), button_interrupt,IRQF_TRIGGER_FALLING,"NEXT",(void *)5);   //下一首 s5
    request_irq(IRQ_EINT(4), button_interrupt,IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING,"FASTFW",(void *)6); //快进 s6
    request_irq(IRQ_EINT(5), button_interrupt,IRQF_TRIGGER_FALLING,"STOP",(void *)7);  //停止 s7
    
    return 0;
}


int button_open(struct inode *node, struct file *file)
{
   return 0; 
}

/**/
ssize_t button_read(struct file *file, char __user *buf, size_t size, loff_t *pos)
{
    wait_event(button_queue,key_num);
    copy_to_user(buf, &key_num, 4);	
    key_num = 0;
    return 0;
}

/*File operations struct for character device */
static const struct file_operations btn_fops = {
	.owner	= THIS_MODULE,
	.open	= button_open,
	.read   = button_read,
};

static struct miscdevice btn_miscdev = {
	.minor	= BUTTON_MINOR,
	.name	= "button",
	.fops	= &btn_fops,
};

void timer_func(unsigned long data)
{
    unsigned int key_val,count = 0;
    
    key_val = readw(gpio_data)&0x1;  
    if (key_val == 0)
       key_num = 2;
    
    key_val = readw(gpio_data)&0x2; //上一首/快退
    if (key_val == 0)
      key_num = 3;
        
    key_val = readw(gpio_data)&0x4;
    if (key_val == 0)
        key_num = 4;
        
    key_val = readw(gpio_data)&0x8;
    if (key_val == 0)
        key_num = 5;
        
    key_val = readw(gpio_data)&0x10;
    if (key_val == 0)
        key_num = 6;
        
    key_val = readw(gpio_data)&0x20;
    if (key_val == 0)
        key_num = 7; 
     
  //  printk(KERN_WARNING"key%d down!\n",key_num);    
    wake_up(&button_queue);      
}

static void work_func(struct work_struct *private_)
{
    mod_timer(&timer,jiffies + HZ/20);  
}

static int timer_init(void)
{
    init_timer(&timer);
    timer.function = timer_func ;
    add_timer(&timer); 
    printk(KERN_WARNING"timer init!\n"); 
    return 0;  
}

static int work_init(void)
{
    work=kmalloc(sizeof(struct work_struct),GFP_KERNEL);//为工作分配空间
    INIT_WORK(work,work_func);//创建工作
    printk(KERN_WARNING"work init!\n"); 
    return 0;
}

/*加载模块*/
static int button_init(void) 
{
    /*注册混杂型驱动设备*/
    misc_register(&btn_miscdev);
    button_irq_init(); 
    timer_init();
    work_init();
    
    init_waitqueue_head(&button_queue);
    
    return 0;
}


/*卸载模块*/
static void button_exit(void)
{
    /*注销混杂型驱动设备*/
    misc_deregister(&btn_miscdev);
      //注销，释放资源操作
    free_irq(IRQ_EINT(0),NULL); 
    free_irq(IRQ_EINT(1),NULL); 
    free_irq(IRQ_EINT(2),NULL); 
    free_irq(IRQ_EINT(3),NULL); 
    free_irq(IRQ_EINT(4),NULL); 
    free_irq(IRQ_EINT(5),NULL); 
}



module_init(button_init);
module_exit(button_exit);