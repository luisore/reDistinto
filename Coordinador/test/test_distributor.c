/*
 * test_distributor.c
 *
 *  Created on: 13 jul. 2018
 *      Author: avinocur
 */

#include "test_distributor.h"
#include "CUnit/Basic.h"
#include <commons/log.h>
#include <commons/collections/list.h>
#include "../src/Distributor.h"
#include <stdlib.h>

t_log* dist_test_log;

int distributor_init_suite(){
	printf("Initializing distributor test suite\n");
	dist_test_log = log_create("distributor-test.log", "distributor-test", false, LOG_LEVEL_TRACE);
	return 0;
}

int distributor_clean_suite(){
	printf("\nCleaning distributor test suite\n");
	log_destroy(dist_test_log);

	return 0;
}

void distributor_setup(){
	printf("\nSetup distributor test\n");
}

void distributor_tear_down(){
	printf("\nTear down distributor test\n");
}

// TEST ADD INSTANCE
void test_add_instance_to_empty_distributor_should_add_instance(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);
	char* instance_name = "NICE_INSTANCE";
	unsigned int space_used = 3;

	distributor_add_instance(distributor, instance_name, space_used);

	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, -1);
	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 1);

	t_instance* instance = (t_instance*)list_get(distributor->instances, 0);
	CU_ASSERT_PTR_NOT_NULL(instance);
	CU_ASSERT_STRING_EQUAL(instance->name, instance_name);
	CU_ASSERT_EQUAL(instance->space_used, space_used);

	distributor_destroy(distributor);
}

void test_remove_instance_from_empty_distributor_should_not_fail(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);

	distributor_remove_instance(distributor, "INST1");

	distributor_destroy(distributor);
}

void test_add_and_remove_should_leave_distributor_empty(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);
	char* instance_name = "NICE_INSTANCE";
	unsigned int space_used = 3;

	distributor_add_instance(distributor, instance_name, space_used);

	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, -1);
	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 1);

	t_instance* instance = (t_instance*)list_get(distributor->instances, 0);
	CU_ASSERT_PTR_NOT_NULL(instance);
	CU_ASSERT_STRING_EQUAL(instance->name, instance_name);
	CU_ASSERT_EQUAL(instance->space_used, space_used);

	distributor_remove_instance(distributor, instance_name);
	CU_ASSERT_TRUE_FATAL(list_is_empty(distributor->instances));

	distributor_destroy(distributor);
}

void test_remove_instance_before_last_used_should_decrement_index(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";

	distributor_add_instance(distributor, instance_name1, 2);
	distributor_add_instance(distributor, instance_name2, 2);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 2);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 4);

	distributor->last_used_instance = 2;

	distributor_remove_instance(distributor, instance_name2);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 3);
	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, 1);

	t_instance* instance = (t_instance*)list_get(distributor->instances, 0);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name1);

	instance = (t_instance*)list_get(distributor->instances, 1);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name3);

	instance = (t_instance*)list_get(distributor->instances, 2);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name4);

	distributor_destroy(distributor);
}

void test_remove_instance_after_last_used_should_not_decrement_index(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";

	distributor_add_instance(distributor, instance_name1, 2);
	distributor_add_instance(distributor, instance_name2, 2);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 2);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 4);

	distributor->last_used_instance = 2;

	distributor_remove_instance(distributor, instance_name4);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 3);
	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, 2);

	t_instance* instance = (t_instance*)list_get(distributor->instances, 0);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name1);

	instance = (t_instance*)list_get(distributor->instances, 1);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name2);

	instance = (t_instance*)list_get(distributor->instances, 2);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name3);

	distributor_destroy(distributor);
}

void test_remove_instance_last_used_should_decrement_index(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";

	distributor_add_instance(distributor, instance_name1, 2);
	distributor_add_instance(distributor, instance_name2, 2);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 2);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 4);

	distributor->last_used_instance = 2;

	distributor_remove_instance(distributor, instance_name3);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 3);
	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, 1);

	t_instance* instance = (t_instance*)list_get(distributor->instances, 0);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name1);

	instance = (t_instance*)list_get(distributor->instances, 1);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name2);

	instance = (t_instance*)list_get(distributor->instances, 2);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name4);

	distributor_destroy(distributor);
}


