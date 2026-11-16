#ifndef MODULE_H
#define MODULE_H

#include <pthread.h>

#define MAX_INPUTS 16

struct config {
    char  *name;
    char  *output;
    char  *inputs[MAX_INPUTS];
    size_t num_inputs;
    int    iterations;
    int    verbose;
};

struct shared_state {
    pthread_mutex_t lock;
    long            counter;
    char           *log;
    size_t          log_len;
    int             done;
};

int   parse_args(int argc, char **argv, struct config *cfg);
void  print_config(const struct config *cfg);
void  free_config(struct config *cfg);

int   state_init(struct shared_state *st);
void  state_append(struct shared_state *st, const char *msg);
void  state_destroy(struct shared_state *st);
void *worker(void *arg);

char *join_inputs(const struct config *cfg);

#endif
