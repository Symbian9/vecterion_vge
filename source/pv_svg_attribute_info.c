#include "pv_svg_attribute_info.h"

#include <strings.h>
#include "pv_error.h"
#include "pv_io_util.h"

static void pv_element_anchor_point_init(PvAnchorPoint *ap)
{
	*ap = PvAnchorPoint_Default;
}

static void _pv_svg_fill_double_array(double *dst, double value, int size)
{
	for(int i = 0; i < size; i++){
		dst[i] = value;
	}
}

static bool _pv_svg_read_args_from_str(double *args, int num_args, const char **str)
{
	const char *p = *str;
	bool res = true;

	int i = 0;
	while('\0' != *p){
		if(',' == *p){
			p++;
			continue;
		}
		char *next = NULL;
		const char *str_error = NULL;
		if(!pv_general_strtod(&args[i], p, &next, &str_error)){
			res = false;
			break;
		}
		p = next;
		i++;
		if(!(i < num_args)){
			break;
		}
	}
	for(; i < num_args; i++){
		args[i] = 0;
	}

	*str = p;

	return res;
}



static bool func_fill_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	bool res = true;

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "fill");
	if(!value){
		pv_error("");
		return false;
	}

	PvColor *color = &(element->color_pair.colors[PvColorPairGround_BackGround]);
	if(! pv_io_util_get_pv_color_from_svg_str_rgba(color, (const char *)value)){
		pv_error("");
		res = false;
		goto failed;
	}

failed:
	xmlFree(value);

	return res;
}

static bool func_stroke_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	bool res = true;

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "stroke");
	if(!value){
		pv_error("");
		return false;
	}

	PvColor *color = &(element->color_pair.colors[PvColorPairGround_ForGround]);
	if(! pv_io_util_get_pv_color_from_svg_str_rgba(color, (const char *)value)){
		pv_error("");
		res = false;
		goto failed;
	}

failed:
	xmlFree(value);

	return res;
}

static bool func_stroke_width_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "stroke-width");
	if(!value){
		pv_error("");
		return false;
	}

	element->stroke.width = pv_io_util_get_double_from_str((const char *)value);

	xmlFree(value);

	return true;
}

static bool func_stroke_linecap_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "stroke-linecap");
	if(!value){
		pv_error("");
		return false;
	}

	int num = get_num_stroke_linecap_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinecapInfo *info = get_stroke_linecap_info_from_id(i);
		if(0 == strcasecmp((char *)value, info->name)){
			element->stroke.linecap = info->linecap;
			return true;
		}
	}

	xmlFree(value);

	return true;
}

static bool func_stroke_linejoin_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "stroke-linejoin");
	if(!value){
		pv_error("");
		return false;
	}

	int num = get_num_stroke_linejoin_infos();
	for(int i = 0; i < num; i++){
		const PvStrokeLinejoinInfo *info = get_stroke_linejoin_info_from_id(i);
		if(0 == strcasecmp((char *)value, info->name)){
			element->stroke.linejoin = info->linejoin;
			return true;
		}
	}

	xmlFree(value);
	return true;
}

