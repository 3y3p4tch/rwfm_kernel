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

void fork_check(char* host_name, int uid, int child_pid, int parent_pid) {
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
}

int open_check(char * host_name, struct stat * file_info, int fd, int uid, int pid){
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

	add_fd_mapping(subject_id_index, object_id_index, fd);

    return 1;
}

int file_read_check(char * host_name, int uid, int pid, int fd) {
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
    int obj_id_index = get_obj_id_index_from_fd_map(sub_id_index, fd);

    //If fd is not a file fd
    if(obj_id_index == -1)
        return -1;

    SUBJECT subject = get_subject(sub_id_index);
    OBJECT object = get_object(obj_id_index);
	
	printf("\nIn file_read_check: %d\t%llx\n\n", subject.owner, object.readers);
    if(is_user_in_set(subject.owner, &object.readers) == 1) {
        subject.readers = set_intersection(&subject.readers, &object.readers);
        subject.writers = set_union(&subject.writers, &object.writers);
        update_subject_label(sub_id_index, subject.readers, subject.writers);

        return 1;
    }

    return 0;
}

int file_write_check(char * host_name, int uid, int pid, int fd) {
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
    int obj_id_index = get_obj_id_index_from_fd_map(sub_id_index, fd);

    //If fd is not a file fd
    if(obj_id_index == -1)
        return -1;

    SUBJECT subject = get_subject(sub_id_index);
    OBJECT object = get_object(obj_id_index);

    if(is_user_in_set(subject.owner, &object.writers)
        && is_superset_of(&subject.readers, &object.readers)
        && is_subset_of(&subject.writers, &object.writers))
        return 1;

    return 0;
}

int file_close_check(char * host_name, int uid, int pid, int fd) {
    int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);

    return remove_fd_mapping(sub_id_index, fd);
}

int socket_check(char * host_name, int uid, int pid, int sock_fd) {
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	if(sub_id_index == -1)
		return -1;
	SUBJECT subject = get_subject(sub_id_index);
	add_socket(sub_id_index, sock_fd, 0, 0, subject.owner, subject.readers, subject.writers);

	return 1;
}

int bind_check(char * host_name, int uid, int pid, int sock_fd) {
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	if(sub_id_index == -1)
		return -1;

	struct sockaddr_in socket_addr;
	socklen_t socket_addr_sz;
	socket_addr_sz = sizeof socket_addr;
	underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);

	int sock_index = get_socket_index_from_sub_id_sock_fd(sub_id_index, sock_fd);
	if(sock_index == -1)
		return -1;
	update_socket_ip_port(sock_index, socket_addr.sin_addr.s_addr, socket_addr.sin_port);

	return 1;
}

int connect_check(char * host_name, int uid, int pid, int sock_fd) {
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	if(sub_id_index == -1)
		return -1;
	int sock_index = get_socket_index_from_sub_id_sock_fd(sub_id_index, sock_fd);
	if(sock_index == -1)
		return -1;
	add_new_connection_map(sock_index);

	return 1;
}

int accept_check(char * host_name, int uid, int pid, int sock_fd) {
	int host_id_index = get_host_index(host_name);

	printf("\n\nHOST ID IND: %d\n\n", host_id_index);

    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);

	printf("\n\nSubject ID IND: %d\n\n", sub_id_index);

	if(sub_id_index == -1)
		return -1;
	SUBJECT subject = get_subject(sub_id_index);
	printf("\n\nSubject FOUND\n\n");

	struct sockaddr_in socket_addr, peer_addr;
	socklen_t socket_addr_sz, peer_addr_sz;
	socket_addr_sz = sizeof socket_addr;
	peer_addr_sz = sizeof peer_addr;
	underlying_getsockname(sock_fd, (struct sockaddr *) &socket_addr, &socket_addr_sz);
	printf("\n\nCURSOCK FOUND\n\n");
	underlying_getpeername(sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_sz);
	printf("\n\nPEERSOCK FOUND\n\n");

	int sock_index = add_socket(sub_id_index, sock_fd, socket_addr.sin_addr.s_addr, socket_addr.sin_port, subject.owner, subject.readers, subject.writers);
	printf("\n\nSOCK ADDED\n\n");
	int peer_sock_index = get_socket_index_from_ip_port(peer_addr.sin_addr.s_addr, peer_addr.sin_port);

	printf("\n\nPEER SOCK IND: %d\n\n", peer_sock_index);

	update_connection_map_sock_index_2(peer_sock_index, sock_index);
	printf("\n\nCONN MAP UPDATED\n\n");

	if(sock_index == -1)
		return -1;

	return 1;
}

int send_check(char * host_name, int uid, int pid, int sock_fd) {
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	if(sub_id_index == -1)
		return -1;

	printf("\n\nSubject ID Found\n\n");

	int sock_index = get_socket_index_from_sub_id_sock_fd(sub_id_index, sock_fd);
	if(sock_index == -1)
		return -1;

	printf("\n\nCur Sock ID Found\n\n");

	int peer_sock_index = get_peer_socket_index(sock_index);
	if(peer_sock_index == -1)
		return -1;

	printf("\n\nPeer Sock ID Found\n\n");

	SOCKET_OBJECT sock_obj = get_socket(sock_index);
	SOCKET_OBJECT peer_sock_obj = get_socket(peer_sock_index);
	SUBJECT subject = get_subject(sub_id_index);

	sock_obj.readers = set_intersection(&subject.readers, &sock_obj.readers);
    sock_obj.writers = set_union(&subject.writers, &sock_obj.writers);
    update_socket_label(sock_index, sock_obj.readers, sock_obj.writers);

	if(is_user_in_set(sock_obj.owner, &peer_sock_obj.writers)
        && is_superset_of(&sock_obj.readers, &peer_sock_obj.readers)
        && is_subset_of(&sock_obj.writers, &peer_sock_obj.writers))
        return 1;

	return 0;
}

int recv_check(char * host_name, int uid, int pid, int sock_fd) {
	int host_id_index = get_host_index(host_name);
    int sub_id_index = get_subject_id_index(host_id_index, uid, pid);
	if(sub_id_index == -1)
		return -1;
	int sock_index = get_socket_index_from_sub_id_sock_fd(sub_id_index, sock_fd);
	if(sock_index == -1)
		return -1;
	int peer_sock_index = get_peer_socket_index(sock_index);
	if(peer_sock_index == -1)
		return -1;

	SOCKET_OBJECT sock_obj = get_socket(sock_index);
	SOCKET_OBJECT peer_sock_obj = get_socket(peer_sock_index);
	SUBJECT subject = get_subject(sub_id_index);

	sock_obj.readers = set_intersection(&subject.readers, &sock_obj.readers);
    sock_obj.writers = set_union(&subject.writers, &sock_obj.writers);
    update_socket_label(sock_index, sock_obj.readers, sock_obj.writers);

	if(is_user_in_set(sock_obj.owner, &peer_sock_obj.readers)) {
        sock_obj.readers = set_intersection(&sock_obj.readers, &peer_sock_obj.readers);
        sock_obj.writers = set_union(&sock_obj.writers, &peer_sock_obj.writers);
        update_socket_label(sock_index, sock_obj.readers, sock_obj.writers);

        return 1;
    }

	return 0;
}

#endif
