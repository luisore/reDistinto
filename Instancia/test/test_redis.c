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
	test_log = log_create("redis-test.log", "redis-test", false, LOG_LEVEL_TRACE);
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
	CU_ASSERT_PTR_NOT_NULL_FATAL(mem_pos);
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
	CU_ASSERT_PTR_NOT_NULL_FATAL(entry_data);
	CU_ASSERT_EQUAL(entry_data->first_position, first_pos);
	CU_ASSERT_EQUAL(entry_data->size, value_size);

	int offset = first_pos * ENTRY_SIZE;

	CU_ASSERT_STRING_EQUAL(redis->memory_region + offset , value);
}

void assert_get_key(char* key, char* expected_value){
	char* retrieved = redis_get(redis, key);
	if(expected_value != NULL){
		CU_ASSERT_PTR_NOT_NULL_FATAL(retrieved);
		CU_ASSERT_STRING_EQUAL(retrieved, expected_value);
		free(retrieved);
	} else {
		CU_ASSERT_PTR_NULL(retrieved);
	}
}

void test_init_should_create_correctly(){
	CU_ASSERT_PTR_NOT_NULL_FATAL(redis);
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

	CU_ASSERT_PTR_NOT_NULL_FATAL(retrieved);
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

	CU_ASSERT_PTR_NOT_NULL_FATAL(retrieved);
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

void test_add_atomic_keys_until_memory_full_should_keep_all_keys(){
	char* keys[10] = {"KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9", "KEY10"};
	char* vals[10] = {  "V" ,  "VA" ,  "VA3", "VA4" , "VA5" , "VA6" , "VA7" ,  "8"  , "VA9" ,  "10"  };
	int  sizes[10] = {   2  ,   3   ,    4  ,   4   ,   4   ,   4   ,   4   ,   2   ,   4   ,   3    };

	for(int i=0; i<10; i++){
		bool res = redis_set(redis, keys[i], vals[i], sizes[i]);
		CU_ASSERT_TRUE(res);
	}

	for(int i=0; i<10; i++){
		assert_key_in_position(0, i, 10, keys[i], vals[i], sizes[i]);
	}
}

void test_add_atomic_to_redis_full_of_atomic_keys_replace_circular_should_replace_first_key(){
	char* keys[10] = {"KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9", "KEY10"};
	char* vals[10] = {  "V" ,  "VA" ,  "VA3", "VA4" , "VA5" , "VA6" , "VA7" ,  "8"  , "VA9" ,  "10"  };
	int  sizes[10] = {   2  ,   3   ,    4  ,   4   ,   4   ,   4   ,   4   ,   2   ,   4   ,   3    };

	bool res;
	for(int i=0; i<10; i++){
		res = redis_set(redis, keys[i], vals[i], sizes[i]);
		CU_ASSERT_TRUE(res);
	}

	for(int i=0; i<10; i++){
		assert_key_in_position(0, i, 10, keys[i], vals[i], sizes[i]);
	}

	char* new_key = "NEW";
	char* new_val = "NVL";
	int new_val_size = 4;

	res = redis_set(redis, new_key, new_val, new_val_size);
	CU_ASSERT_TRUE(res);

	assert_key_in_position(1, 0, 10, new_key, new_val, new_val_size);

	for(int i=1; i<10; i++){
		assert_key_in_position(1, i, 10, keys[i], vals[i], sizes[i]);
	}
}

void test_add_non_atomic_to_redis_full_of_atomic_keys_replace_circular_should_replace_first_keys(){
	char* keys[10] = {"KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9", "KEY10"};
	char* vals[10] = {  "V" ,  "VA" ,  "VA3", "VA4" , "VA5" , "VA6" , "VA7" ,  "8"  , "VA9" ,  "10"  };
	int  sizes[10] = {   2  ,   3   ,    4  ,   4   ,   4   ,   4   ,   4   ,   2   ,   4   ,   3    };

	bool res;
	for(int i=0; i<10; i++){
		res = redis_set(redis, keys[i], vals[i], sizes[i]);
		CU_ASSERT_TRUE(res);
	}

	for(int i=0; i<10; i++){
		assert_key_in_position(0, i, 10, keys[i], vals[i], sizes[i]);
	}

	char* new_key = "NEW";
	char* new_val = "NEWVALUE";
	int new_val_size = 9;

	res = redis_set(redis, new_key, new_val, new_val_size);
	CU_ASSERT_TRUE(res);

	assert_key_in_position(3, 0, 8, new_key, new_val, new_val_size);

	for(int i=3; i<10; i++){
		assert_key_in_position(3, i, 8, keys[i], vals[i], sizes[i]);
	}
}

void test_add_non_atomic_to_redis_full_first_key_not_atomic_replace_circular_should_replace_first_atomic_keys(){
	char* keys[10] = {"KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9", "KEY10"};
	char* vals[10] = {  "V" ,  "VA" ,  "VA3", "VA4" , "VA5" , "VA6" , "VA7" ,  "8"  , "VA9" ,  "10"  };
	int  sizes[10] = {   2  ,   3   ,    4  ,   4   ,   4   ,   4   ,   4   ,   2   ,   4   ,   3    };

	bool res;

	char* first_key = "FST";
	char* first_val = "FIRSTVL";
	int first_size = 8;
	res = redis_set(redis, first_key, first_val, first_size);
	CU_ASSERT_TRUE(res);

	for(int i=2; i<10; i++){
		res = redis_set(redis, keys[i], vals[i], sizes[i]);
		CU_ASSERT_TRUE(res);
	}


	assert_key_in_position(0, 0, 9, first_key, first_val, first_size);

	for(int i=2; i<10; i++){
		assert_key_in_position(0, i, 9, keys[i], vals[i], sizes[i]);
	}

	char* new_key = "NEW";
	char* new_val = "NEWVAL";
	int new_val_size = 7;

	res = redis_set(redis, new_key, new_val, new_val_size);
	CU_ASSERT_TRUE(res);

	assert_key_in_position(4, 2, 8, new_key, new_val, new_val_size);

	for(int i=4; i<10; i++){
		assert_key_in_position(4, i, 8, keys[i], vals[i], sizes[i]);
	}
}

void test_set_non_atomic_with_space_available_not_contiguous_should_signal_compact(){
	char* keys[10] = {"KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9", "KEY10"};
	char* vals[10] = {  "V" ,  "VA" ,  "VA3", "VA4" , "VA5" , "VA6" , "VA7" ,  "8"  , "VA9" ,  "10"  };
	int  sizes[10] = {   2  ,   3   ,    4  ,   4   ,   4   ,   4   ,   4   ,   2   ,   4   ,   3    };

	char* key1 = "KEY";
	char* value1 = "2SLOTS";
	char* value2 = "ATO";
	int size1 = strlen(value1) + 1;
	int size2 = strlen(value2) + 1;

	bool res = redis_set(redis, key1, value1, size1);
	CU_ASSERT_TRUE(res);

	for(int i=2; i<9; i++){
		res = redis_set(redis, keys[i], vals[i], sizes[i]);
		CU_ASSERT_TRUE(res);
	}

	res = redis_set(redis, key1, value2, size2);
	CU_ASSERT_TRUE(res);

	assert_key_in_position(9, 0, 8, key1, value2, size2);

	for(int i=2; i<9; i++){
		assert_key_in_position(9, i, 8, keys[i], vals[i], sizes[i]);
	}

	CU_ASSERT_EQUAL(redis->slots_available, 2);

	char* newkey = "NEWKEY";
	char* newval = "NEWVAL";
	int newsize = strlen(newval) + 1;

	res = redis_set(redis, newkey, newval, newsize);

	CU_ASSERT_FALSE(res); // false signals compaction

	// stored values remain unchanged
	assert_key_in_position(9, 0, 8, key1, value2, size2);

	for(int i=2; i<9; i++){
		assert_key_in_position(9, i, 8, keys[i], vals[i], sizes[i]);
	}

	CU_ASSERT_EQUAL(redis->slots_available, 2);
}

void set_atomic_value_twice_should_replace_in_same_position(){
	char* key = "KEY";
	char* value1 = "ABC";
	char* value2 = "DE";
	unsigned int value_size1 = strlen(value1) + 1;
	unsigned int value_size2 = strlen(value2) + 1;
	int expected_slot = 1;
	int expected_first = 0;
	int expected_keys = 1;

	bool result = redis_set(redis, key, value1, value_size1);

	CU_ASSERT_TRUE(result);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value1, value_size1);

	t_memory_position* mem_pos;

	for(int i =1; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	result = redis_set(redis, key, value2, value_size2);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value2, value_size2);

	for(int i =1; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}
}

