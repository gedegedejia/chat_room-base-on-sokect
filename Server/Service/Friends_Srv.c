
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "Account_Srv.h"
#include "Friends_Srv.h"
#include "Chat_Srv.h"
#include "../Persistence/Friends_Persist.h"
#include "../Persistence/Account_Persist.h"
#include "../Common/cJSON.h"
#include "../Common/List.h"
#define MSG_LEN 1024

extern online_t *OnlineList;
int Friends_Srv_GetList(int sock_fd, const char *JSON)
{
    char buf[MSG_LEN];
    int uid;
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "uid");
    uid = item->valueint;
    cJSON_Delete(root);
    friends_t *FriendsList = NULL;
    List_Init(FriendsList, friends_t);
    Friends_Perst_GetList(FriendsList, uid);
    friends_t *curPos;
    List_ForEach(FriendsList, curPos)
    {
        root = cJSON_CreateObject();
        item = cJSON_CreateString("L");
        cJSON_AddItemToObject(root, "type", item);
        item = cJSON_CreateNumber(curPos->uid);
        cJSON_AddItemToObject(root, "uid", item);
        item = cJSON_CreateString(curPos->name);
        cJSON_AddItemToObject(root, "name", item);
        
        item = cJSON_CreateBool(curPos->is_vip);
        cJSON_AddItemToObject(root, "is_vip", item);
        item = cJSON_CreateBool(curPos->is_follow);
        cJSON_AddItemToObject(root, "is_follow", item);
        item = cJSON_CreateBool(curPos->is_online);
        cJSON_AddItemToObject(root, "is_online", item);
        item = cJSON_CreateBool(curPos->state);
        cJSON_AddItemToObject(root, "state", item);
        char *out = cJSON_Print(root);
        cJSON_Delete(root);
        if (send(sock_fd, (void *)out, MSG_LEN, 0) < 0)
        {
            perror("send 客户端响应失败");
            free(out);
            return 0;
        }
        free(out);
    }
    // 发送一个uid为0的数据告诉客户端发送完成
    root = cJSON_CreateObject();
    item = cJSON_CreateString("L");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(0);
    cJSON_AddItemToObject(root, "uid", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("send 客户端响应失败");
        free(out);
        return 0;
    }
    free(out);
    Account_Srv_SendIsOnline(uid, 1);
    // 销毁链表
    List_Destroy(FriendsList, friends_t);

    root = cJSON_CreateObject();
    item = cJSON_CreateString("R");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateBool(1);
    cJSON_AddItemToObject(root, "res", item);
    out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("recv: 客户端响应失败");
        free(out);
        return 0;
    }
    free(out);
    Chat_Srv_SendOfflienPrivateMsg(uid); // 推送离线消息
    return 1;
}

int Friends_Srv_Add(int sock_fd, const char *JSON)
{
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "uid");
    int uid = item->valueint;
    item = cJSON_GetObjectItem(root, "fname");
    int fuid = Account_Perst_IsUserName(item->valuestring);
    cJSON_Delete(root);
    root = cJSON_CreateObject();
    item = cJSON_CreateString("R");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateBool((fuid != 0));
    cJSON_AddItemToObject(root, "res", item);
    if (fuid == 0)
    {
        item = cJSON_CreateString("用户名不存在");
        cJSON_AddItemToObject(root, "reason", item);
    }
    char *out = cJSON_Print(root);
    // printf("发给 sock_fd = %d :\n%s",sock_fd ,out);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return 0;
    }
    free(out);
    Friends_Perst_Add(uid, fuid);
    if (Chat_Srv_GetFriendSock(fuid) == 0)
        return 1;
    Friends_Srv_SendAdd(uid, fuid, "A");
    return 1;
}

