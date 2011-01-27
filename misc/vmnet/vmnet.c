#include <asm/io.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>

#define NAME "vmnet_skyeye"
#define VMNET_SLEEP      0
#define VMNET_RUN        1
//-------------------------------------------------------------------------
static int major = 222;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major device number");
//-------------------------------------------------------------------------
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("vmnet skyeye Driver");
MODULE_AUTHOR("menghao <menghao@ibm.com.cn>");

void print_bin(uint8_t *string, uint32_t len)
{
	int i = 0;
	uint8_t *string_handle = string;
	if(string_handle == NULL)
	{
		return;
	}
	printk("\n------------------------------------------------------------\n");
	for(i=0; i<len; i++)
	{
		printk("%x ",string[i]);
	}
	printk("\n------------------------------------------------------------\n");
}

void print_bin_with_name(uint8_t *name, uint8_t *string, uint32_t len)
{
	int i = 0;
	uint8_t *string_handle = string;
	if(string_handle == NULL)
	{
		return;
	}
	printk("%s: [",name);
	for(i=0; i<len; i++)
	{
		printk("%x ",string[i]);
	}
	printk("]\n");
}
//--------------------------------------------------------------------------

unsigned long * pREG;
//unsigned char write_data[100];
//static DECLARE_WAIT_QUEUE_HEAD(vmnet_wait);


typedef struct vmnet_data
{
	void *data;
	int   length;
	struct vmnet_data *next;
}vmnet_data;

typedef struct vmnet_private
{
	wait_queue_head_t		inq;
	wait_queue_head_t		outq;
	spinlock_t				vmnet_lock;
	uint8_t					vmnet_mac[6];
	uint32_t				vmnet_state;
	vmnet_data		   *vmnet_data_head;
	struct vmnet_private   *next;
}vmnet_private;

vmnet_private *vmnet_private_head = NULL;

static int free_vmnet_data(vmnet_data *vmnet_data_old)
{
	kfree(vmnet_data_old->data);
	kfree(vmnet_data_old);
	return 0;
}

static int free_all_vmnet_data(vmnet_data *vmnet_data_head)
{
	vmnet_data *vmnet_data_handle = vmnet_data_head,*vmnet_data_handle2;
	while(vmnet_data_handle != NULL)
	{
		vmnet_data_handle2 = vmnet_data_handle->next;
		free_vmnet_data(vmnet_data_handle);
		vmnet_data_handle = vmnet_data_handle2;
	}
	return 0;
}

static int free_vmnet_private(vmnet_private *vmnet_private_old)
{
	free_all_vmnet_data(vmnet_private_old->vmnet_data_head);
	kfree(vmnet_private_old);
	return 0;
}

static vmnet_data *find_end_vmdata(vmnet_data *vmnet_data_head)
{
	vmnet_data *vmnet_data_handle = vmnet_data_head;
	if(vmnet_data_handle == NULL)
	{
		return NULL;
	}
	while(vmnet_data_handle->next != NULL)
	{
		vmnet_data_handle = vmnet_data_handle->next;
	}
	return vmnet_data_handle;
}

static int add_to_vmnet_data(vmnet_data **vmnet_data_head, vmnet_data *vmnet_data_new)
{
	vmnet_data *vmnet_data_handle = NULL;
	if(*vmnet_data_head == NULL)
	{
		*vmnet_data_head = vmnet_data_new;
	}
	else
	{
		vmnet_data_handle		= find_end_vmdata(*vmnet_data_head);
		vmnet_data_handle->next = vmnet_data_new;
	}
	return 0;
}

static vmnet_data *get_one_vmdata(vmnet_data **vmnet_data_head)
{
	vmnet_data *vmnet_data_handle = *vmnet_data_head;
	if(*vmnet_data_head == NULL)
	{
		return NULL;
	}
	else
	{
		*vmnet_data_head		= (*vmnet_data_head)->next;
		vmnet_data_handle->next = NULL;
		return vmnet_data_handle;
	}
}

static vmnet_private *find_end_private(vmnet_private *vmnet_private_head)
{
	//spin_lock(&vmnet_private_head->vmnet_lock);
	vmnet_private *vmnet_private_handle = vmnet_private_head;
	if(vmnet_private_head == NULL)
	{
		return NULL;
	}
	while(vmnet_private_handle->next != NULL)
	{
		vmnet_private_handle = vmnet_private_handle->next;
	}
	//spin_unlock(&vmnet_private_head->vmnet_lock);
	return vmnet_private_handle;
}

static int add_to_vmnet_private(vmnet_private **vmnet_private_head, vmnet_private *vmnet_private_new)
{
	//spin_lock(&(*vmnet_private_head)->vmnet_lock);
	vmnet_private *vmnet_private_handle = NULL;
	if(*vmnet_private_head == NULL)
	{
		*vmnet_private_head = vmnet_private_new;
	}
	else
	{
		vmnet_private_handle	   = find_end_private(*vmnet_private_head);
		vmnet_private_handle->next = vmnet_private_new;
	}
	//spin_unlock(&(*vmnet_private_head)->vmnet_lock);
	return 0;
}

