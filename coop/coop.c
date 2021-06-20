#include "coop.h"

void add(Task task, ListItem *listRoot) {
    ListItem *newItem = malloc(sizeof(ListItem));
    newItem->task = task;
    if (listRoot->next == NULL) {
        newItem->next = NULL;
        listRoot->next = newItem;
    } else {
        newItem->next = listRoot->next;
        listRoot->next = newItem;
    }
}

void delete(Task task, ListItem *currentItem) {
    while (currentItem->next != NULL) {
        if (currentItem->next->task.id == task.id) {
            ListItem *item = currentItem->next;
            currentItem->next = currentItem->next->next;
            free(item);
            break;
        }
        currentItem = currentItem->next;
    }
}

void change(Task task, ListItem *listRoot) {
    ListItem *currentItem = listRoot->next;
    while (currentItem != NULL) {
        if (currentItem->task.id == task.id) {
            strcpy(currentItem->task.listName, task.listName);
            strcpy(currentItem->task.title, task.title);
            strcpy(currentItem->task.description, task.description);
            currentItem->task.deadline = task.deadline;
            break;
        }
        currentItem = currentItem->next;
    }
}

void changeList(Task task, ListItem *listRoot) {
    ListItem *currentItem = listRoot->next;
    while (currentItem != NULL) {
        if (strcmp(currentItem->task.listName, task.listName) == 0 &&
            strcmp(currentItem->task.owner, task.owner) == 0) {
            strcpy(currentItem->task.listName, task.title);
        }
        currentItem = currentItem->next;
    }
}

void deleteList(Task task, ListItem *currentItem) {
    while (currentItem->next != NULL) {
        if (strcmp(currentItem->next->task.listName, task.listName) == 0 &&
            strcmp(currentItem->next->task.owner, task.owner) == 0) {
            ListItem *item = currentItem->next;
            currentItem->next = currentItem->next->next;
            free(item);
        } else {
            currentItem = currentItem->next;
        }
    }
}

void processRequest(TaskPackage taskPackage, ListItem *listRoot) {
    switch (taskPackage.type) {
        case ADD: {
            add(taskPackage.task, listRoot);
            return;
        }
        case CHANGE: {
            change(taskPackage.task, listRoot);
            return;
        }
        case DELETE: {
            delete(taskPackage.task, listRoot);
            return;
        }
        case CHANGE_LIST: {
            changeList(taskPackage.task, listRoot);
            return;
        }
        case DELETE_LIST: {
            deleteList(taskPackage.task, listRoot);
            return;
        }
        default:
            return;
    }
}
