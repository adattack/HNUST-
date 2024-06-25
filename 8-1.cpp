#include <stdio.h>
#include <math.h>

#define MAX 9

typedef struct Disk {
    int share[MAX]; // ������ҵ
    int flag[MAX];  // ��¼�Ƿ���ʴ�����ҵ
} Disk;

// �������ҵ�����λ��ֵ
int max(int position, int disk[]) {
    int maxVal = -1;
    for (int i = 0; i < MAX; i++) {
        if (disk[i] > maxVal) {
            maxVal = disk[i];
        }
    }
    return maxVal > position ? maxVal : position;
}

// FIFO���̵���
void FIFO(int position, int fifo[]) {
	double sum=0;
    printf("FIFO (�Ӵŵ�%d����ʼ)\n", position);
    printf("��һ�������ʵĴŵ�\t���ŵ���\n");
    for (int i = 0; i < MAX; i++) {
    	int a=abs(position - fifo[i]);
        printf("%d\t\t\t%d\n", fifo[i], a);
        position = fifo[i];
        sum+=a;
    }
    printf("ƽ��Ѱ�����ȣ�%.1lf",sum/MAX);
    printf("\n");
}
// SSTF���̵���
void SSTF(int position,int SSTF[],int flag[])
{
	int min=0, temp=0, num=0;
	printf("SSTF (�Ӵŵ�%d����ʼ)\n",position);
	printf("��һ�������ʵĴŵ�\t���ŵ���\n");
	double sum=0;
	while( true )
	{	
		min = max( position,SSTF );
		for( int i=0; i<MAX; i++ )//ѡ���ƶ��������ٵĴ�����ҵ 
		{
			if( ( min > abs( position - SSTF[i] ) ) && flag[i] == 0 )//flag[i] == 0��ʾiλ��û�б����� 
			{
				min = abs( position - SSTF[i] );
				temp = i;
			}
		}
		flag[temp] = 1;//����Ѿ�ʹ�ù����λ�õĴ�����ҵ 
		printf("%d\t\t\t%d\n" ,SSTF[temp], abs(position-SSTF[temp]) );
		sum+= abs(position-SSTF[temp]) ;
		position = SSTF[temp];//��һ��λ�� 
		num++;
		if( num==MAX )//ִ����� 
		break;
	}
    printf("ƽ��Ѱ�����ȣ�%.1lf",sum/MAX);
	printf("\n");
}
// �򵥵�ð�������㷨
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

// SCAN���̵���
void SCAN(int position, int *SCAN) {
	double sum=0;
    int temp;
    sort_min_to_max(SCAN);
    printf("SCAN (�Ӵŵ�%d����ʼ����С����)\n", position);
    printf("��һ�������ʵĴŵ�\t���ŵ���\n");
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
    printf("ƽ��Ѱ�����ȣ�%.1lf",sum/MAX);
    printf("\n");
}

// C-SCAN���̵���
void C_SCAN(int position, int *C_SCAN) {
    int temp;
    double sum=0;
    sort_min_to_max(C_SCAN);
    printf("C-SCAN (�Ӵŵ�%d����ʼ,��С����)\n", position);
    printf("��һ�������ʵĴŵ�\t���ŵ���\n");
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
    printf("ƽ��Ѱ�����ȣ�%.1lf",sum/MAX);
    printf("\n");
}

int main() {
    int position;
    Disk d;
    do {
        for (int i = 0; i < MAX; i++) {
            d.flag[i] = 0;
        }
        
        printf("������ŵ���ʼλ�ã�");
        scanf("%d", &position);
        getchar(); // ����������Ļ��з�
        printf("����ŵ�����!\n\n", MAX);
        for (int i = 0; i < MAX; i++) {
            printf("�������%d���ŵ�����", i + 1);
            scanf("%d", &d.share[i]);
            getchar(); // ����������Ļ��з�
        }
        printf("\n");
        FIFO(position, d.share);
        SSTF(position, d.share, d.flag);
        SCAN(position, d.share);
        C_SCAN(position, d.share);
        
        printf("�Ƿ���Ҫ�������д��̵���ģ�⣿(����'y'�����������κμ��˳�)��");
    } while (getchar() == 'y');
    
    return 0;
}
