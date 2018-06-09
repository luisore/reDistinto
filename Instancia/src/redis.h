/*
 * redis.h
 *
 *  Created on: 8 jun. 2018
 *      Author: avinocur
 */

#ifndef SRC_REDIS_H_
#define SRC_REDIS_H_

#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <stdbool.h>

typedef struct {
	unsigned int size;
	int first_position;
} t_entry_data;

typedef struct {
	bool used;
	bool is_atomic;
	char key[40]; // the key that occupies this position
	int last_reference;
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

	// Configuracion recibida del Coordinador
	int storage_size;
	int entry_size;
	int number_of_entries;

	/*
	 * Algoritmo actual de reemplazo de claves
	 * Recorre la memoria y libera el espacio para colocar la clave del
	 * tamanio suministrado.
	 * Retorna la primera posicion donde ubicar el nuevo valor
	 */
	int (*perform_replacement_and_return_first_position)(struct Redis*, unsigned int);
	int current_slot;

	t_log* log;
	char* mount_dir;
} t_redis;

t_redis* redis_init(int entry_size, int number_of_entries, t_log* log, const char* mount_dir,
		int (*perform_replacement_and_return_first_position)(struct Redis*, unsigned int));


// NOTE: redis_destroy will not free the log. If you created a specific log for redis,
// you should free it yourself.
void redis_destroy(t_redis* redis);

bool redis_set(t_redis* redis, char* key, char* value, unsigned int value_size);
char* redis_get(t_redis* redis, char* key);

/*
 * Store the value in the file system.
 * Result can be:
 *  0: Store was successful.
 *  1: The key was invalid.
 * -1: The operation failed with an unexpected error.
 */
int redis_store(t_redis* redis, char* key);
void redis_compact(t_redis* redis);
void redis_dump(t_redis* redis);
void redis_load_dump_files(t_redis* redis);

// Algoritmos de reemplazo
int redis_replace_circular(struct Redis* redis, unsigned int value_size);

void redis_entry_data_destroy(t_entry_data* entry_data);

t_memory_position* redis_create_empty_memory_position();

int slots_occupied_by(int entry_size, int value_size);

void redis_remove_key(t_redis* redis, char* key, t_entry_data* entry_data, int used_slots);


#endif /* SRC_REDIS_H_ */
