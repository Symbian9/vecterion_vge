#include "pv_type.h"

#include <math.h>
#include "pv_error.h"



// ******** ********
// PvPoint
// ******** ********
PvPoint pv_point_add(PvPoint p, PvPoint p_diff)
{
	PvPoint ret = {
		.x = p.x + p_diff.x,
		.y = p.y + p_diff.y,
	};

	return ret;
}

PvPoint pv_point_sub(PvPoint p0, PvPoint p1)
{
	PvPoint ret = {
		.x = p0.x - p1.x,
		.y = p0.y - p1.y,
	};

	return ret;
}

PvPoint pv_point_add_value(PvPoint p, double diff)
{
	PvPoint ret = {
		.x = p.x + diff,
		.y = p.y + diff,
	};

	return ret;
}

PvPoint pv_point_mul_value(PvPoint p0, double value)
{
	PvPoint ret = {
		.x = p0.x * value,
		.y = p0.y * value,
	};

	return ret;
}

PvPoint pv_point_div_value(PvPoint p, double scale)
{
	PvPoint ret = {
		.x = p.x / scale,
		.y = p.y / scale
	};

	return ret;
}


// ******** ********
// PvRect
// ******** ********
PvRect pv_rect_mul_value(PvRect r, double value)
{
	PvRect ret = {
		.x = r.x * value,
		.y = r.y * value,
		.w = r.w * value,
		.h = r.h * value,
	};

	return ret;
}

PvRect pv_rect_abs_size(PvRect r)
{
	if(r.w < 0){
		r.x += r.w;
		r.w = fabs(r.w);
	}
	if(r.h < 0){
		r.y += r.h;
		r.h = fabs(r.h);
	}

	return r;
}

bool pv_rect_is_inside(PvRect rect, int x, int y)
{
	if(x < rect.x || (rect.x + rect.w) < x){
		return false;
	}
	if(y < rect.y || (rect.y + rect.h) < y){
		return false;
	}

	return true;
}

//! @brief expanding PvRect area, logical operation "and"
PvRect pv_rect_expand(PvRect rect0, PvRect rect1)
{
	PvRect ret = rect0;
	ret.x = (rect0.x < rect1.x)? rect0.x : rect1.x;
	ret.y = (rect0.y < rect1.y)? rect0.y : rect1.y;
	double x0 = rect0.x + rect0.w;
	double y0 = rect0.y + rect0.h;
	double x1 = rect1.x + rect1.w;
	double y1 = rect1.y + rect1.h;
	double x = (x0 > x1)? x0 : x1;
	double y = (y0 > y1)? y0 : y1;
	ret.w = x - ret.x;
	ret.h = y - ret.y;

	return ret;
}

PvPoint pv_rect_get_edge_point(PvRect rect, PvRectEdgeKind edgeKind)
{
	switch(edgeKind){
		case PvRectEdgeKind_UpLeft:
			return (PvPoint){rect.x, rect.y};
		case PvRectEdgeKind_UpRight:
			return (PvPoint){(rect.x + rect.w), rect.y};
		case PvRectEdgeKind_DownLeft:
			return (PvPoint){rect.x, (rect.y + rect.h)};
		case PvRectEdgeKind_DownRight:
			return (PvPoint){(rect.x + rect.w), (rect.y + rect.h)};
		default:
			pv_assertf(false, "%d", edgeKind);
	}
}

