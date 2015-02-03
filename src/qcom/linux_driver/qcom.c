/*
 */

#include <linux/mm.h> /* remap_pfn_range */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/ptrace.h> /* force_successful_syscall_return */
#include <linux/io.h> /* xlate */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */

MODULE_LICENSE("Dual BSD/GPL");
u32 PHYS_MEM_ADDR = 0x30000000;
u32 MEM_SIZE = 0x100000;

/* Declaration of qcom.c functions */
static int qcom_mmap(struct file *file, struct vm_area_struct *vma);
static int qcom_open(struct inode *inode, struct file *filp);
static int qcom_release(struct inode *inode, struct file *filp);
static void qcom_exit(void);
static int qcom_init(void);

struct file_operations qcom_fops = {
        .mmap           = qcom_mmap,
  	.open           = qcom_open,
	.release	= qcom_release,
};

/* Declaration of the init and exit functions */
module_init(qcom_init);
module_exit(qcom_exit);

/* Global variables of the driver */
/* Major number */
int qcom_major = 1;

int qcom_init(void) {
  int result;

  /* Registering device */
  result = register_chrdev(qcom_major, "qcom", &qcom_fops);
  if (result < 0) {
    printk(
      "<1>qcom: cannot obtain major number %d\n", qcom_major);
    return result;
  }

  //qcom_buffer = PHYSICAL MEM LOCATION

  printk("<1>Inserting qcom module\n"); 
  return 0;
}

void qcom_exit(void) {
  /* Freeing the major number */
  unregister_chrdev(qcom_major, "qcom");

  printk("<1>Removing qcom module\n");

}

static atomic_t qcom_available = ATOMIC_INIT(1);

static int qcom_open(struct inode *inode, struct file *filp)
{
    if (!atomic_dec_and_test(&qcom_available)) {
        atomic_inc(&qcom_available);
        return -EBUSY; /* already open */
    }

    return 0;          /* success */
}

static int qcom_release(struct inode *inode, struct file *filp)
{
    atomic_inc(&qcom_available); /* release the device */
    return 0;
}


/* Copied from driver/char/mem.c (then altered for offset)*/
static inline unsigned long size_inside_page(unsigned long start,
                                             unsigned long size)
{
        unsigned long sz;

        sz = PAGE_SIZE - (start & (PAGE_SIZE - 1));

        return min(sz, size);
}

static int qcom_mmap(struct file *file, struct vm_area_struct *vma)
{
        size_t size = vma->vm_end - vma->vm_start;

        if (vma->vm_pgoff + size > MEM_SIZE)
                return -EINVAL;

        /*if (!private_mapping_ok(vma))
                return -ENOSYS;*/

        /*vma->vm_page_prot = phys_mem_access_prot(file, vma->vm_pgoff,
                                                 size,
                                                 vma->vm_page_prot);*/

        /* vma->vm_ops = &mmap_mem_ops; */

        /* Remap-pfn-range will mark the range VM_IO */
        if (remap_pfn_range(vma,
                            vma->vm_start,
                            vma->vm_pgoff + (PHYS_MEM_ADDR>>PAGE_SHIFT),
                            size,
                            vma->vm_page_prot)) {
                return -EAGAIN;
        }
        return 0;
}
