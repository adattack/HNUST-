#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MYSH_TOK_DELIM " \t\r\n"
#define MYSH_TOK_BUFFER_SIZE 64

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_ls(char **args);
int shell_echo(char **args);
int shell_env(char **args);
int shell_jobs(char **args);
int shell_builtin_nums();

char * builtin_cmd[] =
{
    "cd",
    "help",
    "exit",
    "ls",
    "echo",
    "env",
    "jobs"
};

int (*builtin_func[])(char **) = 
{
    &shell_cd,
    &shell_help,
    &shell_exit,
    &shell_ls,
    &shell_echo,
    &shell_env,
    &shell_jobs
};

int shell_cd(char **args)
{
    if(args[1] == NULL) 
    {
        perror("缺少参数\n");
    }
    // cd 
    else
    {
        if(chdir(args[1]) != 0)
            perror("找不到目录\n");
    }
    return 1;
}

int shell_ls(char **args) {
    if (args[1] == NULL) {
        // 如果没有参数，列出当前目录的内容
        system("ls");
    } else {
        // 如果提供了参数，列出指定目录的内容
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "ls %s", args[1]);
        system(cmd);
    }
    return 1;
}

int shell_env(char **args) {
    if (args[1] != NULL) {
        // 如果提供了参数，输出指定的环境变量
        char *value = getenv(args[1]);
        if (value != NULL) {
            printf("%s=%s\n", args[1], value);
        } else {
            fprintf(stderr, "env: %s: No such file or directory\n", args[1]);
        }
    } else {
        // 如果没有提供参数，列出所有环境变量
        char **env = environ;
        for (int i = 0; env[i] != NULL; ++i) {
            printf("%s\n", env[i]);
        }
    }
}

int shell_echo(char **args) {
    for (int i = 1; args[i] != NULL; ++i) {
        printf("%s ", args[i]);
    }
    printf("\n");
    return 1;
}

int shell_jobs(char **args) {
    // 这个函数只是一个示例，实际上并不跟踪作业状态
    printf("Jobs:\n");
    // 这里可以使用ps命令来列出所有子进程
    system("ps");
    // execlp("pstree","-p",NULL);
    return 1;
}


int shell_help(char **args)
{
    puts("目前实现了这些功能：");
    for (int i = 0; i < shell_builtin_nums(); i ++)
        printf("%s\n", builtin_cmd[i]);
    return 1;
}

int shell_exit(char **args)
{
    return 0;
}

int shell_builtin_nums()
{
    return sizeof(builtin_cmd) / sizeof(builtin_cmd[0]);
}

char * shell_read_line()
{
    char * line = NULL;
    size_t bufsize=MYSH_TOK_BUFFER_SIZE;
    getline(&line, &bufsize, stdin);
    return line;
}


char ** shell_split_line(char * line)
{
    int buffer_size = MYSH_TOK_BUFFER_SIZE, position = 0;
    char **tokens = (char**)malloc(buffer_size * sizeof(char *)); 
    char *token;
    // "cd /desk/main"
    token = strtok(line, MYSH_TOK_DELIM);
    while(token != NULL)
    {
        tokens[position++] = token;
        token = strtok(NULL, MYSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int shell_launch(char **args)
{
    // execvp
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        if(execvp(args[0], args) == -1)
            perror("execvp错误\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int shell_execute(char **args)
{
    if(args[0] == NULL) return 1;
    
    for (int i = 0; i < shell_builtin_nums(); i ++)
        if(strcmp(args[0], builtin_cmd[i]) == 0)
            return (*builtin_func[i])(args);
    int result = shell_launch(args);
    if (result == 0) {
        free(args);
    }
    
    return result;
}

void shell_loop()
{
    char *line;
    char **args;
    int status;

    do
    {
        char path[100];
        getcwd(path, 100);
        char now[200] = "[driac ";
        strcat(now, path);
        strcat(now, " ]$");
        printf("%s", now);

        line = shell_read_line();
        args = shell_split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while (status);
    
}

int main(int argc, char *argv[])
{
    shell_loop();
    return 0;
}