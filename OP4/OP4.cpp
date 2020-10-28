﻿// OP4.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#define process_number 20		//进程个数
HANDLE sema[2];					//定义两个信号量
struct PCB{
	int pid;					//进程标识符
	char state;					//'r':运行状态;'w':就绪状态;'b':阻塞状态
	int priority;				//进程优先级
	int arrivalTime;			//进程的创建时间
	int neededTime;				//进程需要的运行时间
	int usedTime;				//进程已累计运行时间
	int totalWaitTime;			//进程已等待的CPU时间总和
	struct PCB* next;
};

struct queue {
	int priority;				//该队列的优先级
	int timeSlice;				//该队列的时间片长度
	struct PCB *list;			//指向该队列中进程PCB链表的头部
};
struct atime {
	int i;						//进程名
	int at;						//达到时间
};
queue q[5];
//void insert(PCB*p,int j) {
//	PCB* temp = p;
//	int i;
//	p[i].next = &p[j];
//	p[j].next = temp->next;
//}
void generator(LPVOID s) {
	
	atime* tp = (atime*)s;
	//创建进程
	PCB* t, *p;
	int r1;	
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	srand(li.QuadPart);				//在短时间内生成大量随机数
	r1 = rand() % 198 + 2;
	t = (PCB*)malloc(sizeof(PCB));
	t->neededTime = r1;				//运行时间为随机数
	t->priority = 1;				//初始优先级为1
	t->state = 'w';
	t->pid=tp->i;
	
	t->usedTime = 0;
	t->totalWaitTime = 0;
	t->next = NULL;
	if (q[0].list == NULL) 
	{
		q[0].list = t;
		t->arrivalTime = tp->at;
		printf("生成器:进程%d已生成,需要的运行时间为%dms,到达时间为%dms\n", t->pid, t->neededTime, t->arrivalTime);
	}
	else {
		printf("生成器:等待%dms后,继续生成进程\n", tp->at);
		//
		p = q[0].list;
		while (p->next != NULL) {
			p = p->next;
		}
		t->arrivalTime = p->arrivalTime + tp->at;
		p->next = t;
		printf("生成器:进程%d已生成,需要的运行时间为%dms,到达时间为%dms\n", t->pid, t->neededTime, t->arrivalTime);
		ReleaseSemaphore(sema[0], 1, NULL);
	}
	t = q[0].list;
	while (t) {
		printf("%d\t", t->pid);
		t = t->next;
	}
	printf("\n");
	Sleep(20);
	//printf("生成器:等待20ms后,继续生成进程\n");
}
PCB* executor(int i) {
	PCB* p;
	p = q[i].list;
	if (p) {
		p->usedTime += q[i].timeSlice;
		Sleep(q[i].timeSlice);
		printf("运行:队列%d的进程%d消耗了时间片%dms\n", i, p->pid, q[i].timeSlice);
		while (p->next != NULL) {
			//睡眠结束后,所有队列中的进程的等待时间都要加上该时间片
			p = p->next;
			p->totalWaitTime += q[i].timeSlice;
		}
		//ReleaseSemaphore(sema[1], 1, NULL);
	}					
	return q[i].list;
}
void scheduler() {
	WaitForSingleObject(sema[0], NULL);
	int i=0,j,count=0;
	PCB* t,*p=NULL;
	t = q[0].list;
	if (t) {
		printf("%d->", t->pid);
		while (t->next != NULL) {
			t = t->next;
			printf("%d->", t->pid);
		}
		printf("%d\n", t->pid);
	}

	while(i<5){
		if (q[i].list != NULL) 
		{
			printf("id=%d\n", q[i].list->pid);
			p=executor(i);
			t = q[i].list;
			if (p && p->usedTime < p->neededTime)
			{
				t = q[i + 1].list;
				if (t) {
					while (t->next != NULL) {
						t = t->next;
					}
					p->next = t->next;				//当前进程下一个指向的PCB块设为空
					t->next = p;					//下一级队列尾部设为当前进程
					q[i].list = q[i].list->next;	//当前队列首部指向下一个进程
					//p->next = NULL;					
				}
				else {
					q[i + 1].list = p;					
					q[i].list = p->next;
				}
			
				printf("调度:进程%d被调度到了队列%d的末尾\n", p->pid, i + 1);
			}
			break;
		}
		i++;
	}
}


