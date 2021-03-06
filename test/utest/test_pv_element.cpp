#include <gtest/gtest.h>

#define BASIC_SHAPE_FILEPATH_1 "./resource/vecterion/daisy_bell_header_r2.jpg"

extern "C"
{
#include "pv_vg.h"
#include "pv_element.h"
}

TEST(Test, Test){
	EXPECT_EQ(1,1);
}

TEST(Test, PvElement_BasicShape){
	PvVg *vg = pv_vg_new();
	assert(NULL != vg);

	PvElement *element_parent = pv_vg_get_layer_top(vg);

	PvElement *element_basic_shape0 = pv_element_new(PvElementKind_BasicShape);
	ASSERT_TRUE(NULL != element_basic_shape0);
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_basic_shape0));

	PvElement *element_basic_shape1 = pv_element_basic_shape_new_from_filepath(BASIC_SHAPE_FILEPATH_1);
	ASSERT_TRUE(NULL != element_basic_shape1);
	PvElementBasicShapeData *element_data = (PvElementBasicShapeData *)element_basic_shape1->data;
	PvRasterData *data = (PvRasterData *)element_data->data;
	ASSERT_TRUE(NULL != data);
	ASSERT_TRUE(NULL != data->path);
	ASSERT_STREQ(data->path, BASIC_SHAPE_FILEPATH_1);
	ASSERT_TRUE(NULL != data->pixbuf);
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_basic_shape1));

	PvElement *element_basic_shape2
		= pv_element_basic_shape_new_from_filepath("./test/testdata/invalid.jpg");
	ASSERT_TRUE(NULL == element_basic_shape2);

	pv_vg_free(vg);
}

TEST(Test, PvVg_OnAllElementKind){
	PvVg *vg = pv_vg_new();
	assert(NULL != vg);

	PvElement *element_parent = pv_vg_get_layer_top(vg);

	// ** vg on all kind (default empty) element
	for(int k = (PvElementKind_NotDefined + 1); k < PvElementKind_EndOfKind; k++){
		if(PvElementKind_Root == k){
			continue;
		}
		PvElement *element = pv_element_new((PvElementKind)k);
		ASSERT_TRUE(NULL != element);
		ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element));
	}

	// **curve
	PvElement *element_curve0 = pv_element_new(PvElementKind_Curve);
	PvAnchorPoint ap = {
		.points = {{0,0},{10,20},{0,0}},
	};
	assert(pv_element_curve_add_anchor_point(element_curve0, ap));
	assert(1 == pv_element_curve_get_num_anchor_point(element_curve0));
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_curve0));
	// ** basic_shape
	PvElement *element_basic_shape1 = pv_element_basic_shape_new_from_filepath(BASIC_SHAPE_FILEPATH_1);
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_basic_shape1));

	// ** copy
	PvVg *vg2 = pv_vg_copy_new(vg);
	PvVg *vg3 = pv_vg_copy_new(vg);

	// ** copy not difference
	bool ret;
	ret = pv_vg_is_diff(vg, vg2);
	ASSERT_FALSE(ret);
	ret = pv_vg_is_diff(vg, vg3);
	ASSERT_FALSE(ret);

	// ** independ vg (copyed vg2,3 is fine when after free vg)
	pv_vg_free(vg);
	pv_vg_is_diff(vg2, vg3);
	pv_vg_free(vg3);
	pv_vg_free(vg2);
}

TEST(Test, append){
	PvVg *vg = pv_vg_new();
	assert(NULL != vg);

	PvElement *element_parent = pv_vg_get_layer_top(vg);

	PvElement *element_curves[4];

	// ** append_nth
	element_curves[0] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[0]);
	ASSERT_TRUE(pv_element_append_nth(element_parent, 0, element_curves[0]));
	ASSERT_EQ(1, (int)pv_general_get_parray_num((void **)element_parent->childs));
	// ** append_nth invalid
	element_curves[1] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[1]);
	ASSERT_TRUE(false == pv_element_append_nth(element_parent, -1, element_curves[1]));
	ASSERT_TRUE(false == pv_element_append_nth(element_parent, 2, element_curves[1]));
	ASSERT_EQ(1, (int)pv_general_get_parray_num((void **)element_parent->childs));
	pv_element_free(element_curves[1]);

	// ** append_child
	element_curves[0] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[0]);
	ASSERT_TRUE(pv_element_append_child(element_parent, NULL, element_curves[0]));
	ASSERT_EQ(2, (int)pv_general_get_parray_num((void **)element_parent->childs));
	ASSERT_EQ(element_parent->childs[1], element_curves[0]);
	//
	element_curves[1] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[1]);
	ASSERT_TRUE(pv_element_append_child(element_parent, element_curves[0], element_curves[1]));
	ASSERT_EQ(3, (int)pv_general_get_parray_num((void **)element_parent->childs));
	ASSERT_EQ(element_parent->childs[1], element_curves[1]);
	// ** append_child invalid
	element_curves[2] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[2]);
	element_curves[3] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[3]);
	ASSERT_TRUE(false == pv_element_append_child(element_parent, element_curves[3], element_curves[2]));
	ASSERT_EQ(3, (int)pv_general_get_parray_num((void **)element_parent->childs));
	pv_element_free(element_curves[2]);
	pv_element_free(element_curves[3]);

	pv_vg_free(vg);
}

