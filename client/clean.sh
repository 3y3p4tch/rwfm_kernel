rm -fr preload.so preload.h database_server database_macros.h enable_rwfm add_users_and_groups /tmp/request_fifo /tmp/response_fifo /lib/secos /opt/secos
unlink /dev/shm/sem.rule_engine_semaphore
unlink /dev/shm/sem.pipe_semaphore
unlink /dev/shm/sem.fork_lock
