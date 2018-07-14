/*
 * InstanceCoordinator.h
 *
 *  Created on: 13 jul. 2018
 *      Author: avinocur
 */

#ifndef SRC_DISTRIBUTOR_H_
#define SRC_DISTRIBUTOR_H_

#include "libs/list.h"
#include <commons/log.h>

typedef enum {
	LSU = 1, EL = 2, KE = 3
} dist_algo_e;

typedef struct {
	unsigned int space_used;
	char* name;
} t_instance;

typedef struct Distributor {
	t_log* log;
	t_list* instances;
	unsigned int last_used_instance;

	char* (*select_instance)(struct Distributor*, char*, unsigned int);
} t_distributor;


t_distributor* distributor_init(dist_algo_e dist_algo, t_log* log);
void distributor_add_instance(t_distributor* distributor, char* instance_name, unsigned int space_used);
void distributor_remove_instance(t_distributor* distributor, char* instance_name);
void distributor_update_space_used(t_distributor* distributor, char* instance_name, unsigned int space_used);

char* distributor_select_instance(t_distributor* distributor, char* key_to_set, unsigned int value_size);

void distributor_destroy(t_distributor* distributor);

void instance_destroy(t_instance* instance);

#endif /* SRC_DISTRIBUTOR_H_ */
