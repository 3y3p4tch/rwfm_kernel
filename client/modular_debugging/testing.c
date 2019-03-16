#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "infer_object_labels.h"

int main() {
	int fd = open("testing.c", O_RDONLY);
	struct stat buf;
	OBJECT obj;
	fstat(fd, &buf);
	infer_labels(&obj, &buf, 0);
	printf("Readers: %llx\n", obj.readers);
	printf("Writers: %llx\n", obj.writers);

	return 0;
}
