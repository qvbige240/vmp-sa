
#include <getopt.h>

#include <bll.h>

void help(char *progname)
{
  fprintf(stderr, "------------------------------------------------------------------\n");
  fprintf(stderr, "Usage: %s\n" \
                  " [-h |  --help ]\tdisplay this help\n" \
		" [-c |  --conf ]\t\tconf file\n" \
                  , progname);
  fprintf(stderr, "------------------------------------------------------------------\n");
}


int main(int argc, char* argv[]) {

	char* strConf = NULL;

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
				strConf = strdup(optarg);
				printf("conf=%s\n", strConf);
				break;
			default:
				help(argv[0]);
				return 0;
		}
	}

	if(strConf== NULL || strlen(strConf) <= 0)
	{
		printf("Need conf file\n");
		help(argv[0]);
		return -1;
	}


	
	bll_init(strConf);
	
	while(bll_cond()){
		bll_idle();
	}
	
	bll_done();
	
	return 0;
}

