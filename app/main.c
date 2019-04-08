/**
 * History:
 * ================================================================
 * 2018-02-05 qing.zou created
 *
 */

#include <signal.h>
#include <getopt.h>

#include <bll.h>

void ignore_sigpipe(void)
{
	/* Ignore SIGPIPE from TCP sockets */
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		perror("Cannot set SIGPIPE handler");
		printf("Cannot set SIGPIPE handler\n");
	}
}

static void help(char *progname)
{
	fprintf(stderr, "------------------------------------------------------------------\n");
	fprintf(stderr, "Usage: %s\n" \
					" [-h |  --help ]\tdisplay this help\n" \
					" [-c |  --conf ]\t\tconf file\n" \
					, progname);
	fprintf(stderr, "------------------------------------------------------------------\n");
}

int main(int argc, char* argv[])
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

	char* config_file = NULL;

	static const struct option long_options[] = {
		{ "help",			no_argument,			NULL, 'h' },
		{ "conf",			required_argument,		NULL, 'c' },
		{ NULL, 0, NULL, 0 }
	};

	optind = 1;
	int o;
	while ((o = getopt_long(argc, argv, "c:h", long_options, NULL)) >= 0) {
		switch(o) {
			case 'c':
				config_file = strdup(optarg);
				printf("config-file = %s\n", config_file);
				break;
			default:
				help(argv[0]);
				return 0;
		}
	}

	if(config_file == NULL || strlen(config_file) <= 0)
	{
		printf("Need conf file\n");
		help(argv[0]);
		return -1;
	}

	bll_init(config_file);
	
	while (bll_cond())
	{
		bll_idle();
	}
	
	bll_done();
	
	return 0;
}
