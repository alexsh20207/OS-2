#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define FAILURE_CODE 1
#define SUCCESS_CODE 0
#define COUNT_OF_SEMAPHORES 2
#define COUNT_OF_STRINGS 10
#define INDEX_OF_SEM_FOR_CHILD 0 
#define INDEX_OF_SEM_FOR_MAIN 1
#define TEXT_FOR_CHILD "Child"
#define TEXT_FOR_MAIN "Main"
#define PSHARED_VALUE 0

sem_t sems[COUNT_OF_SEMAPHORES];

typedef struct args_for_thread {
    const char* text;
    int count_of_strings;
    int index_of_sem;
} args_for_thread;

void print_error(char* additional_msg, int errnum) {
    char buf[256];
    strerror_r(errnum, buf, sizeof buf);
    fprintf(stderr, "%s: %s\n", additional_msg, buf);
}

int destroy_sem(int number) {
    for (int i = 0; i < number; i++) {
        int ret_val = sem_destroy(&sems[i]);
        if (ret_val != SUCCESS_CODE) {
            perror("Destroying semaphore error");
            return ret_val;
        }
    }
    return SUCCESS_CODE;
}

int init_sem() {
    for (int i = 0; i < COUNT_OF_SEMAPHORES; ++i) {
        int ret_val = sem_init(&sems[i], PSHARED_VALUE, i);
        if (ret_val != SUCCESS_CODE) {
            destroy_sem(i);
            perror("Destroying semaphore error");
            return ret_val;
        }
    }
    return SUCCESS_CODE;
}

int wait_sem(int num) {
    int ret_val = sem_wait(&sems[num]);
    if (ret_val != SUCCESS_CODE) {
        perror("Waiting semaphore error");
        return ret_val;
    }
    return SUCCESS_CODE;
}

int post_sem(int num) {
    int ret_val = sem_post(&sems[num]);
    if (ret_val != SUCCESS_CODE) {
        perror("Posting semaphore error");
        return ret_val;
    }
    return SUCCESS_CODE;
}

void* print_strings(void* p) {
    args_for_thread* args = (args_for_thread*)p;
    int ret_val;
    for (int i = 0; i < args->count_of_strings; i++) { 
        int index_of_wait_sem = args->index_of_sem;
        int index_of_post_sem = (args->index_of_sem + 1) % COUNT_OF_SEMAPHORES;
        ret_val = wait_sem(index_of_wait_sem);
        if (ret_val != SUCCESS_CODE) {
            return (void*)FAILURE_CODE;
        }
        
        printf("%d %s\n", i, args->text);
        
        ret_val = post_sem(index_of_post_sem);
        if (ret_val != SUCCESS_CODE) {
            return (void*)FAILURE_CODE;
        }
    }
    return SUCCESS_CODE;
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    args_for_thread child_args = {TEXT_FOR_CHILD, COUNT_OF_STRINGS, INDEX_OF_SEM_FOR_CHILD};
    args_for_thread main_args = {TEXT_FOR_MAIN, COUNT_OF_STRINGS, INDEX_OF_SEM_FOR_MAIN};
    int ret_val;
    ret_val = init_sem();
    if (ret_val != SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }

    ret_val = pthread_create(&thread, NULL, print_strings, &child_args);
    if (ret_val != SUCCESS_CODE) {
        print_error("Creating thread error", ret_val);
        destroy_sem(COUNT_OF_SEMAPHORES);
        exit(FAILURE_CODE);
    }

    void *ret_val_func = print_strings(&main_args);
    if (ret_val_func != (void*)SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }

    int child_ret_val;
    ret_val = pthread_join(thread, (void*)&child_ret_val);
    if (ret_val != SUCCESS_CODE) {
        print_error("Joining thread error", ret_val);
        destroy_sem(COUNT_OF_SEMAPHORES);
        exit(FAILURE_CODE);
    }
    if (child_ret_val != SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }
    
    ret_val = destroy_sem(COUNT_OF_SEMAPHORES);
    if (ret_val != SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }
    pthread_exit(NULL);
}
