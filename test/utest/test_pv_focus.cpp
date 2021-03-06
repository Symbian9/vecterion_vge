#include <gtest/gtest.h>

extern "C"
{
#include "pv_vg.h"
#include "pv_focus.h"
}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

TEST(Test, FocusElement){
	PvVg *vg = pv_vg_new();
	PvFocus *focus = pv_focus_new(vg);
	EXPECT_TRUE(NULL != focus);

	// read focus default
	int num;
	PvElement *focus_element = pv_focus_get_first_element(focus);
	num = pv_general_get_parray_num((void **)focus->elements);
	if(num != 1){
		FAIL();
	}
	EXPECT_TRUE(focus_element == focus->elements[0]);
	num = pv_general_get_parray_num((void **)vg->element_root->childs);
	if(num != 1){
		FAIL();
	}
	EXPECT_TRUE(focus_element == vg->element_root->childs[0]);

	// write
	PvElement *layer0_0 = pv_element_new(PvElementKind_Layer);
	if(!pv_element_append_child(vg->element_root, NULL, layer0_0)){
		FAIL();
	}
	PvElement *layer0_1 = pv_element_new(PvElementKind_Layer);
	if(!pv_element_append_child(layer0_0, NULL, layer0_1)){
		FAIL();
	}
	PvElement *elem0 = pv_element_new(PvElementKind_Curve);
	if(!pv_element_append_child(layer0_1, NULL, elem0)){
		FAIL();
	}
	PvElement *layer1_0 = pv_element_new(PvElementKind_Layer);
	if(!pv_element_append_child(vg->element_root, NULL, layer1_0)){
		FAIL();
	}
	PvElement *layer1_1 = pv_element_new(PvElementKind_Layer);
	if(!pv_element_append_child(layer1_0, NULL, layer1_1)){
		FAIL();
	}
	PvElement *elem1 = pv_element_new(PvElementKind_Curve);
	if(!pv_element_append_child(layer1_1, NULL, elem1)){
		FAIL();
	}

	fprintf(stdout, "elem:%p, layer0:%p, layer1:%p"
			" focus:%p, focus_layer:%p\n",
			elem0, layer0_0, layer0_1,
			pv_focus_get_first_element(focus),
			pv_focus_get_first_layer(focus));
	// add focus element of 1.
	EXPECT_TRUE(pv_focus_add_element(focus, elem0));
	EXPECT_TRUE(elem0 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer0_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	// add focue element of 2
	EXPECT_TRUE(pv_focus_add_element(focus, elem1));
	EXPECT_EQ(elem1, pv_focus_get_first_element(focus));
	EXPECT_EQ(layer1_1, pv_focus_get_first_layer(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements));

	// internal implement.
	EXPECT_NE(focus->elements[0], focus->elements[1]);
	EXPECT_EQ(elem1, focus->elements[0]);
	EXPECT_EQ(elem0, focus->elements[1]);

	// add already focus element to first focus
	EXPECT_TRUE(pv_focus_add_element(focus, elem0));
	EXPECT_EQ(elem0, pv_focus_get_first_element(focus));
	EXPECT_EQ(layer0_1, pv_focus_get_first_layer(focus));
	EXPECT_EQ(2, pv_general_get_parray_num((void **)focus->elements));

	// remove and change element of 1
	EXPECT_TRUE(pv_focus_remove_element(focus, elem0));
	EXPECT_TRUE(elem1 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer1_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	// remove last element move to parent layer
	EXPECT_TRUE(pv_focus_remove_element(focus, elem1));
	EXPECT_TRUE(layer1_1 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer1_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	// already focus to parrent is not move.
	EXPECT_TRUE(pv_focus_clear_to_first_layer(focus));
	EXPECT_TRUE(layer1_1 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer1_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	// set single focus.
	EXPECT_TRUE(pv_focus_clear_set_element(focus, elem1));
	EXPECT_TRUE(elem1 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer1_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));
	EXPECT_TRUE(pv_focus_clear_set_element(focus, elem0));
	EXPECT_TRUE(elem0 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer0_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	// focus clear move to parrent.
	EXPECT_TRUE(pv_focus_clear_to_first_layer(focus));
	EXPECT_TRUE(layer0_1 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer0_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	// set single layer.
	EXPECT_TRUE(pv_focus_clear_set_element(focus, layer1_1));
	EXPECT_TRUE(layer1_1 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer1_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	// free
	pv_focus_free(focus);
}

TEST(Test, Invalid){
	PvVg *vg = pv_vg_new();
	{
		PvFocus *focus = pv_focus_new(NULL);
		EXPECT_TRUE(NULL == focus);
	}
	PvFocus *focus = pv_focus_new(vg);
	if(NULL == focus){
		FAIL();
	}
	// 
	int num;
	PvElement *focus_element = pv_focus_get_first_element(focus);
	num = pv_general_get_parray_num((void **)focus->elements);
	if(num != 1){
		FAIL();
	}
	EXPECT_TRUE(focus_element == focus->elements[0]);
	num = pv_general_get_parray_num((void **)vg->element_root->childs);
	if(num != 1){
		FAIL();
	}
	EXPECT_TRUE(focus_element == vg->element_root->childs[0]);
	//
	PvElement *layer0_0 = pv_element_new(PvElementKind_Layer);
	if(!pv_element_append_child(vg->element_root, NULL, layer0_0)){
		FAIL();
	}
	PvElement *layer0_1 = pv_element_new(PvElementKind_Layer);
	if(!pv_element_append_child(layer0_0, NULL, layer0_1)){
		FAIL();
	}
	PvElement *elem0 = pv_element_new(PvElementKind_Curve);
	if(!pv_element_append_child(layer0_1, NULL, elem0)){
		FAIL();
	}
	PvElement *layer1_0 = pv_element_new(PvElementKind_Layer);
	if(!pv_element_append_child(vg->element_root, NULL, layer1_0)){
		FAIL();
	}
	PvElement *layer1_1 = pv_element_new(PvElementKind_Layer);
	if(!pv_element_append_child(layer1_0, NULL, layer1_1)){
		FAIL();
	}
	PvElement *elem1 = pv_element_new(PvElementKind_Curve);
	if(!pv_element_append_child(layer1_1, NULL, elem1)){
		FAIL();
	}
	//
	if(!pv_focus_clear_set_element(focus, elem0)){
		FAIL();
	}
	if(! (elem0 == pv_focus_get_first_element(focus))){
		FAIL();
	}
	if(! (layer0_1 == pv_focus_get_first_layer(focus))){
		FAIL();
	}
	if(! (1 == pv_general_get_parray_num((void **)focus->elements))){
		FAIL();
	}

	EXPECT_FALSE(pv_focus_add_element(NULL, elem1));
	EXPECT_FALSE(pv_focus_add_element(focus, NULL));
	EXPECT_TRUE(elem0 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer0_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	EXPECT_TRUE(NULL == pv_focus_get_first_element(NULL));
	EXPECT_TRUE(NULL == pv_focus_get_first_layer(NULL));
	EXPECT_TRUE(elem0 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer0_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	EXPECT_FALSE(pv_focus_clear_set_element(NULL, elem1));
	EXPECT_FALSE(pv_focus_clear_set_element(focus, NULL));
	EXPECT_TRUE(elem0 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer0_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	EXPECT_FALSE(pv_focus_clear_to_first_layer(NULL));
	EXPECT_TRUE(elem0 == pv_focus_get_first_element(focus));
	EXPECT_TRUE(layer0_1 == pv_focus_get_first_layer(focus));
	EXPECT_EQ(1, pv_general_get_parray_num((void **)focus->elements));

	// free
	pv_focus_free(focus);
}

TEST(Test, FocusAnchorPoint){
	PvVg *vg = pv_vg_new();
	PvFocus *focus = pv_focus_new(vg);
	EXPECT_TRUE(NULL != focus);

	PvElement *curve_0 = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	if(!pv_element_append_child(vg->element_root->childs[0], NULL, curve_0)){
		FAIL();
	}

	PvElementCurveData *data = (PvElementCurveData *)curve_0->data;
	PvAnchorPoint *anchor_point = pv_anchor_path_get_anchor_point_from_index(
			curve_0->anchor_path,
			1,
			PvAnchorPathIndexTurn_Disable);

	EXPECT_TRUE(pv_focus_add_anchor_point(focus, curve_0, anchor_point));
	EXPECT_TRUE(pv_focus_is_exist_anchor_point(focus, NULL, anchor_point));

	pv_focus_add_element(focus, curve_0);
	EXPECT_TRUE(pv_focus_is_exist_anchor_point(focus, NULL, anchor_point));
}

