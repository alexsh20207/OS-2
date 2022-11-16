#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <string.h>

#define SUCCESS_CODE 0
#define FAILURE_CODE 1
#define COUNT_OF_STRINGS 10
#define INDEX_OF_MAIN_THREAD 0
#define INDEX_OF_CHILD_THREAD 1
#define COUNT_OF_THREADS 2

pthread_mutex_t mutex;
pthread_cond_t cond;
int index_of_cur_thread;

typedef struct args_for_thread {
    const char* text;
    int count_of_strings;
    int number_of_thread;
} args_for_thread;

void print_error(char* additional_msg, int errnum) {
    char buf[256];
    strerror_r(errnum, buf, sizeof buf);
    fprintf(stderr, "%s: %s\n", additional_msg, buf);
}

int destroy_mutex() {
    int ret_val = pthread_mutex_destroy(&mutex);
    if (ret_val != SUCCESS_CODE) {
        print_error("Destroying mutex error", ret_val);
        return ret_val;
    }
    return SUCCESS_CODE;
}

int destroy_cond() {
    int ret_val = pthread_cond_destroy(&cond);
    if (ret_val != SUCCESS_CODE) {
        print_error("Destroying cond error", ret_val);
        return ret_val;
    }
    return SUCCESS_CODE;
}

int destroy_objects() {
    int ret_val = destroy_mutex();
    if (ret_val != SUCCESS_CODE) {
        destroy_cond();
        return ret_val;
    }
    ret_val = destroy_cond();
    if (ret_val != SUCCESS_CODE) {
        return ret_val;
    }
    return SUCCESS_CODE;
}

int init_mutex() {
    pthread_mutexattr_t attr;
    int ret_val = pthread_mutexattr_init(&attr);
    if (ret_val != SUCCESS_CODE) {
        print_error("Attribute initialization error", ret_val);
        return ret_val;
    }

    ret_val = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (ret_val != SUCCESS_CODE) {
        print_error("Setting type of attribute error", ret_val);
        return ret_val;
    }

    ret_val = pthread_mutex_init(&mutex, &attr);
    if (ret_val != SUCCESS_CODE) {
        print_error("Mutex initialization error", ret_val);
        return ret_val;
    }
    return SUCCESS_CODE;
}

int init_cond() {
    int ret_val = pthread_cond_init(&cond, NULL);
    if (ret_val != SUCCESS_CODE) {
        print_error("Cond initialization error", ret_val);
        return ret_val;
    }
    return SUCCESS_CODE;
}

int lock_mutex() {
    int ret_val = pthread_mutex_lock(&mutex);
    if (ret_val != SUCCESS_CODE) {
        print_error("Locking mutex error", ret_val);
        destroy_objects();
        return ret_val;
    }
    return SUCCESS_CODE;
}

int unlock_mutex() {
    int ret_val = pthread_mutex_unlock(&mutex);
    if (ret_val != SUCCESS_CODE) {
        print_error("Unlocking mutex error", ret_val);
        destroy_objects();
        return ret_val;
    }
    return SUCCESS_CODE;
}

int wait_cond() {
    int ret_val = pthread_cond_wait(&cond, &mutex);
    if (ret_val != SUCCESS_CODE) {
        print_error("Waiting cond error", ret_val);
        destroy_objects();
        return ret_val;
    }
    return SUCCESS_CODE;
}

int send_signal_cond() {
    int ret_val = pthread_cond_signal(&cond);
    if (ret_val != SUCCESS_CODE) {
        print_error("Sending signal cond error", ret_val);
        destroy_objects();
        return ret_val;
    }
    return SUCCESS_CODE;
}

void* print_strings(void* p) {
    args_for_thread* args = (args_for_thread*)p;
    int ret_val;
    ret_val = lock_mutex();
    if (ret_val != SUCCESS_CODE) {
        return (void*)FAILURE_CODE;
    }
    for (int i = 0; i < args->count_of_strings; ++i) {
        while (index_of_cur_thread != args->number_of_thread) {
            ret_val = wait_cond();
            if (ret_val != SUCCESS_CODE) {
                return (void*)FAILURE_CODE;
            }
        }
        printf("%d %s\n", i, args->text);
        index_of_cur_thread = (args->number_of_thread + 1) % COUNT_OF_THREADS;
        ret_val = send_signal_cond();
        if (ret_val != SUCCESS_CODE) {
            return (void*)FAILURE_CODE;
        }
    }
    ret_val = unlock_mutex();
    if (ret_val != SUCCESS_CODE) {
        return (void*)FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

int main() {
    int ret_val;
    pthread_t thread_id;
    args_for_thread main_args = {"Main", COUNT_OF_STRINGS, INDEX_OF_MAIN_THREAD};
    args_for_thread child_args = {"Child", COUNT_OF_STRINGS, INDEX_OF_CHILD_THREAD};
    index_of_cur_thread = INDEX_OF_MAIN_THREAD;

    ret_val = init_mutex();
    if (ret_val != SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }
    ret_val = init_cond();
    if (ret_val != SUCCESS_CODE) {
        destroy_mutex();
        exit(FAILURE_CODE);
    }

    ret_val = pthread_create(&thread_id, NULL, print_strings, (void*)&child_args);
    if (ret_val != SUCCESS_CODE) {
        print_error("Creating thread error", ret_val);
        destroy_objects();
        exit(FAILURE_CODE);
    }
    void* ret_main_func_val = print_strings(&main_args);
    if (ret_main_func_val != (void*)SUCCESS_CODE) {
        destroy_objects();
        exit(FAILURE_CODE);
    }
    int thread_ret_val;
    ret_val = pthread_join(thread_id, (void*)&thread_ret_val);
    if (ret_val != SUCCESS_CODE) {
        print_error("Joining thread_error", ret_val);
        destroy_objects();
        exit(FAILURE_CODE);
    }
    if (thread_ret_val != SUCCESS_CODE) {
        destroy_objects();
        exit(FAILURE_CODE);
    }
    ret_val = destroy_objects();
    if (ret_val != SUCCESS_CODE) {
        exit(FAILURE_CODE);
    }

    pthread_exit(NULL);
}