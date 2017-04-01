#include "pv_io.h"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "pv_error.h"
#include "pv_element_general.h"
#include "pv_element_info.h"
#include "pv_svg_element_info.h"
#include "pv_io_util.h"

typedef struct{
	InfoTargetSvg *target;
	const ConfWriteSvg *conf;
}PvIoSvgRecursiveData;



static bool _pv_io_svg_from_element_in_recursive_before(
		PvElement *element, gpointer data, int level)
{
	PvIoSvgRecursiveData *_data = data;
	InfoTargetSvg *target = _data->target;
	const ConfWriteSvg *conf = _data->conf;

	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	if(NULL == info){
		pv_error("");
		return false;
	}
	if(NULL == info->func_write_svg){
		pv_error("");
		return false;
	}

	// push parent node stack
	xmlNodePtr *new_nodes = realloc(target->xml_parent_nodes,
			(sizeof(xmlNodePtr) * (level + 2)));
	if(NULL == new_nodes){
		pv_critical("");
		exit(-1);
	}
	new_nodes[level+1] = NULL;
	new_nodes[level] = target->xml_parent_node;
	target->xml_parent_nodes = new_nodes;

	int ret = info->func_write_svg(target, element, conf);
	if(0 > ret){
		pv_error("%d", ret);
		return false;
	}

	return true;
}

static bool _pv_io_svg_from_element_in_recursive_after(
		PvElement *element, gpointer data, int level)
{
	PvIoSvgRecursiveData *_data = data;
	InfoTargetSvg *target = _data->target;
	// const ConfWriteSvg *conf = _data->conf;

	// pop parent node stack
	int num = pv_general_get_parray_num((void **)target->xml_parent_nodes);
	if(!(level < num)){
		pv_bug("");
		return false;
	}
	target->xml_parent_node = target->xml_parent_nodes[level];

	return true;
}

static bool _pv_io_svg_from_pvvg_element_recursive(
		xmlNodePtr xml_svg, PvElement *element_root,
		const ConfWriteSvg *conf)
{
	if(NULL == xml_svg){
		pv_bug("");
		return false;
	}

	if(NULL == element_root){
		pv_warning("");
		return true;
	}

	if(NULL == conf){
		pv_bug("");
		return false;
	}


	InfoTargetSvg target = {
		.xml_parent_nodes = NULL,
		.xml_parent_node = xml_svg,
		.xml_new_node = NULL,
	};
	PvIoSvgRecursiveData data = {
		.target = &target,
		.conf = conf,
	};
	PvElementRecursiveError error;
	if(!pv_element_recursive_asc(element_root,
				_pv_io_svg_from_element_in_recursive_before,
				_pv_io_svg_from_element_in_recursive_after,
				&data, &error)){
		pv_error("level:%d", error.level);
		return false;
	}
	free(target.xml_parent_nodes);

	return true;
}

