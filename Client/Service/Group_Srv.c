
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include "./Group_Srv.h"
#include "./Friends_Srv.h"
#include "../Common/cJSON.h"
#include "../Common/Common.h"
#include "../Common/List.h"
#define MSG_LEN 1024

extern int gl_uid;
extern int sock_fd;
extern int my_mutex;
extern char massage[1024];
extern friends_t *FriendsList;
extern group_t *curGroup;
group_t *GroupList;

int Group_Srv_Create(const char *gname)
{
    int rtn = 0;
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("c");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateString(gname);
    cJSON_AddItemToObject(root, "gname", item);
    item = cJSON_CreateNumber(gl_uid);
    cJSON_AddItemToObject(root, "uid", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(sock_fd, out, MSG_LEN, 0) <= 0)
    {
        free(out);
        perror("send");
        return 0;
    }
    free(out);
    My_Lock();
    root = cJSON_Parse(massage);
    item = cJSON_GetObjectItem(root, "res");
    int res = item->valueint;
    if (res)
    {
        printf("群创建成功!");
        getchar();
        rtn = 1;
    }
    else
    {
        item = cJSON_GetObjectItem(root, "reason");
        printf("创建失败 :%s ", item->valuestring);
        getchar();
        rtn = 0;
    }
    cJSON_Delete(root);
    My_Unlock();
    return rtn;
}
int Group_Srv_GetList()
{
    int rtn;
    if (NULL != GroupList)
    {
        List_Destroy(GroupList, group_t);
    }
    List_Init(GroupList, group_t);
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("g");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(gl_uid);
    cJSON_AddItemToObject(root, "uid", item);
    char *out = cJSON_Print(root);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    free(out);
    cJSON_Delete(root);
    group_t *newNode = NULL;
    while (1)
    {
        // pthread_mutex_lock(&mutex);
        My_Lock();
        root = cJSON_Parse(massage);
        item = cJSON_GetObjectItem(root, "gid");
        if (item->valueint == 0)
        {
            My_Unlock();
            // pthread_mutex_unlock(&mutex);
            break;
        }
        newNode = (group_t *)malloc(sizeof(group_t));
        newNode->gid = item->valueint;
        item = cJSON_GetObjectItem(root, "name");
        strcpy(newNode->name, item->valuestring);
        item = cJSON_GetObjectItem(root, "owner");
        newNode->owner = item->valueint;
        item = cJSON_GetObjectItem(root, "num");
        newNode->num = item->valueint;
        cJSON_Delete(root);
        newNode->next = NULL;
        List_AddHead(GroupList, newNode);
        My_Unlock();
        // pthread_mutex_unlock(&mutex);
    }
    // pthread_mutex_lock(&mutex);
    My_Lock();
    root = cJSON_Parse(massage);
    item = cJSON_GetObjectItem(root, "res");
    int res = item->valueint;
    if (res == 1)
    {
        rtn = 1;
    }
    else
    {
        item = cJSON_GetObjectItem(root, "reason");
        printf("请求失败: %s", item->valuestring);
        rtn = 0;
    }
    cJSON_Delete(root);
    My_Unlock();
    // pthread_mutex_unlock(&mutex);
    return rtn;
}
int Group_Srv_AddMember(int gid, int uid)
{

    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("M");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(gid);
    cJSON_AddItemToObject(root, "gid", item);
    item = cJSON_CreateNumber(uid);
    cJSON_AddItemToObject(root, "uid", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(sock_fd, out, MSG_LEN, 0) <= 0)
    {
        free(out);
        perror("send");
        return 0;
    }
    free(out);
    My_Lock();
    root = cJSON_Parse(massage);
    item = cJSON_GetObjectItem(root, "res");
    int res = item->valueint;
    if (res)
    {
        printf("邀请成功!");
        getchar();
    }
    else
    {
        printf("邀请失败,该成员已在当前群聊中!");
        getchar();
    }
    cJSON_Delete(root);
    My_Unlock();
    return 1;
}
void Group_Srv_Join(const char *massage)
{
    group_t *newNode =
        (group_t *)malloc(sizeof(group_t));
    cJSON *root = cJSON_Parse(massage);
    cJSON *item = cJSON_GetObjectItem(root, "gid");
    newNode->gid = item->valueint;
    item = cJSON_GetObjectItem(root, "name");
    strcpy(newNode->name, item->valuestring);
    item = cJSON_GetObjectItem(root, "owner");
    newNode->owner = item->valueint;
    item = cJSON_GetObjectItem(root, "num");
    newNode->num = item->valueint;
    newNode->NewMsgNum = 0;
    newNode->next = NULL;
    List_AddHead(GroupList, newNode);
    if (newNode->owner == gl_uid)
        return;
    friends_t *f;
    List_ForEach(FriendsList, f)
    {
        if (newNode->owner == f->uid)
        {
            printf("\n%s 邀请你加入了群聊 %s\n", f->name, newNode->name);
            return;
        }
    }
}

