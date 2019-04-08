/**
 * History:
 * ================================================================
 * 2018-02-05 qing.zou created
 *
 */

#include <signal.h>

#include <bll.h>

void ignore_sigpipe(void)
{
	/* Ignore SIGPIPE from TCP sockets */
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		perror("Cannot set SIGPIPE handler");
		printf("Cannot set SIGPIPE handler\n");
	}
}

int main(void)
{
#if 0
	ignore_sigpipe();
	//signal(SIGPIPE, SIG_IGN);
#else
	struct sigaction action;

	action.sa_handler = SIG_IGN;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGPIPE, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
	//sigaction(SIGABRT, &action, NULL);
	//sigaction(SIGSEGV, &action, NULL);
	//sigaction(SIGFPE,  &action, NULL);
#endif


	bll_init("./tima/tvmpssd.conf");
	
	while (bll_cond())
	{
		bll_idle();
	}
	
	bll_done();
	
	return 0;
}
