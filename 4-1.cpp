#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <stdlib.h> 
#include <unistd.h>
#define MSGKEY 75
struct msgform
{
long mtype;
char mtext[300];
}msg;
int msgqid,i;
void CLIENT()
{
 int i,result;
 msgqid=msgget(MSGKEY,0777);
 if(msgqid == -1) 
 { 
 perror("client msgget"); 
 exit(1); 
 }
 for (i=0;i<10;i++)
 { 
printf("(client) sent \n");
result = msgsnd(msgqid,&msg,300,0);
 if(result == -1) 
 { 
 perror("client msgsnd"); 
 exit(1); 
 }
 }
 exit(0);
}
void SERVER()
{
 int i;
 int getsize = 0;
 msgqid=msgget(MSGKEY,0777);
 if(msgqid == -1) 
 { 
 perror("server msgget"); 
 exit(1); 
 }
 for (i=0;i<10;i++)
 {
 getsize = msgrcv(msgqid,&msg,300,0,0);
printf("(Server) recieved %d bytes\n",getsize);
 } ;
 msgctl(msgqid,IPC_RMID,0);
 exit(0);
}
int main()
{
 msgqid=msgget(MSGKEY,0777|IPC_CREAT);//777是权限控制bit，IPC_creat表示创建
 if(msgqid == -1) 
 { 
perror("main msgget");
 }
 msg.mtype = 1;//必须是非负数，否则报错
 while((i=fork())==-1);
 if(!i) SERVER();
 while((i=fork())==-1);
 if(!i) CLIENT();
 wait(0);
 wait(0);
}