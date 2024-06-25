#include <iostream>
#include <string>
using namespace std;
//假设一个磁盘的每个物理块大小为512个字节
struct MFD   // 16个用户
{
	string username;
	string password;
	//int pufd; //用户目录文件所在的物理块
	struct USER_UFD   * next;  //指向用户目录
};

struct UFD  //一个用户可以用16个文件夹，然后每个文件夹下可以有16个文件
{
	struct file_message
	{
		string filename;
		int protect_code; //保护码
		int length; //文件长度
		int addr; //存放该文件的物理块的第一个的快号
	}ufd[16];
	string directname; //用户目录名
	int cur_file_size = 0; //不能在结构体内附初始值。好像不能成功
};
UFD user_dir = { {}, "", 0 };

struct UOF  //假设一个用户最多打开16个文件
{
	struct uof
	{
		string filename;
		int pointer; //文件的读写指针,其实就是文件的大小
		int protect_code; //2表示可读可写,1表示可读不可写， 0表示不可读不可写
		int addr; //存放文件的第一个磁盘块号
	}uof[16];
	int cur_openfilesize = 0;
};

struct fat  //文件分配表   用一块物理块存放，那么最多可以记录64块数据块的信息。
{
	int next = -1; //下一个磁盘块号 
	int used = 0; //1表示被使用，
//	int self; //自己的快号
}fat[64];

struct USER_UFD  
{
	struct UFD direct[16]; //每个用户最多有16个目录
	int cur_user_direct_size = 0; //当前用户的目录数
};

struct USER_UFD cur_user_all_direct_array[16];  //16个用户的所有目录的对象
int max_usersize = 16;
int max_userfilesize = 16;
int max_openfilesize = 16;
struct MFD  cur_user; //当前用户
//struct UFD cur_user_dirtect[16]; //用户目录
struct UOF * cur_opentable; //当前文件打开表
char *fdisk; //虚拟磁盘的起始位置
struct UOF openfile[16]; //每一个用户对应一个文件打开表对象，只能设置为全局变量
MFD mfd[16]; //16个用户
int cur_user_size = 0; //记录当前用户的人数
string path; //记录当前用户的路径

//函数声明
int open(string name);
void register_user();
void remove(string);
int create(string);
int open(string);
int close(string);
int del(string);
int read(string);
int write(string, char *, int);
void dir();
int login();
void cd();
void mkdir(string);
void input_operation();
void register_user();
void remove(string name);


int create(string name)
{
	//cout << cur_user.next->cur_file_size << endl;
	//system("pause");
	int index; //标识当前目录在direct数组中第几个
	for (index = 0; index < cur_user.next->cur_user_direct_size; index++)
	{
		if (path == cur_user.next->direct[index].directname)
		{
			break;
		}
	}
	int i;
	for (i = 0; i < cur_user.next->direct[index].cur_file_size; i++)   //遍历当前目录，查看是否有文件重名
	{
		if (name == cur_user.next->direct[index].ufd[i].filename)
			break;
	}
	if (i < cur_user.next->direct[index].cur_file_size) //判断文件名是否重复
	{
		cout << "文件名重复" << endl;
		return -1;
	}
	if (cur_user.next->direct[index].cur_file_size == 16)  //判断当前目录的文件到达16个
	{
		cout << "用户文件已经达到16个" << endl;
		return -1;
	}
	int j;
	for (j = 0; j < 64; j++)  //判断是否有空的空闲块。
	{
		if (fat[j].used == 0)
			break;
	}
	if (j >= 64)
	{
		cout << "磁盘没有空闲块了" << endl;
		return -1;
	}
	cur_user.next->direct[index].ufd[cur_user.next->direct[index].cur_file_size].filename = name;
	cur_user.next->direct[index].ufd[cur_user.next->direct[index].cur_file_size].addr = j; //文件起始盘块号
	cur_user.next->direct[index].ufd[cur_user.next->direct[index].cur_file_size].length = 0;
	cur_user.next->direct[index].ufd[cur_user.next->direct[index].cur_file_size].protect_code = 2; //表示可读可写
	cur_user.next->direct[index].cur_file_size++;//用户文件数量加1
	fat[j].used = 1; //被使用
	fat[j].next = -1; //只是个空文件,所有没有后序的块
	//写入文件打开表中,就是调用open（）
	cout << "文件创建成功" << endl;
	int fd = open(name);
	
	return fd;
}

