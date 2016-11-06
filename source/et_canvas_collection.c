#include "et_canvas_collection.h"

#include <stdlib.h>
#include "et_error.h"
#include "et_etaion.h"
#include "et_doc.h"
#include "et_renderer.h"
#include "et_pointing_manager.h"

static EtCanvasCollection *_canvas_collection = NULL;

struct EtCanvasCollectionCollect;
typedef struct EtCanvasCollectionCollect EtCanvasCollectionCollect;
struct EtCanvasCollectionCollect{
	EtDocId doc_id;
	EtCanvas **canvases;
};

struct EtCanvasCollection{
	GtkWidget *widget;
	GtkWidget *widget_tab;
	EtThumbnail *thumbnail;

	int num_collects;
	EtCanvasCollectionCollect *collects;
};



static EtDocId _et_canvas_collection_get_doc_id_from_canvas_frame(GtkWidget *canvas_frame)
{
	EtCanvasCollection *self = _canvas_collection;
	if(NULL == self){
		et_bug("");
		return -1;
	}

	for(int i = 0; i < self->num_collects; i++){
		const EtCanvasCollectionCollect *collect = &(self->collects[i]);
		int num = pv_general_get_parray_num((void **)collect->canvases);
		for(int t = 0; t < num; t++){
			// TODO: const
			// const EtCanvas *canvas = collect->canvases[t];
			EtCanvas *canvas = collect->canvases[t];
			if(NULL == canvas){
				et_bug("");
				goto error;
			}
			const GtkWidget *frame = et_canvas_get_widget_frame(canvas);
			if(NULL == frame){
				et_bug("");
				goto error;
			}
			// ** matching
			if(canvas_frame == frame){
				return collect->doc_id;
			}
		}
	}

error:
	return -1;
}

static gboolean _cb_notebook_change_current_page(GtkNotebook *notebook,
		GtkWidget    *page,
		gint         page_num,
		gpointer     user_data)
{
	et_debug("%d", page_num);

	// ** get current doc_id
	EtDocId doc_id = _et_canvas_collection_get_doc_id_from_canvas_frame(page);
	if(doc_id < 0){
		et_error("");
		goto error;
	}

	// ** change Thumbnail doc_id
	EtCanvasCollection *self = _canvas_collection;
	if(NULL == self){
		et_bug("");
		return false;
	}
	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(self->thumbnail);
	if(!et_canvas_set_doc_id(canvas_thumbnail, doc_id)){
		et_bug("");
		return false;
	}

	if(!et_etaion_set_current_doc_id(doc_id)){
		et_bug("");
		goto error;
	}

	// ** update thumbnail (キャンバスが再描画されるのは記述時点で副作用)
	et_doc_signal_update_from_id(doc_id);

	et_debug("%d", doc_id);

	return false;
error:
	return true;
}

static void _cb_notebook_page_added(GtkNotebook *notebook,
		GtkWidget   *child,
		guint        page_num,
		gpointer     user_data)
{
	et_debug("%u", page_num);

	gtk_widget_show_all(child);
	gtk_notebook_set_current_page(notebook, (gint)page_num);
}

EtCanvasCollection *et_canvas_collection_init()
{
	if(NULL != _canvas_collection){
		et_bug("");
		return NULL;
	}

	EtCanvasCollection *self = (EtCanvasCollection *)malloc(sizeof(EtCanvasCollection));
	if(NULL == self){
		et_error("");
		return NULL;
	}

	self->widget_tab = gtk_notebook_new();
	if(NULL == self->widget_tab){
		et_error("");
		return NULL;
	}

	g_signal_connect(GTK_NOTEBOOK(self->widget_tab), "page-added",
			G_CALLBACK(_cb_notebook_page_added), NULL);
	g_signal_connect(GTK_NOTEBOOK(self->widget_tab), "switch-page",
			G_CALLBACK(_cb_notebook_change_current_page), NULL);

	self->thumbnail = et_thumbnail_new();
	if(NULL == self->thumbnail){
		et_error("");
		return NULL;
	}

	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(self->thumbnail);
	int id = et_canvas_set_slot_change(canvas_thumbnail,
			slot_et_renderer_from_canvas_change, NULL);
	if(id < 0){
		et_error("");
		return NULL;
	}

	self->widget = self->widget_tab;

	self->num_collects = 0;
	self->collects = NULL;

	_canvas_collection = self;

	return self;
}

GtkWidget *et_canvas_collection_get_widget_frame()
{
	EtCanvasCollection *self = _canvas_collection;
	if(NULL == self){
		et_bug("");
		return NULL;
	}

	return self->widget;
}

