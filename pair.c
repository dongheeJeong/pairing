#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pair.h"

#define PEN_WIDTH_85  85
#define PEN_WIDTH_120 120

void pair_contour(Contour *c)
{
	if(c->contour_t == alone)
		pair_contour_type_alone(c);
	else if(c->contour_t == parent)
		pair_contour_type_parent(c);
}

void pair_contour_type_alone(Contour *c)
{
	int i, j, k, l, flag;
	int num_of_points = c->num_of_points, min_ind, min_verti_hori_ind;
	bool is_paired;
	double min_distance, tmp_distance;
	double min_verti_hori_distance;

	// foreach points, check rig.t left point If can be paired
	for(i = 0; i < num_of_points; i++) {
		if(c->points[i]->is_paired)
			continue;

		min_distance = -1;
		min_ind = -1;

		j = (i+1) % num_of_points;
		if(i == 0)
			k = num_of_points - 1;
		else
			k = (i-1) % num_of_points;

		if(!c->points[j]->is_paired && is_valid(c, i, j)) {
			min_distance = distance_points(c->points[i], c->points[j]);
			min_ind = j;
		}

		if(!c->points[k]->is_paired && is_valid(c, i, k)) {
			tmp_distance = distance_points(c->points[i], c->points[k]);
			if(min_distance == -1 || tmp_distance < min_distance) {
				min_ind = k;
			}
		}

		// pair more near one, if more near than PAN_WIDTH
		// check two points next, prev line direction same
		if(min_ind != -1) {
			is_paired = pair_points(c->points[i], c->points[min_ind], &g.pair_num);
			if(is_paired) {
				// determine direction_t
				determine_direction_t(c->points[i], c->points[min_ind], SIDE_TO_SIDE);
				c->num_of_paired_points += 2;
				continue;
			}
		}
	}

	if(c->is_namu_mok) {
		exception_namu_mok(c);
		return ;
	}

	// pair the remain points
	for(i = l = 0; l < num_of_points; i = (i+1) % num_of_points, l++) {
		if(c->points[i]->is_paired)
			continue;

		min_distance = -1;
		min_ind = -1;
		min_verti_hori_distance = -1;
		min_verti_hori_ind = -1;

		j = (i+2) % num_of_points;
		flag = 0;
		/* some points may not be paired, for example vertical or horizental but to far */
		for(k = 0; k < num_of_points; k++) {
			while(c->points[j]->is_paired) {
				j = (j+1) % num_of_points;
				//if(i == j) flag = 1;
			}
			if(flag == 1)
				break;
			// smooth pair with smooth
			if(c->points[i]->is_smooth != c->points[j]->is_smooth) {
				j = (j+1) % num_of_points;
				//if(i == j) flag = 1;
				continue;
			}
			else if((i-j) == 1 || (i-j) == -1 || (i-j) == (num_of_points-1) || (i-j) == -1 * (num_of_points-1) || i == j) {
				j = (j+1) % num_of_points;
				continue;
			}

			// 두 점을 잇는 직선 상의 임의의 점이 contour내부에 속하는지 확인
			if(!are_two_inner_points_in_contour(c->points[i], c->points[j], c)) {
				j = (j+1) % num_of_points;
				continue;
			}

			tmp_distance = distance_points(c->points[i], c->points[j]);
			if(is_verti_or_horiz(c->points[i], c->points[j])) {
				if(min_verti_hori_distance == -1 || tmp_distance < min_verti_hori_distance) {
					min_verti_hori_distance = tmp_distance;
					min_verti_hori_ind = j;
				}
			}
			else {
				if(min_distance == -1 || tmp_distance < min_distance) {
					min_distance = tmp_distance;
					min_ind = j;
				}
			}
			j = (j+1) % num_of_points;
		}

		// priority of verti hori points is higher than else points
		if(min_verti_hori_ind == -1 && min_ind == -1) {
			is_paired = false;
		}
		else if(min_verti_hori_ind != -1) {
			is_paired = pair_points(c->points[i], c->points[min_verti_hori_ind], &g.pair_num);
			if(is_paired)
				determine_direction_t(c->points[i], c->points[min_verti_hori_ind], CROSS_TO_CROSS);
			//if(!is_paired && min_ind != -1)
				//is_paired = pair_points(c->points[i], c->points[min_ind], &g.pair_num);
		}
		else if(min_ind != -1) {
			is_paired = pair_points(c->points[i], c->points[min_ind], &g.pair_num);
			if(is_paired) 
				determine_direction_t(c->points[i], c->points[min_ind], CROSS_TO_CROSS);
		}
		else {
			; 
		}

		if(is_paired) 
			c->num_of_paired_points += 2;
	}

	// force to all the points be paired on alone contour
	if(c->contour_t == alone) {
		i = 0;
		while(c->num_of_paired_points < c->num_of_points) {

			while(c->points[i]->is_paired) {
				i = (i+1) % c->num_of_points;
			}

			j = (i+1) % c->num_of_points;
			while(c->points[j]->is_paired) {
				j = (j+1) % c->num_of_points;
				if(i == j) break;
			}
			pair_points_force(c->points[i], c->points[j], &g.pair_num);
			determine_direction_t(c->points[i], c->points[j], I_DONT_KNOW);
			c->num_of_paired_points += 2;

			i = (j+1) % c->num_of_points;
		}
	}
}

