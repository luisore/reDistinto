/*
 * test_redis.c
 *
 *  Created on: 8 jun. 2018
 *      Author: avinocur
 */

#include "CUnit/Basic.h"
#include <stdlib.h>
#include <unistd.h>
#include "../src/redis.h"
#include <commons/log.h>

const int ENTRY_SIZE = 4;
const int NUMBER_OF_ENTRIES = 10;
const char* MOUNT_DIR = "tmp";

static t_log* test_log;
static t_redis* redis;

int init_suite(){
	test_log = log_create("redis-test.log", "redis-test", false, LOG_LEVEL_DEBUG);
	return 0;
}

int clean_suite(){
	log_destroy(test_log);
	return 0;
}

void setup(){
	redis = redis_init(ENTRY_SIZE, NUMBER_OF_ENTRIES, test_log, MOUNT_DIR, redis_replace_circular);
}

void tear_down(){
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

void assert_key_in_position(int current_slot, int first_pos, int expected_keys, char* key,
		char* value, int value_size){

	int slots_occupied = slots_occupied_by(ENTRY_SIZE, value_size);

	CU_ASSERT_EQUAL(redis->current_slot, current_slot);

	for(int i = 0; i < slots_occupied; i++){
		t_memory_position* mem_pos = redis->occupied_memory_map[first_pos + i];
		assert_memory_position(mem_pos, 0, true, slots_occupied == 1, key);
	}

	CU_ASSERT_EQUAL(dictionary_size(redis->key_dictionary), expected_keys);
	CU_ASSERT_TRUE(dictionary_has_key(redis->key_dictionary, key));
	t_entry_data* entry_data = (t_entry_data*)dictionary_get(redis->key_dictionary, key);
	CU_ASSERT_PTR_NOT_NULL(entry_data);
	CU_ASSERT_EQUAL(entry_data->first_position, first_pos);
	CU_ASSERT_EQUAL(entry_data->size, value_size);

	int offset = first_pos * ENTRY_SIZE;

	CU_ASSERT_STRING_EQUAL(redis->memory_region + offset , value);
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
	unsigned int value_size = strlen(value) + 1;
	int expected_slot = 1;
	int expected_first = 0;
	int expected_keys = 1;

	bool result = redis_set(redis, key, value, value_size);

	CU_ASSERT_TRUE(result);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value, value_size);

	t_memory_position* mem_pos;

	for(int i =1; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}
}

void test_set_and_get_atomic_in_empty_redis_should_add_and_get_key(){
	char* key = "KEY";
	char* value = "VAL";
	unsigned int value_size = strlen(value) + 1;

	bool result = redis_set(redis, key, value, value_size);
	CU_ASSERT_TRUE(result);

	char *retrieved = redis_get(redis, key);

	CU_ASSERT_PTR_NOT_NULL(retrieved);
	CU_ASSERT_STRING_EQUAL(retrieved, value);

	free(retrieved);
}

void test_set_not_atomic_in_empty_redis_should_add_key(){
	char* key = "KEY2";
	char* value = "VAL123";
	unsigned int value_size = strlen(value)+1;
	int expected_slot = 2;
	int expected_first = 0;
	int expected_keys = 1;

	bool result = redis_set(redis, key, value, value_size);

	CU_ASSERT_TRUE(result);
	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value, value_size);

	t_memory_position* mem_pos;
	for(int i =2; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}
}

void test_set_and_get_not_atomic_in_empty_redis_should_add_and_get_key(){
	char* key = "KEY2";
	char* value = "VAL123";
	unsigned int value_size = strlen(value)+1;

	bool result = redis_set(redis, key, value, value_size);
	CU_ASSERT_TRUE(result);

	char *retrieved = redis_get(redis, key);

	CU_ASSERT_PTR_NOT_NULL(retrieved);
	CU_ASSERT_STRING_EQUAL(retrieved, value);

	free(retrieved);
}


