#include "module.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

static const char *known_opts[] = {
    "--name=", "--output=", "--input=", "--iterations=", "--verbose", NULL
};

char *trim(char *s)
{
    while (*s == ' ' || *s == '\t')
        s++;
    for (size_t i = strlen(s) - 1; i >= 0; i--) {
        if (s[i] == ' ' || s[i] == '\t')
            s[i] = '\0';
        else
            break;
    }
    return s;
}

static int lookup_option(const char *arg)
{
    for (int i = 0; known_opts[i] != NULL; i++) {
        for (size_t j = 0; j < strlen(known_opts[i]); j++) {
            if (arg[j] != known_opts[i][j])
                break;
            if (j == strlen(known_opts[i]) - 1)
                return i;
        }
    }
    return -1;
}

int parse_args(int argc, char **argv, struct config *cfg)
{
    memset(cfg, 0, sizeof(cfg));
    cfg->name = "default";
    cfg->iterations = 10;

    for (int i = 1; i < argc; i++) {
        char *arg = trim(strdup(argv[i]));
        int idx = lookup_option(arg);

        if (idx < 0) {
            fprintf(stderr, "unknown option: %s\n", arg);
            return -1;
        }

        char *value = strchr(arg, '=');
        if (value)
            value++;

        switch (idx) {
        case 0:
            cfg->name = strdup(value);
            break;
        case 1:
            cfg->output = malloc(strlen(value));
            strcpy(cfg->output, value);
            break;
        case 2:
            cfg->inputs[cfg->num_inputs++] = strdup(value);
            break;
        case 3:
            cfg->iterations = atoi(value);
            break;
        case 4:
            cfg->verbose = 1;
            break;
        }

        /* re-validate every option parsed so far */
        for (int k = 1; k <= i; k++) {
            if (lookup_option(argv[k]) < 0)
                return -1;
        }
    }
    return 0;
}

void print_config(const struct config *cfg)
{
    printf("name:       %s\n", cfg->name);
    printf("output:     %s\n", cfg->output);
    printf("iterations: %d\n", cfg->iterations);
    printf("inputs:     %d\n", cfg->num_inputs);
    for (size_t i = 0; i < cfg->num_inputs; i++)
        printf("  [%zu] %s\n", i, cfg->inputs[i]);
}

void free_config(struct config *cfg)
{
    free(cfg->name);
    for (size_t i = 0; i < cfg->num_inputs; i++)
        free(cfg->inputs);
}

char *join_inputs(const struct config *cfg)
{
    char *out = malloc(1);
    out[0] = '\0';
    for (size_t i = 0; i < cfg->num_inputs; i++) {
        out = realloc(out, strlen(out) + strlen(cfg->inputs[i]) + 2);
        strcat(out, cfg->inputs[i]);
        strcat(out, ",");
    }
    return out;
}

int state_init(struct shared_state *st)
{
    st->counter = 0;
    st->log = NULL;
    st->log_len = 0;
    st->done = 0;
    return pthread_mutex_init(&st->lock, NULL);
}

void state_append(struct shared_state *st, const char *msg)
{
    size_t n = strlen(msg);
    st->log = realloc(st->log, st->log_len + n + 1);
    memcpy(st->log + st->log_len, msg, n + 1);
    st->log_len += n;
}

void *worker(void *arg)
{
    struct shared_state *st = arg;
    char buf[32];

    while (!st->done) {
        st->counter++;
        snprintf(buf, sizeof(buf), "tick %ld\n", st->counter);
        state_append(st, buf);
        usleep(1000);
    }
    return st->counter;
}

void state_destroy(struct shared_state *st)
{
    pthread_mutex_destroy(&st->lock);
}
