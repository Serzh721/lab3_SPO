#include <curses.h>
#include "../coop/coop.h"

int clientFinish = 0;
int clientSocketFD;
WINDOW *windows[4];
char *titles[4] = {"Lists", "Tasks", "Details", "Edit"};
int curWindow = 0;

int curListNum = 0;
int curTaskNum = 0;
int curDetailNum = 0;
int totalLists = 0;
int totalTasks = 0;
Task curList;
Task curTask;

char editBuffer[256];

ListItem taskListRoot;
ListItem listListRoot;

void refreshEditWindow();

void refreshListWindow();

void refreshTaskWindow();

void refreshDetailsWindow();

void refreshListList() {
    ListItem *currentItem = &listListRoot;
    ListItem *currentTaskItem = &taskListRoot;
    while (currentItem->next != NULL) {
        ListItem *item = currentItem->next;
        currentItem->next = currentItem->next->next;
        free(item);
    }
    while (currentTaskItem != NULL) {
        currentItem = listListRoot.next;
        int flag = 0;
        while (currentItem != NULL) {
            if (strcmp(currentItem->task.listName, currentTaskItem->task.listName) == 0) {
                flag = 1;
                break;
            }
            currentItem = currentItem->next;
        }
        if (!flag) {
            add(currentTaskItem->task, &listListRoot);
        }
        currentTaskItem = currentTaskItem->next;
    }
}

void *clientReaderThread() {
    TaskPackage taskPackage;
    while (!clientFinish) {
        read(clientSocketFD, &taskPackage, sizeof(TaskPackage));
        processRequest(taskPackage, &taskListRoot);
        if (taskPackage.type == DELETE_LIST || taskPackage.type == DELETE) {
            curTask.id = -1;
            curTaskNum = 0;
            curListNum = 0;
        }
        refreshListList();
        refreshListWindow();
    }
    return NULL;
}

void setUpSocket(char *username, char *host) {
    struct sockaddr_in servaddr;

    clientSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocketFD == -1) {
        printf("Socket can't be created\n");
        exit(0);
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(host);
    servaddr.sin_port = htons(PORT);

    if (connect(clientSocketFD, (SA *) &servaddr, sizeof(servaddr)) != 0) {
        printf("Can't connect to the server port: 22121\n");
        exit(0);
    }

    pthread_t receiver;
    pthread_create(&receiver, NULL, clientReaderThread, NULL);
    TaskPackage taskPackage;
    strcpy(taskPackage.task.owner, username);
    taskPackage.type = SERVICE;
    write(clientSocketFD, &taskPackage, sizeof(TaskPackage));
}

void sendRequest(int type, Task task) {
    TaskPackage taskPackage;
    taskPackage.type = type;
    taskPackage.task = task;
    write(clientSocketFD, &taskPackage, sizeof(TaskPackage));
}

void refreshListWindow() {
    wclear(windows[0]);
    curWindow == 0 ? box(windows[0], '|', '-') : box(windows[0], 0, 0);
    wprintw(windows[0], titles[0]);
    int count = 0;
    ListItem *item = listListRoot.next;
    while (item != NULL) {
        if (count == curListNum) {
            mvwprintw(windows[0], count + 2, 1, ">");
            curList = item->task;
        }
        mvwprintw(windows[0], count + 2, 2, item->task.listName);
        item = item->next;
        count++;
    }
    totalLists = count - 1;
    wrefresh(windows[0]);
    refreshTaskWindow();
}

void refreshTaskWindow() {
    wclear(windows[1]);
    curWindow == 1 ? box(windows[1], '|', '-') : box(windows[1], 0, 0);
    wprintw(windows[1], titles[1]);
    int count = 0;
    curTask.id = -1;
    ListItem *item = taskListRoot.next;
    while (item != NULL) {
        if (strcmp(item->task.listName, curList.listName) == 0) {
            if (count == curTaskNum) {
                mvwprintw(windows[1], count + 2, 1, ">");
                curTask = item->task;
            }
            mvwprintw(windows[1], count + 2, 2, item->task.title);
            count++;
        }
        item = item->next;
    }
    totalTasks = count;
    wrefresh(windows[1]);
    refreshDetailsWindow();
}

