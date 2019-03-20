/*
This file contains functions that maintain the entire database.
The actual database resides in the 'database_server.c' file.
All the extern variables declared here are instantiated in database_server.c.
The functions declared here adds, deletes, modifies or fetches various information from the database.
*/
#ifndef _DB_HELPER_FUNCTION_H_
#define _DB_HELPER_FUNCTION_H_

#include "database_model.h"
#include <string.h>

#define MAX_HOST_SIZE 1024

/*
Functions that are related to HOSTs in database.
num_hosts is total number of hosts.
all_hosts is an array containing all the host ids encountered till now.
*/
extern int num_hosts;
extern HOST * all_hosts;

/*
Description :   Helper function to compare two hosts.
Parameters  :   Two hosts that are being compared.
Return Value:   zero if the hosts do not match, non zero otherwise.
*/
int match_hosts(HOST h1, HOST h2) {
    return !strcmp(h1, h2);
}

/*
Description :   Add a new host to the database if it doesn't exist.
Parameters  :   new host to be added
Return Value:   index of 'new host' in the array.
*/
int add_host(HOST new_host) {
    for(int i=0;i<num_hosts;i++) {
        if(match_hosts(all_hosts[i], new_host))
            return i;
    }

    all_hosts = (HOST *)realloc(all_hosts, (num_hosts+1) * sizeof(HOST));
    all_hosts[num_hosts] = (HOST)malloc(MAX_HOST_SIZE * sizeof(char));
    strcpy(all_hosts[num_hosts], new_host);

    return num_hosts++;
}

int get_host_index(HOST new_host) {
    for(int i=0;i<num_hosts;i++) {
        if(match_hosts(all_hosts[i], new_host))
            return i;
    }

    return -1;
}

extern int num_user_ids;
extern USER_ID * all_user_ids;

int match_user_ids(USER_ID u1, USER_ID u2) {
    return ((u1.host_id_index == u2.host_id_index) && (u1.uid == u2.uid));
}

int add_user_id(USER_ID new_user_id) {
    for(int i=0;i<num_user_ids;i++) {
        if(match_user_ids(all_user_ids[i], new_user_id))
            return i;
    }

    all_user_ids = (USER_ID *)realloc(all_user_ids, (num_user_ids+1) * sizeof(USER_ID));
    all_user_ids[num_user_ids].host_id_index = new_user_id.host_id_index;
    all_user_ids[num_user_ids].uid = new_user_id.uid;

    return num_user_ids++;
}

int get_user_id_index(USER_ID user_id) {
    for(int i=0;i<num_user_ids;i++) {
        if(match_user_ids(all_user_ids[i], user_id))
            return i;
    }

    return -1;
}

int get_number_of_users() {
    return num_user_ids;
}

extern int num_group_ids;
extern GROUP_ID * all_group_ids;

int match_group_ids(GROUP_ID u1, GROUP_ID u2) {
    return (u1.host_id_index == u2.host_id_index) && (u1.gid == u2.gid);
}

int add_group_id(GROUP_ID new_group_id) {
    for(int i=0;i<num_group_ids;i++) {
        if(match_group_ids(all_group_ids[i], new_group_id))
            return i;
    }

    all_group_ids = (GROUP_ID *)realloc(all_group_ids, (num_group_ids+1) * sizeof(GROUP_ID));
    all_group_ids[num_group_ids].host_id_index = new_group_id.host_id_index;
    all_group_ids[num_group_ids].gid = new_group_id.gid;
    all_group_ids[num_group_ids].members = new_group_id.members;

    return num_group_ids++;
}

int get_group_id_index(GROUP_ID group_id) {
    for(int i=0;i<num_group_ids;i++) {
        if(match_group_ids(all_group_ids[i], group_id))
            return i;
    }

    return -1;
}

USER_SET get_members_from_group_id(int group_id_index) {
    return all_group_ids[group_id_index].members;
}

extern int num_object_ids;
extern OBJECT_ID * all_object_ids;

int match_object_ids(OBJECT_ID obj_id1, OBJECT_ID obj_id2) {
    return obj_id1.host_id_index == obj_id2.host_id_index
            && obj_id1.device_id == obj_id2.device_id
            && obj_id1.inode_number == obj_id2.inode_number;
}

int add_object_id(OBJECT_ID new_object_id) {
    for(int i=0;i<num_object_ids;i++) {
        if(match_object_ids(all_object_ids[i], new_object_id))
            return i;
    }

    all_object_ids = (OBJECT_ID *)realloc(all_object_ids, (num_object_ids+1) * sizeof(OBJECT_ID));
    all_object_ids[num_object_ids].host_id_index = new_object_id.host_id_index;
    all_object_ids[num_object_ids].device_id = new_object_id.device_id;
    all_object_ids[num_object_ids].inode_number = new_object_id.inode_number;

    return num_object_ids++;
}

