#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pair.h"

extern char cwd_path[256];
extern char new_path[256];

const char *contour_start = "<contour>";
const char *contour_end   = "</contour>";
const char *point_start   = "<point ";

//const char *penpair_pre   = "penPair=\"z";
//const char *penpair_post  = "\" ";

const char *name_pre      = "name=\"";
const char *penpair_pre   = "name=\"'penPair':'z";
const char *penpair_pre2  = "'penPair':'z";
const char *penpair_post  = "'\" ";

// name="'penPair':'z1l', 'innertype':'fill'"
const char *innertypeFILL   = "'innerType':'fill";
const char *innertypeUNFILL = "'innerType':'unfill";

const char *left  = "l";
const char *right = "r";

// depend
const char*depend_x = "'dependX':'";
const char*depend_y = "'dependY':'";

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

            // 이미 존재하는 penPair, depend_x, depend_y를 덮어써야함
            if( (ptr2 = strstr(line_ptr, point_start)) != NULL) {
                ptr = ptr2 + strlen(point_start);
			    strncat(output_buf, line_ptr, strlen(line_ptr) - strlen(ptr));
            }

			// name="hr00" exist?
			if((ptr2 = strstr(line_ptr, name_pre)) != NULL) {
				ptr2 += strlen(name_pre);
                strncat(output_buf, name_pre, strlen(name_pre));
			}

			p = c->points[cnt_point];

			// 'penPair':'z1r'
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

			// 'innertype':'fill'
			if(cnt_point == 0) {
				strncat(output_buf, "', ", 3);
				if(c->contour_t == child) 
					strncat(output_buf, innertypeUNFILL, strlen(innertypeUNFILL));
				else
					strncat(output_buf, innertypeFILL, strlen(innertypeFILL));
			}

            // write depend_x
            if(strlen(p->depend_x) != 0) {
                strncat(output_buf, "', ", 3);
                strncat(output_buf, depend_x, strlen(depend_x));
                strncat(output_buf, p->depend_x, strlen(p->depend_x));
            }

            // write depend_y
            if(strlen(p->depend_y) != 0) {
                strncat(output_buf, "', ", 3);
                strncat(output_buf, depend_y, strlen(depend_y));
                strncat(output_buf, p->depend_y, strlen(p->depend_y));
            }

			// name="hr00" exist
			if(ptr2 == NULL)
				strncat(output_buf, penpair_post, strlen(penpair_post));

			// not exist
			if(ptr2 != NULL) {
				strncat(output_buf, "', ", 3);
                strncat(output_buf, ptr2, 4);
                strncat(output_buf, "\" ", 2);
            }

            if( (ptr = strstr(line_ptr, "smooth")) == NULL) {
                ptr = strstr(line_ptr, "type");
            }

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



