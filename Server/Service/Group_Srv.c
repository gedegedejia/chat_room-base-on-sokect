
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "./Group_Srv.h"
#include "./Chat_Srv.h"
#include "../Common/List.h"
#include "../Common/cJSON.h"
#include "../Persistence/Group_Persist.h"
#include "../Persistence/Account_Persist.h"
#define MSG_LEN 1024
int Group_Srv_AddMember(int client_fd, char *JSON)
{
    int gid, uid;
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "gid");
    gid = item->valueint;
    item = cJSON_GetObjectItem(root, "uid");
    uid = item->valueint;
    cJSON_Delete(root);
    root = cJSON_CreateObject();
    item = cJSON_CreateString("R");
    cJSON_AddItemToObject(root, "type", item);
    if (Group_Perst_AddMember(gid, uid))
    {
        item = cJSON_CreateBool(1);

        // 将群信息下发给被邀请的用户
        Group_Srv_SendInfo(gid, uid);
    }
    else
    {
        item = cJSON_CreateBool(0);
    }
    cJSON_AddItemToObject(root, "res", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(client_fd, out, MSG_LEN, 0) <= 0)
    {
        perror("send");
        free(out);
        return 0;
    }
    return 1;
}
void Group_Srv_SendInfo(int gid, int uid)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("J");
    cJSON_AddItemToObject(root, "type", item);
    group_t *GroupInfo = Group_Perst_GetInfo(gid);
    item = cJSON_CreateNumber(GroupInfo->gid);
    cJSON_AddItemToObject(root, "gid", item);
    item = cJSON_CreateString(GroupInfo->name);
    cJSON_AddItemToObject(root, "name", item);
    item = cJSON_CreateNumber(GroupInfo->owner);
    cJSON_AddItemToObject(root, "owner", item);
    item = cJSON_CreateNumber(GroupInfo->num);
    cJSON_AddItemToObject(root, "num", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    int f_fd = Chat_Srv_GetFriendSock(uid);
    if (send(f_fd, out, MSG_LEN, 0) <= 0)
    {
        perror("send");
        free(out);
        return;
    }
    free(out);
}

int Group_Srv_Create(int client_fd, char *buf)
{
    char gname[30];
    int uid;
    cJSON *root = cJSON_Parse(buf);
    cJSON *item = cJSON_GetObjectItem(root, "gname");
    strcpy(gname, item->valuestring);
    item = cJSON_GetObjectItem(root, "uid");
    uid = item->valueint;
    cJSON_Delete(root);
    root = cJSON_CreateObject();
    item = cJSON_CreateString("R");
    cJSON_AddItemToObject(root, "type", item);

    if (Group_Perst_IsGroup(gname))
    {
        item = cJSON_CreateBool(0);
        cJSON_AddItemToObject(root, "res", item);
        item = cJSON_CreateString("群名称已存在");
        cJSON_AddItemToObject(root, "reason", item);
        char *out = cJSON_Print(root);
        if (send(client_fd, out, MSG_LEN, 0) <= 0)
        {
            perror("send");
            free(out);
            return 0;
        }
        free(out);
        return 0;
    }
    Group_Srv_SendInfo(Group_Perst_Create(uid, gname), uid);

    // 创建群 同时在群成员表中插入群主
    // 然后将群信息发给群主
    item = cJSON_CreateBool(1);

    cJSON_AddItemToObject(root, "res", item);
    char *out = cJSON_Print(root);
    if (send(client_fd, out, MSG_LEN, 0) <= 0)
    {
        perror("send");
        free(out);
        return 0;
    }
    free(out);
    return 1;
}

int Group_Srv_GetList(int client_fd, char *JSON)
{
    // char buf[MSG_LEN];
    int uid;
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "uid");
    uid = item->valueint;
    cJSON_Delete(root);
    group_t *GroupList = NULL;
    List_Init(GroupList, group_t);
    Group_Perst_GetMyGroup(GroupList, uid);
    group_t *curPos;
    List_ForEach(GroupList, curPos)
    {
        root = cJSON_CreateObject();
        item = cJSON_CreateString("l");
        cJSON_AddItemToObject(root, "type", item);
        item = cJSON_CreateNumber(curPos->gid);
        cJSON_AddItemToObject(root, "gid", item);
        item = cJSON_CreateString(curPos->name);
        cJSON_AddItemToObject(root, "name", item);
        item = cJSON_CreateNumber(curPos->owner);
        cJSON_AddItemToObject(root, "owner", item);
        item = cJSON_CreateNumber(curPos->num);
        cJSON_AddItemToObject(root, "num", item);
        char *out = cJSON_Print(root);
        cJSON_Delete(root);
        if (send(client_fd, (void *)out, MSG_LEN, 0) < 0)
        {
            perror("send 客户端响应失败");
            free(out);
            return 0;
        }
        free(out);
    }

    // 发送一个gid为0的数据告诉客户端发送完成
    root = cJSON_CreateObject();
    item = cJSON_CreateString("l");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(0);
    cJSON_AddItemToObject(root, "gid", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(client_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("send 客户端响应失败");
        free(out);
        return 0;
    }
    free(out);
    // 销毁链表
    List_Destroy(GroupList, group_t);
    root = cJSON_CreateObject();
    item = cJSON_CreateString("R");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateBool(1);
    cJSON_AddItemToObject(root, "res", item);
    out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(client_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("recv: 客户端响应失败");
        free(out);
        return 0;
    }
    free(out);
    return 1;
}

