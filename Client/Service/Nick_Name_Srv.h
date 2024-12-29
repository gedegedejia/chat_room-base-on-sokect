#ifndef _NICK_NAME_SRV_H
#define _NICK_NAME_SRV_H

#include "../Common/cJSON.h"
#include "./Connect.h"
#include "../Common/List.h"

void Update_Nick_Name(friends_t *FriendsList, char *name, char *nick_sname) ;
void Nick_Name_Send(char *name, char *nick_name);

#endif