void Group_Srv_ShowMember(const char *massage)
{
    cJSON *root, *item;
    friends_t GroupMember;
    root = cJSON_Parse(massage);
    item = cJSON_GetObjectItem(root, "name");
    strcpy(GroupMember.name, item->valuestring);
    /*
    item = cJSON_GetObjectItem(root, "sex");
    GroupMember.sex = item->valueint;
    */
    item = cJSON_GetObjectItem(root, "is_online");
    GroupMember.is_online = item->valueint;
    item = cJSON_GetObjectItem(root, "is_vip");
    GroupMember.is_vip = item->valueint;
    item = cJSON_GetObjectItem(root, "permission");
    char *is_online[2] = {"●", "\e[32m●\e[0m"};
    char *is_vip[2] = {"", "\e[31m"};
    /*
    char *sex[2] = {"\e[35m♀\e[0m", "\e[36m♂\e[0m"};
    */
    char *per[3] = {"", "[\e[32m管理员\e[0m]", "[\e[33m群主\e[0m]"};
    printf("   %s %s%s\e[0m %s\n",
           is_online[GroupMember.is_online],
           is_vip[GroupMember.is_vip],
           GroupMember.name,
           per[item->valueint]);
    cJSON_Delete(root);
}

void Group_Srv_GetMember(int gid)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "m");
    cJSON_AddNumberToObject(root, "gid", gid);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(sock_fd, out, MSG_LEN, 0) <= 0)
    {
        perror("send");
    }
    free(out);
}

void Group_Srv_Quit(group_t *curGroup)
{
    char choice[5];
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "Q");
    cJSON_AddNumberToObject(root, "gid", curGroup->gid);
    if (curGroup->owner == gl_uid)
    {
        // 解散群
        cJSON_AddStringToObject(root, "do", "解散");
        printf("您是群主,确认解散群聊 %s ?[yes/no]", curGroup->name);
    }
    else
    {
        cJSON_AddStringToObject(root, "do", "退群");
        cJSON_AddNumberToObject(root, "uid", gl_uid);
        printf("确认要退出群聊 %s ?[yes/no]", curGroup->name);
    }
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    sgets(choice, 5);
    if (strcmp(choice, "yes") != 0)
    {
        free(out);
        return;
    }
    if (send(sock_fd, out, MSG_LEN, 0) <= 0)
    {
        perror("send");
    }
    free(out);
    List_FreeNode(GroupList, curGroup, group_t);
    printf("操作成功！");
    getchar();
}

void Group_Srv_Delete(const char *massage)
{
    cJSON *root = cJSON_Parse(massage);
    cJSON *item = cJSON_GetObjectItem(root, "gid");
    int gid = item->valueint;
    cJSON_Delete(root);
    group_t *g;
    List_ForEach(GroupList, g)
    {
        if (g->gid == gid)
        {
            printf("\n群主已将群聊 %s 解散!\n", g->name);
            List_FreeNode(GroupList, g, group_t);
            curGroup = NULL;
            return;
        }
    }
}

void Group_Srv_RemoveMember(group_t *curGroup, char *name)
{
    if (gl_uid != curGroup->owner)
    {
        printf("权限不足，只有群主可以删除群成员。\n");
        getchar();
        return;
    }
    
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("d");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(curGroup->gid);
    cJSON_AddItemToObject(root, "gid", item);
    item = cJSON_CreateString(name);
    cJSON_AddItemToObject(root, "name", item);
    item = cJSON_CreateNumber(curGroup->owner);
    cJSON_AddItemToObject(root, "owner", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(sock_fd, out, MSG_LEN, 0) <= 0)
    {
        free(out);
        perror("send");
        return;
    }
    free(out);
    My_Lock();
    root = cJSON_Parse(massage);
    item = cJSON_GetObjectItem(root, "res");
    int res = item->valueint;
    if (res)
    {
        printf("删除成功!\n");
        getchar();
    }
    else
    {
        item = cJSON_GetObjectItem(root, "reason");
        printf("删除失败:");
        printf("%s\n", item->valuestring);
        getchar();
    }
    cJSON_Delete(root);
    My_Unlock();
    return;
}

void Group_Srv_DelRes(const char *JSON) {
    // printf("接收到的消息：%s\n", JSON);

    // 解析 JSON 数据
    cJSON *root = cJSON_Parse(JSON);
    if (root == NULL) {
        printf("无法解析 JSON 数据。\n");
        return;
    }

    // 获取 "type" 字段
    cJSON *typeItem = cJSON_GetObjectItem(root, "type");
    if (typeItem == NULL || typeItem->type != cJSON_String) {
        printf("缺少 'type' 字段或字段类型错误。\n");
        cJSON_Delete(root);
        return;
    }

    // 根据 "type" 进行逻辑处理
    if (strcmp(typeItem->valuestring, "d") == 0) {
        cJSON *gnameItem = cJSON_GetObjectItem(root, "gname");
        if (gnameItem != NULL && gnameItem->type == cJSON_String) {
            const char *gname = gnameItem->valuestring;
            printf("您已被群主移出%s群聊\n", gname);

            // 查找并释放对应的群聊节点
            group_t *prev = NULL;
            group_t *cur = GroupList;
            while (cur != NULL) {
                if (strcmp(cur->name, gname) == 0) {
                    // 从链表中移除节点
                    if (prev == NULL) {
                        GroupList = cur->next; // 如果是头节点
                    } else {
                        prev->next = cur->next; // 连接前后节点
                    }

                    // 释放节点
                    free(cur);
                    // printf("已释放群聊节点：%s\n", gname);
                    break;
                }
                prev = cur;
                cur = cur->next;
            }
        } else {
            printf("通知字段 'gname' 不存在或类型错误。\n");
        }
    } else {
        printf("未知的 'type' 类型：%s\n", typeItem->valuestring);
    }

    // 清理 JSON 对象
    cJSON_Delete(root);
}