// TEST SELECT ON EMPTY DISTRIBUTOR

void test_select_instance_empty_distributor_EL_should_return_null(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NULL(selected_instance);

	distributor_destroy(distributor);
}

void test_select_instance_empty_distributor_KE_should_return_null(){
	t_distributor* distributor = distributor_init(KE, dist_test_log);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NULL(selected_instance);

	distributor_destroy(distributor);
}

void test_select_instance_empty_distributor_LSU_should_return_null(){
	t_distributor* distributor = distributor_init(LSU, dist_test_log);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NULL(selected_instance);

	distributor_destroy(distributor);
}


// TEST SELECT WITH ONE INSTANCE
void test_select_instance_distributor_with_one_instance_EL_should_return_that_instance(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);

	char* instance_name = "NICE_INSTANCE";
	unsigned int space_used = 3;
	distributor_add_instance(distributor, instance_name, space_used);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);

	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name);

	CU_ASSERT_EQUAL(distributor->last_used_instance, 0);

	free(selected_instance);
	distributor_destroy(distributor);
}

void test_select_instance_distributor_with_one_instance_KE_should_return_that_instance(){
	t_distributor* distributor = distributor_init(KE, dist_test_log);

	char* instance_name = "NICE_INSTANCE";
	unsigned int space_used = 3;
	distributor_add_instance(distributor, instance_name, space_used);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);

	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name);

	CU_ASSERT_EQUAL(distributor->last_used_instance, 0);

	free(selected_instance);
	distributor_destroy(distributor);
}

void test_select_instance_distributor_with_one_instance_LSU_should_return_that_instance(){
	t_distributor* distributor = distributor_init(LSU, dist_test_log);

	char* instance_name = "NICE_INSTANCE";
	unsigned int space_used = 3;
	distributor_add_instance(distributor, instance_name, space_used);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);

	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name);

	CU_ASSERT_EQUAL(distributor->last_used_instance, 0);

	free(selected_instance);
	distributor_destroy(distributor);
}

// TESTS EQUITATIVE LOAD

void test_select_instance_two_instances_EL_first_call_should_return_first_one(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);

	char* instance_name1 = "NICE_INSTANCE";
	unsigned int space_used1 = 8;
	char* instance_name2 = "ANOTHER_INSTANCE";
	unsigned int space_used2 = 2;
	distributor_add_instance(distributor, instance_name1, space_used1);
	distributor_add_instance(distributor, instance_name2, space_used2);

	CU_ASSERT_EQUAL(distributor->last_used_instance, -1);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);

	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name1);

	CU_ASSERT_EQUAL(distributor->last_used_instance, 0);

	free(selected_instance);
	distributor_destroy(distributor);
}

void test_select_instance_two_instances_EL_second_call_should_return_second_one(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);

	char* instance_name1 = "NICE_INSTANCE";
	unsigned int space_used1 = 8;
	char* instance_name2 = "ANOTHER_INSTANCE";
	unsigned int space_used2 = 2;
	distributor_add_instance(distributor, instance_name1, space_used1);
	distributor_add_instance(distributor, instance_name2, space_used2);

	CU_ASSERT_EQUAL(distributor->last_used_instance, -1);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);
	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name1);
	CU_ASSERT_EQUAL(distributor->last_used_instance, 0);
	free(selected_instance);

	selected_instance = distributor_select_instance(distributor, "A KEY");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);
	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name2);
	CU_ASSERT_EQUAL(distributor->last_used_instance, 1);
	free(selected_instance);


	distributor_destroy(distributor);
}