void refreshDetailsWindow() {
    wclear(windows[2]);
    curWindow == 2 ? box(windows[2], '|', '-') : box(windows[2], 0, 0);
    wprintw(windows[2], titles[2]);
    struct tm tm;
    char buff[32];
    if (curTask.id != -1) {
        mvwprintw(windows[2], 2, 2, "Title:");
        mvwprintw(windows[2], 2, 15, curTask.title);
        mvwprintw(windows[2], 3, 2, "Task list:");
        mvwprintw(windows[2], 3, 15, curTask.listName);
        mvwprintw(windows[2], 4, 2, "Description:");
        mvwprintw(windows[2], 4, 15, curTask.description);

        tm = *localtime(&curTask.creation_time);
        sprintf(buff, "Created:     %d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec);
        mvwprintw(windows[2], 5, 2, buff);

        if (curTask.deadline != 0) {
            tm = *localtime(&curTask.deadline);
            sprintf(buff, "Deadline:    %d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec);
            mvwprintw(windows[2], 6, 2, buff);
        } else {
            time_t curr_time = time(0);
            time_t ten_years = 365 * 12 * 30 * 24 * 60;
            curTask.deadline = curr_time + rand() % ten_years;
            mvwprintw(windows[2], 6, 2, "Deadline:    -");
        }

        mvwprintw(windows[2], curDetailNum + 2, 1, ">");
        if (curDetailNum == 0) {
            strcpy(editBuffer, curTask.title);
        }
        if (curDetailNum == 1) {
            strcpy(editBuffer, curTask.listName);
        }
        if (curDetailNum == 2) {
            strcpy(editBuffer, curTask.description);
        }
    }
    wrefresh(windows[2]);
    refreshEditWindow();
}

void refreshEditWindow() {
    wclear(windows[3]);
    curWindow == 3 ? box(windows[3], '|', '-') : box(windows[3], 0, 0);
    wprintw(windows[3], titles[3]);
    mvwprintw(windows[3], 2, 2, editBuffer);
    wrefresh(windows[3]);
}

int clientMode(char *username, char *host) {
    taskListRoot.next = NULL;
    listListRoot.next = NULL;
    setUpSocket(username, host);
    const int listWW = 20;
    const int editWH = 7;
    initscr();
    noecho();
    raw();
    curs_set(0);
    refresh();

    windows[0] = newwin(LINES - editWH, listWW, 0, 0);
    windows[1] = newwin(LINES - editWH, listWW, 0, listWW);
    windows[2] = newwin(LINES - editWH, COLS - listWW * 2, 0, listWW * 2);
    windows[3] = newwin(editWH, COLS, LINES - editWH, 0);
    refreshListWindow();

    while (!clientFinish) {
        char c = getch();
        if (c == 27) {
            clientFinish = 1;
        } else if (c == 127 && curWindow == 3) {
            editBuffer[strlen(editBuffer) - 1] = '\0';
            refreshEditWindow();
        } else if (c == 10) {
            if (curWindow == 2) {
                if (curTask.id >= 0) {
                    sendRequest(CHANGE, curTask);
                } else {
                    sendRequest(ADD, curTask);
                }
            }
            if (curWindow == 3) {
                if (curDetailNum == 0) {
                    strcpy(curTask.title, editBuffer);
                }
                if (curDetailNum == 1) {
                    strcpy(curTask.listName, editBuffer);
                }
                if (curDetailNum == 2) {
                    strcpy(curTask.description, editBuffer);
                }
                curWindow = 2;
                refreshDetailsWindow();
            }
        } else if (c == 9) {
            curWindow = (curWindow + 1) % (curTask.id == -1 ? 2 : 4);
            if (curWindow == 3) {
                refreshDetailsWindow();
            } else {
                refreshListWindow();
            }
        } else if (curWindow == 3) {
            strncat(editBuffer, &c, 1);
            refreshEditWindow();
        } else if (c == 'w' || c == 'W') {
            if (curWindow == 0) {
                curListNum = curListNum == 0 ? totalLists - 1 : curListNum - 1;
                refreshListWindow();
            }
            if (curWindow == 1) {
                curTaskNum = curTaskNum == 0 ? totalTasks - 1 : curTaskNum - 1;
                refreshTaskWindow();
            }
            if (curWindow == 2) {
                curDetailNum = curDetailNum == 0 ? 2 : curDetailNum - 1;
                refreshDetailsWindow();
            }
        } else if (c == 's' || c == 'S') {
            if (curWindow == 0) {
                curListNum = (curListNum + 1) % totalLists;
                refreshListWindow();
            }
            if (curWindow == 1) {
                curTaskNum = (curTaskNum + 1) % totalTasks;
                refreshTaskWindow();
            }
            if (curWindow == 2) {
                curDetailNum = (curDetailNum + 1) % 3;
                refreshDetailsWindow();
            }
        } else if (c == 'd' || c == 'D') {
            if (curWindow == 0) {
                sendRequest(DELETE_LIST, curList);
            }
            if (curWindow == 1) {
                sendRequest(DELETE, curTask);
            }
        } else if ((c == 'n' || c == 'N') && curWindow == 1) {
            curTask.id = -2;
            strcpy(curTask.title, "");
            strcpy(curTask.description, "");
            strcpy(curTask.listName, "");
            curTask.creation_time = time(NULL);
            curWindow = 2;
            refreshDetailsWindow();
        }
    }
    endwin();
    return 0;
}
