#ifndef include_ET_DOC_HISTORY_H
#define include_ET_DOC_HISTORY_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_vg.h"
#include "pv_focus.h"

typedef struct EtDocHistory{
	PvFocus		*focus;
	PvVg		*vg;
}EtDocHistory;


#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_DOC_HISTORY_H