int open(string name)
{
	int index; //标识当前目录在direct数组中第几个
	for (index = 0; index < cur_user.next->cur_user_direct_size; index++)
	{
		if (path == cur_user.next->direct[index].directname)
		{
			break;
		}
	}
	int i;
	for (i = 0; i < cur_user.next->direct[index].cur_file_size; i++) //当前目录有没有这个文件，没有就自然不能打开
	{
		if (name == cur_user.next->direct[index].ufd[i].filename)
			break;
	}
	if (i > cur_user.next->direct[index].cur_file_size)
	{
		cout << "i的值为" << i << endl;
		cout << "该用户没有这个文件" << endl;
		return -1;
	}
	if (cur_opentable->cur_openfilesize == max_openfilesize) //如果打开文件的数量达到最大值，那么就无法打开
	{
		cout << "文件打开数量已经达到最大值" << endl;
		return -1;
	}
	for (int j = 0; j < cur_opentable->cur_openfilesize; j++) //如果文件已经打开，就无需打开
	{
		if (cur_opentable->uof[j].filename == name)
		{
			cout << "文件已经打开" << endl;
			return -1;
		}
	}
	int k;
	for (k = 0; k < cur_user.next->direct[index].cur_file_size; k++) //找到要打开的文件在文件数组中的第几个
	{
		if (cur_user.next->direct[index].ufd[k].filename == name)
			break;
	}
	cur_opentable->uof[cur_opentable->cur_openfilesize].filename = name;
	cur_opentable->uof[cur_opentable->cur_openfilesize].protect_code = cur_user.next->direct[index].ufd[k].protect_code;
	cur_opentable->uof[cur_opentable->cur_openfilesize].pointer = cur_user.next->direct[index].ufd[k].length;
	cur_opentable->uof[cur_opentable->cur_openfilesize].addr = cur_user.next->direct[index].ufd[k].addr;
	cur_opentable->cur_openfilesize++; //文件打开数量加1
	cout << "文件打开成功" << endl;
	return k; //返回文件在文件打开表中的第几项


}

int close(string name)
{
	int fd;
	for (int i = 0; i < cur_opentable->cur_openfilesize; i++)  //找到要关闭的文件在表中的第几项
	{
		if (cur_opentable->uof[i].filename == name)
		{
			fd = i;
			break;
		}
	}
	if (fd >= cur_opentable->cur_openfilesize)
	{
		cout << "没有这个文件或者文件没有打开" << endl;
		return -1;
	}
	//将要删除的项目与最后一个项目交换,因为是数组存放
	cur_opentable->uof[fd].filename = cur_opentable->uof[cur_opentable->cur_openfilesize - 1].filename;
	cur_opentable->uof[fd].pointer = cur_opentable->uof[cur_opentable->cur_openfilesize - 1].pointer;
	cur_opentable->uof[fd].protect_code = cur_opentable->uof[cur_opentable->cur_openfilesize - 1].protect_code;
	cur_opentable->uof[fd].addr = cur_opentable->uof[cur_opentable->cur_openfilesize - 1].addr;
	//cur_opentable->cur_openfilesize--;
	cur_opentable->cur_openfilesize--;
	cout << "文件关闭成功" << endl;
	return 0;

}

