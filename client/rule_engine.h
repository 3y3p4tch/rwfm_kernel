/*
This is the RWFM rule engine. The preload library calls the functions mentioned here to check for the rwfm rules. The rwfm rule engine in turn communnicates with 
the database_server using the database_queries.c to infer the labels of all the subjects(users,groups) and the objects. Then it checks whether the rwfm rule is satisfied 
for that particular operation using the functioned mentioned in 'user_set_manipulation_functions'. If the operation is allowed it sends true else false to preload.. 
*/

#ifndef _RULE_ENGINE_H_
#define _RULE_ENGINE_H_

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include "infer_object_labels.h"

//For ensuring mutual exclusion of database server access
void lock() {
    mode_t prev_mask = umask(0);
    sem_t * sem_id = sem_open(RULE_ENGINE_SEMAPHORE, O_CREAT, 0666, 1);
    umask(prev_mask);
    sem_wait(sem_id);
}

void unlock() {
    mode_t prev_mask = umask(0);
    sem_t * sem_id = sem_open(RULE_ENGINE_SEMAPHORE, O_CREAT, 0666, 1);
    umask(prev_mask);
    sem_post(sem_id);
}

void fork_check(char* host_name, int uid, int child_pid, int parent_pid) {
	lock();
    int host_id_index = get_host_index(host_name);
    int parent_sub_id_index = get_subject_id_index(host_id_index, uid, parent_pid);
    int child_sub_id_index = add_subject_id(host_id_index, uid, child_pid);
    if(parent_sub_id_index == -1) {
        int owner = get_user_id_index(host_id_index, uid);
        USER_SET readers = get_all_users(get_number_of_users());
        USER_SET writers = 0;
        add_user_to_label(owner, &writers);
        add_subject(child_sub_id_index, owner, readers, writers);
    }
    else {
        SUBJECT parent_subject = get_subject(parent_sub_id_index);
        
        int child_subject = add_subject(child_sub_id_index, parent_subject.owner, parent_subject.readers, parent_subject.writers);
        copy_subject_info(parent_sub_id_index, child_sub_id_index);
    }
	unlock();
}

int file_open_check(char * host_name, struct stat * file_info, int fd, int uid, int pid){
	lock();
    int host_id_index = get_host_index(host_name);
	int subject_id_index = get_subject_id_index(host_id_index, uid, pid);
    int object_id_index = get_object_id_index(host_id_index, file_info->st_dev, file_info->st_ino);
    
	if(object_id_index == -1) {
        object_id_index = add_object_id(host_id_index, file_info->st_dev, file_info->st_ino);
   
        OBJECT object;
        object.obj_id_index = object_id_index;
        object.owner = get_user_id_index(host_id_index, file_info->st_uid);
        infer_labels(&object, file_info, host_id_index);
        
        int res_add_obj = add_object(object_id_index, object.owner, object.readers, object.writers);
    }
	unlock();

    return 1;
}

int file_read_check(char * host_name, int uid, int pid, int fd) {
	lock();
	struct stat file_info;
	fstat(fd, &file_info);
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
    int obj_id_index = get_object_id_index(host_id_index, file_info.st_dev, file_info.st_ino);
	int ret_val = 0;

    //If fd is not a file fd
    if(obj_id_index == -1)
        ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);
		OBJECT object = get_object(obj_id_index);
	
		if(is_user_in_set(subject.owner, &object.readers) == 1) {
		    subject.readers = set_intersection(&subject.readers, &object.readers);
		    subject.writers = set_union(&subject.writers, &object.writers);
		    update_subject_label(sub_id_index, subject.readers, subject.writers);

		    ret_val = 1;
		}
	}
	unlock();

    return ret_val;
}

int file_write_check(char * host_name, int uid, int pid, int fd) {
	lock();
	struct stat file_info;
	fstat(fd, &file_info);
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
    int obj_id_index = get_object_id_index(host_id_index, file_info.st_dev, file_info.st_ino);
	int ret_val = 0;

    //If fd is not a file fd
    if(obj_id_index == -1)
        ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);
		OBJECT object = get_object(obj_id_index);

		if(is_user_in_set(subject.owner, &object.writers)
		    && is_superset_of(&subject.readers, &object.readers)
		    && is_subset_of(&subject.writers, &object.writers))
		    ret_val = 1;
	}
	unlock();

    return ret_val;
}

int connect_check(char * host_name, int uid, int pid, int sock_fd, struct sockaddr_in *peer_addr) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int ret_val = 1;
	if(sub_id_index == -1)
		ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);

		struct sockaddr_in socket_addr;
		socklen_t socket_addr_sz;
		socket_addr_sz = sizeof socket_addr;
		underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);

		add_connection(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr->sin_addr.s_addr, peer_addr->sin_port, 1, sub_id_index, subject.readers, subject.writers);
	}
	unlock();

	return ret_val;
}

int accept_check(char * host_name, int uid, int pid, int sock_fd) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int ret_val = 1;
	if(sub_id_index == -1)
		ret_val = -1;
	else {
		SUBJECT subject = get_subject(sub_id_index);

		struct sockaddr_in socket_addr, peer_addr;
		socklen_t socket_addr_sz, peer_addr_sz;
		socket_addr_sz = sizeof socket_addr;
		peer_addr_sz = sizeof peer_addr;

		underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
		underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);

		add_connection(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr.sin_addr.s_addr, peer_addr.sin_port, 1, sub_id_index, subject.readers, subject.writers);
	}
	unlock();

	return ret_val;
}

