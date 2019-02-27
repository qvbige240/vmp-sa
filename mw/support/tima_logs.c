/**
 * History:
 * ================================================================
 * 2017-11-13 qing.zou created
 *
 */

#include "vpk_util.h"
#include "vpk_filesys.h"

#include "vmp_log.h"

#include "tima_logs.h"
//#include "tima_util.h"
//#include "tima_globals.h"


enum {
	PROCESS_APP,
	PROCESS_DAEMON,
	PROCESS_SERVER,
};

#define ZLOG_FILE_COUNT			5
#define ZLOG_CONF_FILE			"./zlog.conf"
#define ZLOG_FILE_PATH			"/tmp/log/"

#ifdef USE_ZLOG
int tima_zlog_init(int procname)
{
	int rc;
	//zlog_category_t *c;

	if (!vpk_exists(ZLOG_FILE_PATH)) {
		int ret = 0;
		char tmp[256] = {0};
		vpk_pathname_get(ZLOG_FILE_PATH, tmp);
		printf("full: %s, pathname: %s", ZLOG_FILE_PATH, tmp);
		ret = vpk_mkdir_mult(ZLOG_FILE_PATH);
		printf("vpk_mkdir_mult \'%s\' ret = %d\n", ZLOG_FILE_PATH, ret);
	}

	//rc = zlog_init(ZLOG_CONF_FILE);
	if (procname == PROCESS_APP)
		rc = dzlog_init(ZLOG_CONF_FILE, "app");
	else if (procname == PROCESS_DAEMON)
		rc = dzlog_init(ZLOG_CONF_FILE, "timad");
	else
		rc = dzlog_init(ZLOG_CONF_FILE, "vmp");

	if (rc)	{
		printf("zlog init failed\n");
		return -1;
	}

	//c = zlog_get_category("my_cat");
	//if (!c) {
	//	printf("get cat fail\n");
	//	zlog_fini();
	//	return -2;
	//}
	//zlog_info(c, "hello, zlog");

	VMP_LOGI("hello, zlog");

	return 0;
}
#else
#endif

static const char* const LOG_LEVELS[] = 
{
	"FATAL",
	"ERROR",
	"WARN",
	"INFO",
	"DEBUG",
};

const char*  get_log_level_value(int level)
{
	unsigned int i = 0;
	for (i = 0; i < sizeof(LOG_LEVELS)/sizeof(LOG_LEVELS[0]); i++)
	{
		if (strcmp(LOG_LEVELS[i], LOG_LEVELS[level]) == 0)
		{
			return LOG_LEVELS[i];
		}
	}
	return LOG_LEVELS[i-1];
}

static int tima_zlog_conf(void)
{
	FILE *fp = NULL;
	//int mode = tima_default_config()->log_mode;
	//int level = tima_default_config()->log_level;
	//char* log_path = tima_default_config()->log_path;
	//char* conf_path = tima_default_config()->data_path;
	int mode = 3;
	int level = 4;
	char* log_path = "./log/";
	char* conf_path = "./tima/";

	if (!vpk_exists(conf_path)) {
		int ret = 0;
		char tmp[256] = {0};
		vpk_pathname_get(conf_path, tmp);
		printf("full: %s, pathname: %s\n", conf_path, tmp);
		ret = vpk_mkdir_mult(conf_path);
		printf("vpk_mkdir_mult \'%s\' ret = %d\n", conf_path, ret);
	}

	unsigned int pos = 0;
	char file[256] = {0};

	if (!vpk_exists(conf_path)) {
		strcpy(file, "/tmp/zlog.conf");
	} else {
		vpk_snprintf(file, &pos, MAX_PATH_SIZE, "%s/%s", conf_path, "zlog.conf");
		//sprintf(file, "%s/%s", conf_path, "zlog.conf");
	}

	//char cmd[256] = {0};
	//sprintf(cmd, "rm -rf %s\n", file);
	//vpk_system_ex(cmd, 3);

	fp = fopen(file, "wb");
	if(!fp) {
		fprintf(0, "fopen ERROR!\n");
		return -1;
	}

	fprintf(fp,"[global]\n");
	fprintf(fp,"buffer min = 1024\n");
	fprintf(fp,"buffer max = 2MB\n");
	fprintf(fp,"rotate lock file = /tmp/zlog.lock\n");
	fprintf(fp,"default format = \"%%d(%%F %%T) %%-6V - %%m%%n\"\n");
	fprintf(fp,"[formats]\n");
	//fprintf(fp,"simple	= \"%%d (%%4p:%%15F:%%4L) %%-5V - %%m\"\n");
	//fprintf(fp,"default	= \"%%d(%%F %%T) %%-6V - %%m%%n\"\n");
	fprintf(fp,"default	= \"%%d(%%F %%T) %%-6V (%%F:%%L) - %%m%%n\"\n");
	fprintf(fp,"[rules]\n");
	if (mode & 0x01)
		fprintf(fp,"*.%s         >stdout;default\n", get_log_level_value(level));
	if (mode & 0x02)
		fprintf(fp,"*.%s       \"%s%s.log\",1MB * 15 ~ \"%s%s.log.#2r\";\n",
			get_log_level_value(level), log_path, "tima", log_path, "tima");
	//fwrite(buffer, 1, strlen(buffer), fp);
	fflush(fp);
	fclose(fp);

	if (!vpk_exists(log_path)) {
		char tmp[256] = {0};
		vpk_pathname_get(log_path, tmp);
		vpk_mkdir_mult(log_path);
	}

	int rc = dzlog_init(file, "vmp");
	if (rc)	{
		printf("zlog init failed\n");
		return -1;
	}

	VMP_LOGI("hello, zlog");

	return 0;
}

int tima_log_init(int procname)
{
#ifdef USE_ZLOG
	//return tima_zlog_init(procname);
	return tima_zlog_conf();
#else
	return 0;
#endif
}