int del(string name) {
    int index;
    for (index = 0; index < cur_user.next->cur_user_direct_size; index++) {
        if (path == cur_user.next->direct[index].directname) {
            break;
        }
    }
    int i;
    for (i = 0; i < cur_user.next->direct[index].cur_file_size; i++) {
        if (cur_user.next->direct[index].ufd[i].filename == name) {
            break;
        }
    }
    if (i >= cur_user.next->direct[index].cur_file_size) {
        cout << "没有这个文件" << endl;
        return -1;
    }

    int j;
    for (j = 0; j < cur_opentable->cur_openfilesize; j++) {
        if (cur_opentable->uof[j].filename == name) {
            break;
        }
    }
    if (j < cur_opentable->cur_openfilesize) {
        cout << "这个文件被打开了，请先关闭" << endl;
        return -1;
    }

    // 更新当前用户目录下文件数组信息
    fat[cur_user.next->direct[index].ufd[i].addr].used = 0; // 没有使用
    cur_user.next->direct[index].ufd[i] = cur_user.next->direct[index].ufd[cur_user.next->direct[index].cur_file_size - 1];
    cur_user.next->direct[index].cur_file_size--; // 用户文件数量减1

    // 回收磁盘
    int temp = fat[cur_user.next->direct[index].ufd[i].addr].next;
    while (temp != -1) {
        fat[temp].used = 0;
        temp = fat[temp].next;
    }
    cout << "删除文件成功" << endl;
    return 0;
}


int read(string name)
{
	int index1; //标识当前目录在direct数组中第几个
	for (index1 = 0; index1 < cur_user.next->cur_user_direct_size; index1++)
	{
		if (path == cur_user.next->direct[index1].directname)
		{
			break;
		}
	}
	int a;
	for (a = 0; a < cur_user.next->direct[index1].cur_file_size; a++)    //判断文件是否存在
	{
		if (cur_user.next->direct[index1].ufd[a].filename == name)
			break;
	}
	if (a >= cur_user.next->direct[index1].cur_file_size)
	{
		cout << "没有这个文件" << endl;
		return -1;
	}
	int i;
	//判读文件是否打开
	for (i = 0; i < cur_opentable->cur_openfilesize; i++)
	{
		if (cur_opentable->uof[i].filename == name)
			break;
	}
	if (i >= cur_opentable->cur_openfilesize)
	{
		cout << "文件没有打开， 无法读取" << endl;
		return -1;
	}
	int fd = i; //获取文件描述字
	//判断读文件的合法性
	if (cur_opentable->uof[fd].protect_code == 0) //我们创建的文件都是默认可读可写的。。。
	{
		cout << "文件不可读" << endl;
		return -1;
	}
	else
	{
		int len = cur_opentable->uof[fd].pointer; //文件的长度
		int block_size = len / 512; //磁盘的个数
		int offset = len % 512; //偏移量
		if (offset != 0)
			block_size++;
		//如果我用一个文件表示磁盘的引导块，用另一个文件表示磁盘的数据块，那么我们计算文件的起始位置就不用加上磁盘的引导块了吧。。
		//关于文件的存放文件，char *fdisk表示一整个磁盘，然后不同文件的内容存放在这个指针所指向的不同字符段
		char * first = fdisk + cur_opentable->uof[fd].addr * 512; //文件的起始地址
		char * buf = (char *)malloc(512 * sizeof(char)); //缓冲区
		cout << "文件的内容为 :";
		for (int k = 0; k < block_size; k++)
		{
			if (k == block_size - 1)  //则是最后一个磁盘块
			{
				for (int j = 0; j < len - k * 512; j++)  //赋值文件剩余的字符，其实就是偏移量
				{
					buf[j + k * 512] = first[j];
				}
				for (int u = 0; u < len - k * 512; u++)
				{
					cout << buf[u + k * 512]; //输出剩余长度，之所以这样输出，printf()，将整个buf的内容全部输出，如果没有读满就出现乱码啦。。
				}
			}
			else //不在最后一个磁盘块，也就是在其他已经读满的磁盘块
			{
				for (int j = 0; j < len - i * 512; j++)
					buf[j + k * 512] = first[j]; //缓冲区读满就输出内容
				printf("%s\n", buf); //输出文件的内容
				int next_block = fat[cur_opentable->uof[fd].addr].next; //读完一个磁盘块后，在接着读下一个磁盘块
				first = fdisk + next_block * 512;
			}
		}
		cout << endl;
		cout << "文件读取成功" << endl;
		free(buf); //释放缓冲区
		return 0;
	}

}


