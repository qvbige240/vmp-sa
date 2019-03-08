
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <execinfo.h>

#include "ThreadPool.h"
#include "tvmpss_client.h"


#define TP_MAX_THREADS 			1000
#define TP_MIN_THREADS 			0
#define TP_JOBS_PER_THREAD 		1
#define TP_THREAD_IDLE_TIME 		5000
#define TP_MAX_JOBS_TOTAL 		1000


void SystemErrorHandler(int signum)
{
	const int len=1024;
	void *func[len];
	size_t size;
	unsigned int i;
	char **funs;

	signal(signum,SIG_DFL);
	size=backtrace(func,len);
	funs=(char**)backtrace_symbols(func,size);
	fprintf(stderr,"System error, Stack trace:\n");
	for(i=0;i<size;++i) 
		fprintf(stderr,"%d %s \n",i,funs[i]);
	free(funs);
	//exit(1);
}


void help(char *progname)
{
  fprintf(stderr, "------------------------------------------------------------------\n");
  fprintf(stderr, "Usage: %s\n" \
                  " [-h |  --help ]\tdisplay this help\n" \
		" [-s |  --ip ]\t\tserver ip\n" \
		" [-p |  --port ]\tserver port\n" \
		" [-f |  --file ]\traw video path\n" \
		" [-n |  --number ]\tclient number\n" \
                  , progname);
  fprintf(stderr, "------------------------------------------------------------------\n");
}


static void PrintThreadPoolStats(ThreadPool* tp)
{
	ThreadPoolStats stats;
	
	ThreadPoolGetStats(tp, &stats);
	printf("job(%d, %d, %d)\t"
		"wait(%lf, %lf, %lf)\t"
		"thread(%d, %d, %d, %d, %d)\t"
		"time(%lf, %lf)\n",
		stats.currentJobsHQ,
		stats.currentJobsMQ,
		stats.currentJobsLQ,
		stats.avgWaitHQ,
		stats.avgWaitMQ,
		stats.avgWaitLQ,
		stats.maxThreads,
		stats.workerThreads,
		stats.persistentThreads,
		stats.idleThreads,
		stats.totalThreads,
		stats.totalWorkTime,
		stats.totalIdleTime);

}

int ReadRawData(char* filepath, char** ppData, int* pLen)
{
	struct stat fileInfo;
	char* infile=filepath;
	if(stat(infile, &fileInfo))
	{
		printf("!!!Can't find the file: %s\n", infile);
		return (-1);
	}

	size_t stream_len = fileInfo.st_size;
	if(stream_len > 100*1024*1024)
	{
		printf("!!!Large H264 file\n");
		return -2;
	}

	char* stream_buf = malloc(stream_len);
	if(!stream_buf)
	{
		printf("!!!Not enough memory to malloc\n");
		return -3;
	}

	FILE* fp = fopen(infile, "rb");
	if(!fp)
	{
		printf("!!!Can't open the test file\n");
		free(stream_buf);
		return -4;
	}

	if(fread(stream_buf, 1, stream_len, fp) != stream_len)
	{
		printf("!!!fread file failed\n");
		free(stream_buf);
		fclose(fp);
		return -5;
	}
	fclose(fp);

	*ppData = stream_buf;
	*pLen = stream_len;

	return stream_len;
}

