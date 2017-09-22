/////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng
//
//   Description:
//     Skeleton of NPHeap Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include "npheap.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>

extern struct miscdevice npheap_dev;
struct node kernel_llist;
struct mutex list_lock;

// Structure is ready
struct node {
    __u64 objectId;
    __u64 size;
    void* k_virtual_addr;
    struct mutex objLock;
    struct list_head list;
};


void createObject(__u64 offset)
{
    printk("Starting createObject function. \n"); 
    struct node *newNode;
    printk("Creating object for offset -> %llu \n", offset);
    newNode = (struct node *)kmalloc(sizeof(struct node), GFP_KERNEL);
    newNode->objectId = offset;
    newNode->size = 0;
    newNode->k_virtual_addr = NULL;
    mutex_init(&(newNode->objLock));
    
    mutex_lock(&list_lock);
    list_add(&(newNode->list), &(kernel_llist.list));
    mutex_unlock(&list_lock);

    printk("Node created and added \n");
}



struct node* getObject(__u64 inputOffset)
{
    printk("Starting getObject function. \n");    
    struct list_head *position;
    struct node *llist;

    printk("Searching for Offset -> %llu \n",inputOffset);

    mutex_lock(&list_lock);
    list_for_each(position, &kernel_llist.list){
        llist = list_entry(position, struct node, list);
        if(llist->objectId == inputOffset){
            printk("Object found. \n");
            mutex_unlock(&list_lock);
            return llist;
        }
    }
    mutex_unlock(&list_lock);
    return NULL;
}

// Memory Allocation is ready
int npheap_mmap(struct file *filp, struct vm_area_struct *vma)
{
    printk("Starting npheap_mmap function. \n");  
    __u64 offset = vma->vm_pgoff;
    __u64 size = vma->vm_end - vma->vm_start;
    struct node *object;

    printk("Entering into MMAP for offset -> %llu \n", offset);
    object = getObject(offset);
    
    if(object == NULL){
        createObject(offset);
        object = getObject(offset);
    }

    if(object->size == 0){
        
        printk("Printing MMAP size: %llu \n", size);
        object->k_virtual_addr = kmalloc(size, GFP_KERNEL);
        memset(object->k_virtual_addr,0, size);
        printk("################Memset Completed################");
        object->size = size;
        //__virt_to_phys
        printk(KERN_INFO "New ObjectID added \n");
    }
    if(remap_pfn_range(vma, vma->vm_start, __pa(object->k_virtual_addr)>>PAGE_SHIFT, (unsigned long) size, vma->vm_page_prot) < 0){
        printk(KERN_ERR "New remap failed");
        return -EAGAIN;
    }
    return 0;
}

int npheap_init(void)
{
    int ret;
    if ((ret = misc_register(&npheap_dev)))
        printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
    else{
        printk(KERN_ERR "\"npheap\" misc device installed\n");
        INIT_LIST_HEAD(&kernel_llist.list);
        mutex_init(&list_lock);
    }
    return ret;
}

void npheap_exit(void)
{
    misc_deregister(&npheap_dev);
}



//References:
//1. Linked List: https://isis.poly.edu/kulesh/stuff/src/klist/
//2. Mutex: http://elixir.free-electrons.com/linux/v4.4/source/include/linux/mutex.h