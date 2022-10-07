#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NUM_OF_ARGS 2
#define INDEX_FOR_NUM_THREADS 1
#define RADIX 10
#define MIN_NUM_THREAD 1
#define MAX_NUM_THREAD 256
#define SUCCESS 0
#define ERROR_CODE -1
#define NUM_OF_STEPS 5120000
#define INDEX_OF_FIRST_THREAD 0

#define TERINATING_SYMBOL '\0'
#define ERR_NUM_OF_ARGS "You need only one argument\n"
#define ERR_INVAL_ARG "This is not a number\n"
#define ERR_RANGE "Number is out of range\n"
#define INSTRUCTION_TEXT "You need to write a number of threads: a number beetwen 1 and 256\n"
#define ERR_MALLOC "Malloc error"
#define ERR_CREATE_THREAD_PARTIAL_SUM "Create thread for partial sum error"
#define ERR_JOIN_WITH_PARTIAL_SUM "Join with partial sum error"

typedef struct args_for_thread {
    int start_index;
    int num_of_iterations;
    double res;
} args_for_thread;

int check_input(int argc, char **argv) {
    if (argc != NUM_OF_ARGS) {
        fprintf(stderr, ERR_NUM_OF_ARGS);
        return ERROR_CODE;
    }
    char *end;
    long num_of_threads = strtol(argv[INDEX_FOR_NUM_THREADS], &end, RADIX);
    if (*end != TERINATING_SYMBOL) {
        fprintf(stderr, ERR_INVAL_ARG);
        return ERROR_CODE;
    }
    if (num_of_threads < MIN_NUM_THREAD || num_of_threads > MAX_NUM_THREAD) {
        fprintf(stderr, ERR_RANGE);
        return ERROR_CODE;
    }
    return SUCCESS;
}

void *calc_partial_sum(void *arg) {
    args_for_thread *args = (args_for_thread* )arg;
    double partial_sum = 0;
    int start_index = args->start_index;
    for (int i = args->start_index; i < args->num_of_iterations; ++i) {
        partial_sum += 1.0 / ((i + start_index) * 4.0 + 1.0);
        partial_sum -= 1.0 / ((i + start_index) * 4.0 + 3.0);
    }
    args->res = partial_sum;
    pthread_exit(NULL);
}

int join_threads(int num_of_threads, pthread_t *thread_id, args_for_thread *thread_args, double *sum) {
    for (int thread_num = 0; thread_num < num_of_threads; ++thread_num) {
        int status = pthread_join(thread_id[thread_num], NULL);
        if (status != SUCCESS) {
            return status;
        }
        *sum += thread_args[thread_num].res;
    }
    return SUCCESS;
}

int create_threads(int num_of_threads, pthread_t *thread_id, args_for_thread *thread_args) {
    int iteration_num = NUM_OF_STEPS / num_of_threads;
    int modulo_of_iterations = NUM_OF_STEPS % num_of_threads;

    for (int thread_num = 0; thread_num < num_of_threads; ++thread_num) {
        thread_args[thread_num].start_index = thread_num * iteration_num;
        thread_args[thread_num].num_of_iterations = iteration_num;

        if (thread_num < modulo_of_iterations) {
            thread_args[thread_num].num_of_iterations++;
        }

        int status = pthread_create(&thread_id[thread_num], NULL, calc_partial_sum, (void*)&thread_args[thread_num]);
        if (status != SUCCESS) {
            return status;
        }
    }
    return SUCCESS;
}

void free_resources(pthread_t *thread_id, args_for_thread *thread_args) {
    free(thread_args);
    free(thread_id);
}

int calc_pi(int num_of_threads, double *res) {
    pthread_t *thread_id = (pthread_t *)malloc(num_of_threads * sizeof(pthread_t));
    args_for_thread *thread_args = (args_for_thread *)malloc(num_of_threads * sizeof(args_for_thread));

    if (thread_id == NULL || thread_args == NULL) {
        free_resources(thread_id, thread_args);
        perror(ERR_MALLOC);
        return ERROR_CODE;
    }

    int status = create_threads(num_of_threads, thread_id, thread_args);
    if (status != SUCCESS) {
        free_resources(thread_id, thread_args);
        fprintf(stderr, "%s\n", strerror(status));
        return ERROR_CODE;
    }

    *res = 0;
    status = join_threads(num_of_threads, thread_id, thread_args, res);
    if (status != SUCCESS) {
        free_resources(thread_id, thread_args);
        fprintf(stderr, "%s\n", strerror(status));
        return ERROR_CODE;
    }

    free_resources(thread_id, thread_args);
    *res = *res * 4.0;
    return SUCCESS;
}

int main(int argc, char **argv) {
    int status = check_input(argc, argv);
    if (status == ERROR_CODE) {
    	printf("%s", INSTRUCTION_TEXT);
        exit(EXIT_FAILURE);
    }

    long num_of_threads = (long)strtol(argv[INDEX_FOR_NUM_THREADS], NULL, RADIX);
    double res = 0;

    status = calc_pi(num_of_threads, &res);
    if (status != SUCCESS) {
        exit(EXIT_FAILURE);
    }

    printf("%.20f\n", res);
    exit(EXIT_SUCCESS);
}
