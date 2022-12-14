#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define TOTAL_THREAD_NUM 4
#define FAILURE_CODE -1
#define SUCCESS_CODE 0
#define STR_TEXT "text"

typedef struct argForThread {
	int thread_num;
	int lines_num;
	const char* text;
} argForThread;

void *print_str(void *p) {
	argForThread* args = (argForThread*)p;
	for (int i = 0; i < args->lines_num; ++i) {
		printf("Thread %d - %s - line %d\n", args->thread_num, args->text, i + 1);
	}
	return SUCCESS_CODE;
}

void init_args(argForThread *args) {
	for (int i = 0; i < TOTAL_THREAD_NUM; ++i) {
		args[i].thread_num = i + 1;
		args[i].lines_num = i + 1;
		args[i].text = STR_TEXT;
	}
}

void print_error(char *additional_msg, int err_code) {
	char buf[256];
	strerror_r(err_code, buf, sizeof buf);
	fprintf(stderr, "%s: %s\n", additional_msg, buf);
}

int main(int argc, char* argv[]) {
	pthread_t thread_id[TOTAL_THREAD_NUM];
	argForThread thread_args[TOTAL_THREAD_NUM];

	init_args(thread_args);

	for (int i = 0; i < TOTAL_THREAD_NUM; ++i) {
		int ret_val;
		ret_val = pthread_create(&thread_id[i], NULL, print_str, (void*)&thread_args[i]);
		if (ret_val != SUCCESS_CODE) {
			print_error("pthread_create error", ret_val);
			exit(FAILURE_CODE);
		}
	}
	for (int i = 0; i < TOTAL_THREAD_NUM; ++i) {
		int ret_val;
		ret_val = pthread_join(thread_id[i], NULL);
		if (ret_val != SUCCESS_CODE) {
			print_error("pthread_join error", ret_val);
			exit(FAILURE_CODE);
		}
	}
	pthread_exit(NULL);
}
