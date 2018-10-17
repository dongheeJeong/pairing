#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pair.h"

extern char cwd_path[256];
extern char new_path[256];

const char *contour_start = "<contour>";
const char *contour_end   = "</contour>";

//const char *penpair_pre   = "penPair=\"z";
//const char *penpair_post  = "\" ";

const char *name_pre      = "name=\"";
const char *penpair_pre   = "name=\"'penPair':'z";
const char *penpair_pre2  = "'penPair':'z";
const char *penpair_post  = "'\" ";


const char *left  = "l";
const char *right = "r";

void output_glif_file(char *buf, char *glif_name)
{
	extern Glif g;
	char *line_buf[LINE_SIZE] = { NULL, }, *line_ptr, *ptr, *ptr2, output_buf[BUFFER_SIZE] = { '\0', };
	char pair[10], direction;
	int i, cnt_contour, cnt_point;
	Contour *c;
	Point *p;

	make_line_by_line(buf, (char**) &line_buf);

	i = 0;
	line_ptr = strstr(line_buf[i], contour_start);
	strncpy(output_buf, line_buf[i], strlen(line_buf[i]));
	output_buf[strlen(line_buf[i])] = '\n';

	while(line_ptr == NULL) {
		line_ptr = strstr(line_buf[++i], contour_start);
		strncat(output_buf, line_buf[i], strlen(line_buf[i]));
		output_buf[strlen(output_buf)] = '\n';
	}

	line_ptr = line_buf[++i]; // start of first point

	// for all of contours
	for(cnt_contour = 0; cnt_contour < g.num_of_contours; cnt_contour++) {
		c = g.contours[cnt_contour];

		// for all of points
		for(cnt_point = 0; cnt_point < c->num_of_points; ) {
			// if already name is exist, continue;
			/*
			if((ptr = strstr(line_ptr, "name")) != NULL) {
				strncat(output_buf, line_ptr, strlen(line_ptr));
				output_buf[strlen(output_buf)] = '\n';
				line_ptr = line_buf[++i];
				cnt_point++;
				continue;
			}
			*/
			if((ptr = strstr(line_ptr, "type")) == NULL) {
				strncat(output_buf, line_ptr, strlen(line_ptr));
				output_buf[strlen(output_buf)] = '\n';
				line_ptr = line_buf[++i];
				continue;
			}

			if((ptr2 = strstr(line_ptr, name_pre)) != NULL) {
				ptr2 += strlen(name_pre);
				ptr = ptr2;
			}

			p = c->points[cnt_point];
			strncat(output_buf, line_ptr, strlen(line_ptr) - strlen(ptr));

			if(ptr2 == NULL)
				strncat(output_buf, penpair_pre, strlen(penpair_pre));
			else
				strncat(output_buf, penpair_pre2, strlen(penpair_pre2));
			if(p->direct_t == R)
				direction = 'r';
			else if(p->direct_t == L)
				direction = 'l';
			else 
				direction = '?';

			sprintf(pair, "%d%c", p->pair_num, direction);
			strncat(output_buf, pair, strlen(pair));

			if(ptr2 == NULL)
				strncat(output_buf, penpair_post, strlen(penpair_post));

			if(ptr2 != NULL)
				strncat(output_buf, "', ", 3);
			strncat(output_buf, ptr, strlen(ptr));
			output_buf[strlen(output_buf)] = '\n';
			cnt_point++;
			line_ptr = line_buf[++i];
		}

		strncat(output_buf, line_ptr, strlen(line_ptr));
		output_buf[strlen(output_buf)] = '\n';
		line_ptr = line_buf[++i];

		strncat(output_buf, line_ptr, strlen(line_ptr));
		output_buf[strlen(output_buf)] = '\n';
		line_ptr = line_buf[++i];
	}

	// cpy remain lines
	while(line_ptr != NULL) {
		strncat(output_buf, line_ptr, strlen(line_ptr));
		output_buf[strlen(output_buf)] = '\n';
		line_ptr = line_buf[++i];
	}

	write_file(output_buf, glif_name);
#ifdef debug
	printf("start\n");
	printf("%s\n", output_buf);
#endif

#ifdef PRINT_glif
	print_glif();
#endif

#ifdef PRINT_namu_mok
	print_namu_mok();
#endif
}

void make_line_by_line(char *buf, char **line_buf)
{
	int i = 0;

	line_buf[i] = strtok(buf, "\n");
	while(line_buf[i] != NULL) 
		line_buf[++i] = strtok(NULL, "\n");
}

void write_file(char *output_buf, char *glif_name)
{
	int fd;

	chdir(new_path);
	fd = open(glif_name, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if(fd < 0) {
		fprintf(stderr, "glif.c : open error for %s\n", glif_name);
		exit(1);
	}
	write(fd, output_buf, strlen(output_buf));
	close(fd);
}