EtThumbnail *et_canvas_collection_get_thumbnail()
{
	EtCanvasCollection *self = _canvas_collection;
	if(NULL == self){
		et_bug("");
		return NULL;
	}

	return self->thumbnail;
}

static EtCanvasCollectionCollect *_get_collect_from_doc_id(EtDocId doc_id)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	for(int i = 0; i < self->num_collects; i++){
		EtCanvasCollectionCollect *collect = &(self->collects[i]);
		if(doc_id == collect->doc_id){
			return collect;
		}
	}

	return NULL;
}

static void _slot_et_canvas_collection_from_doc_change(EtDoc *doc, gpointer data)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	// ** document name to tab
	EtDocId doc_id = et_doc_get_id(doc);

	EtCanvasCollectionCollect *collect = _get_collect_from_doc_id(doc_id);
	et_assertf(collect, "%d", doc_id);

	char *src_str_title = et_doc_get_new_filename_from_id(doc_id);
	const char *str_title = ((src_str_title) ? src_str_title : "(untitled)");

	int num = pv_general_get_parray_num((void **)collect->canvases);
	for(int i = 0; i < num; i++){
		GtkWidget *canvas_frame = et_canvas_get_widget_frame(collect->canvases[i]);
		et_assertf(canvas_frame, "%d %d", doc_id, i);
		GtkWidget *label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(self->widget_tab), canvas_frame);
		et_assertf(label, "%d %d", doc_id, i);
		gtk_label_set_text(GTK_LABEL(label), str_title);
	}

	if(NULL != src_str_title){
		g_free(src_str_title);
	}
}

EtCanvas *et_canvas_collection_new_canvas(EtDocId doc_id)
{
	EtCanvasCollection *self = _canvas_collection;
	et_assert(self);

	// ** canvas new and setup.
	EtCanvas *canvas = et_canvas_new_from_doc_id(doc_id);
	et_assertf(canvas, "%d", doc_id);

	int id1 = et_doc_add_slot_change(doc_id, slot_et_canvas_from_doc_change, canvas);
	et_assertf(0 <= id1, "%d %d", doc_id, id1);

	int id = et_canvas_set_slot_change(canvas, slot_et_renderer_from_canvas_change, NULL);
	et_assertf(0 <= id, "%d %d", doc_id, id);

	GtkWidget *canvas_frame1 = et_canvas_get_widget_frame(canvas);
	et_assertf(canvas_frame1, "%d", doc_id);

	if(0 > et_canvas_set_slot_mouse_action(canvas, et_pointing_manager_slot_mouse_action, NULL)){
		et_error("");
		return NULL;
	}

	EtCanvas *canvas_thumbnail = et_thumbnail_get_canvas(self->thumbnail);
	if(!et_canvas_set_doc_id(canvas_thumbnail, doc_id)){
		et_bug("");
		return false;
	}
	int id2 = et_doc_add_slot_change(doc_id, slot_et_canvas_from_doc_change, canvas_thumbnail);
	et_assertf(0 <= id2, "%d %d", doc_id, id2);

	if(!et_doc_signal_update_from_id(doc_id)){
		et_error("");
		return NULL;
	}

	// ** add collects.
	// TODO: search doc_id in collects alreary.
	EtCanvasCollectionCollect *new = realloc(self->collects,
			sizeof(EtCanvasCollectionCollect) * (self->num_collects + 1));
	if(NULL == new){
		et_error("");
		return NULL;
	}

	unsigned int num_canvases = 0;
	EtCanvas **canvases = realloc(NULL, sizeof(EtCanvas *) * (num_canvases + 2));
	if(NULL == new){
		et_error("");
		return NULL;
	}

	canvases[num_canvases + 1] = NULL;
	canvases[num_canvases] = canvas;

	new[self->num_collects].canvases = canvases;
	new[self->num_collects].doc_id = doc_id;
	(self->num_collects)++;

	self->collects = new;

	char *title = et_doc_get_new_filename_from_id(doc_id);
	GtkWidget *tab_label = gtk_label_new(((title) ? title : "(untitled)"));
	if(NULL != title){
		g_free(title);
	}
	int ix = gtk_notebook_append_page(GTK_NOTEBOOK(self->widget_tab), canvas_frame1, tab_label);
	if(ix < 0){
		et_error("");
		return NULL;
	}

	int id0 = et_doc_add_slot_change(doc_id, _slot_et_canvas_collection_from_doc_change, NULL);
	et_assertf(0 <= id0, "%d %d", doc_id, id0)

	return canvas;
}