int get_object_id_index(OBJECT_ID object_id) {
    for(int i=0;i<num_object_ids;i++) {
        if(match_object_ids(all_object_ids[i], object_id))
            return i;
    }

    return -1;
}

extern int num_objects;
extern OBJECT * all_objects;

int match_objects(OBJECT obj1, OBJECT obj2) {
    return obj1.obj_id_index == obj2.obj_id_index;
}

int add_object(OBJECT new_object) {
    for(int i=0;i<num_objects;i++) {
        if(match_objects(all_objects[i], new_object))
            return i;
    }

    all_objects = (OBJECT *)realloc(all_objects, (num_objects+1) * sizeof(OBJECT));
    all_objects[num_objects].obj_id_index = new_object.obj_id_index;
    all_objects[num_objects].owner = new_object.owner;
    all_objects[num_objects].readers = new_object.readers;
    all_objects[num_objects].writers = new_object.writers;

    return num_objects++;
}

int get_object_from_obj_id_index(int obj_id_index) {
    for(int i=0;i<num_objects;i++) {
        if(all_objects[i].obj_id_index == obj_id_index)
            return i;
    }

    return -1;
}

int update_object_label(int obj_id_index, USER_SET readers, USER_SET writers) {
    int obj_index = get_object_from_obj_id_index(obj_id_index);
    all_objects[obj_index].readers = readers;
    all_objects[obj_index].writers = writers;

    return 0;
}

extern int num_subject_ids;
extern SUBJECT_ID * all_subject_ids;

int match_subject_ids(SUBJECT_ID sub_id1, SUBJECT_ID sub_id2) {
    return sub_id1.host_id_index == sub_id2.host_id_index
            && sub_id1.uid == sub_id2.uid
            && sub_id1.pid == sub_id2.pid;
}

int add_subject_id(SUBJECT_ID new_subject_id) {
    for(int i=0;i<num_subject_ids;i++) {
        if(match_subject_ids(all_subject_ids[i], new_subject_id))
            return i;
    }

    all_subject_ids = (SUBJECT_ID *)realloc(all_subject_ids, (num_subject_ids+1) * sizeof(SUBJECT_ID));
    all_subject_ids[num_subject_ids].host_id_index = new_subject_id.host_id_index;
    all_subject_ids[num_subject_ids].uid = new_subject_id.uid;
    all_subject_ids[num_subject_ids].pid = new_subject_id.pid;

    return num_subject_ids++;
}

int get_subject_id_index(SUBJECT_ID subject_id) {
    for(int i=0;i<num_subject_ids;i++) {
        if(match_subject_ids(all_subject_ids[i], subject_id))
            return i;
    }

    return -1;
}

extern int num_subjects;
extern SUBJECT * all_subjects;

int match_subjects(SUBJECT sub1, SUBJECT sub2) {
    return sub1.sub_id_index == sub2.sub_id_index;
}

int add_subject(SUBJECT new_subject) {
    for(int i=0;i<num_subjects;i++) {
        if(match_subjects(all_subjects[i], new_subject))
            return i;
    }

    all_subjects = (SUBJECT *)realloc(all_subjects, (num_subjects+1) * sizeof(SUBJECT));
    all_subjects[num_subjects].sub_id_index = new_subject.sub_id_index;
    all_subjects[num_subjects].owner = new_subject.owner;
    all_subjects[num_subjects].readers = new_subject.readers;
    all_subjects[num_subjects].writers = new_subject.writers;

    return num_subjects++;
}

int get_subject_from_sub_id_index(int sub_id_index) {
    for(int i=0;i<num_subjects;i++) {
        if(all_subjects[i].sub_id_index == sub_id_index)
            return i;
    }

    return -1;
}

int update_subject_label(int sub_id_index, USER_SET readers, USER_SET writers) {
    int sub_index = get_subject_from_sub_id_index(sub_id_index);
    all_subjects[sub_index].readers = readers;
    all_subjects[sub_index].writers = writers;

    return 0;
}

extern int num_sockets;
extern SOCKET_OBJECT * all_sockets;

int add_socket(SOCKET_OBJECT new_socket) {
	all_sockets = (SOCKET_OBJECT *)realloc(all_sockets, (num_sockets+1) * sizeof(SOCKET_OBJECT));
    all_sockets[num_sockets].sub_id_index = new_socket.sub_id_index;
    all_sockets[num_sockets].sock_fd = new_socket.sock_fd;
    all_sockets[num_sockets].port = new_socket.port;
    all_sockets[num_sockets].ip = new_socket.ip;
    all_sockets[num_sockets].owner = new_socket.owner;
    all_sockets[num_sockets].readers = new_socket.readers;
    all_sockets[num_sockets].writers = new_socket.writers;

    return num_sockets++;
}

int update_socket_ip_port(int socket_index, ulong ip, uint port) {
	all_sockets[socket_index].ip = ip;
	all_sockets[socket_index].port = port;

	return 0;
}