static bool func_d_set_inline_(
		PvElement *element,
		const char *value
		)
{
	const int num_args = 10;
	double args[num_args];
	double prev_args[num_args];
	_pv_svg_fill_double_array(args, 0, num_args);
	_pv_svg_fill_double_array(prev_args, 0, num_args);

	PvAnchorPoint ap;
	pv_assert(element);
	pv_assert(element->data);
	pv_assert(PvElementKind_Curve == element->kind);

	PvElementCurveData *data = element->data;

	const char *p = value;
	while('\0' != *p){
		bool is_append = false;
		switch(*p){
			case 'M':
			case 'L':
				p++;
				if(!_pv_svg_read_args_from_str(args, 2, &p)){
					goto failed;
				}
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_Point].x = args[0];
				ap.points[PvAnchorPointIndex_Point].y = args[1];
				is_append = true;
				break;
			case 'C':
				{
					p++;
					if(!_pv_svg_read_args_from_str(args, 6, &p)){
						goto failed;
					}
					pv_element_anchor_point_init(&ap);
					ap.points[PvAnchorPointIndex_HandlePrev].x = args[2] - args[4];
					ap.points[PvAnchorPointIndex_HandlePrev].y = args[3] - args[5];
					ap.points[PvAnchorPointIndex_Point].x = args[4];
					ap.points[PvAnchorPointIndex_Point].y = args[5];
					ap.points[PvAnchorPointIndex_HandleNext].x = 0;
					ap.points[PvAnchorPointIndex_HandleNext].y = 0;
					size_t num = pv_anchor_path_get_anchor_point_num(data->anchor_path);
					if(0 == num){
						pv_warning("'C' command on top?");
						goto failed;
					}
					PvAnchorPoint *ap_prev = pv_anchor_path_get_anchor_point_from_index(
							data->anchor_path,
							(num - 1),
							PvAnchorPathIndexTurn_Disable);
					PvPoint gpoint_next = {args[0], args[1]};
					pv_anchor_point_set_handle(ap_prev,
							PvAnchorPointIndex_HandleNext, gpoint_next);
					is_append = true;
				}
				break;
			case 'S':
				p++;
				if(!_pv_svg_read_args_from_str(args, 4, &p)){
					goto failed;
				}
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_HandlePrev].x = args[0] - args[2];
				ap.points[PvAnchorPointIndex_HandlePrev].y = args[1] - args[3];
				ap.points[PvAnchorPointIndex_Point].x = args[2];
				ap.points[PvAnchorPointIndex_Point].y = args[3];
				ap.points[PvAnchorPointIndex_HandleNext].x = 0;
				ap.points[PvAnchorPointIndex_HandleNext].y = 0;
				is_append = true;
				break;
			case 'Z':
			case 'z':
				p++;
				pv_anchor_path_set_is_close(data->anchor_path, true);
				break;
			case ' ':
			case ',':
				p++;
				break;
			default:
				p++;
		}

		if(is_append){
			pv_assert(pv_element_curve_add_anchor_point(element, ap));
		}
	}

	return true;
failed:
	pv_debug("'%s' %td", value, (p - value));

	return false;
}

static bool func_d_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 != strcasecmp("path", (char *)xmlnode->name)){
		pv_error("'%s'", attribute->name);
		return false;
	}

	if(PvElementKind_Curve != element->kind){
		pv_error("");
		return false;
	}

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "d");
	if(!value){
		pv_error("");
		return false;
	}

	bool ret = func_d_set_inline_(element, (const char *)value);

	xmlFree(value);

	return ret;
}

static bool func_points_set_inline_(
		PvElement *element,
		const char *value
		)
{
	const int num_args = 10;
	double args[num_args];
	double prev_args[num_args];
	_pv_svg_fill_double_array(args, 0, num_args);
	_pv_svg_fill_double_array(prev_args, 0, num_args);

	PvAnchorPoint ap;
	pv_assert(element);
	pv_assert(element->data);
	pv_assert(PvElementKind_Curve == element->kind);

	// PvElementCurveData *data = element->data;

	const char *p = value;
	while('\0' != *p){
		bool is_append = _pv_svg_read_args_from_str(args, 2, &p);
		pv_element_anchor_point_init(&ap);
		ap.points[PvAnchorPointIndex_Point].x = args[0];
		ap.points[PvAnchorPointIndex_Point].y = args[1];

		if(is_append){
			if(!pv_element_curve_add_anchor_point(element, ap)){
				pv_error("");
				return false;
			}
		}

		p++;
	}

	return true;
}

static bool func_points_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	if(0 == strcasecmp("polygon", (char *)xmlnode->name)){
		// NOP
	}else if(0 == strcasecmp("polyline", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	if(PvElementKind_Curve != element->kind){
		pv_error("");
		return false;
	}

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "points");
	if(!value){
		pv_error("");
		return false;
	}

	bool ret = func_points_set_inline_(element, (const char *)value);

	xmlFree(value);

	return ret;
}

static bool func_x1_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	pv_assert(PvElementKind_Curve == element->kind);

	if(0 == strcasecmp("line", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "x1");
	if(!value){
		pv_error("");
		return false;
	}

	int num = pv_element_curve_get_num_anchor_point(element);
	if(2 != num){
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	PvAnchorPath *anchor_path = pv_element_curve_get_anchor_path(element);
	pv_assert(anchor_path);
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			0,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);
	PvPoint point = pv_anchor_point_get_point(ap);
	point.x = v;
	pv_anchor_point_set_point(ap, point);

	xmlFree(value);

	return true;
}

