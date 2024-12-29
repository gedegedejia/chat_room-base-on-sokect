#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "Account_Srv.h"
#include "../Persistence/Friends_Persist.h"
#include "../Persistence/Account_Persist.h"
#include "../Common/cJSON.h"
#include "../Common/List.h"
#include "../Persistence/Nick_Name_Persist.h"

#define MSG_LEN 1024

int Nick_Name_Res(int client_fd, char *JSON){
    printf("Debug: 收到来自客户端的 NICK_NAME_REQ 消息\n");
    printf("%s\n", JSON);
    printf("Debug: 开始解析 JSON 数据\n");
    cJSON *root = cJSON_Parse(JSON);

    if (root == NULL){
        printf("Error parsing JSON!\n");
        return -1;
    }

    printf("Debug: 成功解析 JSON 数据\n");

    // 获取 "type"
    cJSON *item = cJSON_GetObjectItem(root, "type");
    char *type = item->valuestring;
    printf("Debug: 获取到 type=%s\n", type);

    if (strcmp(type, "N") != 0){
        printf("Error: 类型错误\n");
        cJSON_Delete(root);
        return 0;
    } else {
        printf("Debug: 类型正确\n");

        // 获取 "name"
        item = cJSON_GetObjectItem(root, "name");
        char *name = item->valuestring;
        printf("Debug: 获取到 name=%s\n", name);

        // 获取 "uid"
        item = cJSON_GetObjectItem(root, "uid");
        int uid = item->valueint;
        printf("Debug: 获取到 uid=%d\n", uid);

        // 获取 "nick_name"
        item = cJSON_GetObjectItem(root, "nick_name");
        char *nick_name = item->valuestring;
        printf("Debug: 获取到 nick_name=%s\n", nick_name);

        // 查找目标用户 UID
        int target_uid = Account_Perst_IsUserName(name);
        printf("Debug: 查找用户 '%s' 对应的 target_uid=%d\n", name, target_uid);

        int res = Nick_Name_ADD(uid, target_uid, nick_name);
        if (res == 1){
            printf("Debug: 存储到数据库\n");
            return 1;
        } else {
            printf("Debug: 存储失败\n");
            return 0;
        }
    }
}