void set_not_atomic_value_twice_new_value_same_size_should_replace_in_same_position(){
	char* key = "KEY";
	char* value1 = "ABCDEFGHI";
	char* value2 = "NOTTHESAME";
	unsigned int value_size1 = strlen(value1) + 1;
	unsigned int value_size2 = strlen(value2) + 1;
	int expected_slot = 3;
	int expected_first = 0;
	int expected_keys = 1;

	bool result = redis_set(redis, key, value1, value_size1);

	CU_ASSERT_TRUE(result);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value1, value_size1);

	t_memory_position* mem_pos;

	for(int i =3; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	result = redis_set(redis, key, value2, value_size2);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value2, value_size2);

	for(int i =3; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}
}

void set_not_atomic_value_twice_new_value_is_atomic_should_replace_in_same_position_and_free_slots(){
	char* key = "KEY";
	char* value1 = "ABCDEFGHI";
	char* value2 = "SML";
	unsigned int value_size1 = strlen(value1) + 1;
	unsigned int value_size2 = strlen(value2) + 1;
	int expected_slot = 3;
	int expected_first = 0;
	int expected_keys = 1;

	bool result = redis_set(redis, key, value1, value_size1);

	CU_ASSERT_TRUE(result);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value1, value_size1);

	t_memory_position* mem_pos;

	for(int i =3; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	result = redis_set(redis, key, value2, value_size2);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value2, value_size2);

	for(int i =1; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}
}

