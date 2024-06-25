#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/wait.h>
 
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while(0)
 
union semun
{
    int val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};
 
int semid;
 
#define DELAY (rand() % 5 + 1)
 
int wait_1fork(int no)//
{
    struct sembuf sb = {no, -1, 0};//对信号量集中第no个信号进行操作，对信号量的计数值减1，
    int ret = semop(semid, &sb, 1);//用来进行P操作（信号量集标识，数组，数组中包含元素个数）
    if (ret == -1)
        ERR_EXIT("semop");
 
    return ret;
}
 
void wait_for_2fork(int no)
{
    int left = no;//哲学家左边刀叉号码
    int right = (no + 1) % 5;//哲学家右边刀叉号码
 
    struct sembuf buf[2] =
    {
        {left, -1, 0},//信号量序号、信号量的计数值减1
        {right, -1, 0}
    };
 
    semop(semid, buf, 2);
}
 
void free_2fork(int no)
{
    int left = no;
    int right = (no + 1) % 5;
 
    struct sembuf buf[2] =
    {
        {left, 1, 0},
        {right, 1, 0}
    };
 
    semop(semid, buf, 2);
}
 
void philosopere(int no)//哲学家行为
{
    srand(getpid());
    for (; ;)
    {
 
        printf("%d is thinking\n", no);//思考
        sleep(DELAY);
        printf("%d is hungry\n", no);//饿
        wait_for_2fork(no);//获取刀叉
        printf("%d is eating\n", no);//进食
        sleep(DELAY);
        free_2fork(no);//释放刀叉
/* 死锁--
		int left = no;//哲学家左边刀叉号码
		int right = (no + 1) % 5;//哲学家右边刀叉号码
        printf("%d is thinking\n", no);//思考
        sleep(DELAY);
        printf("%d is hungry\n", no);//饿
        wait_1fork(left);//看到空余的刀叉立刻拿起来
		sleep(DELAY);
		wait_1fork(right);
        printf("%d is eating\n", no);//进食
        sleep(DELAY);
        free_2fork(no);//释放刀叉
*/
    }
}
int main(void)
{
    semid = semget(IPC_PRIVATE, 5, IPC_CREAT | 0666);//创建5把刀叉资源信号集
    if (semid == -1)
        ERR_EXIT("semget");
    union semun su;
    su.val = 1;
    int i;
    for (i = 0; i < 5; i++)//将5把刀叉都置为可用状态
    {
        semctl(semid, i, SETVAL, su);//将信号集中每个信号量初始值都置为1，表示可用
    }
 
    int no = 0;//开始5个哲学家就坐（5个进程）
    pid_t pid;
    for (i = 1; i < 5; i++)//创建4个子进程
    {
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");
 
        if (pid == 0)//说明是子进程
        {
            no = i;
            break;
        }
    }
    philosopere(no);//每个（进程）哲学家的行为
    return 0;
}
