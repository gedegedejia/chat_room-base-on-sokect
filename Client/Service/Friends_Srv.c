
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./Friends_Srv.h"
#include <pthread.h>
#include "./Connect.h"
#include "../Common/Common.h"
#include "../Common/List.h"
#define MSG_LEN 1024
extern int gl_uid;
extern int sock_fd;
// extern pthread_mutex_t mutex;
extern int my_mutex;
// extern int your_mutex;
extern char massage[1024];
friends_t *FriendsList;

int Friends_Srv_GetList()
{
    int rtn;
    if (NULL != FriendsList)
    {
        List_Destroy(FriendsList, friends_t);
    }
    List_Init(FriendsList, friends_t);
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("G");
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
    friends_t *newNode = NULL;
    while (1)
    {
        // pthread_mutex_lock(&mutex);
        My_Lock();
        // printf("massage: %s\n",massage);
        root = cJSON_Parse(massage);
        item = cJSON_GetObjectItem(root, "uid");
        if (item->valueint == 0)
        {
            My_Unlock();
            // pthread_mutex_unlock(&mutex);
            break;
        }
        newNode = (friends_t *)malloc(sizeof(friends_t));
        newNode->uid = item->valueint;
        item = cJSON_GetObjectItem(root, "name");
        strcpy(newNode->name, item->valuestring);
        item = cJSON_GetObjectItem(root, "is_vip");
        newNode->is_vip = item->valueint;
        item = cJSON_GetObjectItem(root, "nick");
        strcpy(newNode->nick, item->valuestring);  
        item = cJSON_GetObjectItem(root, "is_follow");
        newNode->is_follow = item->valueint;
        item = cJSON_GetObjectItem(root, "is_online");
        newNode->is_online = item->valueint;
        item = cJSON_GetObjectItem(root, "state");
        newNode->state = item->valueint;
        cJSON_Delete(root);
        newNode->NewMsgNum = 0;
        newNode->next = NULL;
        List_AddHead(FriendsList, newNode);
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

int Friends_Srv_SendAdd(const char *fname)
{
    int rtn;
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("A");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(gl_uid);
    cJSON_AddItemToObject(root, "uid", item);
    item = cJSON_CreateString(fname);
    cJSON_AddItemToObject(root, "fname", item);
    char *out = cJSON_Print(root);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    free(out);
    cJSON_Delete(root);
    My_Lock();
    root = cJSON_Parse(massage);
    item = cJSON_GetObjectItem(root, "res");
    int res = item->valueint;
    if (res)
    {
        printf("好友请求发送成功!");
        getchar();
        rtn = 1;
    }
    else
    {
        item = cJSON_GetObjectItem(root, "reason");
        printf("请求失败: %s", item->valuestring);
        getchar();
        rtn = 0;
    }
    cJSON_Delete(root);
    My_Unlock();
    return rtn;
}

int Friends_Srv_SendDel(friends_t *f)
{
    int rtn;
    
    // 创建 JSON 对象并添加数据
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("D");
    cJSON_AddItemToObject(root, "type", item);
    
    item = cJSON_CreateNumber(gl_uid);
    cJSON_AddItemToObject(root, "uid", item);
    
    item = cJSON_CreateString(f->name);
    cJSON_AddItemToObject(root, "fname", item);
    
    // 打印完整的 JSON
    char *out = cJSON_Print(root);
    
    // 发送 JSON 数据
    if (send(sock_fd, (void *)out, MSG_LEN, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    
    free(out);
    cJSON_Delete(root);

    // 锁定线程
    My_Lock();
    // printf("Debug: 已锁定线程，等待服务器响应\n");
    
    // 解析服务器返回的响应
    root = cJSON_Parse(massage);
    if (root == NULL)
    {
        printf("Error: 服务器返回的 JSON 无法解析\n");
        My_Unlock();
        return 0;
    }
    // printf("Debug: 解析服务器响应成功\n");

    item = cJSON_GetObjectItem(root, "res");
    if (item == NULL)
    {
        // printf("Error: 响应中缺少 'res' 字段\n");
        cJSON_Delete(root);
        My_Unlock();
        return 0;
    }
    int res = item->valueint;
    // printf("Debug: 响应中的 res=%d\n", res);

    // 根据响应结果处理
    if (res)
    {
        // printf("Debug: 服务器允许删除好友，开始删除本地节点\n");
        List_FreeNode(FriendsList, f, friends_t);
        printf("好友删除成功!\n");
        getchar();
        rtn = 1;
    }
    else
    {
        item = cJSON_GetObjectItem(root, "reason");
        if (item != NULL)
        {
            printf("删除失败: %s\n", item->valuestring);
        }
        else
        {
            printf("Error: 响应中缺少 'reason' 字段\n");
        }
        getchar();
        rtn = 0;
    }

    // 清理资源并解锁
    cJSON_Delete(root);
    My_Unlock();
    printf("Debug: 完成 Friends_Srv_SendDel 函数的执行，返回值=%d\n", rtn);
    return rtn;
}


int Friends_Srv_RecvAdd(const char *JSON)
{
    friends_t *newNode;
    newNode = (friends_t *)malloc(sizeof(friends_t));
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "uid");
    newNode->uid = item->valueint;
    item = cJSON_GetObjectItem(root, "name");
    strcpy(newNode->name, item->valuestring);
    item = cJSON_GetObjectItem(root, "is_vip");
    newNode->is_vip = item->valueint;
    item = cJSON_GetObjectItem(root, "is_follow");
    newNode->is_follow = item->valueint;
    item = cJSON_GetObjectItem(root, "is_online");
    newNode->is_online = item->valueint;
    item = cJSON_GetObjectItem(root, "state");
    newNode->state = item->valueint;
    cJSON_Delete(root);
    newNode->NewMsgNum = 0;
    newNode->next = NULL;
    List_AddHead(FriendsList, newNode);
    if (newNode->state == 0)
        printf("\n%s请求添加你为好友\n", newNode->name);
    return 1;
}

int Friends_Srv_Apply(int uid, int fuid, int is_agree)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("a");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(uid);
    cJSON_AddItemToObject(root, "uid", item);
    item = cJSON_CreateNumber(fuid);
    cJSON_AddItemToObject(root, "fuid", item);
    item = cJSON_CreateBool(is_agree);
    cJSON_AddItemToObject(root, "is_agree", item);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return 0;
    }
    free(out);
    return 1;
}

// 处理好友申请的反馈
int Friends_Srv_ApplyRes(const char *JSON)
{
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "fuid");
    if (NULL == item)
    {
        item = cJSON_GetObjectItem(root, "name");
        printf("\n%s 同意了你的好友请求\n", item->valuestring);
        Friends_Srv_RecvAdd(JSON);
        cJSON_Delete(root);
        return 1;
    }
    item = cJSON_GetObjectItem(root, "fname");
    // item = cJSON_GetObjectItem(root ,"is_agree");
    printf("%s\n", JSON);
    printf("\n%s 拒绝了你的好友请求\n", item->valuestring);
    cJSON_Delete(root);
    return 1;
}


