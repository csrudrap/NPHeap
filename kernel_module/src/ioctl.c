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
#include <asm/page.h>
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

// Project 1: Chinmay Rudrapatna, csrudrap; Rakshit Holkal Ravishankar, rhravish; Shishir Nagendra, sbnagend;

// If exist, return the data.

struct lock_node {
	unsigned long object_id; 
	struct mutex lock;
	struct lock_node* next;
    unsigned long size;
    void* data;
};

struct lock_node* head;
struct lock_node* tail;
DEFINE_MUTEX(list_lock);

/*
*   Authors: Chinmay Rudrapatna, Rakshit Holkal Ravishankar, Shishir Nagendra
*   Purpose: To find an object for a given offset.
*/
struct lock_node* find_obj(unsigned long offset)
{
    struct lock_node* temp = head;
    while (temp != NULL && (temp -> object_id) != offset)
    {
        temp = temp -> next;
    }
    return temp;
}

/*
*   Authors: Chinmay Rudrapatna, Rakshit Holkal Ravishankar, Shishir Nagendra
*   Purpose: To create an object for the associated offset.
*/
struct lock_node* create_obj(unsigned long object_id)
{
    if (head == NULL)
    {
        head = (struct lock_node*) kmalloc(sizeof(struct lock_node), GFP_KERNEL);
        head->next = NULL;
        head->data = NULL;
        head->size = 0;
        head->object_id = object_id;
        mutex_init(&(head->lock));
        tail = head;
        return head;
    }
    else
    {
        struct lock_node* new_node = (struct lock_node*) kmalloc(sizeof(struct lock_node), GFP_KERNEL);
        tail->next = new_node;
        tail = new_node;
        new_node->next = NULL;
        new_node->data = NULL;
        new_node->size = 0;
        new_node->object_id = object_id;
        mutex_init(&(new_node->lock));
        return new_node;
    }
}

/*
*   Authors: Chinmay Rudrapatna, Rakshit Holkal Ravishankar, Shishir Nagendra
*   Purpose: To free the data associated with the passed object. The object must exist.
*/
void delete_obj(struct lock_node* obj)
{
    if (obj && obj->data)
    {
        kfree(obj->data);
        obj->data = NULL;
        obj->size = 0;
    }
}

/*
*   Authors: Chinmay Rudrapatna, Rakshit Holkal Ravishankar, Shishir Nagendra
*   Purpose: To look for an object and lock it if found, create and lock it otherwise.
*/
long npheap_lock(struct npheap_cmd __user *user_cmd)
{
	struct npheap_cmd kernel_mapped_cmd;
    if (copy_from_user(&kernel_mapped_cmd, user_cmd, sizeof(struct npheap_cmd)))
    {
        return EINVAL; 
    }
    mutex_lock(&list_lock);
    struct lock_node* obj = find_obj(kernel_mapped_cmd.offset / PAGE_SIZE);
    if (obj != NULL)
    {   
        mutex_unlock(&list_lock);
        mutex_lock(&(obj->lock));
        return 0;
    }
    else 
    {
        obj = create_obj(kernel_mapped_cmd.offset / PAGE_SIZE);
        if (obj == NULL)
        {
            // This could be because kmalloc failed, return an error.
            mutex_unlock(&list_lock);
            return ENOMEM;
        }
    }
    mutex_unlock(&list_lock);
    mutex_lock(&(obj->lock));
    return 0;
}     

/*
*   Authors: Chinmay Rudrapatna, Rakshit Holkal Ravishankar, Shishir Nagendra
*   Purpose: To look for an object and unlock it if found.
*/
long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
	struct npheap_cmd kernel_mapped_cmd;
    if (copy_from_user(&kernel_mapped_cmd, user_cmd, sizeof(struct npheap_cmd)))
    {
        return EINVAL; 
    }
    mutex_lock(&list_lock);
    struct lock_node* obj = find_obj(kernel_mapped_cmd.offset / PAGE_SIZE);
    if (obj)
    {
        mutex_unlock(&list_lock);
        mutex_unlock(&(obj->lock));
        return 0;
    }
    mutex_unlock(&list_lock);
    return EPERM; 
}

/*
*   Authors: Chinmay Rudrapatna, Rakshit Holkal Ravishankar, Shishir Nagendra
*   Purpose: To get the size of the data allocated for the given offset.
*/
long npheap_getsize(struct npheap_cmd __user *user_cmd)
{   
	struct npheap_cmd kernel_mapped_cmd;
    if (copy_from_user(&kernel_mapped_cmd, user_cmd, sizeof(struct npheap_cmd)))
    {
        return EINVAL; 
    }
    struct lock_node* obj = find_obj(kernel_mapped_cmd.offset / PAGE_SIZE);
    if (obj)
    {
        return obj->size;
    }
    return EINVAL; 
}

/*
*   Authors: Chinmay Rudrapatna, Rakshit Holkal Ravishankar, Shishir Nagendra
*   Purpose: To free the memory previously allocated for the data associated with the object for a given offset.
*/
long npheap_delete(struct npheap_cmd __user *user_cmd)
{
    struct npheap_cmd kernel_mapped_cmd;
    if (copy_from_user(&kernel_mapped_cmd, user_cmd, sizeof(struct npheap_cmd)))
    {
        return EINVAL; 
    }
    mutex_lock(&list_lock);
    struct lock_node* obj = find_obj(kernel_mapped_cmd.offset / PAGE_SIZE);
    if (obj && obj->data)
    {
        mutex_unlock(&list_lock);
        delete_obj(obj);
    }
    else
    {
        mutex_unlock(&list_lock);
        return EPERM; 
    }
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