int write(string name, char * buf, int len)
{
	int index1; //标识当前目录在direct数组中第几个
	for (index1 = 0; index1 < cur_user.next->cur_user_direct_size; index1++)
	{
		if (path == cur_user.next->direct[index1].directname)
		{
			break;
		}
	}
	int i;
	//判读文件是否打开
	for (i = 0; i < cur_opentable->cur_openfilesize; i++)
	{
		if (cur_opentable->uof[i].filename == name)
			break;
	}
	if (i >= cur_opentable->cur_openfilesize)
	{
		cout << "文件没有打开， 无法读取" << endl;
		return -1;
	}
	int fd = i; //获取文件描述字
	//判断读文件的合法性
	if (cur_opentable->uof[fd].protect_code != 2)
	{
		cout << "文件不可写" << endl;
		return -1;
	}
	else
	{
		int temp; //保存当前所写的文件在用户文件目录表的第几项，为了后面修改文件的大小
		int first_block = cur_opentable->uof[fd].addr; //用户文件存放的第一个磁盘块
		for (int k = 0; k < cur_user.next->direct[index1].cur_file_size; k++)
		{
			if (cur_user.next->direct[index1].ufd[k].addr == first_block)
			{
				temp = k;
				break;
			}
		}
		//追加写
		//找到该文件存放的最后一个磁盘块
		while (fat[first_block].next != -1)
		{
			first_block = fat[first_block].next;
		}
		//计算该文件存放的最后一个地址
		char  * first;
		first = fdisk + first_block * 512 + cur_opentable->uof[fd].pointer % 512;
		//如果最后一个文件剩下的空间大于要写入的长度
		if (len <= 512 - cur_opentable->uof[fd].pointer % 512)
		{
			//strcpy(first, buf);  这句代码出现问题，可能是由于buf没有读满，后面的值被访问了，非法！
			for (int i = 0; i < len; i++)
			{
				first[i] = buf[i];//将缓冲区的内容写入虚拟磁盘中
			}
			cur_opentable->uof[fd].pointer = cur_opentable->uof[fd].pointer + len;  //更新文件打开表
			cur_user.next->direct[index1].ufd[temp].length = cur_user.next->direct[index1].ufd[temp].length + len; //更新用户目录文件表
		}
		else  //如果剩下的空间不足写入
		{
			for (i = 0; i < 512 - cur_opentable->uof[fd].pointer % 512; i++)
			{ //写入一部分的内容到最后一个磁盘块的剩余空间
				first[i] = buf[i];
			}
			//计算分配完最后一个磁盘的剩余空间后，还剩下多少字节没有存储
			int last_size = len - (512 - cur_opentable->uof[fd].pointer % 512);
			int need_block_size = last_size / 512;
			int need_offset_size = last_size % 512;
			if (need_offset_size > 0)
				need_block_size++; //总共需要这么磁盘块
			int unused_block_size = 0; //记录没有使用过的磁盘块的个数
			//int first_unused_block; //记录第一个没有被使用过的磁盘
			int flag = 0;
			for (int i = 0; i < 64; i++)
			{
				if (fat[i].used == 0)
				{
					unused_block_size++;
				}
			}
			if (unused_block_size < need_block_size)
			{
				cout << "磁盘没有空间存放了-=-=" << endl;
				return -1;
			}
			else
			{
				int item = cur_opentable->uof[fd].addr;
				for (int p = 0; p < need_block_size; p++) //执行多次寻找空闲磁盘的操作，
				{
					for (int i = 0; i < 64; i++)
					{
						if (fat[i].used == 0) //没有被使用
						{
							first = fdisk + i * 512; //当前要写入的磁盘块的起始地址
							fat[i].used = 1; //标记被使用
							fat[item].next = i; //标记下一个磁盘
							item = i;
							break;
						}
					}
					if (p == need_block_size - 1)
					{
						for (int k = 0; k < need_offset_size; k++)  //将文件的偏移量写入最后一个文件中
							first[k] = buf[k];
						//更新最后一个磁盘块的next值
						fat[i].next = -1;
					}
					else  //如果不是最后一个空闲块
					{ //待解决问题，就是如果更新fat的next值
						for (int k = 0; k < 512; k++)
							first[k] = buf[k];
					}
				}
				cur_opentable->uof[fd].pointer = cur_opentable->uof[fd].pointer + last_size;  //更新文件打开表
				cur_user.next->direct[index1].ufd[temp].length = cur_user.next->direct[index1].ufd[temp].length + last_size; //更新用户目录文件表
			}
		}
		cout << "文件写入成功" << endl;
		return 0;
	}
}

