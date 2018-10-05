#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pair.h"

char buf[BUFFER_SIZE];

void do_pairing(char *glif_name)
{
	int i;
	int points_in_contours[CONTOUR_SIZE] = {0, };
	int num_of_contours = 0;

	read_glif(buf, glif_name);
	num_of_contours = count_contours_points(buf, points_in_contours);
	init_struct_contour(num_of_contours, points_in_contours);
	init_struct_point(buf);
	init_struct_line();

	detect_contour_type(&g);
	detect_namu_mok(&g);

	/* do pairing for each contours */
	for(i = 0; i < g.num_of_contours; i++) 
		pair_contour(g.contours[i]);

	output_glif_file(buf, glif_name);
	free_glif();
}

void read_glif(char *buf, char *glif_name)
{
	extern char cwd_path[256];
	int fd;
	chdir(cwd_path);
	if((fd = open(glif_name, O_RDWR)) < 0) {
		fprintf(stderr, "parse.c : open error for %s\n", glif_name);
		exit(1);
	}

	memset((void*) buf, 0, BUFFER_SIZE);
	read(fd, (void*) buf, BUFFER_SIZE);
	close(fd);
}

int count_contours_points(char *buf, int *points_in_contours)
{
	int i = 0;
	char *ptr, *ptr2;
	char *contour[CONTOUR_SIZE];
	const char *contour_prefix = "<contour>";
	const char *point_prefix = "<point ";
	const char *type_prefix = "type";

	int num_of_contours = 0;

	for(i = 0; i < CONTOUR_SIZE; i++) {
		contour[i] = NULL;
		points_in_contours[i] = 0;
	}

	/* num_of_contours */
	ptr = buf;
	while(true) {
		ptr = strstr(ptr, contour_prefix);
		contour[num_of_contours] = ptr;
		if(contour[num_of_contours] == NULL)
			break;

		ptr += strlen(contour_prefix);
		num_of_contours++;
	}

	/* points_in_contour  */
	for(i = 0; i < num_of_contours; i++) {
		ptr = contour[i];

		while(true) {
			ptr = strstr(ptr, point_prefix);
			if(ptr == NULL)
				break;
			else if(ptr > contour[i+1] && i < num_of_contours - 1)
				break;

			ptr2 = strstr(ptr, "\n");
			ptr2 = strnstr(ptr, type_prefix, strlen(ptr) - strlen(ptr2));
			if(ptr2 == NULL) {
				ptr += strlen(point_prefix);
				continue;
			}
			ptr += strlen(point_prefix);
			points_in_contours[i]++;
		}	
	}

	return num_of_contours;
}

void init_struct_contour(int num_of_contours, int *points_in_contours)
{
	int i;
	Contour **c = malloc(sizeof(Contour*) * num_of_contours);

	for(i = 0; i < num_of_contours; i++) {
		c[i] = malloc(sizeof(Contour));
		c[i]->contour_t = alone;
		c[i]->num_of_points = points_in_contours[i];
		c[i]->num_of_line_points = 0;
		c[i]->num_of_curve_points = 0;

		c[i]->points = malloc(sizeof(Point*) * points_in_contours[i]);
		c[i]->lines = NULL;

		c[i]->num_of_childs = 0;
		c[i]->child = malloc(sizeof(Contour*) * CHILD_NUM);

		c[i]->parent = NULL;
		c[i]->has_parent = false;

		c[i]->is_namu_mok = false;
		c[i]->num_of_paired_points = 0;
	}

	g.contours = c;
	g.num_of_contours = num_of_contours;
	g.pair_num = 1;
}