void test_select_instance_two_instances_EL_third_call_should_circle_back_to_first_one(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);

	char* instance_name1 = "NICE_INSTANCE";
	unsigned int space_used1 = 8;
	char* instance_name2 = "ANOTHER_INSTANCE";
	unsigned int space_used2 = 2;
	distributor_add_instance(distributor, instance_name1, space_used1);
	distributor_add_instance(distributor, instance_name2, space_used2);

	CU_ASSERT_EQUAL(distributor->last_used_instance, -1);

	char* selected_instance = distributor_select_instance(distributor, "A KEY");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);
	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name1);
	CU_ASSERT_EQUAL(distributor->last_used_instance, 0);
	free(selected_instance);

	selected_instance = distributor_select_instance(distributor, "A KEY");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);
	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name2);
	CU_ASSERT_EQUAL(distributor->last_used_instance, 1);
	free(selected_instance);

	selected_instance = distributor_select_instance(distributor, "A KEY");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected_instance);
	CU_ASSERT_STRING_EQUAL_FATAL(selected_instance, instance_name1);
	CU_ASSERT_EQUAL(distributor->last_used_instance, 0);
	free(selected_instance);


	distributor_destroy(distributor);
}

// TEST LEAST SPACE USED
void test_update_spaced_used_should_update_correctly(){
	t_distributor* distributor = distributor_init(EL, dist_test_log);

	char* instance_name1 = "NICE_INSTANCE";
	unsigned int space_used1 = 8;
	char* instance_name2 = "ANOTHER_INSTANCE";
	unsigned int space_used2 = 2;
	unsigned int space_used3 = 4;
	distributor_add_instance(distributor, instance_name1, space_used1);
	distributor_add_instance(distributor, instance_name2, space_used2);

	CU_ASSERT_EQUAL(list_size(distributor->instances), 2);

	t_instance* instance = (t_instance*)list_get(distributor->instances, 0);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name1);
	CU_ASSERT_EQUAL_FATAL(instance->space_used, space_used1);

	instance = (t_instance*)list_get(distributor->instances, 1);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name2);
	CU_ASSERT_EQUAL_FATAL(instance->space_used, space_used2);

	distributor_update_space_used(distributor, instance_name1, space_used3);

	instance = (t_instance*)list_get(distributor->instances, 0);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name1);
	CU_ASSERT_EQUAL_FATAL(instance->space_used, space_used3);

	instance = (t_instance*)list_get(distributor->instances, 1);
	CU_ASSERT_STRING_EQUAL_FATAL(instance->name, instance_name2);
	CU_ASSERT_EQUAL_FATAL(instance->space_used, space_used2);

	distributor_destroy(distributor);
}

void test_select_instance_LSU_many_instances_should_select_the_one_with_least_space_used(){
	t_distributor* distributor = distributor_init(LSU, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";

	distributor_add_instance(distributor, instance_name1, 5);
	distributor_add_instance(distributor, instance_name2, 3);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 7);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 4);

	char* selected = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, 2);

	free(selected);
	distributor_destroy(distributor);
}

void test_select_instance_LSU_many_instances_is_tie_last_used_before_should_tie_break_with_el(){
	t_distributor* distributor = distributor_init(LSU, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";

	distributor_add_instance(distributor, instance_name1, 5);
	distributor_add_instance(distributor, instance_name2, 2);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 7);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 4);

	distributor->last_used_instance = 0;

	char* selected = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, 1);

	free(selected);
	distributor_destroy(distributor);
}

void test_select_instance_LSU_many_instances_is_tie_last_used_middle_should_tie_break_with_el(){
	t_distributor* distributor = distributor_init(LSU, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";

	distributor_add_instance(distributor, instance_name1, 2);
	distributor_add_instance(distributor, instance_name2, 4);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 7);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 4);

	distributor->last_used_instance = 1;


	char* selected = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, 2);

	free(selected);
	distributor_destroy(distributor);
}

void test_select_instance_LSU_many_instances_is_tie_last_used_in_tie_should_tie_break_with_el(){
	t_distributor* distributor = distributor_init(LSU, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";

	distributor_add_instance(distributor, instance_name1, 2);
	distributor_add_instance(distributor, instance_name2, 4);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 7);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 4);

	distributor->last_used_instance = 0;


	char* selected = distributor_select_instance(distributor, "A KEY");

	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	CU_ASSERT_EQUAL_FATAL(distributor->last_used_instance, 2);

	free(selected);
	distributor_destroy(distributor);
}

