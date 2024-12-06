
#include <stdio.h>
#include "./Account_UI.h"
#include "../Common/Common.h"
#include "../Service/Account_Srv.h"
#define MAX_USERNAME_LEN 30  // 定义用户名的最大长度

extern char *login_name;
int Account_UI_SignIn()
{
    printf(
        "==============================\n"
        "*************注册*************\n"
        "==============================\n");
    char name[30], password[30];
    int sex;
    printf("请输入要注册的用户名:");
    scanf("%30s", name);
    ffflush();
    while (1)
    {
        printf("请输入性别(男/女):");
        scanf("%30s", password); // 懒得再开数组,暂借一下
        ffflush();
        if (strcmp(password, "男") == 0)
        {
            sex = 1;
            break;
        }
        else if (strcmp(password, "女") == 0)
        {
            sex = 0;
            break;
        }
        else
        {
            printf("请输入正确的性别！！！\n");
        }
    }
    printf("请输入密码:");
    scanf("%s", password);
    ffflush();
    return Account_Srv_SignIn(name, sex, password);
}

int Account_UI_Login()
{
    printf(
        "==============================\n"
        "*************登陆*************\n"
        "==============================\n");
    char name[30], password[30];
    printf("请输入用户名:");
    scanf("%s", name);
    ffflush();
    printf("请输入密码:");
    scanf("%s", password);
    ffflush();
	
    // 确保 login_name 指针已经分配内存
    login_name = malloc(MAX_USERNAME_LEN * sizeof(char));  // 为 login_name 分配内存

    if (login_name == NULL) {
        printf("内存分配失败！\n");
        return -1;
    }

    strcpy(login_name, name);
    return Account_Srv_Login(name, password);
}

void Account_UI_Exit()
{
    printf("欢迎再次使用～\n");
}
