#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/slab.h>

#define SNAPMEM_ALLOC 0
#define SNAPMEM_LIGHT_OPEN  1
#define SNAPMEM_LIGHT_CLOSE 3
#define SNAPMEM_FREE  4


#define DEVICE_NAME "SnapMem_module"

#define bram0_phy_addr              0xA0000000
#define bram1_phy_addr              0xA0002000
#define bram2_phy_addr              0xA0004000
#define bram3_phy_addr              0xA0006000
#define bram4_phy_addr              0xA0008000
#define SnapMem_pump_phy_addr       0xA0010000
#define SnapMem_switch_phy_addr     0xA0020000



MODULE_AUTHOR("Gerty");
MODULE_DESCRIPTION("SnapMem_driver");
MODULE_VERSION("v1.0");
MODULE_LICENSE("GPL");


static int SnapMem_driver_major;
static struct class*  SnapMem_driver_class   = NULL;
static struct device* SnapMem_driver_device  = NULL;



unsigned long bram0_vir_addr   ;
unsigned long bram1_vir_addr   ;
unsigned long bram2_vir_addr   ;
unsigned long bram3_vir_addr   ;
unsigned long bram4_vir_addr   ;
unsigned long SnapMem_pump_vir_addr;
unsigned long SnapMem_switch_vir_addr;
unsigned long snapmem_state;

unsigned long snapmem_vir_addr[5];
unsigned long snapmem_phy_addr[5];

extern unsigned long snapmem_pid[5];

extern unsigned long snapmem[5][1024];

extern unsigned long snapmem_enable;


void arm_v8_invalidate(void* address)
{
	write_sysreg(0x0, CSSELR_EL1);	
	asm volatile ("DC IVAC, %0" :: "r"((unsigned long)address&(~0x3F)));
	write_sysreg(0x2, CSSELR_EL1);
	asm volatile ("DC IVAC, %0" :: "r"((unsigned long)address&(~0x3F)));	
	asm volatile ("DSB ISH");
	asm volatile ("ISB");
}

void arm_v8_flush(void* address)
{
	write_sysreg(0x0, CSSELR_EL1);	
	asm volatile ("DC CIVAC, %0" :: "r"((unsigned long)address&(~0x3F)));
	write_sysreg(0x2, CSSELR_EL1);
	asm volatile ("DC CIVAC, %0" :: "r"((unsigned long)address&(~0x3F)));	
	asm volatile ("DSB ISH");
	asm volatile ("ISB");
}


static int SnapMem_open(struct inode * inode, struct file * filp)
{	
  return 0;
}


static int SnapMem_release(struct inode * inode, struct file *filp)
{	
  return 0;
}


ssize_t SnapMem_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	int i,snapmem_id;	
	u32 snapmem_offset;
	u32	snapmem_size;
	u32 snapmem_udelay;
	snapmem_offset = ((size & 0xFFF000) >> 12)*4;
	snapmem_size   =  (size & 0xFFF)*4;
	if(snapmem_state==SNAPMEM_LIGHT_CLOSE)
		snapmem_udelay  = snapmem_size*2048/0x2000; 	
	//for(i=0;i<5;i++)
		//printk("SnapMem_read: snapmem_pid[%d]=%ld",i,snapmem_pid[i]);		
	//printk("SnapMem_read: snapmem_offset=0x%x,snapmem_size=0x%x,snapmem_udelay=%d",\
		snapmem_offset,snapmem_size,snapmem_udelay);
	for(snapmem_id=0;snapmem_id<5;snapmem_id++){
		if(snapmem_pid[snapmem_id]==current->pid){
			break;
		}
	}
	//printk("SnapMem_read: snapmem_id=%d",snapmem_id);
	
	
	if(snapmem_id==5)
		return EBUSY;

	if(snapmem_state==SNAPMEM_LIGHT_CLOSE){
		for(i=0;i<128;i++)
			arm_v8_flush((void*)(snapmem_vir_addr[0]+64*i));	
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));		
		iowrite32(snapmem_id,(unsigned long *)SnapMem_pump_vir_addr);
		iowrite32(snapmem_phy_addr[snapmem_id],(unsigned long *)(SnapMem_pump_vir_addr+4));
		iowrite32(8192,(unsigned long *)(SnapMem_pump_vir_addr+8));
		iowrite32(0x0F0F0F0F,(unsigned long *)(SnapMem_pump_vir_addr+12));
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));
		udelay(2048);	
	}
	copy_to_user(buf, (void *)(snapmem_vir_addr[snapmem_id]+snapmem_offset), snapmem_size);


	if(snapmem_state==SNAPMEM_LIGHT_CLOSE){
		for(i=0;i<128;i++)
			arm_v8_flush((void*)(snapmem_vir_addr[0]+64*i));	
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));	
		iowrite32(snapmem_id,(unsigned long *)SnapMem_pump_vir_addr);
		iowrite32(snapmem_phy_addr[snapmem_id],(unsigned long *)(SnapMem_pump_vir_addr+4));
		iowrite32(8192,(unsigned long *)(SnapMem_pump_vir_addr+8));
		iowrite32(0xF0F0F0F0,(unsigned long *)(SnapMem_pump_vir_addr+12));
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));
		udelay(2048);
	}
	
	//printk("SnapMem_read: ......");
    return 0;
}


