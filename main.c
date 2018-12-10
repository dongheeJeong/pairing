#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "pair.h"

char cwd_path[256];
char new_path[256];

int main(int argc, char *argv[])
{
	int cnt = 0, cnt2 = 0;
	DIR *dirp;
	extern struct dirent *entry;
	struct timeval begin_t, end_t;

	if(argc != 2) {
		fprintf(stderr, "usage : ./Pairing [PATH]\n");
		exit(1);
	}

	realpath(argv[1], cwd_path);
    // do not make new directory, just overwrite glif file
	//make_dir();

	if((dirp = opendir(cwd_path)) == NULL) {
		fprintf(stderr, "wrong input path\n");
		exit(1);
	}

	gettimeofday(&begin_t, NULL);
	while((entry = readdir(dirp)) != NULL) {
		if(strstr(entry->d_name, ".glif") == NULL || strstr(entry->d_name, "cid") == NULL)
			continue;
#ifdef PRINT_LIST
		printf("%s\t", entry->d_name);
#endif
		cnt++; cnt2++;
		do_pairing(entry->d_name);

		if(cnt == 8) {
#ifdef PRINT_LIST
			printf("\n");
#endif
			cnt = 0;
		}
	}
	gettimeofday(&end_t, NULL);

	printf("\n%d glif files has namumok. ", num_of_namumoks);
	printf("\n%d glif files converted. ", cnt2);
	runtime(&begin_t, &end_t);

	exit(0);
}

void make_dir(void)
{
	char *ptr;
	ptr = strrchr(cwd_path, '/');

	strncpy(new_path, cwd_path, strlen(cwd_path) - strlen(ptr) + 1);
	new_path[strlen(cwd_path)-strlen(ptr)+1] = '\0';

	strcat(new_path, "glyphs_converted");

	mkdir(new_path, 0755);
}

void runtime(struct timeval *begin_t, struct timeval *end_t)
{
	const int SEC_TO_MICRO = 1000000;

	end_t->tv_sec -= begin_t->tv_sec;
	if(end_t->tv_usec < begin_t->tv_usec) {
		end_t->tv_sec--;
		end_t->tv_usec += SEC_TO_MICRO;
	}

	end_t->tv_usec -= begin_t->tv_usec;
	printf("Runtime : %ld:%06d(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);
}