void dir()
{
	int index1; //标识当前目录在direct数组中第几个
	for (index1 = 0; index1 < cur_user.next->cur_user_direct_size; index1++)
	{
		if (path == cur_user.next->direct[index1].directname)
		{
			break;
		}
	}
	if (path == "") //表示此时路径在用户的目录表，显示文件目录
	{
		cout << "\t" << "目录名" << endl;
		for (int i = 0; i < cur_user.next->cur_user_direct_size; i++)
		{
			cout << "\t" << cur_user.next->direct[i].directname << endl;
		}
	}
	else  //显示目录下的文件
	{
		cout << "\t" << "文件名" << "\t" << "文件保护码" << "\t" << "文件长度" << "\t" <<"文件起始盘块号" << endl;
		for (int i = 0; i < cur_user.next->direct[index1].cur_file_size; i++)  //输出文件的信息
		{
			cout << "\t" << cur_user.next->direct[index1].ufd[i].filename << "\t" << cur_user.next->direct[index1].ufd[i].protect_code << "\t" << "\t" << cur_user.next->direct[index1].ufd[i].length <<"\t" << "\t" << cur_user.next->direct[index1].ufd[i].addr  << endl;
		}
	}
}

// void dir() {
//     int index1; // 标识当前目录在direct数组中的索引
//     for (index1 = 0; index1 < cur_user.next->cur_user_direct_size; index1++) {
//         if (path == cur_user.next->direct[index1].directname) {
//             break;
//         }
//     }
    
//     if (index1 < cur_user.next->cur_user_direct_size) { // 确保目录存在
//         // 如果当前路径是根目录，则列出所有目录
//         if (path == "") {
//             cout << "目录列表：" << endl;
//             for (int i = 0; i < cur_user.next->cur_user_direct_size; i++) {
//                 cout << cur_user.next->direct[i].directname << endl;
//             }
//         } else { // 否则，列出当前目录下的文件和子目录
//             cout << "当前目录下的文件和子目录：" << endl;
//             for (int i = 0; i < cur_user.next->direct[index1].cur_file_size; i++) {
//                 cout << cur_user.next->direct[index1].ufd[i].filename 
//                      << " [文件] 长度: " << cur_user.next->direct[index1].ufd[i].length 
//                      << " 保护码: " << cur_user.next->direct[index1].ufd[i].protect_code 
//                      << " 起始块号: " << cur_user.next->direct[index1].ufd[i].addr << endl;
//             }
//             // 列出子目录（如果有的话）
//             // 这里假设子目录信息以某种方式存储，并且可以通过类似方式访问
//             // 例如，可以添加一个类似的结构来跟踪子目录
//         }
//     }
// }
int login()
{
	string name;
	string password;
	//UFD user;
	cout << "请输入你的姓名" << endl;
	cin >> name;
	cout << "请输入你的密码" << endl;
	cin >> password;
	int i;
	for ( i = 0; i < cur_user_size; i++)
	{
		if (mfd[i].username == name && mfd[i].password == password)
		{
			break;
		}
	}
	if (i >= cur_user_size)
	{
		cout << "没有这个用户或者用户名密码错误" << endl;
		return -1;
	}
	
	mfd[i].next = & (cur_user_all_direct_array[i]); //用户指向自己的所有目录的结构

	//初始化当前用户的信息
	cur_user = mfd[i];
    cur_user.next->cur_user_direct_size = mfd[i].next->cur_user_direct_size; //
	

	cur_opentable = &openfile[cur_user_size]; //指针指向文件打开表对象
	cur_opentable->cur_openfilesize = 0; //设初始值
	path = ""; //指定当前路径为用户的全部目录处	
    cur_user_size++; //用户人数++
	return 1;
}

