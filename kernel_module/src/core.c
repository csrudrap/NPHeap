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

struct lock_node {
    unsigned long object_id;
    struct mutex lock;
    struct lock_node* next;
    unsigned long size;
    void* data;
};

extern struct miscdevice npheap_dev;
extern struct lock_node* head;
extern struct lock_node *find_obj(unsigned long object_id);
extern struct mutex list_lock;


// Project 1: Chinmay Rudrapatna, csrudrap; Rakshit Holkal Ravishankar, rhravish; Shishir Nagendra, sbnagend;

/*
*   Authors: Chinmay Rudrapatna, Rakshit Holkal Ravishankar, Shishir Nagendra
*   Purpose: To allocate data for the requested size and remap the allocated kernel virtual address to the user space.
*/
int npheap_mmap(struct file *filp, struct vm_area_struct *vma)
{
    // Iterate through the extern list to get to the node with object ID equal to vma->pg_off. The node should exist from locking.
    // If the data pointer is not null, simply use the data pointer and remap using the data pointer.
    // If the data pointer is null, remap using a new kmalloced pointer.
    unsigned long size = (vma->vm_end) - (vma->vm_start);
    mutex_lock(&list_lock);
    struct lock_node* node = find_obj(vma->vm_pgoff);
    if (!node)
    {
        // Object should be found because a node should be created for it from the locking function.
        // If it is not found, the user program has not attempted to lock this object, return an error.
        mutex_unlock(&list_lock);
        return EPERM;
    } 
    mutex_unlock(&list_lock);
    void* data_alloc;
    if (node->data == NULL)
    {
        data_alloc = kmalloc(size, GFP_KERNEL);
        if (data_alloc == NULL)
        {
            // kmalloc call was unsuccessful. Return.
            return ENOMEM;
        }
        if (node->size != 0)
        {
            // Has to be zero if node->data is NULL. 
            // Either both have to be initialized or both have to be uninitialized.
            kfree(data_alloc);
            return ENOMEM;
        }
        node->size = size;
        node->data = data_alloc;
    }
    else
    {
        if (size != node->size)
        {
            return EINVAL;
        }
        data_alloc = node->data;
    }
    if (remap_pfn_range(vma, vma->vm_start, virt_to_phys(data_alloc) >> PAGE_SHIFT, size, vma->vm_page_prot))
    {
        // The remap was not successful. Free and return error code.
        kfree(data_alloc);
        node->size = 0;
        node->data = NULL;
        return EAGAIN;
    }
    return 0;
}

int npheap_init(void)
{
    int ret;
    if ((ret = misc_register(&npheap_dev)))
        printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
    else
        printk(KERN_ERR "\"npheap\" misc device installed\n");
    return ret;
}

void npheap_exit(void)
{
    misc_deregister(&npheap_dev);
}

