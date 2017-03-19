/*! @file
 * license: https://github.com/MichinariNukazawa/vecterion_vge/blob/master/LICENSE.md
 * or please contact to author.
 * author: michinari.nukazawa@gmail.com / project daisy bell
 */
#ifndef include_PV_SVG_ATTRIBUTE_INFO_H
#define include_PV_SVG_ATTRIBUTE_INFO_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libxml/xmlwriter.h>
#include <stdbool.h>
#include "pv_element.h"

typedef bool (*PvSvgAttributeFuncSet)(
				PvElement *element,
				const xmlNodePtr xmlnode,
				const xmlAttr *attribute
				);

typedef struct{
	const char *name;
	PvSvgAttributeFuncSet pv_svg_attribute_func_set;
}PvSvgAttributeInfo;

extern const PvSvgAttributeInfo _pv_svg_attribute_infos[];

const PvSvgAttributeInfo *pv_svg_get_svg_attribute_info_from_name(const char *name);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_SVG_ATTRIBUTE_INFO_H

