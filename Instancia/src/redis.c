/*
 * redis.c
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */

#include "redis.h"
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// precondition: the value_size must be less than the memory size.
int redis_replace_circular(struct Redis* redis, unsigned int value_size){
	int slot_cursor = redis->current_slot;
	unsigned int freed_memory = 0;
	t_memory_position* mem_pos;
	t_entry_data* entry_data;
	int entry_slots;

	// try to free memory from the slot till the end.
	// if there is not enough memory from this position until the end, then start from zero.
	if(slot_cursor + value_size - 1 > redis->number_of_entries)
		slot_cursor = 0;

	int first_slot = slot_cursor;

	while(freed_memory < value_size && slot_cursor < redis->number_of_entries){
		mem_pos = redis->occupied_memory_map[slot_cursor];
		entry_data = dictionary_get(redis->key_dictionary, mem_pos->key);
		entry_slots = slots_occupied_by(redis->entry_size, entry_data->size);
		freed_memory += entry_slots * redis->entry_size;
		slot_cursor += entry_slots;
		redis_remove_key(redis, mem_pos->key, entry_data, entry_slots);
	}

	return first_slot;
}

void redis_destroy(t_redis* redis){
	if(redis->memory_region != NULL)
		free(redis->memory_region);

	if(redis->occupied_memory_map != NULL){
		for(int i=0; i<redis->number_of_entries; i++){
			if(redis->occupied_memory_map[i] != NULL) free(redis->occupied_memory_map[i]);
		}
		free(redis->occupied_memory_map);
	}

	dictionary_destroy_and_destroy_elements(redis->key_dictionary, redis_entry_data_destroy);
}

t_memory_position* redis_create_empty_memory_position(){
	t_memory_position* memory_pos = malloc(sizeof(t_memory_position));
	memory_pos->is_atomic = true;
	memory_pos->used = false;
	memory_pos->last_reference = 0;
	memory_pos->key[0] = '\0';

	return memory_pos;
}

t_redis* redis_init(int entry_size, int number_of_entries, t_log* log, char* mount_dir,
		int (*perform_replacement_and_return_first_position)(struct Redis*, unsigned int)){
	t_redis* redis = malloc(sizeof(t_redis));
	redis->entry_size = entry_size;
	redis->number_of_entries = number_of_entries;
	redis->storage_size = entry_size * number_of_entries;

	redis->current_slot = 0;
	redis->memory_region = malloc(redis->storage_size);
	redis->occupied_memory_map = malloc(number_of_entries * sizeof(t_memory_position*));
	for(int i = 0; i < number_of_entries; i++){
		redis->occupied_memory_map[i] = redis_create_empty_memory_position();
	}

	redis->key_dictionary = dictionary_create();
	redis->log = log;

	redis->mount_dir = string_duplicate(mount_dir);
	redis->perform_replacement_and_return_first_position = perform_replacement_and_return_first_position;

	return redis;
}

void redis_entry_data_destroy(t_entry_data* entry_data){
	if(entry_data != NULL)
		free(entry_data);
}

char* redis_get(t_redis* redis, char* key){
	if(!dictionary_has_key(redis->key_dictionary, key)){
		return NULL;
	}

	t_entry_data* entry_data = (t_entry_data*)dictionary_get(redis->key_dictionary, key);
	char* stored_value = malloc(entry_data->size);

	int offset = redis->entry_size * entry_data->first_position;

	log_debug(redis->log, "Key: %s is stored at: %i. Size: %i. Calculated offset: %i",
			key, entry_data->first_position, entry_data->size, offset);

	memcpy(stored_value, redis->memory_region + offset, entry_data->size);

	return stored_value;
}


int slots_occupied_by(int entry_size, int value_size){
	int slots = value_size / entry_size;
	if(value_size % entry_size > 0) slots++;

	return slots;
}

void redis_free_slot(t_redis* redis, int slot_index){
	t_memory_position* mem_pos = redis->occupied_memory_map[slot_index];
	mem_pos->is_atomic = true;
	mem_pos->key[0] = '\0';
	mem_pos->last_reference = 0;
	mem_pos->used = false;
}

void set_in_same_place(t_redis* redis, t_entry_data* entry_data, char* key, char* value,
		int value_size, int needed_slots, int used_slots){

	int offset = entry_data->first_position * redis->entry_size;
	memcpy(redis->memory_region + offset, value, value_size);
	entry_data->size = value_size;

	int slots_to_free = used_slots - needed_slots;

	int slot_index = entry_data->first_position + needed_slots;
	while(slots_to_free > 0){
		redis_free_slot(redis, slot_index);
		slot_index++;
		slots_to_free--;
	}

}

