#include <stdio.h>
#include <unistd.h>

int main(){
	printf("%d", getpagesize());
	return 0;
}
