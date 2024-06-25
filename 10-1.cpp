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
    struct sembuf sb = {no, -1, 0};//���ź������е�no���źŽ��в��������ź����ļ���ֵ��1��
    int ret = semop(semid, &sb, 1);//��������P�������ź�������ʶ�����飬�����а���Ԫ�ظ�����
    if (ret == -1)
        ERR_EXIT("semop");
 
    return ret;
}
 
void wait_for_2fork(int no)
{
    int left = no;//��ѧ����ߵ������
    int right = (no + 1) % 5;//��ѧ���ұߵ������
 
    struct sembuf buf[2] =
    {
        {left, -1, 0},//�ź�����š��ź����ļ���ֵ��1
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
 
void philosopere(int no)//��ѧ����Ϊ
{
    srand(getpid());
    for (; ;)
    {
 
        printf("%d is thinking\n", no);//˼��
        sleep(DELAY);
        printf("%d is hungry\n", no);//��
        wait_for_2fork(no);//��ȡ����
        printf("%d is eating\n", no);//��ʳ
        sleep(DELAY);
        free_2fork(no);//�ͷŵ���
/* ����--
		int left = no;//��ѧ����ߵ������
		int right = (no + 1) % 5;//��ѧ���ұߵ������
        printf("%d is thinking\n", no);//˼��
        sleep(DELAY);
        printf("%d is hungry\n", no);//��
        wait_1fork(left);//��������ĵ�������������
		sleep(DELAY);
		wait_1fork(right);
        printf("%d is eating\n", no);//��ʳ
        sleep(DELAY);
        free_2fork(no);//�ͷŵ���
*/
    }
}
int main(void)
{
    semid = semget(IPC_PRIVATE, 5, IPC_CREAT | 0666);//����5�ѵ�����Դ�źż�
    if (semid == -1)
        ERR_EXIT("semget");
    union semun su;
    su.val = 1;
    int i;
    for (i = 0; i < 5; i++)//��5�ѵ��涼��Ϊ����״̬
    {
        semctl(semid, i, SETVAL, su);//���źż���ÿ���ź�����ʼֵ����Ϊ1����ʾ����
    }
 
    int no = 0;//��ʼ5����ѧ�Ҿ�����5�����̣�
    pid_t pid;
    for (i = 1; i < 5; i++)//����4���ӽ���
    {
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");
 
        if (pid == 0)//˵�����ӽ���
        {
            no = i;
            break;
        }
    }
    philosopere(no);//ÿ�������̣���ѧ�ҵ���Ϊ
    return 0;
}
