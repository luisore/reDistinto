/*
 * test_suite.c
 *
 *  Created on: 13 jul. 2018
 *      Author: avinocur
 */

#include "test_distributor.h"
#include "CUnit/Basic.h"

int run_tests() {
   CU_initialize_registry();

   add_distributor_tests();

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   return CU_get_error();
}


