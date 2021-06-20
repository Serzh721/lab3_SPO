#pragma once

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

#define PORT 22121
#define SA struct sockaddr
typedef int RequestType;
#define ADD 0
#define CHANGE 1
#define DELETE 2
#define CHANGE_LIST 3
#define DELETE_LIST 4
#define SERVICE 5

typedef struct {
    char owner[16];
    char listName[32];
    char title[32];
    char description[256];
    long id;
    time_t creation_time;
    time_t deadline;
} Task;

typedef struct {
    RequestType type;
    Task task;
} TaskPackage;

typedef struct ListItem {
    Task task;
    struct ListItem *next;
} ListItem;

void add(Task task, ListItem *listRoot);

void delete(Task task, ListItem *currentItem);

void change(Task task, ListItem *listRoot);

void changeList(Task task, ListItem *listRoot);

void deleteList(Task task, ListItem *currentItem);

void processRequest(TaskPackage taskPackage, ListItem *listRoot);