void Group_Srv_ShowMember(int client_fd, const char *JSON)
{
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "gid");
    int gid = item->valueint;
    cJSON_Delete(root);
    group_member_t *GroupMember;
    List_Init(GroupMember, group_member_t);
    Group_Perst_GetGroupMember(GroupMember, gid);
    group_member_t *m;
    List_ForEach(GroupMember, m)
    {
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "type", "m");
        cJSON_AddNumberToObject(root, "gid", m->gid);
        cJSON_AddNumberToObject(root, "uid", m->user_info.uid);
        cJSON_AddStringToObject(root, "name", m->user_info.name);
        // cJSON_AddNumberToObject(root, "sex", m->user_info.sex);
        cJSON_AddNumberToObject(root, "is_vip", m->user_info.is_vip);
        cJSON_AddNumberToObject(root, "is_online", m->user_info.is_online);
        cJSON_AddNumberToObject(root, "permission", m->permission);
        char *out = cJSON_Print(root);
        cJSON_Delete(root);
        if (send(client_fd, (void *)out, MSG_LEN, 0) < 0)
        {
            perror("recv: 客户端响应失败");
            free(out);
            continue;
        }
        free(out);
    }
}

void Group_Srv_Quit(int client_fd, const char *JSON)
{
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "do");
    char *_do = item->valuestring;
    item = cJSON_GetObjectItem(root, "gid");
    int gid = item->valueint;
    int f_fd = 0;
    if (strcmp(_do, "解散") == 0)
    {
        group_member_t *GroupMember;
        List_Init(GroupMember, group_member_t);
        Group_Perst_GetGroupMember(GroupMember, gid);
        group_member_t *m;
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "type", "D");
        cJSON_AddNumberToObject(root, "gid", gid);
        char *out = cJSON_Print(root);
        cJSON_Delete(root);
        List_ForEach(GroupMember, m)
        {
            if (m->permission == 2)
                continue;
            if ((f_fd = Chat_Srv_GetFriendSock(m->user_info.uid)) > 0)
            {
                if (send(f_fd, out, MSG_LEN, 0) <= 0)
                {
                    perror("send");
                    free(out);
                }
            }
        }
        Group_Perst_Delete(gid);
    }
    else
    {
        item = cJSON_GetObjectItem(root, "uid");
        Group_Perst_DeleteMember(gid, item->valueint);
    }
}

void Group_Srv_RemoveMember(int client_fd, const char *JSON)
{
    char name[30];

    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "gid");
    int gid = item->valueint;
    item = cJSON_GetObjectItem(root, "name");
    strcpy(name, item->valuestring);
    item = cJSON_GetObjectItem(root, "owner");
    int owner = item->valueint;
    int uid = Account_Perst_GetUserUidFromName(name);
    int res = 0;

    cJSON_Delete(root);
    root = cJSON_CreateObject();
    item = cJSON_CreateString("R");
    cJSON_AddItemToObject(root, "type", item);
    if (uid == owner)
    {
        item = cJSON_CreateString("无法删除群主");
        printf("无法删除群主\n");
    }
    else if (!Group_Perst_HavePermission(gid, owner))
    {
        printf("uid=%u\ngid=%d\n", uid, gid);
        item = cJSON_CreateString("你没有权限");
        printf("你没有权限\n");
    }
    else if (!Group_Perst_FindGroupMember(gid, uid))
    {
        item = cJSON_CreateString("该用户不存在");
        printf("该用户不存在\n");
    }
    else
    {
        if (Group_Perst_DeleteMember(gid, uid))
        {
            res = 1;
            item = cJSON_CreateString("成员删除成功");
            printf("成员删除成功\n");
            
            int f_fd = Chat_Srv_GetFriendSock(uid);

            group_t *GroupInfo = Group_Perst_GetInfo(gid);

            cJSON *notify_root = cJSON_CreateObject();
            cJSON_AddStringToObject(notify_root, "type", "d");
            cJSON_AddStringToObject(notify_root, "gname", GroupInfo->name);
            printf("获取到的群名称: %s\n", GroupInfo->name);

            char *notify_out = cJSON_Print(notify_root);
            cJSON_Delete(notify_root);
            if (send(f_fd, notify_out, MSG_LEN, 0) <= 0) {
                perror("send");
            }
            free(notify_out);
        }
        else
        {
            item = cJSON_CreateString("未知错误");
            printf("未知错误\n");
        }
    }

    cJSON_AddItemToObject(root, "reason", item);
    item = cJSON_CreateBool(res);
    cJSON_AddItemToObject(root, "res", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);

    printf("[DEBUG] Response JSON: %s\n", out);
    
    if (send(client_fd, out, MSG_LEN, 0) <= 0)
    {
        perror("send");
        free(out);
        return;
    }
    return;
}
