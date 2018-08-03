#define REQUEST_FIFO_PATH "/tmp/request_fifo"
#define RESPONSE_FIFO_PATH "/tmp/response_fifo"

#define REQUEST_DELIM " "
#define MAX_REQUEST_LENGTH 1024

#define ADD_HOST_OP 0
#define GET_HOST_INDEX_OP 1
#define ADD_USER_ID_OP 2
#define GET_USER_ID_INDEX_OP 3
#define ADD_GROUP_ID_OP 4
#define GET_GROUP_ID_INDEX_OP 5
#define ADD_OBJECT_ID_OP 6
#define GET_OBJECT_ID_INDEX_OP 7
#define ADD_SUBJECT_ID_OP 8
#define GET_SUBJECT_ID_INDEX_OP 9
#define ADD_OBJECT_OP 10
#define GET_OBJECT_INDEX_OP 11
#define UPDATE_OBJECT_LABEL_OP 12
#define ADD_SUBJECT_OP 13
#define GET_SUBJECT_INDEX_OP 14
#define UPDATE_SUBJECT_LABEL_OP 15
#define ADD_NEW_FD_MAPPING_OP 16
#define GET_OBJ_ID_FROM_FD_MAP_OP 17
#define GET_SUB_ID_FROM_FD_MAP_OP 18
