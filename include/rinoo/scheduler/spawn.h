/**
 * @file   spawn.h
 * @author reginaldl <reginald.l@gmail.com>
 * @date   Tue Sep 22 16:58:02 2013
 *
 * @brief  RiNOO scheduler spawning functions
 *
 *
 */

#ifndef RINOO_SCHEDULER_SPAWN_H_
#define RINOO_SCHEDULER_SPAWN_H_

/* Defined in scheduler.h */
struct s_rinoosched;

typedef struct s_rinoosched_spawns {
	int count;
	pthread_t *thread;
	struct s_rinoosched **sched;
} t_rinoosched_spawns;

int rinoo_spawn(struct s_rinoosched *sched, int count);
void rinoo_spawn_destroy(struct s_rinoosched *sched);
struct s_rinoosched *rinoo_spawn_get(struct s_rinoosched *sched, int id);
int rinoo_spawn_start(struct s_rinoosched *sched);
void rinoo_spawn_stop(struct s_rinoosched *sched);
void rinoo_spawn_join(struct s_rinoosched *sched);

#endif /* !RINOO_SCHEDULER_SPAWN_H_ */
