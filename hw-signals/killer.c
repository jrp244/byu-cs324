#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>

void sigint_handler(int signum) {
	// send SIGKILL to all processes in group, so this process and children
	// will terminate.  Use SIGKILL because SIGTERM and SIGINT (among
	// others) are overridden in the child.
	kill(-getpgid(0), SIGKILL);
}

int main(int argc, char *argv[]) {
	char *scenario = argv[1];
	int pid = atoi(argv[2]);

	struct sigaction sigact;

	// Explicitly set flags
	sigact.sa_flags = SA_RESTART;
	sigact.sa_handler = sigint_handler;
	// Override SIGINT, so that interrupting this process sends SIGKILL to
	// this one and, more importantly, to the child.
	sigaction(SIGINT, &sigact, NULL);

	switch (scenario[0]) {
	case '0':
		kill(pid, SIGHUP);
		break;
	case '1':
		kill(pid,12);
		sleep(1);
		kill(pid,SIGTERM);
		sleep(1);
		break;
	case '2':
		kill(pid, SIGHUP);
		sleep(5);
		kill(pid, 12);
		sleep(1);
		kill(pid, SIGTERM);
		sleep(1);
		break;
	case '3':
		kill(pid, SIGHUP);
                sleep(1);
		kill(pid, SIGHUP);
                sleep(5);
                kill(pid, 12);
                sleep(1);
                kill(pid, SIGTERM);
                sleep(1);
                break;
	case '4':
		kill(pid, SIGHUP);
                sleep(1);
                kill(pid, SIGINT);
		sleep(10);
                kill(pid, 12);
                sleep(1);
                kill(pid, SIGTERM);
                sleep(1);
                break;
	case '5':
		kill(pid, SIGHUP);
                kill(pid, 12);
		sleep(1);
                kill(pid, SIGTERM);
                sleep(1);
                break;
	case '6':
		kill(pid, SIGHUP);
                sleep(5);
                kill(pid, 10);
                sleep(1);
		kill(pid, 16);
		sleep(1);
                kill(pid, 12);
                sleep(1);
                kill(pid, SIGTERM);
                sleep(1);
                break;
	case '7':
		kill(pid, SIGHUP);
                sleep(5);
                kill(pid, 10);
                sleep(1);
                kill(pid, 12);
                sleep(1);
                kill(pid, SIGTERM);
                sleep(1);
                break;
	case '8':
		kill(pid, SIGHUP);
                sleep(5);
		kill(pid, 31);
		sleep(1);
                kill(pid, 10);
		sleep(1);
		kill(pid, 30);
                sleep(1);
                kill(pid, SIGTERM);
                sleep(1);
                kill(pid, 12);
                sleep(1);
                kill(pid, SIGTERM);
                sleep(1);
                break;
	case '9': 
		kill(pid, 31);
                sleep(1);
		kill(pid, SIGQUIT);
		sleep(1);
		kill(pid, 31);
                sleep(1);
		kill(pid, 12);
                sleep(1);
                kill(pid, SIGTERM);
                sleep(1);
		break;
		
	}
	waitpid(pid, NULL, 0);
}