static int del_from_vmnet_private(vmnet_private **vmnet_private_head, vmnet_private *vmnet_private_old)
{
	vmnet_private *vmnet_private_handle = *vmnet_private_head;
	if(vmnet_private_handle == NULL)
	{
		return -1;
	}
	if(*vmnet_private_head == vmnet_private_old)
	{
		*vmnet_private_head = (*vmnet_private_head)->next;
	}
	while((vmnet_private_handle->next != vmnet_private_old) && (vmnet_private_handle->next != NULL))
	{
		vmnet_private_handle = vmnet_private_handle->next;
	}
	if(vmnet_private_handle->next == NULL)
	{
		return -1;
	}
	vmnet_private_handle->next = vmnet_private_handle->next->next;
	return 0;
}

static uint32_t vmnet_mac_match(uint8_t *mac_addr_s, uint8_t *mac_addr_t)
{
	uint32_t i = 0;
	//print_bin_with_name("mac_addr_s", mac_addr_s, 6);
	//print_bin_with_name("mac_addr_t", mac_addr_t, 6);
	for(i=0; i<6; i++)
	{
		if(mac_addr_s[i] != mac_addr_t[i])
		{
			return -1;
		}
	}
	return 0;
}

static vmnet_private *search_vmnet_private(vmnet_private *vmnet_private_head, uint8_t *mac_addr)
{
	//uint32_t i = 0;
	vmnet_private *vmnet_private_handle = vmnet_private_head;
	while(vmnet_private_handle != NULL)
	{
		if((vmnet_mac_match(vmnet_private_handle->vmnet_mac, mac_addr)) == 0)
		{
			return vmnet_private_handle;
		}
		vmnet_private_handle = vmnet_private_handle->next;
		//printk("search vmnet private next\n");
	}
	return NULL;
}

static int add_broad_to_vmnet_private(vmnet_private *vmnet_private_head, vmnet_data *vmnet_data_new, uint8_t *mac_addr_s)
{
	vmnet_private *vmnet_private_handle = vmnet_private_head;
	while(vmnet_private_handle != NULL)
	{
		if((vmnet_mac_match(vmnet_private_handle->vmnet_mac, mac_addr_s)) != 0)
		{
			add_to_vmnet_data(&vmnet_private_handle->vmnet_data_head, vmnet_data_new);
		}
		vmnet_private_handle = vmnet_private_handle->next;
		//printk("search vmnet private next\n");
	}
	return 0;
}

#define mac_addr_offset 0
#define mac_saddr_offset 6
static ssize_t vmnet_skyeye_write(struct file *file, const char __user *data, size_t len, loff_t *ppos)
{
	vmnet_private *my_vmnet_private = NULL;
	uint8_t       *mac_addr			= data + mac_addr_offset;
	uint8_t       *mac_saddr        = data + mac_saddr_offset;
	uint8_t       mac_broadcast[6]  = {0xff,0xff,0xff,0xff,0xff,0xff};

	print_bin_with_name("skyeye_write mac_taddr", mac_addr ,6);
	print_bin_with_name("skyeye_write mac_saddr", mac_saddr ,6);
	print_bin(data,len);

	if(vmnet_mac_match(mac_addr, mac_broadcast) == 0)
	{
		printk("this is broadcast\n");
		vmnet_data    *vmnet_data_new   =  kmalloc(sizeof(vmnet_data), GFP_KERNEL);
		vmnet_data_new->data   = (uint8_t *)kmalloc(len, GFP_KERNEL);
		vmnet_data_new->length = len;
		vmnet_data_new->next   = NULL;
		if(copy_from_user(vmnet_data_new->data, data, vmnet_data_new->length))
		{
			printk("write:error in write\n");
		}
		add_broad_to_vmnet_private(vmnet_private_head, vmnet_data_new, mac_saddr);
		return 0;
	}

	if((my_vmnet_private = search_vmnet_private(vmnet_private_head,mac_addr)) == NULL)
	{
		printk("write:can not find src vmnet\n");
		return 0;
	}
	//printk("search my_vmnet_private %p\n",my_vmnet_private);
	vmnet_data    *vmnet_data_head  =  my_vmnet_private->vmnet_data_head;
	vmnet_data    *vmnet_data_new   =  kmalloc(sizeof(vmnet_data), GFP_KERNEL);
	vmnet_data_new->data   = (uint8_t *)kmalloc(len, GFP_KERNEL);
	vmnet_data_new->length = len;
	vmnet_data_new->next   = NULL;
	if(copy_from_user(vmnet_data_new->data, data, vmnet_data_new->length))
	{
		printk("write:error in write\n");
	}
	printk("vmnet_skyeye_write\n");
	//add_to_vmnet_data(&vmnet_data_head, vmnet_data_new);
	add_to_vmnet_data(&my_vmnet_private->vmnet_data_head, vmnet_data_new);
	return 0;
}

