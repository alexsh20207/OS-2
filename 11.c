#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <string.h>

#define FAILURE_CODE 1
#define SUCCESS_CODE 0
#define COUNT_OF_MUTEXES 3
#define COUNT_OF_THREADS 2
#define COUNT_OF_STRINGS 10
#define INDEX_OF_MUTEX_FOR_MAIN 0 
#define INDEX_OF_MUTEX_FOR_CHILD 2
#define INIT_INDEX_OF_LOCKED_MUTEX -1

pthread_mutex_t mutexes[COUNT_OF_MUTEXES];

typedef struct args_for_thread {
    const char* text;
    int count_of_strings;
    int index_of_start_mutex;
    int index_of_first_locked_mutex;
} args_for_thread;

void print_error(char* additional_msg, int errnum) {
    char buf[256];
    strerror_r(errnum, buf, sizeof buf);
    fprintf(stderr, "%s: %s\n", additional_msg, buf);
}

int destroy_mutexes(int index) {
    for (int i = 0; i < index; ++i) {
        int ret_val = pthread_mutex_destroy(&mutexes[i]);
        if (ret_val != SUCCESS_CODE) {
            print_error("Destroying mutexes error", ret_val);
            return ret_val;
        }
    }
    return SUCCESS_CODE;
}

int init_mutexes() {
    pthread_mutexattr_t attr;
    int ret_val = pthread_mutexattr_init(&attr);
    if (ret_val != SUCCESS_CODE) {
        print_error("Atrribute initialization error", ret_val);
        return ret_val;
    }
    ret_val = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (ret_val != SUCCESS_CODE) {
        print_error("Setting type of attribute error", ret_val);
        return ret_val;
    }
    for (int i = 0; i < COUNT_OF_MUTEXES; ++i) {
        ret_val = pthread_mutex_init(&mutexes[i], &attr);
        if (ret_val != SUCCESS_CODE) {
            print_error("Mutex initialization error", ret_val);
            destroy_mutexes(i);
            return ret_val;
        }
    }
    return SUCCESS_CODE;
}

int lock_mutex(int index_of_mutex) {
    int ret_val = pthread_mutex_lock(&mutexes[index_of_mutex]);
    if (ret_val != SUCCESS_CODE) {
        print_error("Mutex locking error", ret_val);
        destroy_mutexes(COUNT_OF_MUTEXES);
        return ret_val;
    }
    return SUCCESS_CODE;
}

int unlock_mutex(int index_of_mutex) {
    int ret_val = pthread_mutex_unlock(&mutexes[index_of_mutex]);
    if (ret_val != SUCCESS_CODE) {
        destroy_mutexes(COUNT_OF_MUTEXES);
        print_error("Mutex unlocking error", ret_val);
        return ret_val;
    }
    return SUCCESS_CODE;
}


void* print_strings(void* p) {
    args_for_thread* args = (args_for_thread*)p;
    int cur_mutex = 0, ret_val;

    if (args->index_of_first_locked_mutex == INIT_INDEX_OF_LOCKED_MUTEX) {
        cur_mutex = args->index_of_start_mutex;
        ret_val = lock_mutex(cur_mutex);
        if (ret_val != SUCCESS_CODE) { 
            return (void*)FAILURE_CODE; 
        }
        args->index_of_first_locked_mutex = args->index_of_start_mutex;
    }

    for (int i = 0; i < args->count_of_strings; ++i) {
        ret_val = lock_mutex((cur_mutex + 1) % COUNT_OF_MUTEXES);
        if (ret_val != SUCCESS_CODE) { 
            return (void*)FAILURE_CODE; 
        }
        printf("%d %s\n", i, args->text);
        ret_val = unlock_mutex(cur_mutex);
        if (ret_val != SUCCESS_CODE) { 
            return (void*)FAILURE_CODE; 
        }
        cur_mutex = (cur_mutex + 1) % COUNT_OF_MUTEXES;
    }

    ret_val = unlock_mutex(cur_mutex);
    if (ret_val != SUCCESS_CODE){ 
        return (void*)FAILURE_CODE; 
    }
    return (void*)SUCCESS_CODE;
}

int main() {
    int ret_val;
    pthread_t thread_id;
    args_for_thread main_args = {"Main", COUNT_OF_STRINGS, INDEX_OF_MUTEX_FOR_MAIN, INIT_INDEX_OF_LOCKED_MUTEX};
    args_for_thread child_args = {"Child", COUNT_OF_STRINGS, INDEX_OF_MUTEX_FOR_CHILD, INIT_INDEX_OF_LOCKED_MUTEX};
    
    ret_val = init_mutexes();
    if (ret_val != SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }

    ret_val = lock_mutex(main_args.index_of_start_mutex);
    if (ret_val != SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }
    main_args.index_of_first_locked_mutex = main_args.index_of_start_mutex;

    ret_val = pthread_create(&thread_id, NULL, print_strings, (void*)&child_args);
    if (ret_val != SUCCESS_CODE) {
        print_error("Creating thread error", ret_val);
        unlock_mutex(main_args.index_of_start_mutex);
        destroy_mutexes(COUNT_OF_MUTEXES);
        exit(FAILURE_CODE);
    }

    while (child_args.index_of_first_locked_mutex == INIT_INDEX_OF_LOCKED_MUTEX) {};

    void* ret_func_val = print_strings((void*)&main_args);
    if (ret_func_val != (void*)SUCCESS_CODE) {
        destroy_mutexes(COUNT_OF_MUTEXES);
        exit(FAILURE_CODE);
    }

    int ret_child_thread_val;
    ret_val = pthread_join(thread_id, (void*)&ret_child_thread_val);
    if (ret_val != SUCCESS_CODE) {
        print_error("Joining thread error", ret_val);
        destroy_mutexes(COUNT_OF_MUTEXES);
        exit(FAILURE_CODE);
    }
    if (ret_child_thread_val != SUCCESS_CODE) {
        destroy_mutexes(COUNT_OF_MUTEXES);
        exit(FAILURE_CODE);
    }

    ret_val = destroy_mutexes(COUNT_OF_MUTEXES);
    if (ret_val != SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }
    pthread_exit(NULL);
}