static bool func_y1_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	pv_assert(PvElementKind_Curve == element->kind);

	if(0 == strcasecmp("line", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "x1");
	if(!value){
		pv_error("");
		return false;
	}

	int num = pv_element_curve_get_num_anchor_point(element);
	if(2 != num){
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	PvAnchorPath *anchor_path = pv_element_curve_get_anchor_path(element);
	pv_assert(anchor_path);
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			0,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);
	PvPoint point = pv_anchor_point_get_point(ap);
	point.y = v;
	pv_anchor_point_set_point(ap, point);

	xmlFree(value);

	return true;
}
static bool func_x2_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	pv_assert(PvElementKind_Curve == element->kind);

	if(0 == strcasecmp("line", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "x1");
	if(!value){
		pv_error("");
		return false;
	}

	int num = pv_element_curve_get_num_anchor_point(element);
	if(2 != num){
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	PvAnchorPath *anchor_path = pv_element_curve_get_anchor_path(element);
	pv_assert(anchor_path);
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			1,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);
	PvPoint point = pv_anchor_point_get_point(ap);
	point.x = v;
	pv_anchor_point_set_point(ap, point);

	xmlFree(value);

	return true;
}
static bool func_y2_set_(
		PvElement *element,
		const xmlNodePtr xmlnode,
		const xmlAttr *attribute
		)
{
	pv_assert(PvElementKind_Curve == element->kind);

	if(0 == strcasecmp("line", (char *)xmlnode->name)){
		// NOP
	}else{
		pv_error("'%s'", attribute->name);
		return false;
	}

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "x1");
	if(!value){
		pv_error("");
		return false;
	}

	int num = pv_element_curve_get_num_anchor_point(element);
	if(2 != num){
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
		pv_element_curve_add_anchor_point(element, PvAnchorPoint_Default);
	}

	double v = pv_io_util_get_double_from_str((const char *)value);

	PvAnchorPath *anchor_path = pv_element_curve_get_anchor_path(element);
	pv_assert(anchor_path);
	PvAnchorPoint *ap = pv_anchor_path_get_anchor_point_from_index(
			anchor_path,
			1,
			PvAnchorPathIndexTurn_Disable);
	pv_assert(ap);
	PvPoint point = pv_anchor_point_get_point(ap);
	point.y = v;
	pv_anchor_point_set_point(ap, point);

	xmlFree(value);

	return true;
}

const PvSvgAttributeInfo _pv_svg_attribute_infos[] = {
	{
		.name = "fill",
		.pv_svg_attribute_func_set = func_fill_set_,
	},
	{
		.name = "stroke",
		.pv_svg_attribute_func_set = func_stroke_set_,
	},
	{
		.name = "stroke-width",
		.pv_svg_attribute_func_set = func_stroke_width_set_,
	},
	{
		.name = "stroke-linecap",
		.pv_svg_attribute_func_set = func_stroke_linecap_set_,
	},
	{
		.name = "stroke-linejoin",
		.pv_svg_attribute_func_set = func_stroke_linejoin_set_,
	},
	{
		.name = "d",
		.pv_svg_attribute_func_set = func_d_set_,
	},
	{
		.name = "points",
		.pv_svg_attribute_func_set = func_points_set_,
	},
	{
		.name = "x1",
		.pv_svg_attribute_func_set = func_x1_set_,
	},
	{
		.name = "y1",
		.pv_svg_attribute_func_set = func_y1_set_,
	},
	{
		.name = "x2",
		.pv_svg_attribute_func_set = func_x2_set_,
	},
	{
		.name = "y2",
		.pv_svg_attribute_func_set = func_y2_set_,
	},
};

const PvSvgAttributeInfo *pv_svg_get_svg_attribute_info_from_name(const char *name)
{
	int num = sizeof(_pv_svg_attribute_infos) / sizeof(_pv_svg_attribute_infos[0]);
	for(int i = 0; i < num; i++){
		const PvSvgAttributeInfo *info = &_pv_svg_attribute_infos[i];
		if(0 == strcasecmp(name, info->name)){
			return info;
		}
	}

	return NULL;
}