void test_set_two_keys_with_space_should_keep_both(){
	char* key1 = "KEY1";
	char* value1 = "ATO";
	unsigned int value_size1 = strlen(value1)+1;
	char* key2 = "KEY2";
	char* value2 = "LONGVALUE";
	unsigned int value_size2 = strlen(value2)+1;

	// add first key
	bool result = redis_set(redis, key1, value1, value_size1);
	CU_ASSERT_TRUE(result);
	assert_key_in_position(1, 0, 1, key1, value1, value_size1);

	// rest of memory not used
	t_memory_position* mem_pos;
	for(int i =1; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	// add second key
	result = redis_set(redis, key2, value2, value_size2);
	CU_ASSERT_TRUE(result);

	// check that both keys are present
	assert_key_in_position(4, 0, 2, key1, value1, value_size1);
	assert_key_in_position(4, 1, 2, key2, value2, value_size2);

	// rest of memory not used
	for(int i =4; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}
}

void test_add_may_keys_not_enough_space_replace_circular_should_replace(){
	char* key1 = "KEY1";
	char* value1 = "TWOSLOT"; // 2 slots
	unsigned int value_size1 = strlen(value1)+1;
	char* key2 = "KEY2";
	char* value2 = "THISSPANSOVERFIVE"; // 5 slots
	unsigned int value_size2 = strlen(value2)+1;
	char* key3 = "KEY3";
	char* value3 = "THREESLOTS"; // 3 slots
	unsigned int value_size3 = strlen(value3)+1;
	char* key4 = "KEY4";
	char* value4 = "ATO"; // 1 slot
	unsigned int value_size4 = strlen(value4)+1;


	// add first key
	bool result = redis_set(redis, key1, value1, value_size1);
	CU_ASSERT_TRUE(result);
	assert_key_in_position(2, 0, 1, key1, value1, value_size1);

	// rest of memory not used
	t_memory_position* mem_pos;
	for(int i =2; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	// add second key
	result = redis_set(redis, key2, value2, value_size2);
	CU_ASSERT_TRUE(result);

	// check that both keys are present
	assert_key_in_position(7, 0, 2, key1, value1, value_size1);
	assert_key_in_position(7, 2, 2, key2, value2, value_size2);

	// rest of memory not used
	for(int i =7; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	// add third key
	result = redis_set(redis, key3, value3, value_size3);
	CU_ASSERT_TRUE(result);

	// check that keys are present
	assert_key_in_position(0, 0, 3, key1, value1, value_size1);
	assert_key_in_position(0, 2, 3, key2, value2, value_size2);
	assert_key_in_position(0, 7, 3, key3, value3, value_size3);

	// add fourth key. should evict key1
	result = redis_set(redis, key4, value4, value_size4);
	CU_ASSERT_TRUE(result);

	// check that keys are present
	assert_key_in_position(1, 0, 3, key4, value4, value_size4);
	assert_key_in_position(1, 2, 3, key2, value2, value_size2);
	assert_key_in_position(1, 7, 3, key3, value3, value_size3);

	// position 1 of memory should be empty because key1 used 2 slots
	// and key4 only requires 1 slot.
	mem_pos = redis->occupied_memory_map[1];
	assert_memory_position_empty(mem_pos);
}

void test_add_many_with_replaces_has_space_fragmented_should_signal_compaction(){

	char* key1 = "KEY1";
	char* value1 = "THREESLOTS"; // 3 slots
	unsigned int value_size1 = strlen(value1)+1;
	char* key2 = "KEY2";
	char* value2 = "THISSPANSOVER4"; // 4 slots
	unsigned int value_size2 = strlen(value2)+1;
	char* key3 = "KEY3";
	char* value3 = "2SLOTS"; // 2 slots
	unsigned int value_size3 = strlen(value3)+1;
	char* key4 = "KEY4";
	char* value4 = "DOSDOS"; // 2 slot
	unsigned int value_size4 = strlen(value4)+1;
	char* key5 = "KEY5";
	char* value5 = "OTROS2"; // 2 slot
	unsigned int value_size5 = strlen(value4)+1;

	bool result = redis_set(redis, key1, value1, value_size1);
	CU_ASSERT_TRUE(result);
	result = redis_set(redis, key2, value2, value_size2);
	CU_ASSERT_TRUE(result);
	result = redis_set(redis, key3, value3, value_size3);
	CU_ASSERT_TRUE(result);
	result = redis_set(redis, key4, value4, value_size4);
	CU_ASSERT_TRUE(result);

	// Assert status before adding key6. Current slot should be 2
	// key 4 is in slot 0
	assert_key_in_position(2, 0, 3, key4, value4, value_size4);

	// slot 2 is empty
	t_memory_position* mem_pos = redis->occupied_memory_map[2];
	assert_memory_position_empty(mem_pos);

	// key 2 is in slots 3 to 6
	assert_key_in_position(2, 3, 3, key2, value2, value_size2);

	// key 3 is in slots 7 and 8
	assert_key_in_position(2, 7, 3, key3, value3, value_size3);

	// slots 9 is empty
	mem_pos = redis->occupied_memory_map[9];
	assert_memory_position_empty(mem_pos);

	// Now add key 5 of 2 positions should return false, signaling
	// the need for compaction
	result = redis_set(redis, key5, value5, value_size5);
	CU_ASSERT_FALSE(result);

	// Key 5 should not be present
	char* retrieved = redis_get(redis, key5);
	CU_ASSERT_PTR_NULL(retrieved);
}

void add_tests() {
	CU_pSuite redis_test = CU_add_suite_with_setup_and_teardown("Redis", init_suite, clean_suite, setup, tear_down);
	CU_add_test(redis_test, "test_init_should_create_correctly", test_init_should_create_correctly);
	CU_add_test(redis_test, "test_get_on_empty_redis_should_return_null", test_get_on_empty_redis_should_return_null);
	CU_add_test(redis_test, "test_set_atomic_in_empty_redis_should_add_key", test_set_atomic_in_empty_redis_should_add_key);
	CU_add_test(redis_test, "test_set_and_get_atomic_in_empty_redis_should_add_and_get_key", test_set_and_get_atomic_in_empty_redis_should_add_and_get_key);
	CU_add_test(redis_test, "test_set_not_atomic_in_empty_redis_should_add_key", test_set_not_atomic_in_empty_redis_should_add_key);
	CU_add_test(redis_test, "test_set_and_get_not_atomic_in_empty_redis_should_add_and_get_key", test_set_and_get_not_atomic_in_empty_redis_should_add_and_get_key);
	CU_add_test(redis_test, "test_set_two_keys_with_space_should_keep_both", test_set_two_keys_with_space_should_keep_both);
	CU_add_test(redis_test, "test_add_may_keys_not_enough_space_replace_circular_should_replace", test_add_may_keys_not_enough_space_replace_circular_should_replace);
	CU_add_test(redis_test, "test_add_many_with_replaces_has_space_fragmented_should_signal_compaction", test_add_many_with_replaces_has_space_fragmented_should_signal_compaction);
}


int run_tests() {
   CU_initialize_registry();

   add_tests();

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   return CU_get_error();
}