void set_not_atomic_value_twice_new_value_is_smaller_not_atomic_should_replace_in_same_position_and_free_slots(){
	char* key = "KEY";
	char* value1 = "ABCDEFGHI";
	char* value2 = "SMALL";
	unsigned int value_size1 = strlen(value1) + 1;
	unsigned int value_size2 = strlen(value2) + 1;
	int expected_slot = 3;
	int expected_first = 0;
	int expected_keys = 1;

	bool result = redis_set(redis, key, value1, value_size1);

	CU_ASSERT_TRUE(result);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value1, value_size1);

	t_memory_position* mem_pos;

	for(int i =3; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	result = redis_set(redis, key, value2, value_size2);

	assert_key_in_position(expected_slot, expected_first, expected_keys, key, value2, value_size2);

	for(int i =2; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}
}

void set_not_atomic_value_twice_new_value_is_bigger_with_contiguous_space_should_remove_key_and_add_elsewhere(){
	char* key1 = "KEY1";
	char* value1 = "VALUEOFTHRE"; // 3 slots

	char* key2 = "KEY2";
	char* value2 = "ATO";

	char* key3 = "KEY3";
	char* value3 = "B";

	char* new_value_1 = "THISTAKESFIVESLOTS"; // 5 slots

	unsigned int value_size1 = strlen(value1) + 1;
	unsigned int value_size2 = strlen(value2) + 1;
	unsigned int value_size3 = strlen(value3) + 1;
	unsigned int new_value_size = strlen(new_value_1) + 1;

	bool result = redis_set(redis, key1, value1, value_size1);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key2, value2, value_size2);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key3, value3, value_size3);
	CU_ASSERT_TRUE(result);

	assert_key_in_position(5, 0, 3, key1, value1, value_size1);
	assert_key_in_position(5, 3, 3, key2, value2, value_size2);
	assert_key_in_position(5, 4, 3, key3, value3, value_size3);

	t_memory_position* mem_pos;

	for(int i =5; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	result = redis_set(redis, key1, new_value_1, new_value_size);
	CU_ASSERT_TRUE(result);

	// positions of first key have been freed
	for(int i =0; i < 3; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	// keys 2 and 3 are left in place. cursor is at 0
	assert_key_in_position(0, 3, 3, key2, value2, value_size2);
	assert_key_in_position(0, 4, 3, key3, value3, value_size3);

	// key 1 is now placed where the cursor was
	assert_key_in_position(0, 5, 3, key1, new_value_1, new_value_size);
}

void set_not_atomic_value_twice_new_value_is_bigger_with_non_contiguous_space_should_remove_key_and_signal_compact(){
	char* key1 = "KEY1";
	char* value1 = "VALUEOFTHRE"; // 3 slots

	char* key2 = "KEY2";
	char* value2 = "ATO";

	char* key3 = "KEY3";
	char* value3 = "B";

	char* new_value_1 = "THISTAKESSIXSLOTS12345"; // 6 slots

	unsigned int value_size1 = strlen(value1) + 1;
	unsigned int value_size2 = strlen(value2) + 1;
	unsigned int value_size3 = strlen(value3) + 1;
	unsigned int new_value_size = strlen(new_value_1) + 1;

	bool result = redis_set(redis, key1, value1, value_size1);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key2, value2, value_size2);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key3, value3, value_size3);
	CU_ASSERT_TRUE(result);

	assert_key_in_position(5, 0, 3, key1, value1, value_size1);
	assert_key_in_position(5, 3, 3, key2, value2, value_size2);
	assert_key_in_position(5, 4, 3, key3, value3, value_size3);

	t_memory_position* mem_pos;

	for(int i=5; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	result = redis_set(redis, key1, new_value_1, new_value_size);
	CU_ASSERT_FALSE(result); // false signals compaction

	// positions of first key have been freed
	for(int i =0; i < 3; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	// keys 2 and 3 are left in place. cursor is at the same place (5)
	// thre are now 2 keys present, since key1 was evicted
	assert_key_in_position(5, 3, 2, key2, value2, value_size2);
	assert_key_in_position(5, 4, 2, key3, value3, value_size3);

	// last 5 positions are empty
	for(int i=5; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	// key1 is no longer present
	char* retrieved = redis_get(redis, key1);
	CU_ASSERT_PTR_NULL(retrieved);
}

void set_not_atomic_value_twice_new_value_is_bigger_without_space_then_contiguous_should_remove_key_replace_and_set_value(){
	char* key1 = "KEY1";
	char* value1 = "VALUEOFTHRE"; // 3 slots

	char* key2 = "KEY2";
	char* value2 = "ATO"; // 1 slot

	char* key3 = "KEY3";
	char* value3 = "B"; // 1 slot

	char* key4 = "KEY4";
	char* value4 = "TWOSLOT"; // 2 slots

	char* key5 = "KEY5";
	char* value5 = "ABC"; // 1 slot

	char* key6 = "KEY6";
	char* value6 = "ABCDEF"; // 2 slot

	char* new_value_1 = "THISTAKESFIVESLOTS"; // 5 slots

	unsigned int value_size1 = strlen(value1) + 1;
	unsigned int value_size2 = strlen(value2) + 1;
	unsigned int value_size3 = strlen(value3) + 1;
	unsigned int value_size4 = strlen(value4) + 1;
	unsigned int value_size5 = strlen(value5) + 1;
	unsigned int value_size6 = strlen(value6) + 1;
	unsigned int new_value_size = strlen(new_value_1) + 1;

	bool result = redis_set(redis, key1, value1, value_size1);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key2, value2, value_size2);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key3, value3, value_size3);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key4, value4, value_size4);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key5, value5, value_size5);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key6, value6, value_size6);
	CU_ASSERT_TRUE(result);

	assert_key_in_position(0, 0, 6, key1, value1, value_size1);
	assert_key_in_position(0, 3, 6, key2, value2, value_size2);
	assert_key_in_position(0, 4, 6, key3, value3, value_size3);
	assert_key_in_position(0, 5, 6, key4, value4, value_size4);
	assert_key_in_position(0, 7, 6, key5, value5, value_size5);
	assert_key_in_position(0, 8, 6, key6, value6, value_size6);

	result = redis_set(redis, key1, new_value_1, new_value_size);
	CU_ASSERT_TRUE(result);

	// keys 2 and 3 are evicted to make room for new value of key1
	// as space is contiguous, key1 is placed there
	assert_key_in_position(5, 0, 4, key1, new_value_1, new_value_size);

	// keys 4, 5 and 6 are left there
	assert_key_in_position(5, 5, 4, key4, value4, value_size4);
	assert_key_in_position(5, 7, 4, key5, value5, value_size5);
	assert_key_in_position(5, 8, 4, key6, value6, value_size6);

	// key2 is no longer present
	char* retrieved = redis_get(redis, key2);
	CU_ASSERT_PTR_NULL(retrieved);

	// key 3 is no longer present
	retrieved = redis_get(redis, key3);
	CU_ASSERT_PTR_NULL(retrieved);
}

