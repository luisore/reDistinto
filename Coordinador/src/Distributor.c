/*
 * Distributor.c
 *
 *  Created on: 13 jul. 2018
 *      Author: avinocur
 */

#include "Distributor.h"
#include <stdlib.h>
#include <commons/string.h>

char* distributor_select_instance_lsu(struct Distributor* distributor, char* key, unsigned int value_size);
char* distributor_select_instance_el(struct Distributor* distributor, char* key, unsigned int value_size);
char* distributor_select_instance_ke(struct Distributor* distributor, char* key, unsigned int value_size);

t_distributor* distributor_init(dist_algo_e dist_algo, t_log* log){
	t_distributor* distributor = malloc(sizeof(t_distributor));
	distributor->instances = list_create();
	distributor->log = log;

	switch(dist_algo){
	case LSU:
		log_info(log, "Initialized distributor with algorithm: LSU");
		distributor->select_instance = distributor_select_instance_lsu;
		break;
	case EL:
		log_info(log, "Initialized distributor with algorithm: EL");
		distributor->select_instance = distributor_select_instance_el;
		break;
	case KE:
		log_info(log, "Initialized distributor with algorithm: KE");
		distributor->select_instance = distributor_select_instance_ke;
		break;
	}

	distributor->last_used_instance = -1;

	return distributor;
}

void distributor_add_instance(t_distributor* distributor, char* instance_name, unsigned int space_used){
	t_instance* instance = malloc(sizeof(t_instance));
	instance->name = string_duplicate(instance_name);
	instance->space_used = space_used;
	list_add(distributor->instances, instance);
}

void distributor_remove_instance(t_distributor* distributor, char* instance_name){
	bool find_instance_by_name(t_instance* instance){
		return instance != NULL && string_equals_ignore_case(instance->name, instance_name);
	}

	int index = list_first_occurrence(distributor->instances, find_instance_by_name);

	if(index < 0){
		log_error(distributor->log, "Attempted to remove instance: %s but it was not present.", instance_name);
		return;
	}

	list_remove_and_destroy_element(distributor->instances, index, instance_destroy);

	if(index <= distributor->last_used_instance){
		distributor->last_used_instance--;
	}
	log_info(distributor->log, "Instance: %s removed from distributor list", instance_name);
}

void distributor_update_space_used(t_distributor* distributor, char* instance_name, unsigned int space_used){
	bool find_instance_by_name(t_instance* instance){
		return instance != NULL && string_equals_ignore_case(instance->name, instance_name);
	}

	t_instance* instance = (t_instance*)list_find(distributor->instances, find_instance_by_name);

	if(instance == NULL){
		log_error(distributor->log, "Error updating space used for instance: %s. Instance not found!", instance_name);
	} else {
		log_debug(distributor->log, "Updated space used for instance: %s. Space used: %i", instance_name, space_used);
		instance->space_used = space_used;
	}
}

char* distributor_select_instance(t_distributor* distributor, char* key_to_set, unsigned int value_size){
	// If no instances return NULL.
	if(list_is_empty(distributor->instances)){
		log_error(distributor->log, "Cannot select next instance. Instances is empty!");
		return NULL;
	}

	return distributor->select_instance(distributor, key_to_set, value_size);
}

void distributor_destroy(t_distributor* distributor){
	list_destroy_and_destroy_elements(distributor->instances, instance_destroy);

	free(distributor);
}

void instance_destroy(t_instance* instance){
	if(instance == NULL) return;

	if(instance->name != NULL) free(instance->name);
	free(instance);
}


char* distributor_select_instance_lsu(struct Distributor* distributor, char* key, unsigned int value_size){
	int number_of_instances = list_size(distributor->instances);
	t_instance* current_instance;
	t_instance* selected_instance = NULL;

	int next_eq_instance = distributor->last_used_instance + 1;
	int selected_instance_index;

	for(int instance_cursor = 0; instance_cursor < number_of_instances; instance_cursor++){
		int actual_instance_index = (next_eq_instance + instance_cursor) % number_of_instances;
		current_instance = list_get(distributor->instances, actual_instance_index);

		if(selected_instance == NULL || current_instance->space_used < selected_instance->space_used){
			selected_instance = current_instance;
			selected_instance_index = actual_instance_index;
		}
	}

	// update next instance
	distributor->last_used_instance = selected_instance_index;

	return string_duplicate(selected_instance->name);
}

char* distributor_select_instance_el(struct Distributor* distributor, char* key, unsigned int value_size){
	// If no instances return NULL.
	if(list_is_empty(distributor->instances)){
		log_error(distributor->log, "Cannot select next instance. Instances is empty!");
		return NULL;
	}

	// Retrieve the next instance on the list in a circular fashion
	int number_of_instances = list_size(distributor->instances);
	int next_instance = (distributor->last_used_instance + 1) % number_of_instances;
	t_instance* instance = list_get(distributor->instances, next_instance);

	log_info(distributor->log, "Selected next instance. Algorithm: EL. Number of instances: %i, Last instance: %i. Selected instance on index: %i. Instance: $s",
			number_of_instances, distributor->last_used_instance, next_instance, instance->name);

	// Update the last used instance
	distributor->last_used_instance = next_instance;

	return string_duplicate(instance->name);
}

char* distributor_select_instance_ke(struct Distributor* distributor, char* key, unsigned int value_size){
	int number_of_instances = list_size(distributor->instances);

	int letters_to_split = 26;
	int first_letter = 97; // a is 97, z is 122.
	// must round up the letters by instance. last instance may have less letters.
	int letters_by_instance = letters_to_split / number_of_instances + (letters_to_split % number_of_instances != 0);

	int key_letter = (int)key[0];
	int alligned_key = key_letter - first_letter;
	int instance_index = 0;

	while((instance_index+1)*letters_by_instance <= alligned_key){
		instance_index++;
	}

	t_instance* instance = (t_instance*)list_get(distributor->instances, instance_index);

	log_info(distributor->log, "Selected next instance. Algorithm: KE. Letters by instance: %i. First char: %c. Selected instance number %i of %i: %s.",
			letters_by_instance, key_letter, instance_index, number_of_instances, instance->name);

	distributor->last_used_instance = instance_index;
	return string_duplicate(instance->name);
}
