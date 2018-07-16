/*
 * redis.h
 *
 *  Created on: 8 jun. 2018
 *      Author: avinocur
 */

#ifndef SRC_REDIS_H_
#define SRC_REDIS_H_

#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdbool.h>

typedef enum {
	CIRC = 1, LRU = 2, BSU = 3
} replacement_algo_e;

typedef struct {
	unsigned int size;
	int first_position;
	FILE* mapped_file;
	char* mapped_value;
	unsigned long last_reference;
	bool is_atomic;
	struct Redis* redis; // a reference to the redis instance
} t_entry_data;

typedef struct {
	bool used;
	char key[40]; // the key that occupies this position
} t_memory_position;

typedef struct Redis {
	// Porcion de memoria asignada a las entradas
	void* memory_region;

	// Mapa de bools indicando si la porcion de memoria esta disponible o no
	t_memory_position** occupied_memory_map;

	// Diccionario de claves:
	// key: Clave de la entrada
	// value: t_entry_data
	t_dictionary* key_dictionary;

	// list of atomic keys
	t_list* atomic_entries;

	// Configuracion recibida del Coordinador
	int storage_size;
	int entry_size;
	int number_of_entries;

	/*
	 * Comparador de t_entry_data.
	 * Asignado segun el algoritmo de reemplazo
	 */
	bool (*entry_data_comparator)(void*, void*);

	int current_slot;

	// stores the amount of free slots available at any given moment.
	unsigned int slots_available;

	t_log* log;
	char* mount_dir;

	// Incremental counter of operations. To use with replacement algorithms
	unsigned long op_counter;
} t_redis;

typedef struct {
	char* value;
	unsigned int size;
} t_redis_value;

t_redis* redis_init(int entry_size, int number_of_entries, t_log* log,
		const char* mount_dir, replacement_algo_e replacement_algo);


// NOTE: redis_destroy will not free the log. If you created a specific log for redis,
// you should free it yourself.
void redis_destroy(t_redis* redis);

/*
 * PRE: Por aclaracion del enunciado, asumimos que el valor provisto siempre entrara
 * en memoria, ya sea inmediatamente o luego de una compactacion. Si se proporciona
 * un valor que no entre en memoria, el resultado es indefinido y el sistema podria
 * no funcionar correctamente.
 */
bool redis_set(t_redis* redis, char* key, char* value, unsigned int value_size);
t_redis_value* redis_get(t_redis* redis, char* key);

/*
 * Store the value in the file system.
 * Result can be:
 *  0: Store was successful.
 *  1: The key was invalid.
 * -1: The operation failed with an unexpected error.
 */
int redis_store(t_redis* redis, char* key);
void redis_compact(t_redis* redis);
bool redis_dump(t_redis* redis);
bool redis_load_dump_files(t_redis* redis);

void redis_replace_necessary_positions(struct Redis* redis, unsigned int value_size);

// Ordering of the entry_data. Defines the replacement algorithm.
bool redis_entry_data_comparator_circular(void* entry1, void* entry2);
bool redis_entry_data_comparator_lru(void* entry1, void* entry2);
bool redis_entry_data_comparator_bsu(void* entry1, void* entry2);

void redis_entry_data_destroy(t_entry_data* entry_data);
void redis_value_destroy(t_redis_value* redis_value);

t_memory_position* redis_create_empty_memory_position();

int slots_occupied_by(int entry_size, int value_size);

void redis_remove_key(t_redis* redis, char* key, t_entry_data* entry_data, int used_slots);

void redis_print_status(t_redis* redis);

bool is_memory_mapped(t_entry_data* entry);

void redis_update_last_reference(t_redis* redis, char* key);

#endif /* SRC_REDIS_H_ */