/**
 * First, do pairing.on parent contour(side by side point pairing.
 * Second, and do paring.remain points include all of child points
 **/
void pair_contour_type_parent(Contour *c)
{
	int i, j, ind = 0;
	int num_of_child_points = 0;
	int min_ind;
	int num_of_child_paired = 0;
	double min_distance, tmp_distance;
	Point **child_point_arr = NULL;
	Point *p = NULL, *p2 = NULL;

	//
	//  important
	//
	pair_contour_type_alone(c);

	for(i = 0; i < c->num_of_childs; i++) 
		num_of_child_points += (c->child[i])->num_of_points;

	child_point_arr = malloc(sizeof(Point) * num_of_child_points);
	for(i = 0; i < c->num_of_childs; i++) {
		for(j = 0; j < c->child[i]->num_of_points; j++) {
			child_point_arr[ind++] = c->child[i]->points[j];
		}
	}

	// parent points are completed in this loop
	for(i = c->num_of_paired_points, ind = 0; i < c->num_of_points; i++) {
		/* find not paired point in the parent contour */
		p = c->points[ind];
		while(p->is_paired) {
			if(ind == c->num_of_points - 1)
				return ;
			p = c->points[++ind];
		}

		// search nearest child point
		min_distance = -1;
		min_ind = -1;
		for(j = 0; j < num_of_child_points; j++) {
			if(child_point_arr[j]->is_paired)
				continue;

			tmp_distance = distance_points(p, child_point_arr[j]);
			if(min_distance == -1 || tmp_distance < min_distance) {
				min_distance = tmp_distance;
				min_ind = j;
			}
		}

		if(min_ind != -1) {
			//pair_points_force(p, child_point_arr[min_ind], &(g.pair_num));
			if(pair_points(p, child_point_arr[min_ind], &(g.pair_num))) {
				determine_direction_t(p, child_point_arr[min_ind], PARENT_TO_CHILD);
				c->num_of_paired_points++;
				num_of_child_paired++;
			}
			else {
				if(ind == c->num_of_points -1)
					return;
				else
					ind++;
			}
		}
	}

	// remain child points are completed in this loop
	int p_ind = 0, p2_ind = 0;
	for(i = num_of_child_paired; i < num_of_child_points; i = i+2) {
		p = child_point_arr[p_ind];
		while(p->is_paired) {
			if(p_ind == num_of_child_points - 1)
				return;
			p = child_point_arr[++p_ind];
		}

		p2_ind = (p_ind + 1) % num_of_child_points;
		p2 = child_point_arr[p2_ind];
		min_distance = -1;
		min_ind = -1;

		/* find nearest point */
		while(true) {
#ifdef debug
			printf("p_ind : %d, p2_ind : %d\n", p_ind, p2_ind);
#endif
			if(p_ind == p2_ind)
				break;

			while(p2->is_paired) {
				p2_ind = (p2_ind + 1) % num_of_child_points;
				p2 = child_point_arr[p2_ind];
				if(p_ind == p2_ind) 
					goto EXIT;
			}

			/* if two points are in the same contour */
			if(is_points_in_a_contour(p, p2, c)) {
				p2_ind = (p2_ind + 1) % num_of_child_points;
				p2 = child_point_arr[p2_ind];
				if(p_ind == p2_ind)
					goto EXIT;
				continue;
			}

			// search nearest child point
			tmp_distance = distance_points(p, p2);
			if(min_distance == -1 || tmp_distance < min_distance) {
				min_distance = tmp_distance;
				min_ind = p2_ind;
			}

			p2_ind = (p2_ind + 1) % num_of_child_points;
			p2 = child_point_arr[p2_ind];
		}
EXIT:
		if(min_ind != -1) {
			pair_points_force(p, child_point_arr[min_ind], &g.pair_num);
			determine_direction_t(p, child_point_arr[min_ind], CHILD_TO_CHILD);
		}
		num_of_child_paired += 2;
		c->num_of_paired_points++;
	}

	free(child_point_arr);
}