// TODO: Repite codigo con set_in_same_place. Refactor!
void redis_remove_key(t_redis* redis, char* key, t_entry_data* entry_data, int used_slots){
	dictionary_remove_and_destroy(redis->key_dictionary, key, redis_entry_data_destroy);

	int slot_index = entry_data->first_position;
	int slots_to_free = used_slots;

	while(slots_to_free > 0){
		// WARNING: This destroys the key received as parameter!
		redis_free_slot(redis, slot_index);
		slot_index++;
		slots_to_free--;
	}
}

/*
 * Checks the memory slots for available space to fit the required size.
 * Returns True if there is space enough to fit, either contiguous or not.
 * If threre is contiguous space available, first_slot will be set with the first
 * slot of that contiguous space. Otherwise, it will be set to -1.
 */
bool check_if_free_slots_available(t_redis* redis, int required_slots, int* first_slot){
	int total_free_slots = 0;
	int contiguous_free_slots = 0;
	int current_first = -1;
	int cursor;
	t_memory_position* mem_pos;

	for(cursor = 0; cursor < redis->number_of_entries && contiguous_free_slots < required_slots; cursor++){
		mem_pos = redis->occupied_memory_map[cursor];
		if(mem_pos->used){
			current_first = -1;
			contiguous_free_slots = 0;
		} else{
			total_free_slots++;
			if(current_first >= 0){
				contiguous_free_slots++;
			} else {
				current_first = cursor;
				contiguous_free_slots = 1;
			}
		}
	}

	if(contiguous_free_slots == required_slots){
		*first_slot = current_first;
	}

	return total_free_slots >= required_slots;
}

bool redis_set(t_redis* redis, char* key, char* value, unsigned int value_size){
	int need_slots = slots_occupied_by(redis->entry_size, value_size);

	// Check if key is already present
	if(dictionary_has_key(redis->key_dictionary, key)){
		t_entry_data* entry_data = (t_entry_data*)dictionary_get(redis->key_dictionary, key);
		int used_slots = slots_occupied_by(redis->entry_size, entry_data->size);

		if(need_slots <= used_slots){
			// If the new value fits in the previously reserved slots, use those slots
			set_in_same_place(redis, entry_data, key, value, value_size, need_slots, used_slots);
			return true;
		} else{
			// If it does not fit, remove the key, free the slots and check if it fits somewhere else.
			redis_remove_key(redis, key, entry_data, used_slots);
		}
	}

	// At this point the key is either new or it was removed because the new value did not fit.

	// verificar si hay espacio contiguo disponible
	// si hay espacio contiguo disponible, usarlo.
	// si no hay espacio contiguo, pero si espacio, hay que compactar.
	// sino, llamar al algoritmo de reemplazo.

	int first_slot; // This value is filled by the next call to check_if_free_slots_available.
	bool space_available = check_if_free_slots_available(redis, need_slots, &first_slot);

	if(space_available) {
		if(first_slot < 0){
			// Need to compact because there is space available but it is not contiguous
			log_info(redis->log, "There is space available but not contiguous to SET the value with size: %s. Need to compact.",
					value_size);
			return false;
		}
	} else {
		first_slot = redis->perform_replacement_and_return_first_position(redis, value_size);
	}

	// create the new key for the dictionary
	t_entry_data* entry_data = malloc(sizeof(t_entry_data));
	entry_data->first_position = first_slot;
	entry_data->size = value_size;

	// mark the slots as used by this key
	t_memory_position* mem_pos;
	bool isAtomic = need_slots == 1;

	for(int pos = first_slot; pos < first_slot + need_slots; pos++){
		mem_pos = redis->occupied_memory_map[pos];
		mem_pos->is_atomic = isAtomic;
		strcpy(mem_pos->key, key);
		mem_pos->last_reference = 0; // TODO: FALTA SETEAR UNA VARIABLE CON EL NRO DE ITERACION
		mem_pos->used = true;
	}

	// memcpy the new value
	int offset = first_slot * redis->entry_size;
	memcpy(redis->memory_region + offset, value, value_size);

	// save the new key in the dictionaty
	dictionary_put(redis->key_dictionary, key, entry_data);


	// Set the current_slot to the next position after the entry that was just inserted.
	// if the size is exceeded, it returns to zero.
	// NOTE: a value is always stored in contiguous memory. That means that it will never be
	// split between the end of the memory region and the beginning of it. The first_position
	// returned by the replacement algorithm must ensure that there is enough space after that
	// position to store the value.
	redis->current_slot = (first_slot + need_slots) % redis->number_of_entries;

	return true;
}

int redis_store(t_redis* redis, char* key){
	return 0; // TODO! IMPLEMENTAR
}


void redis_compact(t_redis* redis){
	// TODO! IMPLEMENTAR
}

void redis_load_dump_files(t_redis* redis){
	// TODO! IMPLEMENTAR
}

