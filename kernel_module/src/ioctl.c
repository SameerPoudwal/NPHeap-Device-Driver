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
#include "core.c"

// If exist, return the data.

// mutex that will be shared among the threads
// static DEFINE_MUTEX(k_mutex); 

long npheap_lock(struct npheap_cmd __user *user_cmd)
{
    printk("Calling npheap_lock function.");
    struct mutex *obj_lock;
    //obj_lock = getMutex((__u64) user_cmd->offset); 
    struct npheap_cmd copy;
    if(copy_from_user(&copy, (void __user *)user_cmd, sizeof(struct npheap_cmd)))
        obj_lock = getMutex(copy.offset);
    else        
        return -EFAULT;

    mutex_lock(&obj_lock);
    return 0;
}     

long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
    printk("Calling npheap_unlock function.");
    struct mutex *obj_lock;
    //obj_lock = getMutex((__u64) user_cmd->offset); 
    struct npheap_cmd copy;
    if(copy_from_user(&copy, (void __user *)user_cmd, sizeof(struct npheap_cmd)))
        obj_lock = getMutex(copy.offset);
    else        
        return -EFAULT;

    mutex_unlock(&obj_lock);
    return 0;
}

long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
    printk("Calling npheap_getsize function.");
    struct node *object;
    struct npheap_cmd copy;
    if(copy_from_user(&copy, (void __user *)user_cmd, sizeof(struct npheap_cmd)))
        object = getObject(copy.offset);
    else        
        return -EFAULT;

    //object = getObject((__u64) user_cmd->offset);
    return (long) object->size;
}


long npheap_delete(struct npheap_cmd __user *user_cmd)
{
    printk("Calling npheap_delete function.");
    struct node *object;
    struct npheap_cmd copy;
    if(copy_from_user(&copy, (void __user *)user_cmd, sizeof(struct npheap_cmd)))
        object = getObject(copy.offset);
    else        
        return -EFAULT;

    object->size = 0;
    object->k_virtual_addr = NULL;
    return 0;
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