void set_not_atomic_value_twice_new_value_is_bigger_without_space_then_non_contiguous_should_remove_key_replace_and_signal_compact() {
	char* key1 = "KEY1";
	char* value1 = "VALUEOFTHRE"; // 3 slots

	char* key2 = "KEY2";
	char* value2 = "ATO"; // 1 slot

	char* key3 = "KEY3";
	char* value3 = "B"; // 1 slot

	char* key4 = "KEY4";
	char* value4 = "TWOSLOT"; // 2 slots

	char* key5 = "KEY5";
	char* value5 = "ABC"; // 1 slot

	char* new_value_1 = "THISTAKESSIXSLOTS12345"; // 6 slots

	unsigned int value_size1 = strlen(value1) + 1;
	unsigned int value_size2 = strlen(value2) + 1;
	unsigned int value_size3 = strlen(value3) + 1;
	unsigned int value_size4 = strlen(value4) + 1;
	unsigned int value_size5 = strlen(value5) + 1;
	unsigned int new_value_size = strlen(new_value_1) + 1;

	bool result = redis_set(redis, key1, value1, value_size1);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key2, value2, value_size2);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key3, value3, value_size3);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key4, value4, value_size4);
	CU_ASSERT_TRUE(result);

	result = redis_set(redis, key5, value5, value_size5);
	CU_ASSERT_TRUE(result);

	assert_key_in_position(8, 0, 5, key1, value1, value_size1);
	assert_key_in_position(8, 3, 5, key2, value2, value_size2);
	assert_key_in_position(8, 4, 5, key3, value3, value_size3);
	assert_key_in_position(8, 5, 5, key4, value4, value_size4);
	assert_key_in_position(8, 7, 5, key5, value5, value_size5);

	t_memory_position* mem_pos;
	for(int i=8; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	result = redis_set(redis, key1, new_value_1, new_value_size);
	CU_ASSERT_FALSE(result); // false signals compaction

	// positions of first key have been freed
	// key2 has been evicted
	for(int i =0; i < 4; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	// keys 3, 4 and 5 are left in place. cursor is at the same place (5)
	// there are now 2 keys present, since key1 was evicted
	assert_key_in_position(8, 4, 3, key3, value3, value_size3);
	assert_key_in_position(8, 5, 3, key4, value4, value_size4);
	assert_key_in_position(8, 7, 3, key5, value5, value_size5);

	// last 2 positions are empty
	for(int i=8; i < NUMBER_OF_ENTRIES; i++){
		mem_pos = redis->occupied_memory_map[i];
		assert_memory_position_empty(mem_pos);
	}

	// key1 is no longer present
	char* retrieved = redis_get(redis, key1);
	CU_ASSERT_PTR_NULL(retrieved);

	// key 2 is no longer present
	retrieved = redis_get(redis, key2);
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
	CU_add_test(redis_test, "test_add_atomic_keys_until_memory_full_should_keep_all_keys", test_add_atomic_keys_until_memory_full_should_keep_all_keys);
	CU_add_test(redis_test, "test_add_atomic_to_redis_full_of_atomic_keys_replace_circular_should_replace_first_key", test_add_atomic_to_redis_full_of_atomic_keys_replace_circular_should_replace_first_key);
	CU_add_test(redis_test, "test_add_non_atomic_to_redis_full_of_atomic_keys_replace_circular_should_replace_first_keys", test_add_non_atomic_to_redis_full_of_atomic_keys_replace_circular_should_replace_first_keys);
	CU_add_test(redis_test, "test_add_non_atomic_to_redis_full_first_key_not_atomic_replace_circular_should_replace_first_atomic_keys", test_add_non_atomic_to_redis_full_first_key_not_atomic_replace_circular_should_replace_first_atomic_keys);
	CU_add_test(redis_test, "set_atomic_value_twice_should_replace_in_same_position", set_atomic_value_twice_should_replace_in_same_position);
	CU_add_test(redis_test, "test_set_non_atomic_with_space_available_not_contiguous_should_signal_compact", test_set_non_atomic_with_space_available_not_contiguous_should_signal_compact);
	CU_add_test(redis_test, "set_not_atomic_value_twice_new_value_same_size_should_replace_in_same_position", set_not_atomic_value_twice_new_value_same_size_should_replace_in_same_position);
	CU_add_test(redis_test, "set_not_atomic_value_twice_new_value_is_atomic_should_replace_in_same_position_and_free_slots", set_not_atomic_value_twice_new_value_is_atomic_should_replace_in_same_position_and_free_slots);
	CU_add_test(redis_test, "set_not_atomic_value_twice_new_value_is_smaller_not_atomic_should_replace_in_same_position_and_free_slots", set_not_atomic_value_twice_new_value_is_smaller_not_atomic_should_replace_in_same_position_and_free_slots);
	CU_add_test(redis_test, "set_not_atomic_value_twice_new_value_is_bigger_with_contiguous_space_should_remove_key_and_add_elsewhere" ,set_not_atomic_value_twice_new_value_is_bigger_with_contiguous_space_should_remove_key_and_add_elsewhere);
	CU_add_test(redis_test, "set_not_atomic_value_twice_new_value_is_bigger_with_non_contiguous_space_should_remove_key_and_signal_compact", set_not_atomic_value_twice_new_value_is_bigger_with_non_contiguous_space_should_remove_key_and_signal_compact);
	CU_add_test(redis_test, "set_not_atomic_value_twice_new_value_is_bigger_without_space_then_contiguous_should_remove_key_replace_and_set_value", set_not_atomic_value_twice_new_value_is_bigger_without_space_then_contiguous_should_remove_key_replace_and_set_value);
	CU_add_test(redis_test, "set_not_atomic_value_twice_new_value_is_bigger_without_space_then_non_contiguous_should_remove_key_replace_and_signal_compact", set_not_atomic_value_twice_new_value_is_bigger_without_space_then_non_contiguous_should_remove_key_replace_and_signal_compact);
}


int run_tests() {
   CU_initialize_registry();

   add_tests();

   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   return CU_get_error();
}

