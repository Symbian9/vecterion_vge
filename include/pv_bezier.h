#ifndef include_PV_BEZIER_H
#define include_PV_BEZIER_H

#include "pv_anchor_point.h"


typedef enum{
	PvBezierIndexTurn_Disable,
	PvBezierIndexTurn_OnlyLastInClosed,
}PvBezierIndexTurn;

struct PvBezier;
typedef struct PvBezier PvBezier;


PvBezier *pv_bezier_new();
void pv_bezier_free(PvBezier *);
PvBezier *pv_bezier_copy_new(const PvBezier *);
PvBezier *pv_bezier_copy_new_range(const PvBezier *, int head, int foot);
void pv_bezier_add_anchor_point(PvBezier *, PvAnchorPoint);
int pv_bezier_insert_anchor_point(PvBezier *, PvAnchorPoint, int index);
PvAnchorPoint *pv_bezier_get_anchor_point_from_index(PvBezier *, int index, PvBezierIndexTurn);
const PvAnchorPoint *pv_bezier_get_anchor_point_from_index_const(const PvBezier *, int index);
bool pv_bezier_set_anchor_point_from_index(PvBezier *, int index, PvAnchorPoint ap);
size_t pv_bezier_get_anchor_point_num(const PvBezier *);
void pv_bezier_set_is_close(PvBezier *, bool is_close);
bool pv_bezier_get_is_close(const PvBezier *);
bool pv_bezier_is_diff(const PvBezier *, const PvBezier *);
bool pv_bezier_split_anchor_point_from_index(PvBezier *, int index);
bool pv_bezier_get_anchor_point_p4_from_index(const PvBezier *, PvAnchorPointP4 *, int index);

void pv_bezier_debug_print(const PvBezier *);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_BEZIER_H

