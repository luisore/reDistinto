/*
 * test_redis.c
 *
 *  Created on: 8 jun. 2018
 *      Author: avinocur
 */

#include "CUnit/Basic.h"
#include <unistd.h>
#include "../src/redis.h"
#include <commons/log.h>

const int ENTRY_SIZE = 4;
const int NUMBER_OF_ENTRIES = 10;
const char* MOUNT_DIR = "tmp";

static t_log* test_log;
static t_redis* redis;

int init_suite(){
	return 0;
}

int clean_suite(){
	return 0;
}

void setup(){
	test_log = log_create("redis-test.log", "redis-test", false, LOG_LEVEL_DEBUG);
	redis = redis_init(ENTRY_SIZE, NUMBER_OF_ENTRIES, test_log, MOUNT_DIR, redis_replace_circular);
}

void tear_down(){
	log_destroy(test_log);
	redis_destroy(redis);
}

void assert_memory_position(t_memory_position* mem_pos, int last_ref, bool used, bool atomic, char* key){
	CU_ASSERT_PTR_NOT_NULL(mem_pos);
	CU_ASSERT_EQUAL(mem_pos->last_reference, last_ref);
	CU_ASSERT_EQUAL(mem_pos->used, used);
	CU_ASSERT_EQUAL(mem_pos->is_atomic, atomic);
	CU_ASSERT_STRING_EQUAL(mem_pos->key, key);
}

void assert_memory_position_empty(t_memory_position* mem_pos){
	assert_memory_position(mem_pos, 0, false, true, "");
}

void test_init_should_create_correctly(){
	CU_ASSERT_PTR_NOT_NULL(redis);
	CU_ASSERT_EQUAL(redis->current_slot, 0);
	CU_ASSERT_EQUAL(redis->entry_size, ENTRY_SIZE);
	CU_ASSERT_EQUAL(redis->number_of_entries, NUMBER_OF_ENTRIES);
	CU_ASSERT_EQUAL(redis->storage_size, ENTRY_SIZE * NUMBER_OF_ENTRIES);
	CU_ASSERT_TRUE(dictionary_is_empty(redis->key_dictionary));


	t_memory_position* mem_pos;
	for(int i =0; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}
}

void test_get_on_empty_redis_should_return_null(){
	char* res = redis_get(redis, "A_KEY");
	CU_ASSERT_PTR_NULL(res);
}

void test_set_atomic_in_empty_redis_should_add_key(){
	char* key = "KEY";
	char* value = "VAL";

	bool result = redis_set(redis, key, value, strlen(value)+1);

	CU_ASSERT_TRUE(result);
	CU_ASSERT_EQUAL(redis->current_slot, 1);
	CU_ASSERT_EQUAL(dictionary_size(redis->key_dictionary), 1);
	CU_ASSERT_TRUE(dictionary_has_key(redis->key_dictionary, key));

	t_memory_position* mem_pos = redis->occupied_memory_map[0];
	assert_memory_position(mem_pos, 0, true, true, key);

	for(int i =1; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	t_entry_data* entry_data = (t_entry_data*)dictionary_get(redis->key_dictionary, key);
	CU_ASSERT_PTR_NOT_NULL(entry_data);
	CU_ASSERT_EQUAL(entry_data->first_position, 0);
	CU_ASSERT_EQUAL(entry_data->size, strlen(value)+1);

	CU_ASSERT_STRING_EQUAL(redis->memory_region, value);
}

void test_set_not_atomic_in_empty_redis_should_add_key(){
	char* key = "KEY2";
	char* value = "VAL123";

	bool result = redis_set(redis, key, value, strlen(value)+1);

	CU_ASSERT_TRUE(result);
	CU_ASSERT_EQUAL(redis->current_slot, 2);
	CU_ASSERT_EQUAL(dictionary_size(redis->key_dictionary), 1);
	CU_ASSERT_TRUE(dictionary_has_key(redis->key_dictionary, key));

	t_memory_position* mem_pos = redis->occupied_memory_map[0];
	assert_memory_position(mem_pos, 0, true, false, key);

	mem_pos = redis->occupied_memory_map[1];
	assert_memory_position(mem_pos, 0, true, false, key);

	for(int i =2; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	t_entry_data* entry_data = (t_entry_data*)dictionary_get(redis->key_dictionary, key);
	CU_ASSERT_PTR_NOT_NULL(entry_data);
	CU_ASSERT_EQUAL(entry_data->first_position, 0);
	CU_ASSERT_EQUAL(entry_data->size, strlen(value)+1);

	CU_ASSERT_STRING_EQUAL(redis->memory_region, value);
}


void add_tests() {
	CU_pSuite redis_test = CU_add_suite_with_setup_and_teardown("Redis", init_suite, clean_suite, setup, tear_down);
	CU_add_test(redis_test, "test_init_should_create_correctly", test_init_should_create_correctly);
	CU_add_test(redis_test, "test_get_on_empty_redis_should_return_null", test_get_on_empty_redis_should_return_null);
	CU_add_test(redis_test, "test_set_atomic_in_empty_redis_should_add_key", test_set_atomic_in_empty_redis_should_add_key);
	CU_add_test(redis_test, "test_set_not_atomic_in_empty_redis_should_add_key", test_set_not_atomic_in_empty_redis_should_add_key);
}


int run_tests() {
   CU_initialize_registry();

   add_tests();

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   return CU_get_error();
}

