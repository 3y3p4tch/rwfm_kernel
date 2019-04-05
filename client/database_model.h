/*
This file contains all the structures for all the objects which we keep in our database
*/

#ifndef _DB_MODEL_
#define _DB_MODEL_

typedef unsigned long long int USER_SET;
typedef char * HOST;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct mq_msg {
	int msg_type;
	char msg_str[1024];
} MQ_MSG;

typedef struct mq_buffer {
	long pid;
	MQ_MSG msg;
} MQ_BUFFER;

typedef struct user_id {
    uint uid;
    uint host_id_index;
} USER_ID;

typedef struct group_id {
    uint gid;
    uint host_id_index;
    USER_SET members;
} GROUP_ID;

typedef struct object_id {
    ulong device_id;
    ulong inode_number;
    int host_id_index;
} OBJECT_ID;

typedef struct object {
    uint obj_id_index;
    uint owner;//index of the user id in ALL_UID array
    USER_SET readers;//the bits in places where uid are present from ALL_UID are 1
    USER_SET writers;//the bits in places where uid are present from ALL_UID are 1
} OBJECT;

typedef struct subject_id {
    uint uid;
    uint pid;
    int host_id_index;
} SUBJECT_ID;

typedef struct subject {
    uint sub_id_index;
    uint owner;
    USER_SET readers;
    USER_SET writers;
} SUBJECT;

typedef struct address {
	ulong ip;
	uint port;
} ADDRESS;

typedef struct socket_connection_object {
	ADDRESS src, dstn;
	uint num_peers;
    int peer_ids[2];
    USER_SET readers;
    USER_SET writers; 
} SOCKET_CONNECTION_OBJECT;

typedef struct pipe_object {
	int host_id_index;
	ulong device_id;
	ulong inode_number;
	int pipe_ref_count;
	USER_SET readers;
	USER_SET writers;
} PIPE_OBJECT;

typedef struct file_descriptor_map {
    uint sub_id_index;
    uint obj_id_index;
    uint fd;
} FD_MAP;

typedef struct pipe_ref_map {
	uint sub_id_index;
	uint pipe_index;
	uint ref_count;
} PIPE_REF_MAP;

#endif
