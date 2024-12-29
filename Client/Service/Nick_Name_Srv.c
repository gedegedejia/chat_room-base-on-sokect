
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "./Connect.h"
#include "../Common/Common.h"
#include "../Common/List.h"
#include "../Common/cJSON.h"
#include "../Service/Friends_Srv.h"

#define MSG_LEN 1024
extern char massage[1024];

extern int gl_uid;
extern int sock_fd;
extern friends_t *FriendsList;

void Update_Nick_Name(friends_t *FriendsList, char *name, char *nick_name) {
    // 遍历链表查找目标节点
    friends_t *curPos;
    List_ForEach(FriendsList, curPos) {
        if (strcmp(curPos->name, name) == 0) { // 匹配名称
            // 更新备注字段
            strncpy(curPos->nick, nick_name, sizeof(curPos->nick) - 1);
            curPos->nick[sizeof(curPos->nick) - 1] = '\0'; // 确保字符串以 '\0' 结束
            printf("更新备注成功: 用户 %s 的备注修改为 %s\n", name, nick_name);
            return;
        }
    }
    printf("用户 %s 未找到，无法更新备注。\n", name);
}

void Nick_Name_Send(char *name, char *nick_name){
    // printf("选择用户:   %s\n", name);
    // printf("备注为:     %s\n", nick_name);

    cJSON *root = cJSON_CreateObject();
    
    cJSON *item = cJSON_CreateString("N");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(gl_uid);
    cJSON_AddItemToObject(root, "uid", item);
    item = cJSON_CreateString(name);
    cJSON_AddItemToObject(root, "name", item);
    item = cJSON_CreateString(nick_name);
    cJSON_AddItemToObject(root, "nick_name", item);

    char *out = cJSON_Print(root);
    // printf("发送数据:   %s\n", out);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("send: 请求服务器失败");
    }else{    
    	printf("发送成功\n");
        Update_Nick_Name(FriendsList, name, nick_name);
	}    
	free(out);
    cJSON_Delete(root);

}
