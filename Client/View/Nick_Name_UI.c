#include <string.h>
#include <stdio.h>
#include "../Service/Nick_Name_Srv.h"
#include "Nick_Name_UI.h"

#define MAX_USERNAME_LEN 30  // 定义用户名的最大长度

extern char *login_name;
extern int gl_uid;
char *login_name;

void Nick_name_UI_Init()
{
    char name[30];
    char nick_name[30];
    
    printf("请选择用户: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0; // 去掉换行符

    printf("请输入对该用户的昵称: ");
    fgets(nick_name, sizeof(nick_name), stdin);
    nick_name[strcspn(nick_name, "\n")] = 0; // 去掉换行符

    Nick_Name_Send(name, nick_name);
}
