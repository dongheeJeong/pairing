#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pair.h"

double distance_points(Point *a, Point *b)
{
	return sqrt((double)(pow(a->x - b->x, 2) + pow(a->y - b->y, 2)));
}

bool is_point_in_contour(int x, int y, Contour *c)
{
	int i;
	int num_intersects = 0;
	bool is_intersect, ret;
	Line *line = make_line_from_point(x, y);

	for(i = 0; i < c->num_of_points; i++) {
		is_intersect = is_line_intersect_line(line, c->lines[i]);

		if(is_intersect) {
#ifdef debug
			printf("[%d, %d] intersects with line[%d] x1:%d, y1:%d, x2:%d, y2:%d\n", x, y, i, c->lines[i]->x1, c->lines[i]->y1, c->lines[i]->x2, c->lines[i]->y2);
#endif
			num_intersects++;
		}
	}

#ifdef debug
	printf("num_intersect : %d\n", num_intersects);
#endif
	if(num_intersects % 2 == 1)
		ret = true;
	else
		ret = false;

	free(line);
	return ret;
}

Line * make_line_from_point(int x, int y)
{
	const int X = 1000, Y = 1000;

	Line *line = malloc(sizeof(Line));
	line->x1 = x;
	line->y1 = y;
	line->x2 = X;
	line->y2 = Y;
	line->a  = (double)(y - Y) / (x - X);
	line->intercept_y = (double)(y - (line->a * x));

	return line;
}

bool is_line_intersect_line(Line *a, Line *line)
{
	double a1 = a->a, intercept1 = a->intercept_y;
	double a2 = line->a, intercept2 = line->intercept_y;
	double a3;
	double intercept3;
	double interX, interY;
	/* if line is vertical */
	if(a2 == 999) {
		interX = line->x1;
		interY = a->a * interX + a->intercept_y;
	}
	else {
		a3 = a1 - a2;
		intercept3 = intercept2 - intercept1;

		if(a3 != 0)
			interX = intercept3 / a3;
		else
			return false; // never intersects

		interY = a1 * interX + intercept1;
	}

	if(line->x1 > line->x2) {
		if(interX <= line->x2 || interX >= line->x1)
			return false;
	}
	else if(line->x2 > line->x1) {
		if(interX <= line->x1 || interX >= line->x2)
			return false;
	}

	if(line->y1 > line->y2) {
		if(interY <= line->y2 || interY >= line->y1)
			return false;
	}
	else if(line->y2 > line->y1) {
		if(interY <= line->y1 || interY >= line->y2)
			return false;
	}

	if(interX <= a->x1 || interX >= a->x2)
        return false;

    if(interY <= a->y1 || interY >= a->y2)
        return false;


#ifdef debug
	printf("interX : %f, interY : %f\n", interX, interY);
#endif

	return true;
}

bool is_verti_or_horiz(Point *a, Point *b)
{
	return (a->x == b->x || a->y == b->y)? true:false;
}







