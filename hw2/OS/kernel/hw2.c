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
    // Check if current weight is bigger than max.
    if (current->weight > *max)
    {
        // Set the max.
        *max = current->weight;
        *heaviest = current->pid;
    }

    // Check if we have a parent.
    if (current->parent->pid != 0)
    {
        // Get the next parent.
        getHeaviestWeight(current->parent, max, heaviest);
    }
}

asmlinkage pid_t sys_get_heaviest_ancestor(void)
{
    // We have to iterate through the parents and return a maximum.
    int max = 0;
    pid_t heaviest = current->pid;
    getHeaviestWeight(current, &max, &heaviest);
    return heaviest;
}