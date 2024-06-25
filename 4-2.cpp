#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMKEY 75                   /* 定义共享区关键词 */
void SERVER(int *addr) {
    for (;;) {
        *addr = -1; // 服务器发送信号
        while (*addr == -1) { // 等待客户端接收
            // 这里可以添加一些延时，比如 sleep(1)，以避免活锁
        }
        printf("(server) received\n"); // 打印服务器接收信息
    }
}

void CLIENT(int *addr) {
    for (int i = 9; i >= 0; --i) {
        while (*addr != -1); // 等待服务器发送信号
        printf("(client) sent\n"); // 打印客户端发送信息
        *addr = i; // 客户端发送数据
    }
    exit(0);
}
int main() {
    int shmid, i;
    int *addr;

    // 创建共享区
    shmid = shmget(SHMKEY, 1024, 0777 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // 将共享区附加到进程地址空间
    addr = (int*) shmat(shmid, NULL, 0); // 进行类型转换
    if (addr == (int*) -1) { // 检查地址是否成功附加
        perror("shmat");
        exit(1);
    }

    // 启动服务器和客户端进程
    if (fork() == 0) {
        SERVER(addr); // 传递共享区地址给服务器
        // 服务器退出后，不需要执行下面的代码
        exit(0);
    }

    if (fork() == 0) {
        CLIENT(addr); // 传递共享区地址给客户端
        // 客户端退出后，不需要执行下面的代码
        exit(0);
    }

    // 父进程等待子进程结束
    wait(NULL);
    wait(NULL);

    // 从系统中删除共享区
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        exit(1);
    }

    return 0;
}