int main(int argc, char* argv[])
{
	signal(SIGSEGV,SystemErrorHandler); //Invaild memory address
	signal(SIGABRT,SystemErrorHandler); // Abort signal

	char* strIp = NULL;
	char* strPort = NULL;
	char* file = NULL;
	char* number = NULL;

	static const struct option long_options[] = {
		{ "help",			no_argument,			NULL, 'h' },
		{ "server",			required_argument,		NULL, 's' },
		{ "port",			required_argument,		NULL, 'p' },
		{ "file",			required_argument,		NULL, 'f' },
		{ "number",		required_argument,		NULL, 'n' },
		{ NULL, 0, NULL, 0 }
	};

	optind = 1;
	int o;
	while ((o = getopt_long(argc, argv, "s:p:f:n:h", long_options, NULL)) >= 0) {
		switch(o) {
			case 'f':
				file = strdup(optarg);
				printf("file=%s\n", file);
				break;
			case 's':
				strIp= strdup(optarg);
				printf("ip=%s\n", strIp);
				break;
			case 'p':
				strPort= strdup(optarg);
				printf("port=%s\n", strPort);
				break;
			case 'n':
				number= optarg;
				printf("number=%s\n", number);
				break;
			default:
				help(argv[0]);
				return 0;
		}
	}

	if(strIp== NULL || strlen(strIp) <= 0 || strPort == NULL || number == NULL)
	{
		printf("input params\n");
		help(argv[0]);
		return -3;
	}

	int nRet = 0;
	char* pData = NULL;
	int len = 0;
	nRet = ReadRawData(file, &pData,&len);
	if(nRet <= 0)
	{
		printf("Read file failed, path=%s\n", file);
		return -1;
	}

	char* ip = strIp;
	int port = atoi(strPort);
	int num = atoi(number);

	nRet = 0;
	ThreadPool* pTP1 = NULL;
	pTP1 = (ThreadPool*)malloc(sizeof(ThreadPool));
	if(pTP1)
	{
		ThreadPoolAttr attr;
		TPAttrInit(&attr);
		TPAttrSetMaxThreads(&attr, TP_MAX_THREADS);
		TPAttrSetMinThreads(&attr, TP_MIN_THREADS);
		TPAttrSetJobsPerThread(&attr, TP_JOBS_PER_THREAD);
		TPAttrSetIdleTime(&attr, TP_THREAD_IDLE_TIME);
		TPAttrSetMaxJobsTotal(&attr, TP_MAX_JOBS_TOTAL);

		nRet = ThreadPoolInit(pTP1, &attr);
		if(nRet != 0)
		{
			printf("TP1 init failed, nRet=%d\n", nRet);
			return -2;
		}
	}

	nRet = 0;
	ThreadPool* pTP2 = NULL;
	pTP2 = (ThreadPool*)malloc(sizeof(ThreadPool));
	if(pTP2)
	{
		ThreadPoolAttr attr;
		TPAttrInit(&attr);
		TPAttrSetMaxThreads(&attr, TP_MAX_THREADS);
		TPAttrSetMinThreads(&attr, TP_MIN_THREADS);
		TPAttrSetJobsPerThread(&attr, TP_JOBS_PER_THREAD);
		TPAttrSetIdleTime(&attr, TP_THREAD_IDLE_TIME);
		TPAttrSetMaxJobsTotal(&attr, TP_MAX_JOBS_TOTAL);

		nRet = ThreadPoolInit(pTP2, &attr);
		if(nRet != 0)
		{
			printf("TP2 init failed, nRet=%d\n", nRet);
			return -2;
		}
	}

	int i;
RETRY:	
	
	for(i = 0; i<num; i++)
	{
		StClient* c = (StClient*)malloc(sizeof(StClient));
		c->data = pData;
		c->len = len;
		c->ip = ip;
		c->port = port;
		c->tp = pTP2;
		
		ThreadPoolJob job;
		TPJobInit( &job, ( start_routine) tvmpss_client_thread, c);
		TPJobSetFreeFunction( &job, ( free_routine ) NULL );
		TPJobSetPriority( &job, HIGH_PRIORITY );
		ThreadPoolAddPersistent( pTP1, &job, &job.jobId);
		c->jobId = job.jobId;
	}
#if 1
	while(1)
	{
		PrintThreadPoolStats(pTP1);
		PrintThreadPoolStats(pTP2);
		sleep(10);
	}
#else
	char c = getchar();
	if(c != 'q')
	{
		goto RETRY;
	}
#endif


	ThreadPoolShutdown(pTP1);
	ThreadPoolShutdown(pTP2);
	//free(pTP1);
		
	return 0;
}