void cd()
{
	string temp_path;
	cin >> temp_path;
	if (temp_path == "..")
	{
		path = "";
		return;
	}
	
	int i;
	for (i = 0; i < cur_user.next->cur_user_direct_size; i++)  //判断path是否存在
	{
		if (temp_path == cur_user.next->direct[i].directname)
			break;
	}
	if (i >= cur_user.next->cur_user_direct_size)
	{
		cout << "没有这个目录" << endl;
		return;
	}
	path = temp_path;
	return;
	
}

void show_files() {
    // 寻找当前工作路径在用户目录数组中对应的索引
    int index1;
    for (index1 = 0; index1 < cur_user.next->cur_user_direct_size; index1++) {
        if (path == cur_user.next->direct[index1].directname) {
            break;
        }
    }
    // 检查是否找到对应的目录
    if (index1 < cur_user.next->cur_user_direct_size) { // 确保目录存在
        cout << "当前目录下的文件有：" << endl;
        for (int i = 0; i < cur_user.next->direct[index1].cur_file_size; i++) {
            // 打印每个文件的详细信息
            cout << "文件名: " << cur_user.next->direct[index1].ufd[i].filename 
                 << ", 长度: " << cur_user.next->direct[index1].ufd[i].length
                 << ", 保护码: " << cur_user.next->direct[index1].ufd[i].protect_code
                 << ", 起始块号: " << cur_user.next->direct[index1].ufd[i].addr << endl;
        }
    } else {
        cout << "当前路径不存在或没有文件。" << endl;
    }
}

// void show_files() {
//     int index1; // 标识当前目录在direct数组中第几个
//     for (index1 = 0; index1 < cur_user.next->cur_user_direct_size; index1++) {
//         if (path == cur_user.next->direct[index1].directname) {
//             break;
//         }
//     }

//     if (index1 < cur_user.next->cur_user_direct_size) { // 确保目录存在
//         cout << "当前目录下的文件有：" << endl;
//         for (int i = 0; i < cur_user.next->direct[index1].cur_file_size; i++) {
//             cout << cur_user.next->direct[index1].ufd[i].filename << " ";
//         }
//         cout << endl;
//     } else {
//         cout << "当前路径不存在或没有文件。" << endl;
//     }
// }

// void show_files() {
//     // 寻找当前工作路径在用户目录数组中对应的索引
// 	system("ps");
// }
// void show_files() {
//     // 寻找当前工作路径在用户目录数组中对应的索引
//     int index1;
//     for (index1 = 0; index1 < cur_user.next->cur_user_direct_size; index1++) {
//         if (path == cur_user.next->direct[index1].directname) {
//             break;
//         }
//     }

