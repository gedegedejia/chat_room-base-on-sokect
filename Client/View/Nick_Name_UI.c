#include <string.h>
#include <stdio.h>
#include "Nick_Name_UI.h"
#include "../Service/Friends_Srv.h"
#include "../Common/List.h"
#include "../Service/Nick_Name_Srv.h"
#define MAX_USERNAME_LEN 30  // 定义用户名的最大长度

extern char *login_name;
extern int gl_uid;
extern friends_t *FriendsList;
char *login_name;

void Nick_name_UI_Init() {
    char name[30];
    char nick_name[30];
    int found = 0;

    // 用户选择环节
    while (!found) {
        printf("请选择用户: ");
        if (fgets(name, sizeof(name), stdin) == NULL) {
            printf("输入错误，请重新输入！\n");
            continue;
        }

        name[strcspn(name, "\n")] = '\0'; // 去掉换行符
        if (strlen(name) == 0) {
            printf("输入不能为空，请重新输入！\n");
            continue;
        }

        // 遍历好友列表查找用户
        friends_t *curPos;
        List_ForEach(FriendsList, curPos) {
            if (strcmp(curPos->name, name) == 0) { // 匹配名称
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("该用户不存在或不是你的好友，请重新输入！\n");
        }
    }

    // 用户备注环节
    while (1) {
        printf("请输入对该用户的昵称: ");
        if (fgets(nick_name, sizeof(nick_name), stdin) == NULL) {
            printf("输入错误，请重新输入！\n");
            continue;
        }

        nick_name[strcspn(nick_name, "\n")] = '\0'; // 去掉换行符

        // 检查昵称是否为空
        if (strlen(nick_name) == 0) {
            printf("昵称不能为空，请重新输入！\n");
        } else {
            break;
        }
    }

    // 调用发送函数
    Nick_Name_Send(name, nick_name);
    printf("昵称设置成功！用户: %s, 昵称: %s\n", name, nick_name);
}

