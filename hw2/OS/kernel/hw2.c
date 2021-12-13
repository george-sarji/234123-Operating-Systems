#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/sched.h>

asmlinkage long sys_hello(void)
{
    printk("Hello, World!\n");
    return 0;
}

asmlinkage long sys_get_weight(void)
{
    return current->weight;
}

asmlinkage pid_t sys_get_heaviest_ancestor(void)
{
    // We have to iterate through the parents and return a maximum.
    int max = current->weight;
    pid_t heaviest = current->pid;
    struct task_struct *current_task = current->parent;
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

asmlinkage long sys_set_weight(int weight)
{
    if (weight < 0)
    {
        return -EINVAL;
    }
    current->weight = weight;
    return 0;
}

int get_children_weights(struct task_struct *root)
{
    struct task_struct *task;
    struct list_head *list;
    int sum = root->weight;

    list_for_each(list, &root->children)
    {
        task = list_entry(list, struct task_struct, sibling);
        sum += get_children_weights(task);
    }
    return sum;
}

asmlinkage long sys_get_leaf_children_sum(void)
{
    return get_children_weights(current);
}