int get_socket_index_from_sub_id_sock_fd(int sub_id_index, int sock_fd) {
	for(int i=0;i<num_sockets;i++)
		if(all_sockets[i].sub_id_index == sub_id_index && all_sockets[i].sock_fd == sock_fd)
			return i;

	return -1;
}

int get_socket_index_from_ip_port(ulong ip, uint port) {
	for(int i=0;i<num_sockets;i++)
		if(all_sockets[i].ip == ip && all_sockets[i].port == port)
			return i;

	return -1;
}

int update_socket_label(int socket_index, USER_SET readers, USER_SET writers) {
	all_sockets[socket_index].readers = readers;
	all_sockets[socket_index].writers = writers;

	return 0;
}

extern int num_fd_maps;
extern FD_MAP * fd_map;

int match_fd_map(FD_MAP m1, FD_MAP m2) {
    return m1.sub_id_index == m2.sub_id_index
            && m1.obj_id_index == m2.obj_id_index
            && m1.fd == m2.fd;
}

int add_new_mapping(FD_MAP new_map) {
    for(int i=0;i<num_fd_maps;i++) {
        if(match_fd_map(fd_map[i], new_map))
            return i;
    }

    fd_map = (FD_MAP *)realloc(fd_map, (num_fd_maps+1) * sizeof(FD_MAP));
    fd_map[num_fd_maps].sub_id_index = new_map.sub_id_index;
    fd_map[num_fd_maps].obj_id_index = new_map.obj_id_index;
    fd_map[num_fd_maps].fd = new_map.fd;

    return num_fd_maps++;
}

int get_obj_id_index_from_sub_id_and_fd(uint sub_id_index, uint fd) {
    for(int i=0;i<num_fd_maps;i++) {
        if(fd_map[i].sub_id_index == sub_id_index && fd_map[i].fd == fd)
            return fd_map[i].obj_id_index;
    }

    return -1;
}

int get_sub_id_index_from_obj_id_and_fd(uint obj_id_index, uint fd) {
    for(int i=0;i<num_fd_maps;i++) {
        if(fd_map[i].obj_id_index == obj_id_index && fd_map[i].fd == fd)
            return fd_map[i].sub_id_index;
    }

    return -1;
}

int remove_fd_map(uint sub_id_index, uint fd) {
    for(int i=0;i<num_fd_maps;i++) {
        if(fd_map[i].sub_id_index == sub_id_index && fd_map[i].fd == fd) {
            for(int j=i;j<num_fd_maps-1;j++)
                fd_map[j] = fd_map[j+1];
            fd_map = (FD_MAP *)realloc(fd_map, (--num_fd_maps) * sizeof(FD_MAP));
            return num_fd_maps;
        }
    }

    return -1;
}

extern int num_connection_maps;
extern CONNECTION_MAP * connection_map;

int add_new_connection_map(int sock_index_1) {
	connection_map = (CONNECTION_MAP *)realloc(connection_map, (num_connection_maps+1) * sizeof(CONNECTION_MAP));
	connection_map[num_connection_maps].sock_index_1 = sock_index_1;
	connection_map[num_connection_maps].sock_index_2 = -1;

	return num_connection_maps++;
}

int update_connection_map_sock_index_2(int sock_index_1, int sock_index_2) {
	for(int i=0;i<num_connection_maps;i++) {
		if(connection_map[i].sock_index_1 == sock_index_1)	{
			connection_map[i].sock_index_2 = sock_index_2;
			return 0;
		}
	}

	return -1;
}

int get_peer_socket_index(int sock_index) {
	for(int i=0;i<num_connection_maps;i++) {
		if(connection_map[i].sock_index_1 == sock_index)
			return connection_map[i].sock_index_2;
		if(connection_map[i].sock_index_2 == sock_index)
			return connection_map[i].sock_index_1;
	}

	return -1;
}

/*
Description :   Copy all the information contained in source subject id to destination subject id.
                The information include all the fds (file, pipe, socket, etc) opened by the source.
Parameters  :   subject id index of source and destination subjects
Return Value:   Always succeeds and returns 0.
*/
int copy_subject_info(uint src_sub_id_index, uint dstn_sub_id_index) {
    int fd_maps_to_copy[1024], num_fd_maps_to_copy = 0;
    for(int i=0;i<num_fd_maps;i++) {
        if(fd_map[i].sub_id_index == src_sub_id_index)
            fd_maps_to_copy[num_fd_maps_to_copy++] = i;
    }
    for(int i=0;i<num_fd_maps_to_copy;i++) {
        FD_MAP new_map;
        new_map.sub_id_index = dstn_sub_id_index;
        new_map.obj_id_index = fd_map[fd_maps_to_copy[i]].obj_id_index;
        new_map.fd = fd_map[fd_maps_to_copy[i]].fd;
        add_new_mapping(new_map);
    }

    return 0;
}

#endif
