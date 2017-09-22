// Project 1: Animesh Sinsinwal, assinsin; Sameer Poudwal, spoudwa; Sayali Godbole, ssgodbol;
//////////////////////////////////////////////////////////////////////
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
/*
* @author: a_sinsinwal, ssgodbol, spoudwa
* References are added at the last. [Conceptual knowledge gained from references]
*/

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
#include <linux/list.h>

// Global variables instantiated
extern struct node kernel_llist;
extern struct mutex list_lock;

// Structure redefine
struct node {
    __u64 objectId;
    __u64 size;
    void* k_virtual_addr;
    struct mutex objLock;
    struct list_head list;
};

// Function Declaration (by default extern)
__u64 getSize(__u64 inputOffset);
void createObject(__u64 offset);
struct node* getObject(__u64 inputOffset);

// Lock function calls come here
long npheap_lock(struct npheap_cmd __user *user_cmd)
{
    printk("Starting npheap_lock function: \n");

    struct npheap_cmd copy;
    struct node *obj;
    // Copying data from user space
    if(copy_from_user(&copy, user_cmd, sizeof(struct npheap_cmd))==0){

        printk("%llu -> Offset ID, lock in progess... \n", (copy.offset/PAGE_SIZE));
        // Get object from our linked list
        obj = getObject((__u64)copy.offset/PAGE_SIZE);

        if(obj == NULL){
            createObject((__u64)copy.offset/PAGE_SIZE);
            obj= getObject((__u64)copy.offset/PAGE_SIZE);
        }

        // Kernel Locking Function
        // OBJECT LEVEL (NODE LEVEL) LOCK
        mutex_lock(&(obj->objLock));
    }else{
        printk(KERN_ERR "copy_from_user failed in Lock function. \n" );       
        return -EFAULT;
    }
    printk("OffsetID Locked.!! \n");
    return 0;
}     

// Unlock function calls come here
long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
    printk("Starting npheap_unlock function: \n");
    
    struct node *obj;
    struct npheap_cmd copy;
    // Copying data from user space
    if(copy_from_user(&copy, user_cmd, sizeof(struct npheap_cmd))==0){
        obj= getObject((__u64)copy.offset/PAGE_SIZE);
        // Kernel UnLocking Function
        // OBJECT LEVEL (NODE LEVEL) UNLOCK
        mutex_unlock(&(obj->objLock));
    }else{
        printk(KERN_ERR "copy_from_user failed in unLock function \n" );       
        return -EFAULT;
    }
    printk("OffsetID Unlocked.!!\n");
    return 0;
}

// GetSize function calls come here
long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
    printk("Starting npheap_getsize function: \n ");
    __u64 size;
    struct npheap_cmd copy;
    // Copying data from user space
    if(copy_from_user(&copy, user_cmd, sizeof(struct npheap_cmd))==0){
        size = getSize((__u64)copy.offset/PAGE_SIZE);
        printk("Size: %llu \n returned.", size);
        return (int) size;
    }
    else{ 
        printk(KERN_ERR "copy_from_user failed in getsize function \n" );       
        return -EFAULT;
    }
}

// Delete function calls come here
long npheap_delete(struct npheap_cmd __user *user_cmd)
{
    printk("Starting npheap_delete function: \n");
    struct npheap_cmd copy;
    // Copying data from user space
    if(copy_from_user(&copy, user_cmd, sizeof(struct npheap_cmd))==0){
        struct list_head *position;
        struct node *llist;
        
        llist = getObject(copy.offset/PAGE_SIZE);
        
        /*
        * Delete Logic - We won't be deleting the node from the linked list.
        * All nodes get deleted, when device is added into system
        * Device - 'npheap'
        */
        if(llist != NULL){
            mutex_lock(&list_lock);
            kfree(llist->k_virtual_addr);
            llist->size = 0;
            llist->k_virtual_addr=NULL;
            mutex_unlock(&list_lock);
            printk("Freed Offset :%llu  Exiting Delete. \n" ,llist->objectId);
        }
    }
    else{    
        printk(KERN_ERR "copy_from_user failed in delete function. \n");    
        return -EFAULT;
    }
    return 0;
}


//npheap_getsize calls come here
__u64 getSize(__u64 inputOffset)
{
    printk("Starting getSize function: \n");   
    struct list_head *position;
    struct node *llist;
    __u64 size = 0;

    printk("Searching size for Offset -> %llu \n",inputOffset);

    llist = getObject(inputOffset);
    if(llist != NULL){
        printk("Object found.!");
        size = llist->size;
    }
    return size;
}

long npheap_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    switch (cmd) {
    case NPHEAP_IOCTL_LOCK:
        return npheap_lock((void __user *) arg);
    case NPHEAP_IOCTL_UNLOCK:
        return npheap_unlock((void __user *) arg);
    case NPHEAP_IOCTL_GETSIZE:
        return npheap_getsize((void __user *) arg);
    case NPHEAP_IOCTL_DELETE:
        return npheap_delete((void __user *) arg);
    default:
        return -ENOTTY;
    }
}


//References:
//1. Linked List: https://isis.poly.edu/kulesh/stuff/src/klist/
//2. Mutex: http://elixir.free-electrons.com/linux/v4.4/source/include/linux/mutex.h