//     // 检查是否找到对应的目录
//     if (index1 < cur_user.next->cur_user_direct_size) { // 确保目录存在
//         cout << "当前目录下的文件有：" << endl;
//         for (int i = 0; i < cur_user.next->direct[index1].cur_file_size; i++) {
//             // 打印每个文件的详细信息
//             cout << cur_user.next->direct[index1].ufd[i].filename 
//                  << " [长度: " << cur_user.next->direct[index1].ufd[i].length 
//                  << ", 保护码: " << cur_user.next->direct[index1].ufd[i].protect_code 
//                  << ", 起始块号: " << cur_user.next->direct[index1].ufd[i].addr << "]" << endl;
//         }
//     } else {
//         cout << "当前路径不存在或没有文件。" << endl;
//     }
// }
void mkdir(string name)
{
	if (cur_user.next->cur_user_direct_size == 16)
	{
		cout << "用户目录已经达到最大值，不能在创建目录了" << endl;
		return;
	}
	int i;
	for (i = 0; i < cur_user.next->cur_user_direct_size; i++)  //判断创建的目录是否存在
	{
		if (cur_user.next->direct[i].directname == name)
			break;
	}
	if (i < cur_user.next->cur_user_direct_size)
	{
		cout << "该目录已经存在了" << endl;
		return;
	}

	cur_user.next->direct[cur_user.next->cur_user_direct_size].directname = name;
	cur_user.next->direct[cur_user.next->cur_user_direct_size].cur_file_size = 0; //新创建的目录里面的文件个数为0
	cur_user.next->cur_user_direct_size++; //用户的目录数加1

	cout << "创建目录成功" << endl;
	return;
	
}
void input_operation()  //用户输入命令
{
    cout << "******************************************************" << endl;
	cout << "\t" << "\t" << "命令" << "\t" << "\t" << "功能" << endl;
	cout << "\t" << "\t" << "register" << "\t" << "注册" << endl;
	cout << "\t" << "\t" << "login" << "\t" << "\t" << "登录" << endl;
	cout << "\t" << "\t" << "create" << "\t" << "\t" << "创建文件" << endl;
	cout << "\t" << "\t" << "mkdir" << "\t" << "\t" << "创建目录" << endl;
    cout << "\t" << "\t" << "dir" << "\t" << "\t" << "显示当前位置的文件" << endl;
	cout << "\t" << "\t" << "cd" << "\t" << "\t" << "切换目录（根目录..）" << endl;
	cout << "\t" << "\t" << "open" << "\t" << "\t" << "打开文件" << endl;
	cout << "\t" << "\t" << "close" << "\t" << "\t" << "关闭文件" << endl;
	cout << "\t" << "\t" << "read" << "\t" << "\t" << "读文件" << endl;
	cout << "\t" << "\t" << "write" << "\t" << "\t" << "写文件" << endl;
	cout << "\t" << "\t" << "del" << "\t" << "\t" << "删除文件" << endl;
	cout << "\t" << "\t" << "exit" << "\t" << "\t" << "退出系统" << endl;
	cout << "*****************************************************" << endl;
	if (cur_user.username == "")
		cout << "localhost :";
	else
		cout << cur_user.username << "@localhost  home/" <<path << "：";
	string operation;

	cin >> operation;
	if (operation == "login")
	{
		login();
	}
	else if (operation == "dir")
		dir();
	else if (operation == "create")
	{
		string filename;
		cin >> filename;
		create(filename);
	}
	else if (operation == "del")
	{
		string filename;
		cin >> filename;
		del(filename);
	}
	else if (operation == "open")
	{
		string name;
		cin >> name;
		open(name);
	}
	else if (operation == "close")
	{
		string name;
		cin >> name;
		close(name);
	}
	else if (operation == "read")
	{
		string name;
		cin >> name;
		read(name);
	}
	else if (operation == "write")
	{
		string content;
		string name;
		cout << "请输入要写入的文件：";
		cin >> name;
		cin.ignore(); //清空缓冲区的内容，不然getline读到上一个回车直接结束。。。
		cout << "请输入文件要写入的内容: " << endl;;
		getline(cin, content); //读入一整行内容
		//cout << content << endl;
		//system("pause");
		//cin >> content;
		char buf[512];

		int times = content.length() / 512;
		int offset = content.length() % 512;
		if (offset != 0)
			times++;
		for (int i = 0; i < times; i++)
		{
			if (i == times - 1) //注意这里不能写成times-- o.o!!!
			{
				for (int j = 0; j < offset; j++)
					buf[j] = content[j];
			}
			else
			{
				for (int j = 0; j < 512; j++)
					buf[j] = content[j];
			}
			write(name, buf, content.length());
		}
	}
	else if (operation == "exit")
	{
		exit(0);
	}
	else if (operation == "cd")
	{
		cd();
	}
	else if (operation == "mkdir")
	{
		string name;
		cin >> name;
		mkdir(name);
	}
	else if (operation == "register")
	{
		register_user();
	}
	else if (operation == "remove")
	{
		string name;
		cin >> name;
		remove(name);
	}
    else if (operation == "ls") {
         show_files();
        }
	else
	{
		cout << "你的命令错误，重新输入" << endl;
	}

}

