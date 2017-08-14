/*************************************************************************
	>    File Name: Friends_Srv.c
	>       Author: fujie
	>         Mail: fujie.me@qq.com
	> Created Time: 2017年08月12日 星期六 09时46分39秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"./Friends_Srv.h"
#include"./List.h"
extern int gl_uid;
extern int sock_fd;
friends_t *FriendsList;

int Friends_Srv_GetList(){
    char buf[1024];
    int rtn;
    if(NULL != FriendsList){
        List_Destroy(FriendsList ,friends_t);
    }
    List_Init(FriendsList ,friends_t);
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("G");
    cJSON_AddItemToObject(root ,"type" ,item);
    cJSON *data = cJSON_CreateObject();
    item = cJSON_CreateNumber(gl_uid);
    cJSON_AddItemToObject(data, "uid" ,item);
    char *out = cJSON_Print(data);
    item = cJSON_CreateString(out);
    cJSON_AddItemToObject(root ,"data" ,item);
    free(out);
    out = cJSON_Print(root);
    if(send(sock_fd ,(void *)out ,strlen(out)+1 ,0) < 0){
        perror("send: 请求服务器失败");
        return 0;
    }
    free(out);
    cJSON_Delete(data);
    cJSON_Delete(root);
    friends_t *newNode = NULL;
    while(1){
        newNode = (friends_t *)malloc(sizeof(friends_t));
        if(recv(sock_fd ,(void *)newNode ,sizeof(friends_t) ,0) <= 0){
            perror("recv: 接收服务器响应失败");
            return 0;
        }
        if(newNode->uid = 0){
            free(newNode);
            break;
        }
        List_AddHead(FriendsList ,newNode);
    }
    if(recv(sock_fd ,(void *)buf ,sizeof(buf) ,0) <= 0){
        perror("recv : 接收服务器响应失败");
        return 0;
    }
    root = cJSON_Parse(buf);
    item = cJSON_GetObjectItem(root,"res");
    int res = item -> valueint;
    if(res == 1){
        rtn = 1;
    }else{
        item = cJSON_GetObjectItem(root ,"reason");
        printf("请求失败: %s",item -> valuestring);
        rtn = 0;
    }
    cJSON_Delete(root);
    return rtn;
}

int Friends_Srv_Add(int fuid ){
    char buf[1024];
    int rtn;
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("A");
    cJSON_AddItemToObject(root ,"type" ,item);
    cJSON *data = cJSON_CreateObject();
    item = cJSON_CreateNumber(gl_uid);
    cJSON_AddItemToObject(data, "uid" ,item);
    item = cJSON_CreateNumber(fuid);
    cJSON_AddItemToObject(data ,"fuid" ,item);
    char *out = cJSON_Print(data);
    item = cJSON_CreateString(out);
    cJSON_AddItemToObject(root ,"data" ,item);
    free(out);
    out = cJSON_Print(root);
    if(send(sock_fd ,(void *)out ,strlen(out)+1 ,0) < 0){
        perror("send: 请求服务器失败");
        return 0;
    }
    free(out);
    cJSON_Delete(data);
    cJSON_Delete(root);
    if(recv(sock_fd ,(void *)buf ,sizeof(buf) ,0) < 0){
        perror("recv: 接收服务器响应失败");
        return 0;
    }
    root = cJSON_Parse(buf);
    item = cJSON_GetObjectItem(root,"res");
    int res = item -> valueint;
    if(res == 1){
        printf("好友请求发送成功!");
        getchar();
        rtn = 1;
    }else{
        item = cJSON_GetObjectItem(root ,"reason");
        printf("请求失败: %s",item -> valuestring);
        rtn = 0;
    }
    cJSON_Delete(root);
    return rtn;
}