static ssize_t vmnet_skyeye_read(struct file *file, char __user *data, size_t len, loff_t *ppos)
{
	vmnet_private *my_vmnet_private = (vmnet_private *)file->private_data;
	vmnet_data    *vmnet_data_head  =  my_vmnet_private->vmnet_data_head;
	//printk("vmnet_skyeye_read %p\n",vmnet_data_head);
	vmnet_data    *vmnet_data_one   = get_one_vmdata(&my_vmnet_private->vmnet_data_head);
	if(vmnet_data_one == NULL)
	{
		return 0;
	}
	if(vmnet_data_one->data == NULL)
	{
		return 0;
	}
	if(copy_to_user(data, vmnet_data_one->data, vmnet_data_one->length))
	{
		printk("read:error in read\n");
	}
	return vmnet_data_one->length;
}

static int vmnet_skyeye_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd)
	{
		case 0:
			{
				//FIXME   make sure arg is a handle
				memcpy(((vmnet_private *)file->private_data)->vmnet_mac, (void *)arg, 6);
				((vmnet_private *)file->private_data)->vmnet_state = VMNET_RUN;
				print_bin_with_name("skyeye_ioctl mac_taddr", arg ,6);
				break;
			}
		case 1:
			{
				break;
			}
		default:
			return -EINVAL;
	}
	return 1;
}

static int vmnet_skyeye_open(struct inode *inode, struct file *file)
{
	printk("vmnet skyeye driver opened!\n");
	vmnet_private *vmnet_private_new   = (vmnet_private *)kmalloc(sizeof(vmnet_private),GFP_KERNEL);
	vmnet_private_new->next			   = NULL;
	vmnet_private_new->vmnet_data_head = NULL;
	vmnet_private_new->vmnet_state     = VMNET_SLEEP;
	spin_lock_init(&vmnet_private_new->vmnet_lock);
	init_waitqueue_head(&vmnet_private_new->inq);
	init_waitqueue_head(&vmnet_private_new->outq);
	file->private_data = vmnet_private_new;
	printk("private_data =%p \n",file->private_data);
	if(vmnet_private_head == NULL)
	{
		vmnet_private_head = vmnet_private_new;
	}
	else
	{
		add_to_vmnet_private(&vmnet_private_head,vmnet_private_new);
	}
	return 0;
}

static int vmnet_skyeye_release(struct inode *inode, struct file *file)
{
	printk("vmnet skyeye driver released!\n");
	del_from_vmnet_private(&vmnet_private_head, (vmnet_private *)file->private_data);
	free_vmnet_private((vmnet_private *)file->private_data);
	printk("vmnet_private_head %p\n",vmnet_private_head);
	//vmnet_private_head = NULL;
	return 0;
}

static unsigned int vmnet_skyeye_poll(struct file *filp, poll_table *wait)
{
	vmnet_private *my_vmnet_private = (vmnet_private *)filp->private_data;
	unsigned int mask = POLLOUT | POLLWRNORM;
#if 0
	struct scull_pipe *dev = filp->private_data;
	down(&dev->sem);
	poll_wait(filp, &dev->inq,  wait);
	poll_wait(filp, &dev->outq, wait);

	if (dev->rp != dev->wp)
		mask |= POLLIN | POLLRDNORM;  /* readable */
	if (spacefree(dev))
		mask |= POLLOUT | POLLWRNORM;  /* writable */
	up(&dev->sem);
#endif
	poll_wait(filp, &my_vmnet_private->inq,  wait);
	poll_wait(filp, &my_vmnet_private->outq, wait);
	spin_lock(&my_vmnet_private->vmnet_lock);
	if(my_vmnet_private->vmnet_data_head != NULL)
	{
		mask |= POLLIN | POLLRDNORM;  /* readable */
	}
	mask |= POLLOUT | POLLWRNORM;  /* writable */
	spin_unlock(&my_vmnet_private->vmnet_lock);
	return mask;
}

static struct file_operations vmnet_skyeye_fops = {
	.owner   = THIS_MODULE,
	.ioctl   = vmnet_skyeye_ioctl,
	.write   = vmnet_skyeye_write,
	.read    = vmnet_skyeye_read,
	.open    = vmnet_skyeye_open,
	.release = vmnet_skyeye_release,
	.poll    = vmnet_skyeye_poll,
};

static int __init vmnet_skyeye_init(void)
{
	int ret;
	printk("-------------------------vmnet init-------------------------\n");
	printk("this is skyeye vhub\n\n");
	ret = register_chrdev(major, NAME, &vmnet_skyeye_fops);

	if (ret < 0) {
		printk("Unable to register character device!\n");
		return ret;
	}
	printk("vmnet skyeye Driver initiated.\n");
	return 0;
}

static void __exit vmnet_skyeye_cleanup(void)
{
	int ret = 0;
	printk("module_exit\n");
	unregister_chrdev(major, NAME);
	if (ret < 0)
		printk("Unable to register character device!\n");
	else
		printk("vmnet skyeye Driver unloaded!");
}
module_init(vmnet_skyeye_init);
module_exit(vmnet_skyeye_cleanup);
