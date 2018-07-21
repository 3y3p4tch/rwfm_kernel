typedef long long int LABEL_SET;

typedef char * HOST;

typedef struct user_id {
    int uid;
    HOST * host_id;
} USER_ID;

typedef struct group_id {
    int gid;
    HOST * host_id;
} GROUP_ID;

typedef struct object_id {
    int device_id;
    int inode_number;
    HOST * host_id;
} OBJECT_ID;

typedef struct object {
    OBJECT_ID * obj_id;
    int owner;//index of the user id in ALL_UID array
    LABEL_SET readers;//the bits in places where uid are present from ALL_UID are 1
    LABEL_SET writers;//the bits in places where uid are present from ALL_UID are 1
} OBJECT;

typedef struct subject_id {
    int uid;
    int pid;
    HOST * host_id;
} SUBJECT_ID;

typedef struct subject {
    SUBJECT_ID * sub_id;
    int owner;
    LABEL_SET readers;
    LABEL_SET writers;
} SUBJECT;

typedef struct file_descriptor_map {
    SUBJECT_ID * sub_id;
    OBJECT_ID * obj_id;
    int fd;
} FD_MAP;
