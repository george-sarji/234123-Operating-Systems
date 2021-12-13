#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/sched.h>

asmlinkage long sys_hello(void)
{
    printk("Hello, World!\n");
    return 0;
}

asmlinkage int sys_get_weight(void)
{
    return current->weight;
}

asmlinkage pid_t sys_get_heaviest_ancestor(void)
{
    // We have to iterate through the parents and return a maximum.
    int max = current->weight;
    pid_t heaviest = current->pid;
    struct task_struct* current_task = current->parent;
    while (current_task->pid != 0)
    {
        if (max < current_task->weight)
        {
            // New max. Set.
            max = current_task->weight;
            heaviest = current_task->pid;
        }
        current_task = current_task->parent;
    }

    return heaviest;
}

asmlinkage int sys_set_weight(int weight){

    if (weight < 0 ){
        return -EINVAL;
    }
    current->weight = weight;
    return 0;
}

asmlinkage int sys_get_leaf_children_sum(void){

}