ssize_t SnapMem_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	int i,snapmem_id;	
	u32 snapmem_offset;
	u32	snapmem_size;
	u32 snapmem_udelay;
	snapmem_offset = ((size & 0xFFF000) >> 12)*4;
	snapmem_size   =  (size & 0xFFF)*4;	
	
	if(snapmem_state==SNAPMEM_LIGHT_CLOSE)
		snapmem_udelay  = snapmem_size*2048/0x2000; 
		
	//for(i=0;i<5;i++)
		//printk("SnapMem_write: snapmem_pid[%d]=%ld",i,snapmem_pid[i]);		
	//printk("SnapMem_write: snapmem_offset=0x%x,snapmem_size=0x%x,snapmem_udelay=%d",\
		snapmem_offset,snapmem_size,snapmem_udelay);
	for(snapmem_id=0;snapmem_id<5;snapmem_id++){
		if(snapmem_pid[snapmem_id]==current->pid){
			break;
		}
	}
	//printk("SnapMem_write: snapmem_id=%d",snapmem_id);
	
	
	if(snapmem_id==5)
		return EBUSY;

	copy_from_user((void *)(snapmem_vir_addr[snapmem_id]+snapmem_offset), buf, snapmem_size);

	if(snapmem_state==SNAPMEM_LIGHT_CLOSE){	
		for(i=0;i<128;i++)
			arm_v8_flush((void*)(snapmem_vir_addr[0]+64*i));	
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));	
		iowrite32(snapmem_id,(unsigned long *)SnapMem_pump_vir_addr);
		iowrite32(snapmem_phy_addr[snapmem_id],(unsigned long *)(SnapMem_pump_vir_addr+4));
		iowrite32(8192,(unsigned long *)(SnapMem_pump_vir_addr+8));
		iowrite32(0xF0F0F0F0,(unsigned long *)(SnapMem_pump_vir_addr+12));
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));	
		udelay(2048);
	}
	//printk("SnapMem_write: ......");
	return 0;
}




ssize_t	SnapMem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int i,snapmem_id;
	
	
	if(cmd==SNAPMEM_ALLOC)
	{
		printk("SnapMem_ioctl: Alloc SnapMem");
		for(i=0;i<5;i++){
			if(snapmem_pid[i]==current->pid){
				return EEXIST;
			}			
			if(snapmem_pid[i]==0){
				snapmem_pid[i]=current->pid;
				break;
			}
		}
		if(i==5)
			return ENOMEM;
	}
	
	if(cmd==SNAPMEM_LIGHT_OPEN){
		printk("SnapMem_ioctl: Open SnapMem_light");
		snapmem_state=SNAPMEM_LIGHT_OPEN;
		for(snapmem_id=0;snapmem_id<5;snapmem_id++){
			if(snapmem_pid[snapmem_id]==current->pid){
				break;
			}
		}
		for(i=0;i<128;i++)
			arm_v8_flush((void*)(snapmem_vir_addr[0]+64*i));	
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));		
		iowrite32(snapmem_id,(unsigned long *)SnapMem_pump_vir_addr);
		iowrite32(snapmem_phy_addr[snapmem_id],(unsigned long *)(SnapMem_pump_vir_addr+4));
		iowrite32(8192,(unsigned long *)(SnapMem_pump_vir_addr+8));
		iowrite32(0x0F0F0F0F,(unsigned long *)(SnapMem_pump_vir_addr+12));
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));
		udelay(2048);			
	}


	if(cmd==SNAPMEM_LIGHT_CLOSE){
		printk("SnapMem_ioctl: Close SnapMem_light");
		snapmem_state=SNAPMEM_LIGHT_CLOSE;
		for(snapmem_id=0;snapmem_id<5;snapmem_id++){
			if(snapmem_pid[snapmem_id]==current->pid){
				break;
			}
		}
		for(i=0;i<128;i++)
			arm_v8_flush((void*)(snapmem_vir_addr[0]+64*i));	
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));	
		iowrite32(snapmem_id,(unsigned long *)SnapMem_pump_vir_addr);
		iowrite32(snapmem_phy_addr[snapmem_id],(unsigned long *)(SnapMem_pump_vir_addr+4));
		iowrite32(8192,(unsigned long *)(SnapMem_pump_vir_addr+8));
		iowrite32(0xF0F0F0F0,(unsigned long *)(SnapMem_pump_vir_addr+12));
		for(i=0;i<128;i++)		
			arm_v8_invalidate((void*)(snapmem_vir_addr[0]+64*i));
		udelay(2048);	
	}

	
	if(cmd==SNAPMEM_FREE){
		printk("SnapMem_ioctl: Free SnapMem");
		for(i=0;i<5;i++){
			if(snapmem_pid[i]==current->pid){
				snapmem_pid[i]=0;
				break;
			}
		}		
	}
	
	//printk("SnapMem_ioctl: ......");
	return 0;	
}