int main()
{
	HANDLE T[process_number];
	DWORD ID[process_number];
	int t[5] = { 10,20,40,80,160 };
	for (int i = 0; i < 5; i++) {
		q[i].priority = i + 1;
		q[i].timeSlice = t[i];
		q[i].list = NULL;
	}
	sema[0] = CreateSemaphore(NULL, 0, 0, NULL);
	sema[1] = CreateSemaphore(NULL, 0, 1, NULL);
	int r, i = 1;
	atime x;
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	srand(li.QuadPart);
	//srand((unsigned int)time(NULL));
	while (i < process_number+1) {
		if (i > 1) {
			r = rand() % 100 + 1;//[1,100]
			Sleep(r);
			x.at = r;
		}
		else {
			x.at = 0;
		}
		x.i = i;
		T[i-1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)generator, &x, 0, &ID[i-1]);

		scheduler();
		i++;
	}
	WaitForMultipleObjects(process_number,T,true, INFINITE);
	for (i = 0; i < 20; i++) {
		CloseHandle(T[i]);
	}
    return 0; 
}
//#include <stdio.h>  
//#include <stdlib.h>  
//#include <string.h>  
//typedef struct node   /*进程节点信息*/
//{
//	char name[20];   /*进程的名字*/
//	int prio;    /*进程的优先级*/
//	int round;    /*分配CPU的时间片*/
//	int cputime;   /*CPU执行时间*/
//	int needtime;   /*进程执行所需要的时间*/
//	char state;    /*进程的状态，W——就绪态，R——执行态，F——完成态*/
//	int count;    /*记录执行的次数*/
//	struct node* next;  /*链表指针*/
//}PCB;
//typedef struct Queue  /*多级就绪队列节点信息*/
//{
//	PCB* LinkPCB;   /*就绪队列中的进程队列指针*/
//	int prio;    /*本就绪队列的优先级*/
//	int round;    /*本就绪队列所分配的时间片*/
//	struct Queue* next;  /*指向下一个就绪队列的链表指针*/
//}ReadyQueue;
//PCB* run = NULL, * finish = NULL; /*定义三个队列，就绪队列，执行队列和完成队列*/
//ReadyQueue* Head = NULL; /*定义第一个就绪队列*/
//int num;     /*进程个数*/
//int ReadyNum;    /*就绪队列个数*/
//void Output();          /*进程信息输出函数*/
//void InsertFinish(PCB* in);       /*将进程插入到完成队列尾部*/
//void InsertPrio(ReadyQueue* in);     /*创建就绪队列，规定优先数越小，优先级越低*/
//void PrioCreate();         /*创建就绪队列输入函数*/
//void GetFirst(ReadyQueue* queue);     /*取得某一个就绪队列中的队头进程*/
//void InsertLast(PCB* in, ReadyQueue* queue);   /*将进程插入到就绪队列尾部*/
//void ProcessCreate();        /*进程创建函数*/
//void RoundRun(ReadyQueue* timechip);    /*时间片轮转调度算法*/
//void MultiDispatch();        /*多级调度算法，每次执行一个时间片*/
//
//int main(void)
//{
//	PrioCreate(); /*创建就绪队列*/
//	ProcessCreate();/*创建就绪进程队列*/
//	MultiDispatch();/*算法开始*/
//	Output();  /*输出最终的调度序列*/
//	return 0;
//}
//void Output()  /*进程信息输出函数*/
//{
//	ReadyQueue* print = Head;
//	PCB* p;
//	printf("进程名/t优先级/t轮数/tcpu时间/t需要时间/t进程状态/t计数器/n");
//	while (print)
//	{
//		if (print->LinkPCB != NULL)
//		{
//			p = print->LinkPCB;
//			while (p)
//			{
//				printf("%s/t%d/t%d/t%d/t%d/t/t%c/t/t%d/n", p->name, p->prio, p->round, p->cputime, p->needtime, p->state, p->count);
//				p = p->next;
//			}
//		}
//		print = print->next;
//	}
//	p = finish;
//	while (p != NULL)
//	{
//		printf("%s/t%d/t%d/t%d/t%d/t/t%c/t/t%d/n", p->name, p->prio, p->round, p->cputime, p->needtime, p->state, p->count);
//		p = p->next;
//	}
//	p = run;
//	while (p != NULL)
//	{
//		printf("%s/t%d/t%d/t%d/t%d/t/t%c/t/t%d/n", p->name, p->prio, p->round, p->cputime, p->needtime, p->state, p->count);
//		p = p->next;
//	}
//
//
//}
//void InsertFinish(PCB* in)  /*将进程插入到完成队列尾部*/
//{
//	PCB* fst;
//	fst = finish;
//
//	if (finish == NULL)
//	{
//		in->next = finish;
//		finish = in;
//	}
//	else
//	{
//		while (fst->next != NULL)
//		{
//			fst = fst->next;
//		}
//		in->next = fst->next;
//		fst->next = in;
//	}
//}
//void InsertPrio(ReadyQueue* in)  /*创建就绪队列，规定优先数越小，优先级越低*/
//{
//	ReadyQueue* fst, * nxt;
//	fst = nxt = Head;
//
//	if (Head == NULL)    /*如果没有队列，则为第一个元素*/
//	{
//		in->next = Head;
//		Head = in;
//	}
//	else       /*查到合适的位置进行插入*/
//	{
//		if (in->prio >= fst->prio)  /*比第一个还要大，则插入到队头*/
//		{
//			in->next = Head;
//			Head = in;
//		}
//		else
//		{
//			while (fst->next != NULL)  /*移动指针查找第一个别它小的元素的位置进行插入*/
//			{
//				nxt = fst;
//				fst = fst->next;
//			}
//
//			if (fst->next == NULL)  /*已经搜索到队尾，则其优先级数最小，将其插入到队尾即可*/
//			{
//				in->next = fst->next;
//				fst->next = in;
//			}
//			else     /*插入到队列中*/
//			{
//				nxt = in;
//				in->next = fst;
//			}
//		}
//	}
//}
//void PrioCreate() /*创建就绪队列输入函数*/
//{
//	ReadyQueue* tmp;
//	int i;
//
//	printf("输入就绪队列的个数：/n");
//	scanf_s("%d", &ReadyNum);
//
//	printf("输入每个就绪队列的CPU时间片：/n");
//	for (i = 0; i < ReadyNum; i++)
//	{
//		if ((tmp = (ReadyQueue*)malloc(sizeof(ReadyQueue))) == NULL)
//		{
//			perror("malloc");
//			exit(1);
//		}
//		scanf_s("%d", &(tmp->round));  /*输入此就绪队列中给每个进程所分配的CPU时间片*/
//		tmp->prio = 50 - tmp->round;  /*设置其优先级，时间片越高，其优先级越低*/
//		tmp->LinkPCB = NULL;    /*初始化其连接的进程队列为空*/
//		tmp->next = NULL;
//		InsertPrio(tmp);     /*按照优先级从高到低，建立多个就绪队列*/
//	}
//}
//void GetFirst(ReadyQueue* queue)     /*取得某一个就绪队列中的队头进程*/
//{
//	run = queue->LinkPCB;
//
//	if (queue->LinkPCB != NULL)
//	{
//		run->state = 'R';
//		queue->LinkPCB = queue->LinkPCB->next;
//		run->next = NULL;
//	}
//}
//void InsertLast(PCB* in, ReadyQueue* queue)  /*将进程插入到就绪队列尾部*/
//{
//	PCB* fst;
//	fst = queue->LinkPCB;
//
//	if (queue->LinkPCB == NULL)
//	{
//		in->next = queue->LinkPCB;
//		queue->LinkPCB = in;
//	}
//	else
//	{
//		while (fst->next != NULL)
//		{
//			fst = fst->next;
//		}
//		in->next = fst->next;
//		fst->next = in;
//	}
//}
//void ProcessCreate() /*进程创建函数*/
//{
//	PCB* tmp;
//	int i;
//
//	printf("输入进程的个数：/n");
//	scanf_s("%d", &num);
//	printf("输入进程名字和进程所需时间：/n");
//	for (i = 0; i < num; i++)
//	{
//		if ((tmp = (PCB*)malloc(sizeof(PCB))) == NULL)
//		{
//			perror("malloc");
//			exit(1);
//		}
//		scanf_s("%s", tmp->name);
//		getchar();        /*吸收回车符号*/
//		scanf_s("%d", &(tmp->needtime));
//		tmp->cputime = 0;
//		tmp->state = 'W';
//		tmp->prio = 50 - tmp->needtime;  /*设置其优先级，需要的时间越多，优先级越低*/
//		tmp->round = Head->round;
//		tmp->count = 0;
//		InsertLast(tmp, Head);      /*按照优先级从高到低，插入到就绪队列*/
//	}
//}
//void RoundRun(ReadyQueue* timechip)    /*时间片轮转调度算法*/
//{
//
//	int flag = 1;
//
//	GetFirst(timechip);
//	while (run != NULL)
//	{
//		while (flag)
//		{
//			run->count++;
//			run->cputime++;
//			run->needtime--;
//			if (run->needtime == 0) /*进程执行完毕*/
//			{
//				run->state = 'F';
//				InsertFinish(run);
//				flag = 0;
//			}
//			else if (run->count == timechip->round)/*时间片用完*/
//			{
//				run->state = 'W';
//				run->count = 0;   /*计数器清零，为下次做准备*/
//				InsertLast(run, timechip);
//				flag = 0;
//			}
//		}
//		flag = 1;
//		GetFirst(timechip);
//	}
//}
//void MultiDispatch()   /*多级调度算法，每次执行一个时间片*/
//{
//	int flag = 1;
//	int k = 0;
//
//	ReadyQueue* point;
//	point = Head;
//
//	GetFirst(point);
//	while (run != NULL)
//	{
//		Output();
//		if (Head->LinkPCB != NULL)
//			point = Head;
//		while (flag)
//		{
//			run->count++;
//			run->cputime++;
//			run->needtime--;
//			if (run->needtime == 0) /*进程执行完毕*/
//			{
//				run->state = 'F';
//				InsertFinish(run);
//				flag = 0;
//			}
//			else if (run->count == run->round)/*时间片用完*/
//			{
//				run->state = 'W';
//				run->count = 0;   /*计数器清零，为下次做准备*/
//				if (point->next != NULL)
//				{
//					run->round = point->next->round;/*设置其时间片是下一个就绪队列的时间片*/
//					InsertLast(run, point->next);  /*将进程插入到下一个就绪队列中*/
//					flag = 0;
//				}
//				else
//				{
//					RoundRun(point);   /*如果为最后一个就绪队列就调用时间片轮转算法*/
//					break;
//				}
//			}
//			++k;
//			if (k == 3)
//			{
//				ProcessCreate();
//			}
//		}
//		flag = 1;
//		if (point->LinkPCB == NULL)/*就绪队列指针下移*/
//			point = point->next;
//		if (point->next == NULL)
//		{
//			RoundRun(point);
//			break;
//		}
//		GetFirst(point);
//	}
//}
// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
