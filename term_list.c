#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/delay.h>


#define PSW_THREAD_NUM 4

MODULE_LICENSE( "GPL" );

struct psw_list
{
	int value;
	struct psw_list *next;
};

struct psw_param
{
	int index;
	struct psw_list *head;
};
void psw_list_add(int value, struct psw_list *head)
{
	struct psw_list *cur;

	cur = head;

	struct psw_list *new = (struct psw_list*)kmalloc(sizeof(struct psw_list), GFP_KERNEL);
	new->value = value;
	new->next = NULL;
	/* First add function call*/
	if(head == NULL)
	{
		head->value = value;
		head->next = NULL;
		return;
	}
	else
	{
		/* Find position to add */
		while(cur->next != NULL)
		{
			cur = cur->next;
		}
	}
	cur->next = new;
	return;
}

void psw_list_delete(struct psw_list *head)
{
	struct psw_list *cur = head;

	while(head != NULL)
	{
		cur = head;
		head = head->next;
		kfree(cur);
	}
	return;
}

int __psw_list_traverse(void *_arg)
{
	struct psw_param *arg = (struct psw_param *)_arg;
	if(arg == NULL)
	{
		printk("asd\n");
		return 0;
	}
	int i, k;
	struct psw_list *cur = arg->head;
	for(i = 0; i < arg->index; i++)
	{
		cur = cur->next;
	}
	
	while(cur != NULL)
	{
//		printk("traverse %d : %d\n", arg->index, cur->value);
		for(k = 0; k < PSW_THREAD_NUM; k++)
		{

			cur = cur->next;
			if(cur == NULL)
			{
				break;
			}
		}
	}
	kfree(arg);
	return 0;
}

void psw_list_traverse(struct psw_list *head)
{
	int i = 0;
	for(i = 0; i < PSW_THREAD_NUM; i++)
	{		
		struct psw_param *arg = (struct psw_param *)kmalloc(sizeof(struct psw_param), GFP_KERNEL);
		arg->index = i;
		arg->head = head;
		kthread_run(&__psw_list_traverse, (void *)arg, "__psw_list_traverse");
	}
}
int __init term_list_init(void)
{
	int i = 0;
	int num = 99999;
	struct psw_list *head;
	head = (struct psw_list *) kmalloc (sizeof(struct psw_list), GFP_KERNEL);
	head->value = 0;
	head->next = NULL;
	long long unsigned start, end;
	start = ktime_get();
	for(i = 0; i < num;i++)
	{
		psw_list_add(i, head);
	}
	end = ktime_get();
	printk("insert time = %llu\n", end - start);
	start = ktime_get();
	psw_list_traverse(head);
	end = ktime_get();
	printk("Travel time = %llu\n", end - start);

	start = ktime_get();
	psw_list_delete(head);
	end = ktime_get();
	printk("delete time = %llu\n", end - start);
	return 0;
}

void __exit term_list_exit(void)
{
	printk("term_list_exit\n");
}

module_init(term_list_init);
module_exit(term_list_exit);

