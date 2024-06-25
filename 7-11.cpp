#include<stdio.h>
#define a 100000
#define m 4
#define n 3
int C[m][n];	//C为最大需求矩阵
int A[m][n],B[m][n];	//A为当前已分配资源矩阵，B为尚需的资源矩阵
int R[n],V[n],S[a]={0};	//V为可利用资源向量Available，R为Resource向量 ,S为每种资源的已使用量
int Test(int Ai[m][n],int Bi[m][n],int Vi[n],int x[n],int flag1,int p){		//这里数组Ai、Bi、Ci只是试探，并不能分配资源
	int b[a]={0};//b为标志数组，0、1区分,1代表此进程已分配资源并完成任务
	//b[p]=1;int flag=1;
    int i,j,k,flag=0,c=0,w=0; //flag为完成任务的进程数，w为分配时是否有进程满足条件，c为测试是否每一列都满足所需资源量小于等于可利用资源量
    if(flag1==1) { b[p]=1;  flag=1; }       //若一个进程申请的资源等于尚需的资源，那么得到资源后就会紧接着完成任务释放资源，不必要再对该进程进行扫描
	while(flag!=m){		//没有全部进程都实现任务，不存在安全序列
     	for(i=0;i<m;i++){
      		 if(b[i]==0){
         		 for(j=0;j<n;j++){
    		  		 if(Bi[i][j]>Vi[j]){		//if-else试探
    		      		 break;			//当前所需资源量大于可利用资源量，不能分配资源，直接退出循环
		     		}
		      		 else{
		         		 c++;				//测试是否每一列都满足所需资源量小于等于可利用资源量
		    		 }
   				}
   	   		     if(c==n)  {
   	 	  			 w=1;			//该进程（该行)满足条件，w置为1
    	   			 flag++;		//已经实现任务的进程数目加1
          			 b[i]=1;		//将该进程标志为1，下次检索时跳过此行
          			 if(flag1==0){
                        for(j=0;j<n;j++)
                            Vi[j]=Vi[j]+Ai[i][j]+x[j]; //一个进程完成任务切记要释放资源！
          			 }
          			 else{
                        for(j=0;j<n;j++)
                            Vi[j]=Vi[j]+Ai[i][j];   //一个进程完成任务释放资源
          			 }
           }
      }
   			if(flag==m)  break; //所有进程都完成任务，退出循环，避免进入无限循环
			c=0; 		//这一行测试完毕，测试下一行是否每一列都满足所需资源量小于等于可利用资源量，重新置为0
  }
      if(w==0)  break;		//w在遍历所有行后并没有置为1，说明检索后可利用资源量不能让这些进程中的任意一个完成任务，直接退出，该状态不安全
                            //这样可以不按顺序完成进程，因为遍历一次有进程可以执行w置为1，就不会退出while循环
   	  w=0;		           //测试下一行，将w置为0
}
  if(flag==m)  {
  	printf("试探分配后系统状态安全\n");
  	return 1;
  }
  else  {
  	printf("试探分配后系统状态不安全，不分配资源\n");
  	return 0;
  }
}


void apply(int A[m][n],int B[m][n],int V[n],int p){
	int i,c,j,flag1=1;	//flag1代表申请分配的资源数是否小于尚需的资源数，若申请分配的资源数是大于等于尚需的资源数，则分配后进程完成任务可以释放，小于分配后不能释放
	int x[n];		//进程所需的各资源的数量
	int V1[n];     //传入Test()中的向量V1，与向量V相同，因为数组V是全局变量，所以如果在Test()中修改V，apply()输出的V也会改变，但Test()只是试探分配，并不是真的分配，所以设置V1
	for(i=0;i<3;i++){
		printf("资源");
		printf("%d：",i);
		printf("%d",B[p][i]);
		x[i]=B[p][i];
	}
    if(flag1==1){
            for(i=0;i<n;i++){
                V[i]=V[i]+A[p][i];		//申请分配的资源数是大于等于尚需的资源数，则分配后进程完成任务释放
                //printf("%d",V[i]);
                V1[i]=V[i];
                		}
    }
    else{
            for(i=0;i<n;i++){
                V[i]=V[i]-x[i];			//申请分配的资源数小于尚需的资源数，则分配后进程完成任务不能释放 ，可利用资源量减少
                B[p][i]=B[p][i]-x[i];     //先试探分配给申请进程资源，因为分配了，所以进程尚需的资源数（数组B）减少
                V1[i]=V[i];
        }
    }
	c=Test(A,B,V1,x,flag1,p);
	if(c==1){
		for(i=0;i<n;i++){
			if(flag1==1){
				A[p][i]=0;
				B[p][i]=0;
			}
			else{
				A[p][i]=A[p][i]+x[i];
			}
	}
}
	if(c==0){
		for(i=0;i<n;i++){
			if(flag1==1){
				V[i]=V[i]-A[p][i];
			}
			else{
				V[i]=V[i]+x[i];
				B[p][i]=B[p][i]+x[i];
			}
	}
}
	printf("当前已分配资源矩阵为：\n");
	for(i=0;i<m;i++){
		printf("进程");
     	printf("%d：",i);
		for(j=0;j<n;j++){
			printf("%d ",A[i][j]);
			if(j==n-1) printf("\n");
		}
	}
	printf("各进程还需要的资源量为：\n");
	for(i=0;i<m;i++){
		printf("进程");
        printf("%d：",i);
		for(j=0;j<n;j++){
			printf("%d ",B[i][j]);
			if(j==n-1) printf("\n");
		}
	}
	printf("当前可利用资源量为：\n");
		for(j=0;j<n;j++){
			printf("%d ",V[j]);
			if(j==n-1) printf("\n");
		}

}


int main (void)
{
	int p;	//m表示进程数P，n表示资源数R
	char ch;
    int i,j,k;	//flag为完成任务的进程数
    printf("请输入最大需求矩阵，即每个进程所需的最大资源数\n");
    for(i=0;i<m;i++){
      printf("进程");
      printf("%d：",i);
      for(j=0;j<n;j++){
        scanf("%d",&C[i][j]);
    }
  }
    printf("请输入当前已分配矩阵\n");
    for(i=0;i<m;i++){
      printf("进程");
      printf("%d：",i);
      for(j=0;j<n;j++){
        scanf("%d",&A[i][j]);
    }
 }
    for(i=0;i<m;i++){
      for(j=0;j<n;j++){
        B[i][j]=C[i][j]-A[i][j];
    }
 }
    for(j=0;j<n;j++){
        for(i=0;i<m;i++){
            S[j]=A[i][j]+S[j];	//计算每一列的和，即每种资源的已使用量
        }
    }
    printf("请输入Resource向量\n");
    for(i=0;i<n;i++){
        scanf("%d",&R[i]);
    }
    for(i=0;i<n;i++){
        V[i]=R[i]-S[i];
    }
    printf("请输入需要申请资源的进程号：进程");
    scanf("%d",&p);
    apply(A,B,V,p);
    printf("是否想要继续动态分配资源（输入'y'继续，输入'n'退出）：");
    getchar(); 			//回车 
    scanf("%c",&ch);
    while(ch=='y'){
			printf("请输入需要申请资源的进程号：");
    		scanf("%d",&p);
    		apply(A,B,V,p);
    		printf("是否想要继续动态分配资源（输入'y'继续，输入'n'退出）：");
    		getchar();
    		scanf("%c",&ch);
	}
	return 0;
}



