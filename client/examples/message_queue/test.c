#include <stdio.h>

char * print_msg() {
	return "hello world";
}

int main() {
	printf("My msg: %s\n", print_msg()); 
	return 0;
}
