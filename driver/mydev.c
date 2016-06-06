#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/types.h>   // for dev_t typedef
#include <linux/kdev_t.h>  // for format_dev_t
#include <linux/fs.h>      // for alloc_chrdev_region()
#include <linux/list.h>
#include <linux/leds.h>
#include <linux/gpio.h> 
#include <asm/uaccess.h>



#define BUF_MAX_SIZE		1024
#define BINARY_MAX_SIZE     8192

static char buff[BUF_MAX_SIZE];
static dev_t mydev;       // (major,minor) value
struct cdev my_cdev;

struct Buffer {
    char input_char;
    struct list_head list;
};


static char binary[BINARY_MAX_SIZE];

static int INPUT_LENGTH;

static struct Buffer my_buffer;

ssize_t my_write (struct file *flip, const char __user *buf, size_t count, loff_t *f_ops)
{

    if(count >  BUF_MAX_SIZE)
    {
        copy_from_user(buff, buf, BUF_MAX_SIZE);

        return 1024;
    }
    else
    copy_from_user(buff, buf, count);

    INPUT_LENGTH = count;

    return count;
}



void save_to_linked_list(struct Buffer dest)
{
    int i;
    for(i=0; i<INPUT_LENGTH-1; i++)
    {

        struct Buffer *bufferPtr = (struct Buffer *)vmalloc(sizeof(struct Buffer));           
        bufferPtr->input_char = buff[i];
        INIT_LIST_HEAD(&bufferPtr->list);
 
        list_add_tail(&bufferPtr->list, &dest.list);
        
    }
 
}


void to_binary(struct Buffer dest)
{
    int i;
    int counter = 0;
    struct list_head *iter;
    struct Buffer *objPtr;

    list_for_each(iter, &dest.list) {
        if(counter < INPUT_LENGTH-1)
        {
        objPtr = list_entry(iter, struct Buffer, list);

        if(&(objPtr->input_char) == '\0')
        {
            break;
        }
        
        for (i = 7; i >= 0; --i)
        {
            if(((objPtr->input_char) & (1 << i )) == 0)
            {
                strcat(binary, "0");
            }
            else
            {
                strcat(binary, "1");
            }
        }
            counter = counter+1;
        }
        else
        {
            break;
        }
    }

}





ssize_t my_read(struct file *flip, char __user *buf, size_t count, loff_t *f_ops)
{

    INIT_LIST_HEAD(&my_buffer.list);
    save_to_linked_list(my_buffer);
    to_binary(my_buffer);

printk("functions called in my_read...\n");

if(*f_ops==(INPUT_LENGTH-1)*8)
{
 
    return 0; // end the process ...
}


copy_to_user(buf, &binary[*f_ops],1);

*f_ops+=1;
return 1;

} 

struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
     .write=my_write,
};

static int __init chardrv_in(void)
{
    printk(KERN_INFO "module chardrv being loaded.\n");

    alloc_chrdev_region(&mydev, 0, 1, "eadriver");
//    printk(KERN_INFO "%s\n", format_dev_t(buffer, mydev));

    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;
    cdev_add(&my_cdev, mydev, 1);

    return 0;
}

static void __exit chardrv_out(void)
{
    printk(KERN_INFO "module chardrv being unloaded.\n");
    cdev_del(&my_cdev);
    unregister_chrdev_region(mydev, 1);
}

module_init(chardrv_in);
module_exit(chardrv_out);

MODULE_AUTHOR("Hamid Hosseini, hamidhosseini417@gmail.com");
MODULE_LICENSE("GPL");
