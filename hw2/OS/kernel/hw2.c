#include <linux/kernel.h>

asmlinkage long sys_hello(void)
{
    printk("Hello, World!\n");
    return 0;
}

asmlinkage int sys_get_weight(void)
{
    return current->weight;
}

void getHeaviestWeight(struct task_struct *current, int *max, pid_t *heaviest)
{
    
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