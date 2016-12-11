#include "pv_bezier.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"


struct PvBezier{
	bool is_close;
	size_t anchor_points_num;
	PvAnchorPoint *anchor_points;
};


PvBezier *pv_bezier_new()
{
	PvBezier *self = malloc(sizeof(PvBezier));
	pv_assert(self);

	self->is_close = false;
	self->anchor_points_num = 0;
	self->anchor_points = NULL;

	return self;
}

void pv_bezier_free(PvBezier *self)
{
	if(NULL != self->anchor_points){
		free(self->anchor_points);
	}

	free(self);
}

PvBezier *pv_bezier_copy_new(const PvBezier *self)
{
	PvBezier *new_ = pv_bezier_new();

	*new_ = *self;

	size_t num = pv_bezier_get_anchor_point_num(self);

	if(0 < num){
		size_t size = num * sizeof(PvAnchorPoint);
		new_->anchor_points = malloc(size);
		pv_assert(new_->anchor_points);

		memcpy(new_->anchor_points, self->anchor_points, size);
		new_->anchor_points_num = self->anchor_points_num;
	}

	return new_;
}

void pv_anchor_points_copy_(PvAnchorPoint *dst_aps, PvAnchorPoint *src_aps, size_t num)
{
	for(int i = 0; i < (int)num; i++){
		dst_aps[i] = src_aps[i];
	}
}

PvBezier *pv_bezier_copy_new_range(const PvBezier *src_bezier, int head, int foot)
{
	pv_assert(src_bezier);
	size_t src_num = pv_bezier_get_anchor_point_num(src_bezier);
	pv_assertf((head <= (int)src_num && foot <= (int)src_num), "%zu %d %d", src_num , head, foot);
	int num = foot - head + 1;
	pv_assertf((0 < num), "%zu %d %d", src_num , head, foot);

	PvBezier *dst_bezier = pv_bezier_new();

	*dst_bezier = *src_bezier;

	if(0 < num){
		dst_bezier->anchor_points = malloc(sizeof(PvAnchorPoint) * num);
		pv_assert(dst_bezier->anchor_points);

		pv_anchor_points_copy_(dst_bezier->anchor_points, &(src_bezier->anchor_points[head]), num);
		dst_bezier->anchor_points_num = num;
	}

	return dst_bezier;
}

void pv_bezier_add_anchor_point(PvBezier *self, PvAnchorPoint anchor_point)
{
	PvAnchorPoint *anchor_points = (PvAnchorPoint *)realloc(self->anchor_points,
			sizeof(PvAnchorPoint) * (self->anchor_points_num + 1));
	pv_assert(anchor_points);

	anchor_points[self->anchor_points_num] = anchor_point;
	self->anchor_points = anchor_points;
	(self->anchor_points_num) += 1;
}

PvAnchorPoint *pv_bezier_get_anchor_point_from_index(PvBezier *self, int index)
{
	return &(self->anchor_points[index]);
}

size_t pv_bezier_get_anchor_point_num(const PvBezier *self)
{
	return self->anchor_points_num;
}

void pv_bezier_set_is_close(PvBezier *self, bool is_close)
{
	self->is_close = is_close;
}

bool pv_bezier_get_is_close(const PvBezier *self)
{
	return self->is_close;
}

bool pv_bezier_is_diff(const PvBezier *bezier0, const PvBezier *bezier1)
{
	if(bezier0->anchor_points_num != bezier1->anchor_points_num){
		return true;
	}

	size_t num = pv_bezier_get_anchor_point_num(bezier0);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap0 = &(bezier0->anchor_points[i]);
		const PvAnchorPoint *ap1 = &(bezier1->anchor_points[i]);
		if(!(true
					&& ap0->points[0].x == ap1->points[0].x
					&& ap0->points[0].y == ap1->points[0].y
					&& ap0->points[1].x == ap1->points[1].x
					&& ap0->points[1].y == ap1->points[1].y
					&& ap0->points[2].x == ap1->points[2].x
					&& ap0->points[2].y == ap1->points[2].y))
		{
			return true;
		}
	}

	return false;
}

static void pv_anchor_points_change_head_index_(
	PvAnchorPoint *anchor_points,
	size_t anchor_points_num,
	int head)
{
	PvAnchorPoint *aps = malloc(sizeof(PvAnchorPoint) * anchor_points_num);
	pv_assert(aps);

	pv_debug( "%d %zu", head, anchor_points_num);
	pv_assertf((head < (int)anchor_points_num), "%d %zu", head, anchor_points_num);
	for(int i = 0; i < (int)anchor_points_num; i++){
		aps[i] = anchor_points[(head + i) % (int)anchor_points_num];
	}
	for(int i = 0; i < (int)anchor_points_num; i++){
		anchor_points[i] = aps[i];
	}

	free(aps);
}

static PvAnchorPoint *pv_anchor_points_new_duplicating_(
	PvAnchorPoint *anchor_points,
	size_t *anchor_points_num,
	int index)
{
	PvAnchorPoint *aps = malloc(sizeof(PvAnchorPoint) * ((*anchor_points_num) + 1));
	pv_assert(aps);
	memcpy(&aps[0], &anchor_points[0], sizeof(PvAnchorPoint) * index);
	aps[index] = anchor_points[index];
	memcpy(&aps[index + 1], &anchor_points[index], sizeof(PvAnchorPoint) * ((*anchor_points_num) - index));

	(*anchor_points_num) += 1;

	return aps;
}

bool pv_bezier_split_anchor_point_from_index(PvBezier *self, int index)
{
	size_t num = pv_bezier_get_anchor_point_num(self);
	if(index < 0 || (int)num <= index){
		pv_bug("");
		return false;
	}

	if(!self->is_close){
		pv_bug("");
		return false;
	}

	PvAnchorPoint *aps = pv_anchor_points_new_duplicating_(self->anchor_points, &self->anchor_points_num, index);
	pv_assert(aps);
	PvAnchorPoint *old_aps = self->anchor_points;
	self->anchor_points = aps;
	free(old_aps);

	pv_anchor_points_change_head_index_(self->anchor_points, self->anchor_points_num, index + 1);
	self->is_close = false;

	return true;
}

void pv_bezier_debug_print(const PvBezier *self)
{
	pv_debug("anchor:%zu(%s)", self->anchor_points_num, (self->is_close)? "true":"false");

	size_t num = pv_bezier_get_anchor_point_num(self);
	for(int i = 0; i < (int)num; i++){
		const PvAnchorPoint *ap = &self->anchor_points[i];
		pv_debug("%d:% 3.2f,% 3.2f, % 3.2f,% 3.2f, % 3.2f,% 3.2f, ",
				i,
				ap->points[PvAnchorPointIndex_HandlePrev].x,
				ap->points[PvAnchorPointIndex_HandlePrev].y,
				ap->points[PvAnchorPointIndex_Point].x,
				ap->points[PvAnchorPointIndex_Point].y,
				ap->points[PvAnchorPointIndex_HandleNext].x,
				ap->points[PvAnchorPointIndex_HandleNext].y);
	}
}