bool pv_io_write_file_svg_from_vg(PvVg *vg, const char *path)
{
	if(NULL == vg){
		pv_bug("");
		return false;
	}

	if(NULL == path){
		pv_bug("");
		return false;
	}

	char *p = NULL;

	xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "svg");
	if(NULL == doc || NULL == root_node){
		pv_error("");
		return false;
	}
	xmlNewProp(root_node, BAD_CAST "xmlns",
			BAD_CAST "http://www.w3.org/2000/svg");
	xmlNewProp(root_node, BAD_CAST "xmlns:svg",
			BAD_CAST "http://www.w3.org/2000/svg");
	xmlNewProp(root_node, BAD_CAST "xmlns:inkscape",
			BAD_CAST "http://www.inkscape.org/namespaces/inkscape");
	xmlNewProp(root_node, BAD_CAST "xmlns:xlink",
			BAD_CAST "http://www.w3.org/1999/xlink");
	xmlDocSetRootElement(doc, root_node);

	// ** width, height
	pv_debug("x:%f y:%f w:%f h:%f",
			(vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);

	xmlNewProp(root_node, BAD_CAST "version", BAD_CAST "1.1");

	p = g_strdup_printf("%f", (vg->rect).x);
	xmlNewProp(root_node, BAD_CAST "x", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f", (vg->rect).y);
	xmlNewProp(root_node, BAD_CAST "y", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f", (vg->rect).w);
	xmlNewProp(root_node, BAD_CAST "width", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f", (vg->rect).h);
	xmlNewProp(root_node, BAD_CAST "height", BAD_CAST p);
	g_free(p);

	p = g_strdup_printf("%f %f %f %f",
			(vg->rect).x, (vg->rect).y, (vg->rect).w, (vg->rect).h);
	xmlNewProp(root_node, BAD_CAST "viewBox", BAD_CAST p);
	g_free(p);

	//		vg->element_root;
	ConfWriteSvg conf;
	if(!_pv_io_svg_from_pvvg_element_recursive(root_node, vg->element_root, &conf)){
		pv_error("");
		return false;
	}

	xmlSaveFormatFileEnc(path, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	return true;
}

/*
   static void print_element_names(xmlNode * a_node)
   {
   xmlNode *cur_node = NULL;

   for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
   if (cur_node->type == XML_ELEMENT_NODE) {
   pv_debug("node type: Element, name: %s(%d)", cur_node->name, cur_node->line);
   }

   print_element_names(cur_node->children);
   }
   }
 */

static bool _pv_io_get_svg_from_xml(xmlNode **xmlnode_svg, xmlNode *xmlnode)
{
	*xmlnode_svg = NULL;

	for (xmlNode *cur_node = xmlnode; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			if(0 == strcmp("svg", (char*)cur_node->name)){
				*xmlnode_svg = cur_node;
				return true;
			}
		}
		if(_pv_io_get_svg_from_xml(xmlnode_svg, cur_node->children)){
			return true;
		}
	}

	return false;
}

static bool _pv_io_get_px_from_str(double *value, const char *str, const char **str_error)
{
	char *endptr = NULL;
	if(!pv_general_strtod(value, str, &endptr, str_error)){
		return false;
	}

	// convert to px
	double dpi = 90.0;
	if(NULL == endptr){
		*str_error = "Internal error.";
		return false;
	}

	if(0 == strcmp("", endptr)){
		// return true;
	}else if(0 == strcmp("px", endptr)){
		// return true;
	}else if(0 == strcmp("pt", endptr)){
		*value *= 1.25 * (dpi / 90.0);
	}else if(0 == strcmp("pc", endptr)){
		*value *= 15 * (dpi / 90.0);
	}else if(0 == strcmp("mm", endptr)){
		*value *= 3.543307 * (dpi / 90.0);
	}else if(0 == strcmp("cm", endptr)){
		*value *= 35.43307 * (dpi / 90.0);
	}else if(0 == strcmp("in", endptr)){
		*value *= 90.0 * (dpi / 90.0);
	}else{
		*str_error = "Unit undefined.";
		pv_error("%s'%s'", *str_error, endptr);
		return false;
	}

	return true;
}

static bool _pv_io_set_vg_from_xmlnode_svg(PvVg *vg, xmlNode *xmlnode_svg)
{
	pv_debug("");
	PvRect rect = {0, 0, -1, -1};
	xmlAttr* attribute = xmlnode_svg->properties;
	while(attribute)
	{
		bool isOk = false;
		xmlChar* xmlValue = xmlNodeListGetString(xmlnode_svg->doc,
				attribute->children, 1);
		const char *strValue = (char*)xmlValue;
		const char *name = (char*)attribute->name;
		const char *str_error = "Not process.";
		if(0 == strcmp("x", name)){
			double value = 0.0;
			if(_pv_io_get_px_from_str(&value, strValue, &str_error)){
				rect.x = value;
				isOk = true;
			}
		}else if(0 == strcmp("y", name)){
			double value = 0.0;
			if(_pv_io_get_px_from_str(&value, strValue, &str_error)){
				rect.y = value;
				isOk = true;
			}
		}else if(0 == strcmp("width", name)){
			double value = 0.0;
			if(_pv_io_get_px_from_str(&value, strValue, &str_error)){
				rect.w = value;
				isOk = true;
			}
		}else if(0 == strcmp("height", name)){
			double value = 0.0;
			if(_pv_io_get_px_from_str(&value, strValue, &str_error)){
				rect.h = value;
				isOk = true;
			}
		}

		if(!isOk){
			pv_debug("Can not use:'%s':'%s' %s",
					name, strValue, str_error);
		}

		xmlFree(xmlValue); 
		attribute = attribute->next;
	}

	if(rect.w <= 0 && rect.h <= 0){
		rect.w = rect.h = 1;
	}else if(rect.w <= 0){
		rect.w = rect.h; // TODO: svg spec is "100%"
	}else if(rect.h <= 0){
		rect.h = rect.w; // TODO: svg spec is "100%"
	}

	vg->rect = rect;

	return true;
}

static PvStrMap *_new_css_str_maps_from_str(const char *style_str)
{
	const char *head = style_str;

	int num = 0;
	PvStrMap *map = NULL;
	map = realloc(map, sizeof(PvStrMap) * (num + 1));
	map[num - 0].key = NULL;
	map[num - 0].value = NULL;

	while(NULL != head && '\0' != *head){

		char *skey;
		char *svalue;
		if(2 != sscanf(head, " %m[^:;] : %m[^;]", &skey, &svalue)){
			break;
		}
		num++;

		map = realloc(map, sizeof(PvStrMap) * (num + 1));
		pv_assert(map);

		map[num - 0].key = NULL;
		map[num - 0].value = NULL;
		map[num - 1].key = skey;
		map[num - 1].value = svalue;

		head = strchr(head, ';');
		if(NULL == head){
			break;
		}
		if(';' == *head){
			head++;
		}
	}

	return map;
}


/*! @brief get the value string by key from the string of SVG style attribute css liked.
 * @return match: cstr from g_strdup_*() / not match:NULL
 */
/*
   static char *_strdup_cssvaluestr(const char *style_str, const char *key_str)
   {
   const char *head = style_str;
   while('\0' != *head){
   char *ret = NULL;

   char *skey;
   char *svalue;
   if(2 == sscanf(head, " %m[^:;] : %m[^;]", &skey, &svalue)){
   if(0 == strncmp(skey, key_str, strlen(key_str))){
   ret = g_strdup(svalue);
   }

   free(skey);
   free(svalue);
   }

   if(NULL == ret){
   head = strchr(head, ';');
   if(NULL == head){
   return NULL;
   }
   if(';' == *head){
   head++;
   }
   }else{
   return ret;
   }
   }

   return NULL;
   }
 */

// PvColorPair _pv_io_get_pv_color_pair_from_xmlnode_simple(const xmlNode *xmlnode)
bool _overwrite_conf_read_svg_from_xmlnode(PvSvgReadConf *conf, xmlNode *xmlnode)
{
	PvColorPair color_pair = conf->color_pair;
	double stroke_width = conf->stroke_width;

	xmlChar *xc_fill = xmlGetProp(xmlnode, BAD_CAST "fill");
	if(NULL != xc_fill){
		PvColor color;
		if(!pv_io_util_get_pv_color_from_svg_str_rgba(&color, (char *)xc_fill)){
			pv_warning("'%s'(%d)", (char *)xc_fill, xmlnode->line);
		}else{
			color_pair.colors[PvColorPairGround_BackGround] = color;
		}
	}
	xmlFree(xc_fill);

	xmlChar *xc_stroke = xmlGetProp(xmlnode, BAD_CAST "stroke");
	if(NULL != xc_stroke){
		PvColor color;
		if(!pv_io_util_get_pv_color_from_svg_str_rgba(&color, (char *)xc_stroke)){
			pv_warning("'%s'(%d)", (char *)xc_stroke, xmlnode->line);
		}else{
			color_pair.colors[PvColorPairGround_ForGround] = color;
		}
	}
	xmlFree(xc_stroke);

	xmlChar *xc_stroke_width = xmlGetProp(xmlnode, BAD_CAST "stroke-width");
	if(NULL != xc_stroke_width){
		stroke_width = pv_io_util_get_double_from_str((char *)xc_stroke_width);
	}
	xmlFree(xc_stroke_width);

	xmlChar *xc_style = xmlGetProp(xmlnode, BAD_CAST "style");
	PvStrMap *css_str_maps = _new_css_str_maps_from_str((char *)xc_style);
	for(int i = 0; NULL != css_str_maps[i].key; i++){
		char *str = g_strdup(css_str_maps[i].value);
		if(0 == strcmp("fill", css_str_maps[i].key)){
			PvColor color;
			if(!pv_io_util_get_pv_color_from_svg_str_rgba(&color, str)){
				pv_warning("'%s'(%d)", (char *)xc_style, xmlnode->line);
			}else{
				color_pair.colors[PvColorPairGround_BackGround] = color;
			}
		}else if(0 == strcmp("stroke", css_str_maps[i].key)){
			PvColor color;
			if(!pv_io_util_get_pv_color_from_svg_str_rgba(&color, str)){
				pv_warning("'%s'(%d)", (char *)xc_style, xmlnode->line);
			}else{
				color_pair.colors[PvColorPairGround_ForGround] = color;
			}
		}else if(0 == strcmp("stroke-width", css_str_maps[i].key)){
			stroke_width = pv_io_util_get_double_from_str(str);
		}else{
			pv_warning("unknown css style key: '%s'(%d)'%s':'%s'(%d)",
					(char *)xc_style, xmlnode->line,
					css_str_maps[i].key, css_str_maps[i].value, i);
			if(conf->imageFileReadOption->is_strict){
				pv_error("strict");
				return false;
			}
		}
		g_free(str);
	}
	pv_str_maps_free(css_str_maps);
	xmlFree(xc_style);

	PvSvgReadConf dst_conf = *conf;
	dst_conf.color_pair = color_pair;
	dst_conf.stroke_width = stroke_width;

	*conf = dst_conf;

	return true;
}

static bool _new_elements_from_svg_elements_recursive_inline(PvElement *element_parent,
		xmlNode *xmlnode,
		gpointer data,
		PvSvgReadConf *conf)
{
	const PvSvgElementInfo *svg_element_info = pv_svg_get_svg_element_info_from_tagname((char *)xmlnode->name);
	if(NULL == svg_element_info){
		pv_warning("Not implement:'%s'(%d)",
				xmlnode->name, xmlnode->line);
		if(0 == strcmp("comment", (char *)xmlnode->name)){
			return true;
		}
		if(conf->imageFileReadOption->is_strict){
			goto failed0;
		}
		goto skiped;
	}
	pv_assertf(svg_element_info->func_new_element_from_svg, "'%s'", (char *)xmlnode->name);

	PvSvgReadConf conf_save = *conf;

	PvSvgAttributeCache attribute_cache_;
	PvSvgAttributeCache *attribute_cache = &attribute_cache_;
	pv_svg_attribute_cache_init(attribute_cache);

	bool isDoChild = true;
	PvElement *element_current = svg_element_info->func_new_element_from_svg(
			element_parent,
			attribute_cache,
			xmlnode,
			&isDoChild,
			data,
			conf);
	if(NULL == element_current){
		pv_error("");
		goto failed0;
	}

	{
		xmlAttr* attribute = xmlnode->properties;
		while(attribute){
			const PvSvgAttributeInfo *info = pv_svg_get_svg_attribute_info_from_name((const char *)attribute->name);

			if(info){
				bool ret = info->pv_svg_attribute_func_set(
						element_current, attribute_cache, conf, xmlnode, attribute);
				if(!ret){
					pv_warning("'%s'(%d) on '%s'",
							attribute->name, xmlnode->line, xmlnode->name);
				}
			}else{
				pv_warning("Not implement:'%s'(%d) on '%s'",
						attribute->name, xmlnode->line, xmlnode->name);
				if(conf->imageFileReadOption->is_strict){
					pv_error("strict");
					goto failed1;
				}
			}

			attribute = attribute->next;
		}
	}

	if(PvElementKind_BasicShape == element_current->kind){
		PvElementBasicShapeData *element_data = element_current->data;
		if(PvBasicShapeKind_Raster == element_data->kind){
			if(!attribute_cache->attributes[PvSvgAttributeKind_xlink_href].is_exist){
				if(conf->imageFileReadOption->is_strict){
					pv_error("strict");
					goto failed1;
				}
				pv_warning("remove empty image element by vecterion.");
				pv_element_remove_free_recursive(element_current);
				goto skiped;
			}
		}
	}

	bool ret = svg_element_info->func_set_attribute_cache(element_current, attribute_cache);
	if(!ret){
		pv_warning("%d %s(%d)", ret, (char *)xmlnode->name, xmlnode->line);
		if(conf->imageFileReadOption->is_strict){
			pv_error("strict");
			goto failed1;
		}
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element_current->kind);
	pv_assert(info);
	PvAppearance *a[3] = {NULL, NULL, NULL,};
	a[0] = &(conf->appearances[PvAppearanceKind_Translate]);
	a[1] = &(conf->appearances[PvAppearanceKind_Resize]);
	info->func_apply_appearances(element_current, a);

	// element_current->color_pair = _pv_io_get_pv_color_pair_from_xmlnode_simple(xmlnode);
	if(0 == strcmp("g", svg_element_info->tagname)){
		conf->color_pair = PvColorPair_TransparentBlack;
		conf->stroke_width = 1.0;
	}
	if(! _overwrite_conf_read_svg_from_xmlnode(conf, xmlnode)){
		pv_error("");
		goto failed1;
	}
	element_current->color_pair = conf->color_pair;
	element_current->stroke.width = conf->stroke_width;

	if(isDoChild){
		for (xmlNode *cur_node = xmlnode->children; cur_node; cur_node = cur_node->next) {
			if(!_new_elements_from_svg_elements_recursive_inline(element_current,
						cur_node,
						data,
						conf))
			{
				pv_error("");
				goto failed1;
			}
		}
	}

skiped:
	*conf = conf_save;

	return true;
failed1:
	pv_element_remove_free_recursive(element_current);
failed0:

	pv_debug("fail:'%s'(%d)",
				xmlnode->name, xmlnode->line);
	return false;
}

static bool _new_elements_from_svg_elements_recursive(
		PvElement *parent_element,
		xmlNodePtr xml_svg, 
		PvSvgReadConf *conf)
{
	pv_assert(parent_element);
	pv_assert(xml_svg);
	pv_assert(conf);

	if(!_new_elements_from_svg_elements_recursive_inline(
				parent_element,
				xml_svg,
				NULL,
				conf))
	{
		pv_error("");
		return false;
	}

	return true;
}

static PvElement *pv_io_new_element_from_filepath_with_vg_(
		PvVg *vg,
		const char *filepath,
		const PvImageFileReadOption *imageFileReadOption)
{
	PvElement *top_layer = NULL;

	LIBXML_TEST_VERSION

		xmlDoc *xml_doc = xmlReadFile(filepath, NULL, 0);
	if(NULL == xml_doc){
		pv_error("");
		goto error;
	}
	xmlNode *xml_root_element = xmlDocGetRootElement(xml_doc);
	xmlNode *xmlnode_svg = NULL;
	if(!_pv_io_get_svg_from_xml(&xmlnode_svg, xml_root_element)){
		pv_error("");
		goto error;
	}

	if(NULL != vg){
		if(!_pv_io_set_vg_from_xmlnode_svg(vg, xmlnode_svg)){
			pv_error("");
			goto error;
		}
	}

	PvElement *layer = pv_element_new(PvElementKind_Layer);
	pv_assert(layer);
	PvSvgReadConf conf = PvSvgReadConf_Default;
	conf.imageFileReadOption = imageFileReadOption;
	if(!_new_elements_from_svg_elements_recursive(layer, xmlnode_svg, &conf)){
		pv_error("");
		goto error;
	}

	int num_svg_top = pv_general_get_parray_num((void **)layer->childs);
	if(1 == num_svg_top && PvElementKind_Layer == layer->childs[0]->kind){
		// cut self root layer if exist root layer in svg
		top_layer = layer->childs[0];
		top_layer->parent = NULL;
		pv_element_free(layer);
	}else{
		top_layer = layer;
	}

error:
	xmlFreeDoc(xml_doc);
	xmlCleanupParser();

	return top_layer;
}

PvVg *pv_io_new_from_file(const char *filepath, const PvImageFileReadOption *imageFileReadOption)
{
	if(NULL == filepath){
		pv_error("");
		return NULL;
	}

	PvVg *vg = pv_vg_new();
	if(NULL == vg){
		pv_error("");
		return NULL;
	}

	PvElement *layer = pv_io_new_element_from_filepath_with_vg_(vg, filepath, imageFileReadOption);
	if(NULL == layer){
		pv_warning("");
		goto error;
	}

	bool is_toplevel_layer_all = false;
	int num_svg_top = pv_general_get_parray_num((void **)layer->childs);
	for(int i = 0; i < num_svg_top; i++){
		is_toplevel_layer_all = true;
		if(PvElementKind_Layer != layer->childs[i]->kind){
			is_toplevel_layer_all = false;
			break;
		}
	}
	if(is_toplevel_layer_all){
		// castle root layer
		PvElement *_root = vg->element_root;
		layer->kind = PvElementKind_Root;
		vg->element_root = layer;
		pv_assert(pv_element_remove_free_recursive(_root));
	}else{
		// append root child
		pv_assert(pv_element_append_child(vg->element_root, NULL, layer));
		pv_assert(pv_element_remove_free_recursive(vg->element_root->childs[0]));
		assert(1 == pv_general_get_parray_num((void **)(vg->element_root->childs)));
	}

	return vg;

error:
	pv_vg_free(vg);

	return NULL;
}

PvElement *pv_io_new_element_from_filepath(const char *filepath)
{
	return pv_io_new_element_from_filepath_with_vg_(NULL, filepath, &PvImageFileReadOption_Default);
}

