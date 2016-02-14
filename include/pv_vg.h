#ifndef __PV_VG_H__
#define __PV_VG_H__
/** ******************************
 * @brief PhotonVector Vector Graphics Format.
 *
 ****************************** */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element.h"

struct _PvVg;
typedef struct _PvVg PvVg;
struct _PvVg{
	PvRect rect;
	PvElement *element_root;
};

/** @brief pointer arrayの内容数を返す
 * (実長さは番兵のNULL終端があるため、return+1)
 */
int pv_general_get_parray_num(void **pointers);

PvVg *pv_vg_new();
/** @brief 
 * @return vg->element_root->childs[0];
 */
PvElement *pv_vg_get_layer_top(PvVg *vg);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_VG_H__