int send_check(char * host_name, int uid, int pid, int sock_fd) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int ret_val = 1;
	if(sub_id_index == -1)
		ret_val = -1;
	else {
		struct sockaddr_in socket_addr, peer_addr;
		socklen_t socket_addr_sz, peer_addr_sz;
		socket_addr_sz = sizeof socket_addr;
		peer_addr_sz = sizeof peer_addr;

		underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
		underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);

		int connection_index = get_connection_index(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr.sin_addr.s_addr, peer_addr.sin_port);
		if(connection_index == -1)
			ret_val = -1;
		else {
			SOCKET_CONNECTION_OBJECT conn_obj = get_connection(connection_index);
			SUBJECT subject = get_subject(sub_id_index);

			if(conn_obj.readers != subject.readers || conn_obj.writers != subject.writers)
				update_connection_label(connection_index, set_intersection(&subject.readers, &conn_obj.readers), set_union(&subject.writers, &conn_obj.writers));
		}
	}
	unlock();
	return ret_val;
}

int recv_check(char * host_name, int uid, int pid, int sock_fd) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int ret_val = 0;
	if(sub_id_index == -1)
		ret_val = -1;
	else {
		struct sockaddr_in socket_addr, peer_addr;
		socklen_t socket_addr_sz, peer_addr_sz;
		socket_addr_sz = sizeof socket_addr;
		peer_addr_sz = sizeof peer_addr;

		underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
		underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);

		int connection_index = get_connection_index(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr.sin_addr.s_addr, peer_addr.sin_port);
		if(connection_index == -1)
			ret_val = -1;
		else {
			SOCKET_CONNECTION_OBJECT conn_obj = get_connection(connection_index);
			SUBJECT subject = get_subject(sub_id_index);

			if(is_user_in_set(subject.owner, &conn_obj.readers)) {
				if(conn_obj.readers != subject.readers || conn_obj.writers != subject.writers)
					update_subject_label(sub_id_index, set_intersection(&subject.readers, &conn_obj.readers), set_union(&subject.writers, &conn_obj.writers));

				ret_val = 1;
			}
		}
	}
	unlock();
	return ret_val;
}

int socket_close_check(char * host_name, int uid, int pid, int sock_fd) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);

    struct sockaddr_in socket_addr, peer_addr;
	socklen_t socket_addr_sz, peer_addr_sz;
	socket_addr_sz = sizeof socket_addr;
	peer_addr_sz = sizeof peer_addr;

	underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
	underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);

    int ret_val = remove_peer_from_connection(socket_addr.sin_addr.s_addr, socket_addr.sin_port, peer_addr.sin_addr.s_addr, peer_addr.sin_port, sub_id_index);
	unlock();
	return ret_val;
}

int open_fifo_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int pipe_index = get_pipe_index(host_id_index, pipe_info->st_dev, pipe_info->st_ino);
	if(pipe_index == -1) {
		pipe_index = add_new_pipe(host_id_index, pipe_info->st_dev, pipe_info->st_ino, 1, get_all_users(get_number_of_users()), 0);
		add_new_pipe_mapping(sub_id_index, pipe_index, 1);
	} else {
		increase_pipe_ref_count(pipe_index);
		if(increment_pipe_mapping_ref_count(sub_id_index, pipe_index) == -1)
			add_new_pipe_mapping(sub_id_index, pipe_index, 1);
	}
	unlock();

	return 1;
}

int create_pipe_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

	int pipe_index = add_new_pipe(host_id_index, pipe_info->st_dev, pipe_info->st_ino, 2, subject.readers, subject.writers);
	add_new_pipe_mapping(sub_id_index, pipe_index, 2);
	unlock();

	return 1;
}

int pipe_read_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();
	int ret_val = 0;

	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

	int pipe_index = get_pipe_index(host_id_index, pipe_info->st_dev, pipe_info->st_ino);
	PIPE_OBJECT pipe_obj = get_pipe(pipe_index);

	if(is_user_in_set(subject.owner, &pipe_obj.readers)) {
		if(pipe_obj.readers != subject.readers || pipe_obj.writers != subject.writers)
			update_subject_label(sub_id_index, set_intersection(&subject.readers, &pipe_obj.readers), set_union(&subject.writers, &pipe_obj.writers));

		ret_val = 1;
	}
	unlock();

	return ret_val;
}

int pipe_write_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();
	int ret_val = 1;

	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	SUBJECT subject = get_subject(sub_id_index);

	int pipe_index = get_pipe_index(host_id_index, pipe_info->st_dev, pipe_info->st_ino);
	PIPE_OBJECT pipe_obj = get_pipe(pipe_index);

	if(pipe_obj.readers != subject.readers || pipe_obj.writers != subject.writers)
		update_pipe_label(pipe_index, set_intersection(&subject.readers, &pipe_obj.readers), set_union(&subject.writers, &pipe_obj.writers));

	unlock();

	return ret_val;
}

int pipe_close_check(char * host_name, int uid, int pid, struct stat *pipe_info) {
	lock();

	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	int pipe_index = get_pipe_index(host_id_index, pipe_info->st_dev, pipe_info->st_ino);

	remove_pipe(host_id_index, pipe_info->st_dev, pipe_info->st_ino);
	decrement_pipe_mapping_ref_count(sub_id_index, pipe_index);

	unlock();

	return 1;
}

#endif