// continuous index, abs(p_ind - p2_ind) = 1 or first_index and last_index
bool is_valid(Contour *c, int p_ind, int p2_ind)
{
	int prev, next;
	int plus_or_minus = 0;
	int num_of_points = c->num_of_points;
	Point *p1, *p2, *p3, *p4;

	if(c->points[p_ind]->point_t == curve && c->points[p2_ind]->point_t == curve)
		return false;

	if(c->points[p_ind]->is_smooth | c->points[p2_ind]->is_smooth)
		return false;

	if(p_ind == 0 && p2_ind == c->num_of_points-1) {
		p2 = c->points[p2_ind];
		p3 = c->points[p_ind];
		prev = num_of_points - 2;
		next = p_ind + 1;
	}
	else if(p_ind == c->num_of_points-1 && p2_ind == 0) {
		p2 = c->points[p_ind];
		p3 = c->points[p2_ind];
		prev = num_of_points - 2;
		next = p2_ind + 1;
	}
	else if(p_ind < p2_ind) {
		p2 = c->points[p_ind];
		p3 = c->points[p2_ind];
		if(p_ind == 0)
			prev = num_of_points - 1;
		else
			prev = (p_ind-1) % num_of_points;
		next = (p2_ind+1) % num_of_points;
	}
	else {
		p2 = c->points[p2_ind];
		p3 = c->points[p_ind];
		if(p2_ind == 0)
			prev = num_of_points - 1;
		else
			prev = (p2_ind-1) % num_of_points;
		next = (p_ind+1) % num_of_points;
	}

	p1 = c->points[prev];
	p4 = c->points[next];

	// when direction different each
	if(p2->x == p3->x) {
		if((p1->x - p2->x)*(p3->x - p4->x) > 0)
			return false;

		if((p1->x - p2->x) < 0)
			plus_or_minus = -1;
		else
			plus_or_minus = 1;
	}
	else if(p2->y == p3->y) {
		if((p1->y - p2->y)*(p3->y - p4->y) > 0)
			return false;

		if((p1->y - p2->y) < 0)
			plus_or_minus = -1;
		else
			plus_or_minus = 1;
	}
	// not a vertical or horizontal line (diagonal line)
	else if(p1->y == p2->y && p3->y == p4->y) {
		// same direction
		if(p1->x < p2->x && p3->x > p4->x) {
			plus_or_minus = -1;
		}
		else if(p1->x > p2->x && p3->x < p4->x) {
			plus_or_minus = 1;
		}
		// different direction
		else 
			return false;
	}
	else if(p1->x == p2->x && p3->x == p4->x) {
		// same direction
		if(p3->y < p4->y && p2->y < p1->y) {
			plus_or_minus = 1;
		}
		else if(p3->y > p4->y && p2->y > p1->y) {
			plus_or_minus = -1;
		}
		// different direction
		else
			return false;
	}
	// different direction flase
	else {
		if((p1->x - p2->x) * (p3->x - p4->x) >= 0) {
			return false;
		}
		else if((p1->y - p2->y) * (p3->y - p4->y) >= 0) {
			return false;
		}
		return true;
	}

	// if out of contour -> return false
	const int depart = 15;
	double interX = (p2->x + p3->x) / 2;
	double interY = (p2->y + p3->y) / 2;

	if(p2->x == p3->x || (p1->y == p2->y && p3->y == p4->y))
		interX += (plus_or_minus) * depart;
	else if(p2->y == p3->y || (p1->x == p2->x && p3->x == p4->x))
		interY += (plus_or_minus) * depart;

	return is_point_in_contour((int) interX, (int) interY, c);
}


/**
 * in two points, if they are on hori or vertical, return true on strcit PEN_WIDTH
 * buf if they are on diag.nal, return true on loose PEN_WIDTH
 **/
bool pair_points(Point *a, Point *b, int *pair_num)
{
	double distance;
	int pen_width;

	if(a == b)
		return false;

	if(is_verti_or_horiz(a, b))
		pen_width = PEN_WIDTH_85;
	else
		pen_width = PEN_WIDTH_120;

	distance = distance_points(a, b);
	if(distance < pen_width) {
		a->pair_num = *pair_num;
		a->is_paired = true;
		b->pair_num = *pair_num;
		b->is_paired = true;

		(*pair_num)++;
		return true;
	}
	else
		return false;
}

void pair_points_force(Point *a, Point *b, int *pair_num)
{
	a->pair_num = *pair_num;
	a->is_paired = true;
	b->pair_num = *pair_num;
	b->is_paired = true;

	(*pair_num)++;
}

