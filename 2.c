#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define FAILURE_CODE -1
#define SUCCESS_CODE 0

typedef struct argForThread {
    const char* text;
    int count;
} argForThread;

void* printText(void* p) {
    argForThread* val = (argForThread*)p;
    for (int i = 0; i < val->count; ++i) {
        printf("%s %d\n", val->text, i);
    }
    return SUCCESS_CODE;
}

void print_error(char *additional_msg, int err_code) {
	char buf[256];
	strerror_r(err_code, buf, sizeof buf);
	fprintf(stderr, "%s: %s\n", additional_msg, buf);
}

int main() {
    pthread_t thread;
    int ret_val;
    
    argForThread mainArg = {"Main", 10};
    argForThread newArg = {"Child", 10};
    
    ret_val = pthread_create(&thread, NULL, printText, (void*)&newArg);
    if (ret_val != SUCCESS_CODE) {
    	print_error("pthread_create error", ret_val);
        exit(FAILURE_CODE);
    }
    ret_val = pthread_join(thread, NULL);
    if (ret_val != SUCCESS_CODE) {
    	print_error("pthread_join error", ret_val);
        exit(FAILURE_CODE);
    }
    printText(&mainArg);
    pthread_exit(NULL);
}
