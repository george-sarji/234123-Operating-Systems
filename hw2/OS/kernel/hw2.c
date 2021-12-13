#include <linux/kernel.h>

asmlinkage long sys_hello(void)
{
    printk("Hello, World!\n");
    return 0;
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