/**
 * find children contours from input parent
 * if a and b are in the one of child contour, return true
 **/
bool is_points_in_a_contour(Point *a, Point *b, Contour *parent)
{
	int i, j;
	Contour *child = NULL;
	bool is_a_in = false, is_b_in = false;

	for(i = 0; i < g.num_of_contours; i++) {
		is_a_in = false; is_b_in = false;

		if(g.contours[i]->has_parent && g.contours[i]->parent == parent) {
			child = g.contours[i];

			for(j = 0; j < child->num_of_points; j++) {
				if(child->points[j] == a)
					is_a_in = true;
				else if(child->points[j] == b)
					is_b_in = true;
			}

			if(is_a_in && is_b_in)
				return true;
		}
	}

	return false;
}

bool are_two_inner_points_in_contour(Point *a, Point *b, Contour *c)
{
	bool ret = true;
	double x1, x2 ,y1, y2;

	x1 = ((double)(b->x - a->x) / 3) + a->x;
	y1 = ((double)(b->y - a->y) / 3) + a->y;

	x2 = ((double)(b->x - a->x) / 3) * 2 + a->x;
	y2 = ((double)(b->y - a->y) / 3) * 2 + a->y;

	if(!is_point_in_contour(x1, y1, c))
		ret &= false;

	if(!is_point_in_contour(x2, y2, c))
		ret &=false;

	return ret;
}

void exception_namu_mok(Contour *c)
{
	int i, index, index2;
	int curve_index1, curve_index2;
	Point *curve_p1 = NULL, *curve_p2 = NULL;

	// find two unpaired curve points
	for(i = index = 0; i < c->num_of_points; i++) {
		if(!c->points[index]->is_paired && c->points[index]->point_t == curve) {
			if(curve_p1 == NULL) {
				curve_p1 = c->points[index];
				curve_index1 = index;
			}
			else {
				curve_p2 = c->points[index];
				curve_index2 = index;
			}
		}

		index = next_index(c, index);
	}

	// pair two unpaired curve points correctlly
	index = prev_index(c, curve_index1);
	index = prev_index(c, index);
	if(c->points[index]->is_paired && c->points[index]->point_t == curve) {
		index = prev_index(c, index);
		pair_points_force(curve_p1, c->points[index], &g.pair_num);

		index = prev_index(c, curve_index2);
		index = prev_index(c, index);
		index = prev_index(c, index);
		pair_points_force(curve_p2, c->points[index], &g.pair_num);
	}
	else {
		index = next_index(c, curve_index1);
		index = next_index(c, index);
		index = next_index(c, index);
		pair_points_force(curve_p1, c->points[index], &g.pair_num);

		index = next_index(c, curve_index2);
		index = next_index(c, index);
		index = next_index(c, index);
		pair_points_force(curve_p2, c->points[index], &g.pair_num);
	}

	// pair final two line points
	for(i = index = 0; i < c->num_of_points; i++) {
		if(c->points[index]->point_t == line && !c->points[index]->is_paired)
			break;
		index = next_index(c, index);
	}

	index2 = next_index(c, index);
	index2 = next_index(c, index2);
	index2 = next_index(c, index2);

	if(c->points[index2]->is_paired) {
		index2 = prev_index(c, index);
		index2 = prev_index(c, index2);
		index2 = prev_index(c, index2);
	}

	pair_points_force(c->points[index], c->points[index2], &g.pair_num);
}

void determine_direction_t(Point *a, Point *b, int flag)
{
    if(a->direct_t != no && b->direct_t != no)
        return ;

	switch(flag) {
		case SIDE_TO_SIDE : 
		case CROSS_TO_CROSS :
		case CHILD_TO_CHILD :
		case I_DONT_KNOW :
			if(a->x == b->x) {
				if(a->y > b->y) {
					a->direct_t = R;
					b->direct_t = L;
				}
				else {
					a->direct_t = L;
					b->direct_t = R;
				}
			}
			else if(a->y == b->y) {
				if(a->x > b->x) {
					a->direct_t = R;
					b->direct_t = L;
				}
				else {
					a->direct_t = L;
					b->direct_t = R;
				}
			}
			// 에잇 모르겠다
			else {
				if(a->x > b->x) {
					a->direct_t = R;
					b->direct_t = L;
				}
				else {
					a->direct_t = L;
					b->direct_t = R;
				}
			}
			break;
		case PARENT_TO_CHILD :
			// in this case, a would parent, b would child
			a->direct_t = R;
			b->direct_t = L;
			break;
	}
}


