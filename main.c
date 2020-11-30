#include <pthread.h>
#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "list.h"
#include "service.h"

typedef struct Context{
	List *list;
	int ret_code;
	unsigned int delay;
} Context;

#define THREAD_CNT 10

void readLoop(List *list){
	//1 for \0
    char buff[MAX_LEN + 1];
	char new_line = 1;
	int err = SUCCESS_CODE;
	do{
		errno = SUCCESS_CODE;
		ssize_t err_read = read(STDIN_FILENO, buff, MAX_LEN);
		if (err_read == ERROR_CODE){
			assertSuccess("readLoop:read", err_read);
		}
		
		size_t read_cnt = err_read;
		//eof
		if (read_cnt == 0)
			break;

		buff[read_cnt] = '\0';
		if (new_line){
			if (buff[0] == '\n'){
				//show list
				err = showList(list);
				assertSuccess("readLoop:showList", err);
			}
			else{
				char *new_line_pos = strchr(buff, '\n');
				if (new_line_pos == NULL)
					new_line = 0;
				else 
					*new_line_pos = '\0';

				err = addElement(list, buff);
				assertSuccess("readLoop:addElement", err);
			}
		}
		else{
			char *new_line_pos = strchr(buff, '\n');
			if (new_line_pos != NULL){
				new_line = 1;
				*new_line_pos = '\0';
			}

			if (new_line_pos != buff){
				err = addElement(list, buff);
				assertSuccess("readLoop:addElement", err);
			}
		}

	} while (1);
}

void *routine(void *data){
	if (data == NULL)
		pthread_exit(SUCCESS_CODE);

	Context *cntx = (Context*)data;
	while (1){
		int err = sortList(cntx->list);
		assertSuccess("routine: sortList", err);
		sleep(cntx->delay);
	}
}

int main(int argc, char **argv){

	List list;
    int err = createList(&list);
    assertSuccess("createList", err);

	Context cntx[THREAD_CNT];
	pthread_t pid[THREAD_CNT];
	fori(THREAD_CNT){
		cntx[i].list = &list;
		cntx[i].ret_code = SUCCESS_CODE;
		cntx[i].delay = 5 + i;
		err = pthread_create(&pid[i], NULL, routine, (void*)(&cntx[i]));
		assertSuccess("main:pthread_create", err);
	}

	readLoop(&list);

	fori(THREAD_CNT){
		err = pthread_cancel(pid[i]);
		assertSuccess("main:pthread_cancel", err);
	}

	fori(THREAD_CNT){
		err = pthread_join(pid[i], NULL);
		if (err != ESRCH)
			assertSuccess("main:pthread_join", err);
	}

	err = destroyList(&list);
	assertSuccess("main:destroyList", err);

	return 0;
}
