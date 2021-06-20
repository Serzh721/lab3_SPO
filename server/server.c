#include "server.h"

int ConNum = 0;
int serverFinish = 0;
atomic_int MessNum = 0;
atomic_long TaskId = 0;
TaskPackage serverBuffer;
ListItem serverListRoot;
pthread_cond_t WriterCV = PTHREAD_COND_INITIALIZER;
pthread_cond_t ReaderCV = PTHREAD_COND_INITIALIZER;
pthread_mutex_t SocketLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t BufferLock = PTHREAD_MUTEX_INITIALIZER;

struct WriterThreadData {
    int fd;
    char user[16];
    int endFlag;
};

void *serverWriterThread(void *voidParams) {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    struct WriterThreadData *threadData = (struct WriterThreadData *) voidParams;

    while (!serverFinish) {
        pthread_cond_wait(&WriterCV, &lock);
        if (strcmp(threadData->user, serverBuffer.task.owner) == 0 && !threadData->endFlag) {
            pthread_mutex_lock(&SocketLock);
            write(threadData->fd, &serverBuffer, sizeof(TaskPackage));
            pthread_mutex_unlock(&SocketLock);
        }
        MessNum++;
        if (MessNum == ConNum) {
            pthread_cond_signal(&ReaderCV);
        }
    }
    return NULL;
}

void *serverReaderThread(void *voidParams) {
    int sockfd = *(int *) voidParams;
    char user[16];
    TaskPackage taskPackage;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    read(sockfd, &taskPackage, sizeof(TaskPackage));
    strcpy(user, taskPackage.task.owner);

    pthread_mutex_lock(&BufferLock);
    ListItem *currentItem = serverListRoot.next;
    while (currentItem != NULL) {
        if (strcmp(currentItem->task.owner, user) == 0) {
            taskPackage.task = currentItem->task;
            taskPackage.type = ADD;
            pthread_mutex_lock(&SocketLock);
            write(sockfd, &taskPackage, sizeof(TaskPackage));
            pthread_mutex_unlock(&SocketLock);
        }
        currentItem = currentItem->next;
    }

    ConNum++;
    printf("Server accepted the client. Username is %s. Now %i connections\n", user, ConNum);
    pthread_t writer;
    struct WriterThreadData data;
    data.fd = sockfd;
    data.endFlag = 0;
    strcpy(data.user, user);
    pthread_create(&writer, NULL, serverWriterThread, &data);
    pthread_mutex_unlock(&BufferLock);

    while (recv(sockfd, &taskPackage, sizeof(TaskPackage), 0) > 0) {
        printf("From client:\n\t Title : %s", taskPackage.task.title);
        printf("\t Description: %s", taskPackage.task.description);
        struct tm tm = *localtime(&taskPackage.task.creation_time);
        printf("\t Created: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
               tm.tm_min, tm.tm_sec);
        strcpy(taskPackage.task.owner, user);
        if (taskPackage.type == ADD) {
            taskPackage.task.creation_time = time(NULL);
            taskPackage.task.id = TaskId++;
        }
        pthread_mutex_lock(&BufferLock);
        processRequest(taskPackage, &serverListRoot);
        MessNum = 0;
        serverBuffer = taskPackage;
        pthread_cond_broadcast(&WriterCV);
        pthread_cond_wait(&ReaderCV, &lock);
        pthread_mutex_unlock(&BufferLock);
    }
    data.endFlag = 1;
    pthread_mutex_lock(&BufferLock);
    ConNum--;
    printf("User %s disconnected. Now %i connections\n", user, ConNum);
    pthread_cancel(writer);
    pthread_mutex_unlock(&BufferLock);
    return NULL;
}

void *listenerThread(void *voidParams) {
    int sock_desc, conn_desc, length;
    struct sockaddr_in servaddr, cli;
    pthread_t clientReader;
    serverListRoot.next = NULL;

    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        printf("Socket can't be created\n");
        return NULL;
    } else
        printf("Socket created\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sock_desc, (SA *) &servaddr, sizeof(servaddr))) != 0) {
        printf("Socket can't be binded\n");
        return NULL;
    } else
        printf("Socket binded\n");

    if ((listen(sock_desc, 5)) != 0) {
        printf("Can't listen the socket with port: 22121");
        return NULL;
    } else
        printf("Server listening...\n");
    length = sizeof(cli);

    while (!serverFinish) {
        conn_desc = accept(sock_desc, (SA *) &cli, &length);
        if (conn_desc < 0) {
            printf("Server can't accept a connection\n");
            return NULL;
        } else {
            pthread_create(&clientReader, NULL, serverReaderThread, (void *) &conn_desc);
        }
    }
    close(sock_desc);
    return NULL;
}

int serverMode() {
    pthread_t listener;
    pthread_create(&listener, NULL, listenerThread, NULL);
    while (!serverFinish) {
        char c = getchar();
        if (c == 'Q') {
            serverFinish = 1;
        }
    }
    return 0;
}