void init_struct_point(char *buf)
{
	int i;
	char value[6];
	char *ptr, *ptr2;
	const char *point_prefix = "<point ";
	const char *point_x_prefix = "x=\"";
	const char *point_y_prefix = "y=\"";
	const char *smooth = "smooth=\"yes\"";
	const char *type = "type=\"";
	const char *type_line = "type=\"line\"";
	const char *type_curve = "type=\"curve\"";
	const char *type_qcurve = "type=\"qcurve\"";
	enum Point_t point_t;

	int x, y;

	int num_of_points = 0;

	Contour *c;
	Point *p;

	for(i = 0, ptr = buf; i < g.num_of_contours; ) {
		/* parse info of point */
		ptr = strstr(ptr, point_prefix);
        if(ptr == NULL)
            break;
		ptr2 = strstr(ptr, "\n");
		if(ptr2 == NULL) {
			printf("%s\n", ptr);
			printf("ptr2 NULL\n");
			break;
		}

        if(strnstr(ptr, type_line, strlen(ptr) - strlen(ptr2)) != NULL) {
			point_t = line;
		}
        else if(strnstr(ptr, type_curve, strlen(ptr) - strlen(ptr2)) != NULL) {
			point_t = curve;
		}
        else if(strnstr(ptr, type_qcurve, strlen(ptr) - strlen(ptr2)) != NULL) {
			point_t = curve;
		}
        else if(strnstr(ptr, type, strlen(ptr) - strlen(ptr2)) != NULL) {
			point_t = etc;
		}
		else {
			ptr++;
			continue;
		}

		/* init struct point */
		c = g.contours[i];
		p = malloc(sizeof(Point));

		p->is_paired = false;
		p->pair_num = -1;
		if(point_t == line) {
			p->point_t = line;
			c->num_of_line_points++;
		}
		else if(point_t == curve) {
			p->point_t = curve;
			c->num_of_curve_points++;
		}
		else if(point_t == etc) {
			p->point_t = etc;
		}

		if(strnstr(ptr, smooth, strlen(ptr) - strlen(ptr2)) != NULL) 
			p->is_smooth = true;
		else
			p->is_smooth = false;

		ptr = strstr(ptr, point_x_prefix);
		ptr += strlen(point_x_prefix);
        ptr2 = strstr(ptr, "\"");
        strncpy(value, ptr, strlen(ptr) - strlen(ptr2));
        value[strlen(ptr)-strlen(ptr2)] = '\0';

        x = atoi(value);

        ptr = ptr2;
        ptr = strstr(ptr, point_y_prefix);
        ptr += strlen(point_y_prefix);

        ptr2 = strstr(ptr, "\"");
        strncpy(value, ptr, strlen(ptr) - strlen(ptr2));
        value[strlen(ptr)-strlen(ptr2)] = '\0';

        y = atoi(value);

		p->x = x;
		p->y = y;

		c->points[num_of_points] = p;

		num_of_points++;
		if(num_of_points == c->num_of_points) {
			num_of_points = 0;
			i++;
		}
	}

}

void init_struct_line(void)
{
	int i, j, tmp, num_of_lines;
	Contour *c;
	extern Glif g;

	for(i = 0; i < g.num_of_contours; i++) {
		c = g.contours[i];
		num_of_lines = c->num_of_points;
		c->lines = malloc(sizeof(Line*) * num_of_lines);

		for(j = 0, tmp = 0; j < num_of_lines; j++) {
			c->lines[j] = malloc(sizeof(Line));

			c->lines[j]->x1 = c->points[tmp]->x;
			c->lines[j]->y1 = c->points[tmp]->y;

#ifdef debug
			printf("[%d, ", tmp);
#endif

			tmp = (tmp+1) % c->num_of_points;
			c->lines[j]->x2 = c->points[tmp]->x;
			c->lines[j]->y2 = c->points[tmp]->y;
#ifdef debug
			printf("%d]", tmp);
#endif

			if(c->lines[j]->x1 == c->lines[j]->x2)
				c->lines[j]->a = 999;
			else
				c->lines[j]->a = (double)(c->lines[j]->y1 - c->lines[j]->y2) / (c->lines[j]->x1 - c->lines[j]->x2);
			c->lines[j]->intercept_y = (double)(c->lines[j]->y1) - (c->lines[j]->a)*(c->lines[j]->x1);
		}
	}
}

void detect_contour_type(Glif *g)
{
	int i, j, k, in_count;
	int num_of_contours = g->num_of_contours;
	Contour *c1, *c2;
	bool in;

	/* for every contour */
	for(i = 0; i < num_of_contours; i++) {
		c1 = g->contours[i];

		for(j = (i+1) % num_of_contours; j != i; j = (j+1) % num_of_contours) {
			c2 = g->contours[j];

			/* check is point in the contour */
			for(k = 0, in_count = 0; k < c2->num_of_points; k++) {
				in = is_point_in_contour(c2->points[k]->x, c2->points[k]->y, c1);
				if(in) in_count++;

				in = is_point_in_contour((c2->lines[k]->x1 + c2->lines[k]->x2) / 2, (c2->lines[k]->y1 + c2->lines[k]->y2) / 2, c1);
				if(in) in_count++;
			}

			/* if every points are in the contour, detect contour type*/
			if(in_count == c2->num_of_points * 2) {
				c2->contour_t = child;
				c1->contour_t = parent;

				c1->child[c1->num_of_childs++] = (struct contour*)c2;
				c2->parent = (struct contour*) c1;
				c2->has_parent = true;
			}
		}
	}
}

