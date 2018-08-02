typedef unsigned long long int LABEL_SET;
typedef char * HOST;
typedef unsigned int uint;

typedef struct user_id {
    uint uid;
    uint host_id_index;
} USER_ID;

typedef struct group_id {
    uint gid;
    uint host_id_index;
} GROUP_ID;

typedef struct object_id {
    uint device_id;
    uint inode_number;
    int host_id_index;
} OBJECT_ID;

typedef struct object {
    uint obj_id_index;
    uint owner;//index of the user id in ALL_UID array
    LABEL_SET readers;//the bits in places where uid are present from ALL_UID are 1
    LABEL_SET writers;//the bits in places where uid are present from ALL_UID are 1
} OBJECT;

typedef struct subject_id {
    uint uid;
    uint pid;
    int host_id_index;
} SUBJECT_ID;

typedef struct subject {
    uint sub_id_index;
    uint owner;
    LABEL_SET readers;
    LABEL_SET writers;
} SUBJECT;

typedef struct file_descriptor_map {
    uint sub_id_index;
    uint obj_id_index;
    uint fd;
} FD_MAP;
