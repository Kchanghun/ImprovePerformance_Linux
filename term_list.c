#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <linux/completion.h>

#define TEAM14_THREAD_NUM 4

MODULE_LICENSE( "GPL" );
/*for checking all thread is done*/
struct completion thread_done;
/* Linked List Structure  */
struct team14_list
{
	int value;
	struct team14_list *next;
};
/* kthread_run Parameter Structure  */
struct team14_param
{
	int index;
	struct team14_list *head;
};
/* List Insertion Function */
void team14_list_add(int value, struct team14_list *head)
{
	struct team14_list *cur;

	cur = head;

	/* Assign value  */
	struct team14_list *new = (struct team14_list*)kmalloc(sizeof(struct team14_list), GFP_KERNEL);
	new->value = value;

	/* Insert next to head */
	new->next = head->next;
	head->next = new;
	
	return ;
}

/* Delete Function  */
void team14_list_delete(struct team14_list *head)
{
	struct team14_list *cur = head;

	/* Delete Starting from head  */
	while(head != NULL)
	{
		cur = head;
		head = head->next;
		kfree(cur);
	}
	return;
}

/* Actual Traversing Function  */
int __team14_list_traverse(void *_arg)
{
	struct team14_param *arg = (struct team14_param *)_arg;
	if(arg == NULL)
	{
		return 0;
	}
	int i, k;

	struct team14_list *cur = arg->head;
	for(i = 0; i < arg->index; i++)
	{
		cur = cur->next;
	}
	
	while(cur != NULL)
	{
	//	printk("traverse %d : %d\n", arg->index, cur->value);
		/* Move to next "THREAD_NUM" times */
		for(k = 0; k < TEAM14_THREAD_NUM; k++)
		{

			cur = cur->next;
			if(cur == NULL)
			{
				break;
			}
		}
	}
	if(arg->index==(TEAM14_THREAD_NUM-1)){
		complete_and_exit(&thread_done,0);
	}
	kfree(arg);
	return 0;
}
/* Multithreading traverse function  */
void team14_list_traverse(struct team14_list *head)
{
	int i = 0;
	for(i = 0; i < TEAM14_THREAD_NUM; i++)
	{		
		struct team14_param *arg = (struct team14_param *)kmalloc(sizeof(struct team14_param), GFP_KERNEL);
		arg->index = i;
		arg->head = head;
		kthread_run(&__team14_list_traverse, (void *)arg, "__team14_list_traverse");
	}
	wait_for_completion(&thread_done);
}
int __init term_list_init(void)
{
	int i = 0;
	int num = 99999;
	struct team14_list *head;
	head = (struct team14_list *) kmalloc (sizeof(struct team14_list), GFP_KERNEL);
	head->value = 0;
	head->next = NULL;
	init_completion(&thread_done);
	long long unsigned start, end;
	start = ktime_get();
	for(i = 0; i < num;i++)
	{
		team14_list_add(i, head);
	}
	end = ktime_get();
	printk("insert time = %llu\n", end - start);
	start = ktime_get();
	team14_list_traverse(head);
	end = ktime_get();
	printk("Travel time = %llu\n", end - start);

	start = ktime_get();
	team14_list_delete(head);
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

