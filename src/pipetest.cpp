#include <cstdio> 
#include <unistd.h> 
#include <cstdlib>
#include <time.h>

#define MSGSIZE 16 
union pipestruct {
	int arr[2];
	struct {
		int in;
		int out;
	};
};

char msg1[] = "hello, world #1"; 
char msg2[] = "hello, world #2"; 
char msg3[] = "hello, world #3"; 
  
int child_func(int num, int p[2]);

int main() 
{ 
    char inbuf[MSGSIZE]; 
    int p[2][3], i; 
    pipestruct pipes[3];
  
    for (int i = 0; i < 3; ++i) {
	    if (pipe(pipes[i].arr) < 0)  {
		    printf("cos nie dziala\n");
		    exit(1);
	    }

    }
  
    /* continued */
    /* write pipe */
  
    /* write(p[1][0], msg1, MSGSIZE); */ 
    /* write(p[1][1], msg2, MSGSIZE); */ 
    /* write(p[1][2], msg3, MSGSIZE); */ 

    int pid;
    int child_num = -1;
  
    for (i = 0; i < 3; i++) { 
        /* read pipe */
        /* read(p[0], inbuf, MSGSIZE); */ 
        /* printf("% s\n", inbuf); */ 

	    if((pid = fork()) > 0) {
		    child_func(i, pipes[i].arr);
		    exit(0);
	    }
    } 


    fd_set fd;
    fd_set fd_copy;
    FD_ZERO(&fd);
    for (int i = 0; i < 3; ++i) {
    	FD_SET(pipes[i].in, &fd);
    }

    int max_file_desc = 0;

    for (int i = 0; i < 3; ++i) {
    	if (pipes[i].in > max_file_desc)
		max_file_desc = pipes[i].in;
    }

    int res;
    timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    printf("Proces matka tez zyje!\n");
    while(1) {
	fd_copy = fd;
	printf("przed selectem\n");
	res = select(max_file_desc + 1, &fd_copy, NULL, NULL, &timeout);
	if(res == 0) {
		printf("Proces matka widząc śmierć wszystkich swoich dzieci też umiera ;(\n");
		exit(0);
	}
	printf("po selectcie: %d\n", res);

	for (int i = 0; i < 3; ++i) {
		char buf[32];
		if(FD_ISSET(pipes[i].in, &fd_copy)) {
			read(pipes[i].in, buf, 32);
			printf("Wiadomosc od procesu %d: %s\n", i, buf);
		}
	}
    }
    printf("Proces matka umiera ;(\n");
    return 0; 
} 

int child_func(int num, int p[2]) {
	printf("Dziecko numer %d zyje!\n", num);
	char buf[32];
	sprintf(buf, "Dziecko numer %d pozdrawia", num);
	sleep(num * 5);
	write(p[1], buf, 32);

	printf("Dziecko numer %d umiera ;(\n", num);
	return 1;
}
