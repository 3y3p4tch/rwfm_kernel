typedef unsigned long long int LABEL_SET;

typedef char * HOST;

typedef struct user_id {
    unsigned int uid;
    HOST * host_id;
} USER_ID;

typedef struct group_id {
    unsigned int gid;
    HOST * host_id;
} GROUP_ID;

typedef struct object_id {
    unsigned int device_id;
    unsigned int inode_number;
    HOST * host_id;
} OBJECT_ID;

typedef struct object {
    OBJECT_ID * obj_id;
    unsigned int owner;//index of the user id in ALL_UID array
    LABEL_SET readers;//the bits in places where uid are present from ALL_UID are 1
    LABEL_SET writers;//the bits in places where uid are present from ALL_UID are 1
} OBJECT;

typedef struct subject_id {
    unsigned int uid;
    unsigned int pid;
    HOST * host_id;
} SUBJECT_ID;

typedef struct subject {
    SUBJECT_ID * sub_id;
    unsigned int owner;
    LABEL_SET readers;
    LABEL_SET writers;
} SUBJECT;

typedef struct file_descriptor_map {
    SUBJECT_ID * sub_id;
    OBJECT_ID * obj_id;
    unsigned int fd;
} FD_MAP;
