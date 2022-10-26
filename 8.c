#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NUM_OF_ARGS 2
#define ARG_INDEX_FOR_NUM_THREADS 1
#define RADIX 10
#define MIN_NUM_THREAD 1
#define MAX_NUM_THREAD 1024
#define SUCCESS_CODE 0
#define FAILURE_CODE -1
#define NUM_OF_ITERATIONS 5120000
#define INDEX_OF_FIRST_THREAD 0
#define INDEX_OF_FIRST_ITERATION 0

#define TERINATING_SYMBOL '\0'
#define ERR_NUM_OF_ARGS "Wrong number of arguments\n"
#define ERR_INVAL_ARG "This is not a number\n"
#define ERR_RANGE "Number is out of range\n"
#define INSTRUCTION_TEXT "You need to write a number of threads: a number beetwen 1 and 1024\n"
#define ERR_CREATE_THREAD_PARTIAL_SUM "Create thread for partial sum error"
#define ERR_JOIN_WITH_PARTIAL_SUM "Join with partial sum error"

typedef struct argsForThread {
    int start_index;
    int num_of_iterations;
    double res;
} argsForThread;


void *calc_partial_sum(void *arg) {
    argsForThread *args = (argsForThread*)arg;
    for (int i = args->start_index; i < args->start_index + args->num_of_iterations; ++i) {
        args->res += 1.0 / (i * 4.0 + 1.0);
        args->res -= 1.0 / (i * 4.0 + 3.0);
    }
    return SUCCESS_CODE;
}

int check_input(int argc, char **argv) {
    if (argc != NUM_OF_ARGS) {
        fprintf(stderr, ERR_NUM_OF_ARGS);
        return FAILURE_CODE;
    }
    char *end;
    long int num_of_threads = strtol(argv[ARG_INDEX_FOR_NUM_THREADS], &end, RADIX);
    if (*end != TERINATING_SYMBOL) {
        fprintf(stderr, ERR_INVAL_ARG);
        return FAILURE_CODE;
    }
    if (num_of_threads < MIN_NUM_THREAD || num_of_threads > MAX_NUM_THREAD) {
        fprintf(stderr, ERR_RANGE);
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

void init_args(int num_of_threads, argsForThread thread_args[]) {
    int iteration_num = NUM_OF_ITERATIONS / num_of_threads;
    int modulo_of_iterations = NUM_OF_ITERATIONS % num_of_threads;

    for (int thread_num = INDEX_OF_FIRST_THREAD; thread_num < num_of_threads; thread_num++) {
        thread_args[thread_num].num_of_iterations = iteration_num;
        if (thread_num < modulo_of_iterations) {
            thread_args[thread_num].num_of_iterations++;
        }

        if (thread_num == INDEX_OF_FIRST_THREAD) {
            thread_args[thread_num].start_index = INDEX_OF_FIRST_ITERATION;
        }
        else {
            thread_args[thread_num].start_index = thread_args[thread_num - 1].start_index 
                + thread_args[thread_num - 1].num_of_iterations;
        }
        
        thread_args[thread_num].res = 0;
    }
}

int create_threads(int num_of_threads, pthread_t thread_id[], argsForThread thread_args[]) {
    for (int thread_num = INDEX_OF_FIRST_THREAD; thread_num < num_of_threads; ++thread_num) {
        int ret_val = pthread_create(&thread_id[thread_num], NULL, calc_partial_sum, (void*)&thread_args[thread_num]);
        if (ret_val != SUCCESS_CODE) {
            return ret_val;
        }
    }
    return SUCCESS_CODE;
}

int join_threads(int num_of_threads, pthread_t thread_id[], argsForThread thread_args[]) {
    for (int thread_num = INDEX_OF_FIRST_THREAD; thread_num < num_of_threads; ++thread_num) {
        int ret_val = pthread_join(thread_id[thread_num], NULL);
        if (ret_val != SUCCESS_CODE) {
            return ret_val;
        }
    }
    return SUCCESS_CODE;
}

double get_sum(int num_of_threads, argsForThread *thread_args) {
    double sum = 0;
    for (int thread_num = INDEX_OF_FIRST_THREAD; thread_num < num_of_threads; thread_num++) {
        sum += thread_args[thread_num].res;
    }
    return sum;
}

void print_error(char *msg, int ret_code) {
    char buf[256];
    strerror_r(ret_code, buf, sizeof buf);
    fprintf(stderr, "%s: %s\n", msg, buf);
}

int calc_pi(int num_of_threads, double *res) {
    pthread_t thread_id[num_of_threads];
    argsForThread thread_args[num_of_threads];

    init_args(num_of_threads, thread_args);

    int ret_val = create_threads(num_of_threads, thread_id, thread_args);
    if (ret_val != SUCCESS_CODE) {
        print_error("Creating threads error", ret_val);
        return FAILURE_CODE;
    }

    ret_val = join_threads(num_of_threads, thread_id, thread_args);
    if (ret_val != SUCCESS_CODE) {
        print_error("Joining threads error", ret_val);
        return FAILURE_CODE;
    }

    *res = get_sum(num_of_threads, thread_args);
    *res = *res * 4.0;

    return SUCCESS_CODE;
}

int main(int argc, char **argv) {
    int ret_val = check_input(argc, argv);
    if (ret_val != SUCCESS_CODE) {
    	printf("%s", INSTRUCTION_TEXT);
        exit(EXIT_FAILURE);
    }

    long int num_of_threads = strtol(argv[ARG_INDEX_FOR_NUM_THREADS], NULL, RADIX);
    double res = 0;

    ret_val = calc_pi(num_of_threads, &res);
    if (ret_val != SUCCESS_CODE) {
        exit(EXIT_FAILURE);
    }

    printf("%.20f\n", res);
    pthread_exit(NULL);
}