int Friends_Srv_SendAdd(int uid, int fuid, char *type)
{
    printf("uid = %d\n", uid);
    printf("fuid = %d\n", fuid);
    
    int f_sock_fd = -1;
    friends_t *NewFriends = (friends_t *)malloc(sizeof(friends_t));
    NewFriends->uid = uid;
    Friends_Perst_GetFriendInfo(NewFriends);
    f_sock_fd = Chat_Srv_GetFriendSock(fuid);
    if (*type != 'A')
    {
        NewFriends->state = 1;
    }
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString(type);
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(NewFriends->uid);
    cJSON_AddItemToObject(root, "uid", item);
    item = cJSON_CreateString(NewFriends->name);
    cJSON_AddItemToObject(root, "name", item);
    item = cJSON_CreateBool(NewFriends->is_vip);
    cJSON_AddItemToObject(root, "is_vip", item);
    item = cJSON_CreateBool(NewFriends->is_follow);
    cJSON_AddItemToObject(root, "is_follow", item);
    item = cJSON_CreateBool(NewFriends->is_online);
    cJSON_AddItemToObject(root, "is_online", item);
    item = cJSON_CreateBool(NewFriends->state);
    cJSON_AddItemToObject(root, "state", item);
    free(NewFriends);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(f_sock_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("send");
        printf("发给sock_fd = %d 失败\n", f_sock_fd);
        free(out);
        return 0;
    }

    printf("*****发给sock_fd = %d 成功\n", f_sock_fd);
    free(out);
    return 1;
}
int Friends_Srv_Apply(int sock_fd, const char *JSON)
{
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "uid");
    int uid = item->valueint;
    item = cJSON_GetObjectItem(root, "fuid");
    int fuid = item->valueint;
    item = cJSON_GetObjectItem(root, "is_agree");
    int is_agree = item->valueint;
    int f_sock_fd = Chat_Srv_GetFriendSock(fuid);
    Friends_Perst_Apply(uid, fuid, is_agree);
    if (is_agree)
    {
        Friends_Srv_SendAdd(fuid, uid, "a");
    }
    else
    {
        friends_t *NewFriends = (friends_t *)malloc(sizeof(friends_t));
        NewFriends->uid = fuid;
        Friends_Perst_GetFriendInfo(NewFriends);
        item = cJSON_CreateString(NewFriends->name);
        cJSON_AddItemToObject(root, "fname", item);
        char *out = cJSON_Print(root);
        cJSON_Delete(root);
        free(NewFriends);
        if (send(f_sock_fd, (void *)out, MSG_LEN, 0) <= 0)
        {
            perror("send");
            return 0;
        }
        free(out);
    }
    return 1;
}

int Friends_Srv_Del(int sock_fd, const char *JSON)
{
    printf("Debug: 进入 Friends_Srv_Del 函数\n");

    // 解析客户端传来的 JSON 数据
    cJSON *root = cJSON_Parse(JSON);
    if (root == NULL)
    {
        printf("Error: JSON 解析失败\n");
        return 0;
    }
    printf("Debug: 成功解析 JSON 数据\n");

    // 获取 uid 和 fname
    cJSON *item = cJSON_GetObjectItem(root, "uid");
    int uid = item->valueint;
    printf("Debug: 获取到 uid=%d\n", uid);

    item = cJSON_GetObjectItem(root, "fname");
    const char *fname = item->valuestring;
    printf("Debug: 获取到 fname='%s'\n", fname);

    // 查找好友的用户 ID (fuid)
    int fuid = Account_Perst_IsUserName(fname);
    printf("Debug: 查找用户 '%s' 对应的 fuid=%d\n", fname, fuid);

    // 删除解析的 root 对象
    cJSON_Delete(root);

    // 创建响应 JSON
    root = cJSON_CreateObject();
    item = cJSON_CreateString("R");
    cJSON_AddItemToObject(root, "type", item);
    printf("Debug: 添加 type='R' 到响应 JSON\n");

    // 根据 fuid 是否有效来决定响应结果
    item = cJSON_CreateBool((fuid != 0));
    cJSON_AddItemToObject(root, "res", item);
    if (fuid == 0)
    {
        // 如果 fuid 为 0，表示用户不存在，返回失败信息
        item = cJSON_CreateString("用户名不存在");
        cJSON_AddItemToObject(root, "reason", item);
        printf("Debug: 用户不存在，添加原因信息\n");
    }
    else
    {
        // 如果 fuid 有效，表示成功找到用户
        printf("Debug: 用户存在，准备删除好友\n");
    }

    // 打印准备发送的 JSON 字符串
    char *out = cJSON_Print(root);
    printf("Debug: 准备发送的响应 JSON: %s\n", out);

    // 发送响应到客户端
    if (send(sock_fd, (void *)out, MSG_LEN, 0) <= 0)
    {
        perror("send");
        free(out);
        cJSON_Delete(root);
        printf("Error: 发送失败\n");
        return 0;
    }
    printf("Debug: 响应成功发送\n");

    // 释放发送的响应数据
    free(out);
    cJSON_Delete(root);

    // 调用删除好友的持久化操作
    Friends_Perst_Del(uid, fuid);
    printf("Debug: 删除好友操作完成\n");

    return 1;
}


