#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define PTHREAD_CREATE_ERR -1
#define SUCCESS 0

typedef struct argForThread {
    const char* text;
    int count;
} argForThread;

void* printText(void* p) {
    argForThread* val = (argForThread*)p;
    for (int i = 0; i < val->count; ++i) {
        printf("%s %d\n", val->text, i);
    }
    return SUCCESS;
}
 
int main() {
    pthread_t thread;
    int status;
    
    argForThread mainArg = {"Main", 10};
    argForThread newArg = {"Child", 10};
    
    status = pthread_create(&thread, NULL, printText, (void*)&newArg);
    if (status != SUCCESS) {
        fprintf(stderr,"pthread_create error:%d\n", status);
        return PTHREAD_CREATE_ERR;
    }
    printText(&mainArg);
    pthread_exit(NULL);
}
