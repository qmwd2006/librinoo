/**
 * @file   rinoo_socket_inotify.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2014
 * @date   Wed Feb  1 18:56:27 2017
 *
 * @brief  Test file for rinoo inotify.
 *
 *
 */

#include "rinoo/rinoo.h"

#define TEST_DIRECTORY	"/tmp/.inotify.test/"
#define NB_EVENT	20

static int nb_create = 0;
static int nb_create_event = 0;
static int nb_rm = 0;
static int nb_rm_event = 0;

void event_generator(void *sched)
{
	int i;
	int fd;
	char path[100];

	rn_task_wait(sched, 200);
	for (i = 0; i < NB_EVENT / 2; i++) {
		snprintf(path, sizeof(path), TEST_DIRECTORY ".inotify.XXXXXX");
		fd = mkstemp(path);
		close(fd);
		rn_log("Event generator: file created.");
		nb_create++;
		rn_task_wait(sched, 200);
		unlink(path);
		nb_rm++;
		rn_log("Event generator: file removed.");
		rn_task_wait(sched, 200);
	}
}

void check_file(void *sched)
{
	int i;
	rn_inotify_t *inotify;
	rn_inotify_event_t *event;

	inotify = rn_inotify(sched);
	rn_inotify_add_watch(inotify, "/tmp", INOTIFY_CREATE | INOTIFY_DELETE, true);
	for (i = 0; i < NB_EVENT && (event = rn_inotify_event(inotify)) != NULL; i++) {
		if (event->type & INOTIFY_CREATE) {
			rn_log("File created.");
			nb_create_event++;
		} else if (event->type & INOTIFY_DELETE) {
			rn_log("File deleted.");
			nb_rm_event++;
		}
	}
	XTEST(nb_create_event == nb_create);
	XTEST(nb_rm_event == nb_rm);
	rn_inotify_destroy(inotify);
}

/**
 * Main function for this unit test.
 *
 *
 * @return 0 if test passed
 */
int main()
{
	rn_sched_t *sched;

	mkdir(TEST_DIRECTORY, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	sched = rn_scheduler();
	XTEST(sched != NULL);
	XTEST(rn_task_start(sched, check_file, sched) == 0);
	XTEST(rn_task_start(sched, event_generator, sched) == 0);
	rn_scheduler_loop(sched);
	rn_scheduler_destroy(sched);
	rmdir(TEST_DIRECTORY);
	XPASS();
}