void register_user()  //注册用户
{
	cout << "请输入用户名：";
	string username;
	cin >> username;
	cout << "请输入密码：";
	string password;
	cin >> password;
	int i;
	for (i = 0; i < 16; i++)
	{
		if (mfd[i].username == username)
		{
			break;
		}
	}
	if (i < 16)
	{
		cout << "该用户已经存在" << endl;
		return;
	}
	mfd[cur_user_size].username = username;
	mfd[cur_user_size].password = password;
	cur_user_size++; //用户人数加1

	
	cout << "用户注册成功！" << endl;

}

void remove(string name)  //删除目录
{
	int index;
	for (int i = 0; i < cur_user.next->cur_user_direct_size; i++)
	{
		if (name == cur_user.next->direct[i].directname)
		{
			index = i;
			break;
		}
	}
	for (int i = 0; i < cur_user.next->direct[index].cur_file_size; i++)   //删除目录里面的文件
	{//直接释这些文件所占的磁盘块
		fat[cur_user.next->direct[index].ufd[i].addr].used = 0; //没有使用
		int temp = fat[cur_user.next->direct[index].ufd[i].addr].next;
		while (temp != -1)
		{
			fat[temp].used = 0;
			temp = fat[temp].next;
		}
	}
	//删除目录项，就是将两个目录项的内容进行交换

	cur_user.next->direct[index].cur_file_size = cur_user.next->direct[cur_user.next->cur_user_direct_size-1].cur_file_size;  //注意这里需要减一，由于本身结构的限制
	cur_user.next->direct[index].directname = cur_user.next->direct[cur_user.next->cur_user_direct_size-1].directname;
	for (int i = 0; i < cur_user.next->direct[cur_user.next->cur_user_direct_size-1].cur_file_size; i++)  //注意这里的减一
	{
		cur_user.next->direct[index].ufd[i].addr = cur_user.next->direct[cur_user.next->cur_user_direct_size-1].ufd[i].addr;
		cur_user.next->direct[index].ufd[i].filename = cur_user.next->direct[cur_user.next->cur_user_direct_size-1].ufd[i].filename;
		cur_user.next->direct[index].ufd[i].length = cur_user.next->direct[cur_user.next->cur_user_direct_size-1].ufd[i].length;
		cur_user.next->direct[index].ufd[i].protect_code = cur_user.next->direct[cur_user.next->cur_user_direct_size-1].ufd[i].protect_code;
	}
	cur_user.next->cur_user_direct_size--; //目录数量减1
	cout << "删除目录成功" << endl;
	return;
}
int main()
{
	cur_user.username = ""; //初始化当前用户的用户名为空
	path = "";
	//用内存模拟外存，申请内存空间
	fdisk = (char *)malloc(1024 * 1024 * sizeof(char)); //初始换fdisk

	// cout << "******************************************************" << endl;
	// cout << "\t" << "\t" << "命令" << "\t" << "\t" << "功能" << endl;
	// cout << "\t" << "\t" << "register" << "\t" << "注册" << endl;
	// cout << "\t" << "\t" << "login" << "\t" << "\t" << "登录" << endl;
	// cout << "\t" << "\t" << "create" << "\t" << "\t" << "创建文件" << endl;
	// cout << "\t" << "\t" << "mkdir" << "\t" << "\t" << "创建目录" << endl;
    // cout << "\t" << "\t" << "dir" << "\t" << "\t" << "显示当前位置的文件" << endl;
	// cout << "\t" << "\t" << "cd" << "\t" << "\t" << "切换目录（根目录..）" << endl;
	// cout << "\t" << "\t" << "open" << "\t" << "\t" << "打开文件" << endl;
	// cout << "\t" << "\t" << "close" << "\t" << "\t" << "关闭文件" << endl;
	// cout << "\t" << "\t" << "read" << "\t" << "\t" << "读文件" << endl;
	// cout << "\t" << "\t" << "write" << "\t" << "\t" << "写文件" << endl;
	// cout << "\t" << "\t" << "del" << "\t" << "\t" << "删除文件" << endl;
	// cout << "\t" << "\t" << "exit" << "\t" << "\t" << "退出系统" << endl;
	// cout << "*****************************************************" << endl;

	while (true)
		input_operation();
    
    free(fdisk);
	return 0;
}
