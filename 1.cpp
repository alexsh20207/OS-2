#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define PTHREAD_CREATE_ERR -1
#define SUCCESS 0

void *printMsg(void *p) {
    for (int i = 0; i < 10; ++i) {
        printf("Child - %d\n", i);
    }
    return NULL;
}
 
int main() {
    pthread_t thread;
    int status;
    status = pthread_create(&thread, NULL, printMsg, NULL);
    if (status != SUCCESS) {
        printf("pthread_create error:%d\n", status);
        return PTHREAD_CREATE_ERR;
    }
    for (int i = 0; i < 10; ++i) {
        printf("Main - %d\n", i);
    }
    pthread_exit(NULL);
}
