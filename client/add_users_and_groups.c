#include "database_queries.h"
#define USER 0
#define GROUP 1

int main(int argc, char* argv[]) {
    if(argc!=4)
        return -1;
    int host_index = add_host(argv[2]);
    switch(strtol(argv[1], NULL, 10)) {
    case USER:
        add_user_id(host_index, strtol(argv[3], NULL, 10));
        break;
    case GROUP:
        add_group_id(host_index, strtol(argv[3], NULL, 10));
        break;
    default:
        return -1;
    }
    return 0;
}
