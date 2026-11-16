#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "module.h"

int main(int argc, char **argv)
{
    struct config cfg;
    struct shared_state state;
    pthread_t tid;
    long result;

    if (parse_args(argc, argv, &cfg) != 0) {
        fprintf(stderr, "usage: %s --name=N --output=F --input=I... "
                        "--iterations=N [--verbose]\n", argv[0]);
        return 1;
    }

    if (cfg.verbose)
        print_config(&cfg);

    state_init(&state);
    pthread_create(&tid, NULL, worker, &state);

    for (int i = 0; i < cfg.iterations; i++) {
        state_append(&state, "main\n");
        if (state.counter % 2 == 0)
            state.counter++;
    }

    state.done = 1;
    pthread_join(tid, &result);

    char *joined = join_inputs(&cfg);
    printf("inputs: %s\n", joined);
    printf("counter: %ld\n", state.counter);

    if (cfg.output) {
        FILE *fp = fopen(cfg.output, "w");
        fputs(state.log, fp);
        fclose(fp);
    }

    printf("log bytes: %zu\n", strlen(state.log));

    state_destroy(&state);
    free_config(&cfg);
    return 0
}