static struct file_operations SnapMem_driver_fops = {
	.owner   =    THIS_MODULE,
	.open    =    SnapMem_open,
	.release =    SnapMem_release,
	.read    =    SnapMem_read,
	.write   =    SnapMem_write, 
	.unlocked_ioctl = SnapMem_ioctl,	
};


static int __init SnapMem_driver_module_init(void)
{

	int i;	
		
	SnapMem_driver_major = register_chrdev(0, DEVICE_NAME, &SnapMem_driver_fops);
	if(SnapMem_driver_major < 0){
		printk("failed to register device.\n");
		return -1;
	}
	
	
	SnapMem_driver_class = class_create(THIS_MODULE, "SnapMem_driver");
    if (IS_ERR(SnapMem_driver_class)){
        printk("failed to create SnapMem moudle class.\n");
        unregister_chrdev(SnapMem_driver_major, DEVICE_NAME);
        return -1;
    }
	
	
    SnapMem_driver_device = device_create(SnapMem_driver_class, NULL, MKDEV(SnapMem_driver_major, 0), NULL, "SnapMem_device");
    if (IS_ERR(SnapMem_driver_device)){
        printk("failed to create device .\n");
        unregister_chrdev(SnapMem_driver_major, DEVICE_NAME);
        return -1;
    }


	snapmem_enable=0xdeadbeef;

	bram0_vir_addr = (unsigned long)ioremap(bram0_phy_addr, 0x2000);
	bram1_vir_addr = (unsigned long)ioremap(bram1_phy_addr, 0x2000);
	bram2_vir_addr = (unsigned long)ioremap(bram2_phy_addr, 0x2000);
	bram3_vir_addr = (unsigned long)ioremap(bram3_phy_addr, 0x2000);
	bram4_vir_addr = (unsigned long)ioremap(bram4_phy_addr, 0x2000);
	SnapMem_pump_vir_addr = (unsigned long)ioremap(SnapMem_pump_phy_addr, 0x10000);	
	SnapMem_switch_vir_addr = (unsigned long)ioremap(SnapMem_switch_phy_addr, 0x10000);

	
	iowrite32(0x01,(unsigned long *)SnapMem_switch_vir_addr);
	iowrite32(0x02,(unsigned long *)SnapMem_switch_vir_addr);
	iowrite32(0x04,(unsigned long *)SnapMem_switch_vir_addr);
	iowrite32(0x08,(unsigned long *)SnapMem_switch_vir_addr);	
	iowrite32(0x0f,(unsigned long *)SnapMem_switch_vir_addr);
	printk("Open bram0...4!");	


	for(i=0;i<128;i++)
	{
		iowrite32(i*0,(unsigned long *)(bram0_vir_addr+4*i));
		iowrite32(i*1,(unsigned long *)(bram1_vir_addr+4*i));
		iowrite32(i*2,(unsigned long *)(bram2_vir_addr+4*i));
		iowrite32(i*3,(unsigned long *)(bram3_vir_addr+4*i));
		iowrite32(i*4,(unsigned long *)(bram4_vir_addr+4*i));		
	}
	printk("Bram0......4 initialization completed!\n");	
	

	for(i=0;i<5;i++){
		snapmem_vir_addr[i] = (unsigned long)snapmem[i];
		snapmem_phy_addr[i] = (unsigned long)virt_to_phys((void *) snapmem_vir_addr[i]);
	}

	snapmem_state=SNAPMEM_LIGHT_CLOSE;


	for(i=0;i<5;i++){
		snapmem_pid[i]=0;
	}
	
	printk("SnapMem driver initial successfully!\n");
    return 0;
}


static void __exit SnapMem_driver_module_exit(void)
{
	int i;	
	snapmem_enable=0x0;
	iounmap((void *)bram0_vir_addr);
	iounmap((void *)bram1_vir_addr);
	iounmap((void *)bram2_vir_addr);
	iounmap((void *)bram3_vir_addr);
	iounmap((void *)bram4_vir_addr);
	iounmap((void *)SnapMem_pump_vir_addr);
	iounmap((void *)SnapMem_switch_vir_addr);
	printk("exit module.\n");
    device_destroy(SnapMem_driver_class, MKDEV(SnapMem_driver_major, 0));
    class_unregister(SnapMem_driver_class);
	class_destroy(SnapMem_driver_class);
	unregister_chrdev(SnapMem_driver_major, DEVICE_NAME);

	snapmem_state=SNAPMEM_LIGHT_OPEN;

	for(i=0;i<5;i++){
		snapmem_pid[i]=0;
	}
	
    printk("SnapMem module exit.\n");	
}

module_init(SnapMem_driver_module_init);
module_exit(SnapMem_driver_module_exit);

	
	
	

