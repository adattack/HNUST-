#include <stdio.h>
#include <math.h>

#define MAX 9

typedef struct Disk {
    int share[MAX]; // 磁盘作业
    int flag[MAX];  // 记录是否访问磁盘作业
} Disk;

// 求磁盘作业的最大位置值
int max(int position, int disk[]) {
    int maxVal = -1;
    for (int i = 0; i < MAX; i++) {
        if (disk[i] > maxVal) {
            maxVal = disk[i];
        }
    }
    return maxVal > position ? maxVal : position;
}

// FIFO磁盘调度
void FIFO(int position, int fifo[]) {
	double sum=0;
    printf("FIFO (从磁道%d处开始)\n", position);
    printf("下一个被访问的磁道\t横跨磁道数\n");
    for (int i = 0; i < MAX; i++) {
    	int a=abs(position - fifo[i]);
        printf("%d\t\t\t%d\n", fifo[i], a);
        position = fifo[i];
        sum+=a;
    }
    printf("平均寻道长度：%.1lf",sum/MAX);
    printf("\n");
}
// SSTF磁盘调度
void SSTF(int position,int SSTF[],int flag[])
{
	int min=0, temp=0, num=0;
	printf("SSTF (从磁道%d处开始)\n",position);
	printf("下一个被访问的磁道\t横跨磁道数\n");
	double sum=0;
	while( true )
	{	
		min = max( position,SSTF );
		for( int i=0; i<MAX; i++ )//选出移动距离最少的磁盘作业 
		{
			if( ( min > abs( position - SSTF[i] ) ) && flag[i] == 0 )//flag[i] == 0表示i位置没有被访问 
			{
				min = abs( position - SSTF[i] );
				temp = i;
			}
		}
		flag[temp] = 1;//标记已经使用过这个位置的磁盘作业 
		printf("%d\t\t\t%d\n" ,SSTF[temp], abs(position-SSTF[temp]) );
		sum+= abs(position-SSTF[temp]) ;
		position = SSTF[temp];//下一个位置 
		num++;
		if( num==MAX )//执行完毕 
		break;
	}
    printf("平均寻道长度：%.1lf",sum/MAX);
	printf("\n");
}
// 简单的冒泡排序算法
void sort_min_to_max(int *b) {
    for (int i = 0; i < MAX - 1; i++) {
        for (int j = i + 1; j < MAX; j++) {
            if (b[i] > b[j]) {
                int v = b[i];
                b[i] = b[j];
                b[j] = v;
            }
        }
    }
}

// SCAN磁盘调度
void SCAN(int position, int *SCAN) {
	double sum=0;
    int temp;
    sort_min_to_max(SCAN);
    printf("SCAN (从磁道%d处开始，从小到大)\n", position);
    printf("下一个被访问的磁道\t横跨磁道数\n");
    for (int i = 0; i < MAX; i++) {
        if (SCAN[i] > position) {
            temp = i;
            break;
        }
    }
    for (int i = temp; i < MAX; i++) {
    	int a=abs(position - SCAN[i]);
        printf("%d\t\t\t%d\n", SCAN[i], a);
        position = SCAN[i];
        sum+=a;
    }
    for (int i = temp - 1; i >= 0; i--) {
    	int a=abs(position - SCAN[i]);
        printf("%d\t\t\t%d\n", SCAN[i], a);
        position = SCAN[i];
        sum+=a;
    }
    printf("平均寻道长度：%.1lf",sum/MAX);
    printf("\n");
}

// C-SCAN磁盘调度
void C_SCAN(int position, int *C_SCAN) {
    int temp;
    double sum=0;
    sort_min_to_max(C_SCAN);
    printf("C-SCAN (从磁道%d处开始,从小到大)\n", position);
    printf("下一个被访问的磁道\t横跨磁道数\n");
    for (int i = 0; i < MAX; i++) {
        if (position < C_SCAN[i]) {
            temp = i;
            break;
        }
    }
    for (int i = temp; i < MAX; i++) {
    	int a=abs(position - C_SCAN[i]);
        printf("%d\t\t\t%d\n", C_SCAN[i], a);
        position = C_SCAN[i];
        sum+=a;
    }
    for (int i = 0; i < temp; i++) {
    	int a=abs(position - C_SCAN[i]);
        printf("%d\t\t\t%d\n", C_SCAN[i], a);
        position = C_SCAN[i];
        sum+=a;
    }
    printf("平均寻道长度：%.1lf",sum/MAX);
    printf("\n");
}

int main() {
    int position;
    Disk d;
    do {
        for (int i = 0; i < MAX; i++) {
            d.flag[i] = 0;
        }
        
        printf("请输入磁道开始位置：");
        scanf("%d", &position);
        getchar(); // 清除缓冲区的换行符
        printf("输入磁道请求!\n\n", MAX);
        for (int i = 0; i < MAX; i++) {
            printf("请输入第%d个磁道请求：", i + 1);
            scanf("%d", &d.share[i]);
            getchar(); // 清除缓冲区的换行符
        }
        printf("\n");
        FIFO(position, d.share);
        SSTF(position, d.share, d.flag);
        SCAN(position, d.share);
        C_SCAN(position, d.share);
        
        printf("是否需要继续进行磁盘调度模拟？(输入'y'继续，其他任何键退出)：");
    } while (getchar() == 'y');
    
    return 0;
}
