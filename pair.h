#ifndef __PAIR_H__
#define __PAIR_H__

#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <dirent.h>

//#define debug
//#define PRINT_glif
#define PRINT_namu_mok

#define BUFFER_SIZE 4096*4
#define LINE_SIZE 512
#define CONTOUR_SIZE 30
#define CHILD_NUM 10

enum Point_t { line, curve, etc, none };
enum Contour_t { parent, child, alone };

typedef struct point {
	bool is_paired;
	bool is_smooth;	
	int pair_num;
	enum Point_t point_t;
	int x;
	int y;
} Point;

typedef struct line {
	int x1, y1;
	int x2, y2;
	double a;
	double intercept_y;
} Line;

typedef struct contour {
	char name[20];
	enum Contour_t contour_t;
	int num_of_points;
	int num_of_line_points;
	int num_of_curve_points;

	Point **points;
	Line  **lines;

	struct contour **child;
	int num_of_childs;
	struct contour *parent;
	bool has_parent;

	bool is_namu_mok;
	int num_of_paired_points;
} Contour;

typedef struct glif {
	Contour **contours;
	int num_of_contours;
	int pair_num;
} Glif;

Glif g;
struct dirent *entry;
int num_of_namumoks;

// main.c
void make_dir(void);
void runtime(struct timeval*, struct timeval*);

// parse.c
void do_pairing(char *glif_name);
void read_glif(char *buf, char *glif_name);
int count_contours_points(char *buf, int *points_in_contours);
void init_struct_contour(int num_of_contours, int *points_in_contours);
void init_struct_point(char *buf);
void init_struct_line(void);
void detect_contour_type(Glif *);
void detect_namu_mok(Glif *);
void print_glif(void);
void print_namu_mok(void);
char * strnstr(const char *haystack, const char *needle, size_t len);
void free_glif(void);

// line.c
double distance_points(Point *a, Point *b);
bool is_point_in_contour(int x, int y, Contour *c);
Line * make_line_from_point(int x, int y);
bool is_line_intersect_line(Line*, Line*);
bool is_verti_or_horiz(Point*, Point*);

// pair.c
void pair_contour(Contour*);
void pair_contour_type_alone(Contour*);
void pair_contour_type_parent(Contour*);
bool is_valid(Contour *c, int p_ind, int p2_ind);
bool pair_points(Point*, Point*, int*);
void pair_points_force(Point*, Point*, int*);
bool is_points_in_a_contour(Point*, Point*, Contour *parent);
bool are_two_inner_points_in_contour(Point*, Point*, Contour*);

// glif.c
void output_glif_file(char *buf, char *glif_name);
void make_line_by_line(char *buf, char **line_buf);
void write_file(char *output_buf, char *glif_name);

#endif
	
