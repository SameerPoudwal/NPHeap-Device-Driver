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

// mutex that will be shared among the threads
//static DEFINE_MUTEX(k_mutex); 

extern struct node kernel_llist;
extern struct mutex list_lock;

// structure redefine
struct node {
    __u64 objectId;
    __u64 size;
    void* k_virtual_addr;
    struct mutex objLock;
    struct list_head list;
};

__u64 getSize(__u64 inputOffset);
//struct mutex getLock(__u64 inputOffset);
void createObject(__u64 offset);
struct node* getObject(__u64 inputOffset);

long npheap_lock(struct npheap_cmd __user *user_cmd)
{
    printk("Starting npheap_lock function. \n");

    struct npheap_cmd copy;
    struct node *obj;
    if(copy_from_user(&copy, user_cmd, sizeof(struct npheap_cmd))==0){

        printk("%llu Offset ID in lock \n", (copy.offset/PAGE_SIZE));
        obj = getObject((__u64)copy.offset/PAGE_SIZE);

        if(obj == NULL){
            createObject((__u64)copy.offset/PAGE_SIZE);
            obj= getObject((__u64)copy.offset/PAGE_SIZE);
        }

        mutex_lock(&(obj->objLock));
    }else{
        printk(KERN_ERR "copy_from_user failed in Lock \n" );       
        return -EFAULT;
    }
    printk("locked \n");
    return 0;
}     

long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
    printk("Starting npheap_unlock function. \n");
    
    struct node *obj;
    struct npheap_cmd copy;
    if(copy_from_user(&copy, user_cmd, sizeof(struct npheap_cmd))==0){
        obj= getObject((__u64)copy.offset/PAGE_SIZE);
        mutex_unlock(&(obj->objLock));
    }else{
        printk(KERN_ERR "copy_from_user failed in unLock \n" );       
        return -EFAULT;
    }
    printk("Unlocked\n");
    return 0;
}

long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
    printk("Starting npheap_getsize function. \n ");
    __u64 size;
    struct npheap_cmd copy;
    if(copy_from_user(&copy, user_cmd, sizeof(struct npheap_cmd))==0){
        size = getSize((__u64)copy.offset/PAGE_SIZE);
        printk("Size : %llu \n", size);
        printk("Exiting npheap_getsize \n");
        return (int) size;
    }
    else{ 
        printk(KERN_ERR "copy_from_user failed in getsize \n" );       
        return -EFAULT;
    }

    //object = getObject((__u64) user_cmd->offset);
    //return (long) object->size;
}


long npheap_delete(struct npheap_cmd __user *user_cmd)
{
    printk("Starting npheap_delete function. \n");
    struct npheap_cmd copy;
    if(copy_from_user(&copy, user_cmd, sizeof(struct npheap_cmd))==0){
        struct list_head *position;
        struct node *llist;
        
        llist = getObject(copy.offset/PAGE_SIZE);
        
        if(llist != NULL){
            mutex_lock(&list_lock);
            kfree(llist->k_virtual_addr);
            //kfree(llist);
            llist->size = 0;
            llist->k_virtual_addr=NULL;
            mutex_unlock(&list_lock);
            printk("Freed offset(object ID) :%llu \n Exiting Delete \n" ,llist->objectId);
        }
       
        
    }
    else{    
        printk(KERN_ERR "copy_from_user failed in delete \n");    
        return -EFAULT;
    }
    //object->size = 0;
    //object->k_virtual_addr = NULL;
    return 0;
}


//function to get size
__u64 getSize(__u64 inputOffset)
{
    printk("Starting getSize function. \n");   
    struct list_head *position;
    struct node *llist;
    __u64 size = 0;

    printk("Searching size for Offset -> %llu \n",inputOffset);

    llist = getObject(inputOffset);
    if(llist != NULL){
        size = llist->size;
    }
    return size;
}

/*
struct mutex getLock(__u64 inputOffset){
    printk("Starting getLock function");
    struct list_head *position;
    struct node *llist;

    printk("Searching lock for Offset -> %llu \n",inputOffset);

    list_for_each(position, &kernel_llist.list){
        llist = list_entry(position, struct node, list);
        if(llist->objectId == inputOffset){
            return &(llist->objLock);
         }
    }
    return NULL;
}*/

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