#ifndef include_PV_RENDER_CONTEXT_H
#define include_PV_RENDER_CONTEXT_H

struct PvRenderContext;
typedef struct PvRenderContext PvRenderContext;
struct PvRenderContext{
	bool is_focus;
	bool is_extent_view;
	double scale;
	int margin;
};
static const PvRenderContext PvRenderContext_Default = {
	.is_focus	= false,
	.is_extent_view	= false,
	.scale		= 1.0,
	.margin		= 0,
};

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_RENDER_CONTEXT_H
