#include <stdio.h>
#include <string.h>
#include "./MySQL.h"
#include "../Common/List.h"

extern MYSQL *mysql;

int Nick_Name_ADD(int uid, int target_uid, const char *nick_name) {
    char SQL[256]; // 扩大缓冲区，避免超出限制
    snprintf(SQL, sizeof(SQL),
             "INSERT INTO user_notes (uid, target_uid, nick_name) VALUES ('%d', '%d', '%s') "
             "ON DUPLICATE KEY UPDATE nick_name = '%s'",
             uid, target_uid, nick_name, nick_name);

    if (mysql_real_query(mysql, SQL, strlen(SQL))) {
        printf("MySQL Error [Nick_Name_ADD]: %s\n", mysql_error(mysql));
        return 0;
    }

    return 1;
}

char* Nick_Name_GET(int uid, int target_uid) {
    static char SQL[256];  // 扩展缓冲区，避免超出限制
    static char nick_name[256]; // 用于存储返回的昵称

    snprintf(SQL, sizeof(SQL),
             "SELECT nick_name FROM user_notes WHERE uid = '%d' AND target_uid = '%d'",
             uid, target_uid);

    // 执行查询
    if (mysql_real_query(mysql, SQL, strlen(SQL))) {
        printf("MySQL Error [Nick_Name_GET]: %s\n", mysql_error(mysql));
        return ""; // 查询失败时返回空字符串
    }

    MYSQL_RES *res = mysql_store_result(mysql);
    if (res == NULL) {
        printf("MySQL Error [Nick_Name_GET]: %s\n", mysql_error(mysql));
        return ""; // 如果没有结果，返回空字符串
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row == NULL) {
        printf("没有找到对应的用户数据\n");
        mysql_free_result(res);
        return ""; // 如果没有行数据，返回空字符串
    }

    // 处理可能为 NULL 的值
    if (row[0] != NULL) {
        strncpy(nick_name, row[0], sizeof(nick_name) - 1); // 安全复制字符串
        nick_name[sizeof(nick_name) - 1] = '\0'; // 确保以'\0'结尾
    } else {
        nick_name[0] = '\0'; // 如果昵称为 NULL，则返回空字符串
    }

    mysql_free_result(res);
    return nick_name; // 返回昵称
}