// TEST KEY EXPLICIT
void test_select_instance_KE_two_instances_should_select_according_to_first_char_of_key(){
	t_distributor* distributor = distributor_init(KE, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";

	distributor_add_instance(distributor, instance_name1, 5);
	distributor_add_instance(distributor, instance_name2, 3);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 2);

	char* selected = distributor_select_instance(distributor, "axxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "fxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "mxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "nxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "txxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "zxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	distributor_destroy(distributor);
}

void test_select_instance_KE_three_instances_should_select_according_to_first_char_of_key(){
	t_distributor* distributor = distributor_init(KE, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";

	distributor_add_instance(distributor, instance_name1, 5);
	distributor_add_instance(distributor, instance_name2, 3);
	distributor_add_instance(distributor, instance_name3, 2);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 3);

	char* selected = distributor_select_instance(distributor, "axxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "exxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "ixxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "jxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "nxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "rxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "sxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	selected = distributor_select_instance(distributor, "wxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	selected = distributor_select_instance(distributor, "zxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	distributor_destroy(distributor);
}


void test_select_instance_KE_four_instances_should_select_according_to_first_char_of_key(){
	t_distributor* distributor = distributor_init(KE, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";

	distributor_add_instance(distributor, instance_name1, 5);
	distributor_add_instance(distributor, instance_name2, 3);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 7);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 4);

	char* selected = distributor_select_instance(distributor, "axxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "cxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "gxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "hxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "kxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "nxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "oxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	selected = distributor_select_instance(distributor, "rxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	selected = distributor_select_instance(distributor, "uxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	selected = distributor_select_instance(distributor, "vxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name4);
	free(selected);

	selected = distributor_select_instance(distributor, "xxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name4);
	free(selected);

	selected = distributor_select_instance(distributor, "zxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name4);
	free(selected);

	distributor_destroy(distributor);
}

void test_select_instance_KE_five_instances_should_select_according_to_first_char_of_key(){
	t_distributor* distributor = distributor_init(KE, dist_test_log);
	char* instance_name1 = "INST1";
	char* instance_name2 = "INST2";
	char* instance_name3 = "INST3";
	char* instance_name4 = "INST4";
	char* instance_name5 = "INST5";

	distributor_add_instance(distributor, instance_name1, 5);
	distributor_add_instance(distributor, instance_name2, 3);
	distributor_add_instance(distributor, instance_name3, 2);
	distributor_add_instance(distributor, instance_name4, 7);
	distributor_add_instance(distributor, instance_name5, 1);

	CU_ASSERT_EQUAL_FATAL(list_size(distributor->instances), 5);

	char* selected = distributor_select_instance(distributor, "axxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "cxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "fxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name1);
	free(selected);

	selected = distributor_select_instance(distributor, "gxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "ixxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "lxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name2);
	free(selected);

	selected = distributor_select_instance(distributor, "mxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	selected = distributor_select_instance(distributor, "oxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	selected = distributor_select_instance(distributor, "rxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name3);
	free(selected);

	selected = distributor_select_instance(distributor, "sxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name4);
	free(selected);

	selected = distributor_select_instance(distributor, "txxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name4);
	free(selected);

	selected = distributor_select_instance(distributor, "xxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name4);
	free(selected);

	selected = distributor_select_instance(distributor, "yxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name5);
	free(selected);

	selected = distributor_select_instance(distributor, "zxxx");
	CU_ASSERT_PTR_NOT_NULL_FATAL(selected);
	CU_ASSERT_STRING_EQUAL_FATAL(selected, instance_name5);
	free(selected);

	distributor_destroy(distributor);
}



void add_distributor_tests(){
	CU_pSuite dist_test = CU_add_suite_with_setup_and_teardown("Distributor", distributor_init_suite,
			distributor_clean_suite, distributor_setup, distributor_tear_down);
	CU_add_test(dist_test, "test_add_instance_to_empty_distributor_should_add_instance", test_add_instance_to_empty_distributor_should_add_instance);
	CU_add_test(dist_test, "test_remove_instance_from_empty_distributor_should_not_fail", test_remove_instance_from_empty_distributor_should_not_fail);
	CU_add_test(dist_test, "test_remove_instance_before_last_used_should_decrement_index", test_remove_instance_before_last_used_should_decrement_index);
	CU_add_test(dist_test, "test_remove_instance_after_last_used_should_not_decrement_index", test_remove_instance_after_last_used_should_not_decrement_index);
	CU_add_test(dist_test, "test_remove_instance_last_used_should_decrement_index", test_remove_instance_last_used_should_decrement_index);
	CU_add_test(dist_test, "test_add_and_remove_should_leave_distributor_empty", test_add_and_remove_should_leave_distributor_empty);

	CU_add_test(dist_test, "test_select_instance_empty_distributor_EL_should_return_null", test_select_instance_empty_distributor_EL_should_return_null);
	CU_add_test(dist_test, "test_select_instance_empty_distributor_KE_should_return_null", test_select_instance_empty_distributor_KE_should_return_null);
	CU_add_test(dist_test, "test_select_instance_empty_distributor_LSU_should_return_null", test_select_instance_empty_distributor_LSU_should_return_null);

	CU_add_test(dist_test, "test_select_instance_distributor_with_one_instance_EL_should_return_that_instance", test_select_instance_distributor_with_one_instance_EL_should_return_that_instance);
	CU_add_test(dist_test, "test_select_instance_distributor_with_one_instance_KE_should_return_that_instance", test_select_instance_distributor_with_one_instance_KE_should_return_that_instance);
	CU_add_test(dist_test, "test_select_instance_distributor_with_one_instance_LSU_should_return_that_instance", test_select_instance_distributor_with_one_instance_LSU_should_return_that_instance);

	CU_add_test(dist_test, "test_select_instance_two_instances_EL_first_call_should_return_first_one", test_select_instance_two_instances_EL_first_call_should_return_first_one);
	CU_add_test(dist_test, "test_select_instance_two_instances_EL_second_call_should_return_second_one", test_select_instance_two_instances_EL_second_call_should_return_second_one);
	CU_add_test(dist_test, "test_select_instance_two_instances_EL_third_call_should_circle_back_to_first_one", test_select_instance_two_instances_EL_third_call_should_circle_back_to_first_one);

	CU_add_test(dist_test, "test_update_spaced_used_should_update_correctly", test_update_spaced_used_should_update_correctly);
	CU_add_test(dist_test, "test_select_instance_LSU_many_instances_should_select_the_one_with_least_space_used", test_select_instance_LSU_many_instances_should_select_the_one_with_least_space_used);
	CU_add_test(dist_test, "test_select_instance_LSU_many_instances_is_tie_last_used_before_should_tie_break_with_el", test_select_instance_LSU_many_instances_is_tie_last_used_before_should_tie_break_with_el);
	CU_add_test(dist_test, "test_select_instance_LSU_many_instances_is_tie_last_used_middle_should_tie_break_with_el", test_select_instance_LSU_many_instances_is_tie_last_used_middle_should_tie_break_with_el);
	CU_add_test(dist_test, "test_select_instance_LSU_many_instances_is_tie_last_used_in_tie_should_tie_break_with_el", test_select_instance_LSU_many_instances_is_tie_last_used_in_tie_should_tie_break_with_el);

	CU_add_test(dist_test, "test_select_instance_KE_two_instances_should_select_according_to_first_char_of_key", test_select_instance_KE_two_instances_should_select_according_to_first_char_of_key);
	CU_add_test(dist_test, "test_select_instance_KE_three_instances_should_select_according_to_first_char_of_key", test_select_instance_KE_three_instances_should_select_according_to_first_char_of_key);
	CU_add_test(dist_test, "test_select_instance_KE_four_instances_should_select_according_to_first_char_of_key", test_select_instance_KE_four_instances_should_select_according_to_first_char_of_key);
	CU_add_test(dist_test, "test_select_instance_KE_five_instances_should_select_according_to_first_char_of_key", test_select_instance_KE_five_instances_should_select_according_to_first_char_of_key);

}

