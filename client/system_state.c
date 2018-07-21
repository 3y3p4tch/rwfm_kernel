#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "struct_headers.h"

HOST * hosts = NULL;
USER_ID * user_ids = NULL;
GROUP_ID * group_ids = NULL;
OBJECT_ID * object_ids = NULL;
SUBJECT_ID * subject_ids = NULL;
OBJECT * objects = NULL;
SUBJECT * subjects = NULL;
FD_MAP * fd_map = NULL;

void init_hosts() {
    HOST current_host = (HOST) malloc (1024 * sizeof char);
    gethostname(current_host, sizeof current_host);
    hosts = &current_host;
}

void init_user_ids() {
    FILE * fp = fopen("/etc/passwd", "r");
    char * line = NULL;
    char delim[] = ":";
    size_t len = 0;
    ssize_t read;

    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        strtok(line, delim);
        strtok(NULL, delim);
        int uid = atoi(strtok(NULL, delim));
        
    }

    fclose(fp);
    free(line);    
}

int main() {
    init_hosts();
    init_user_ids();
    return 0;
}