void detect_namu_mok(Glif *g)
{
	Contour *c;
	bool is_valid;
	Point *p_src = NULL, *p_tmp = NULL;
	int i, j, k, point_ind, cnt_curve_point, cnt_line_points[4] = {0, };
	const int valid[4][3] = { 
		{3, 1, 9}, 
		{1, 9, 1},
		{9, 1, 3},
		{1, 3, 1}
	};

	for(i = 0; i < g->num_of_contours; i++) {
		c = g->contours[i];
		if(c->num_of_line_points != 14 || c->num_of_curve_points != 4) 
			continue;
	
		// find first curve point
		point_ind = -1;
		while(1) {
			p_src = c->points[++point_ind];
			if(p_src->point_t == curve)
				break;
		}

		p_tmp = p_src;
		cnt_curve_point = 0;
		point_ind++;
		while(1) {
			if(c->points[point_ind]->point_t == line)
				cnt_line_points[cnt_curve_point]++;
			else if(c->points[point_ind]->point_t == curve) {
				cnt_curve_point++;
				p_tmp = c->points[point_ind];

				if(p_tmp == p_src)
					break;
			}

			point_ind = (point_ind+1) % c->num_of_points;
		}

		// check cnt_line_points[] with valid[] same
		for(j = 0; j < 4; j++) {

			is_valid = true;
			for(k = 0; k < 3; k++) {
				if(valid[j][k] != cnt_line_points[k]) {
					is_valid &= false;
				}
			}

			if(is_valid) {
				c->is_namu_mok = true;
				strcpy(c->name, entry->d_name);
				break;
			}
		}
	}
}

void free_glif(void)
{
	int i, j;
	Contour *c;

	for(i = 0; i < g.num_of_contours; i++) {
		c = g.contours[i];

		for(j = 0; j < c->num_of_points; j++) {
			free(c->points[j]);
			free(c->lines[j]);
		}
		free(c->points);
		free(c->lines);
		free(c->child);

		free(c);
	}
	free(g.contours);

	g.contours = NULL;
	g.num_of_contours = 0;
	g.pair_num = -1;
}

char *strnstr(const char *haystack, const char *needle, size_t len)
{
        int i;
        size_t needle_len;

        if (0 == (needle_len = strnlen(needle, len)))
                return (char *)haystack;

        for (i=0; i<=(int)(len-needle_len); i++)
        {
                if ((haystack[0] == needle[0]) &&
                        (0 == strncmp(haystack, needle, needle_len)))
                        return (char *)haystack;

                haystack++;
        }
        return NULL;
}

void print_glif(void)
{
	int i, j;
	Line *line;
	Contour *c;
	Point *p;
	char *str;

	for(i = 0; i < g.num_of_contours; i++) {
		c = g.contours[i];
		if(c->contour_t == 0)
			str = "parent";
		else if(c->contour_t == 1)
			str = "child";
		else if(c->contour_t == 2)
			str = "alone";
		printf("contours[%d] = { type: %s, num_of_points: %d, num_of_line_points: %d, num_of_curve_points: %d }\n", 
				i, str, c->num_of_points, c->num_of_line_points, c->num_of_curve_points);
	}

	for(i = 0; i < g.num_of_contours; i++) {
		c = g.contours[i];
		for(j = 0; j < c->num_of_points; j++) {
			p = c->points[j];
			printf("contours[%d].points[%d] = { x: %d, y: %d, pair_num: %d }\n", i, j, p->x, p->y, p->pair_num);
		}
	}
	for(i = 0; i < g.num_of_contours; i++) {
		c = g.contours[i];
		for(j = 0; j < c->num_of_points; j++) {
			line = c->lines[j];
			printf("contours[%d].lines[%d] = { x1: %d, y1: %d, x2: %d, y2: %d, a: %f, intercept_y: %f }\n", i, j, line->x1, line->y1, line->x2, line->y2, line->a, line->intercept_y);
		}
	}
}

void print_namu_mok(void)
{
	int i;
	Contour *c;

	for(i = 0; i < g.num_of_contours; i++) {
		c = g.contours[i];
		if(c->is_namu_mok) {
			printf("%s\n", c->name);
			num_of_namumoks++;
		}
	}
}

