TEST(Test, remove){
	PvVg *vg = pv_vg_new();
	assert(NULL != vg);

	PvElement *element_parent = pv_vg_get_layer_top(vg);

	PvElement *element_curves[4];

	// ** remove_delete_recursive
	element_curves[0] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[0]);
	assert(pv_element_append_child(element_parent, NULL, element_curves[0]));
	assert(1 == (int)pv_general_get_parray_num((void **)element_parent->childs));
	ASSERT_TRUE(pv_element_remove_free_recursive(element_curves[0]));
	assert(0 == (int)pv_general_get_parray_num((void **)element_parent->childs));

	// ** remove
	element_curves[0] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[0]);
	assert(pv_element_append_child(element_parent, NULL, element_curves[0]));
	assert(1 == (int)pv_general_get_parray_num((void **)element_parent->childs));
	ASSERT_TRUE(pv_element_remove(element_curves[0]));
	assert(0 == (int)pv_general_get_parray_num((void **)element_parent->childs));
	pv_element_free(element_curves[0]);

	pv_vg_free(vg);
}

TEST(Test, get_first_parent_layer_or_root){
	PvVg *vg = pv_vg_new();
	assert(NULL != vg);

	PvElement *element_root = pv_vg_get_layer_top(vg);

	PvElement *element_layer0 = pv_element_new(PvElementKind_Layer);
	PvElement *element_group0 = pv_element_new(PvElementKind_Group);

	PvElement *element_curves[4];

	element_curves[0] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[0]);
	element_curves[1] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[1]);
	element_curves[2] = pv_element_curve_new_from_rect((PvRect){0,0,0,0});
	assert(element_curves[2]);
	ASSERT_TRUE(element_root == pv_element_get_first_parent_layer_or_root(element_root));
	ASSERT_TRUE(element_layer0 == pv_element_get_first_parent_layer_or_root(element_layer0));
	ASSERT_TRUE(NULL == pv_element_get_first_parent_layer_or_root(element_group0));
	ASSERT_TRUE(NULL == pv_element_get_first_parent_layer_or_root(element_curves[0]));
	ASSERT_TRUE(NULL == pv_element_get_first_parent_layer_or_root(element_curves[1]));

	assert(pv_element_append_child(element_layer0, NULL, element_curves[0]));
	assert(pv_element_append_child(element_group0, NULL, element_curves[1]));
	ASSERT_TRUE(element_root == pv_element_get_first_parent_layer_or_root(element_root));
	ASSERT_TRUE(element_layer0 == pv_element_get_first_parent_layer_or_root(element_layer0));
	ASSERT_TRUE(NULL == pv_element_get_first_parent_layer_or_root(element_group0));
	ASSERT_TRUE(element_layer0 == pv_element_get_first_parent_layer_or_root(element_curves[0]));
	ASSERT_TRUE(NULL == pv_element_get_first_parent_layer_or_root(element_curves[1]));

	assert(pv_element_append_child(element_root, NULL, element_layer0));
	assert(pv_element_append_child(element_layer0, NULL, element_group0));
	ASSERT_TRUE(element_root == pv_element_get_first_parent_layer_or_root(element_root));
	ASSERT_TRUE(element_layer0 == pv_element_get_first_parent_layer_or_root(element_layer0));
	ASSERT_TRUE(element_layer0 == pv_element_get_first_parent_layer_or_root(element_group0));
	ASSERT_TRUE(element_layer0 == pv_element_get_first_parent_layer_or_root(element_curves[0]));
	ASSERT_TRUE(element_layer0 == pv_element_get_first_parent_layer_or_root(element_curves[1]));
}

