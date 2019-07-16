#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include <signal.h>
#include <pthread.h>

#include <alsa/asoundlib.h>

#include "vpk_list.h"

#define LOG_D printf

//#define PORT  25341
#define PORT 9001
//#define PORT 4369

#define PERIOD_SIZE_MULT	2		

int jt1078_package(char *buffer, const char *data, unsigned short len)
{
	int i = 0;

	char *p = buffer;
	p[i++] = 0x30;
	p[i++] = 0x31;
	p[i++] = 0x63;
	p[i++] = 0x64;

	p[8] = 0x01;
	p[9] = 0x36;
	p[10] = 0x84;
	p[11] = 0x09;
	p[12] = 0x11;
	p[13] = 0x97;

	p[15] = 0x30;
	p[28] = (len >> 8) & 0xff;
	p[29] = len & 0xff;

	memcpy(p+30, data, len);

	return len+30;
}


typedef struct _PrivInfo
{
	// test..
	FILE*				fp;
	size_t				offset;
	int					file_size;
	unsigned			buf_size;
	char				*buf;

	list_t				audio_head;
	int					list_size;
	pthread_mutex_t		list_mutex;
	pthread_cond_t		cond_empty;
} PrivInfo;
PrivInfo file_info = {0};
PrivInfo* priv = &file_info;

static int vpk_file_open(void* p, const char* filename)
{
	//TimaRdtChannel* thiz = (TimaRdtChannel*)p;
	//PrivInfo* priv = thiz->priv;

	//size_t size = 1024;
	size_t size = 1600;

	int result;
	priv->fp = fopen(filename, "r");
	fseek(priv->fp, 0, SEEK_END);
	priv->file_size = ftell(priv->fp);
	fseek(priv->fp, 0, SEEK_SET);

	priv->buf_size = (priv->offset + size) <= priv->file_size ? size : priv->file_size;
	priv->buf = malloc(priv->buf_size);
	return 0;
}

static int vpk_file_read()
{
	//TimaRdtChannel* thiz = (TimaRdtChannel*)p;
	//PrivInfo* priv = thiz->priv;
	int result;

restart:
	if (priv->offset < priv->file_size) {
		memset(priv->buf, 0x00, priv->buf_size);
		fseek(priv->fp, priv->offset, SEEK_SET);
		result = fread(priv->buf, 1, priv->buf_size, priv->fp);
		if (result < 0) {
			printf("=======read file end!\n");
			return -1;
		}
		priv->offset += result;

		//printf("%05d ", cnt++);

		//tpc_packet_send(priv->buf, result);

		return result;
	} else {
		printf("\n=========== read end!!\n");

		priv->offset = 0;
		fseek(priv->fp, 0, SEEK_SET);
		goto restart;

		if (priv->fp) 
			fclose(priv->fp);

	}
	printf("\n===========111111 read end!!\n");

	return -1;
}

#if 0
snd_pcm_t *record_handle;	//PCM设备句柄pcm.h
snd_pcm_uframes_t frames;

static void alsa_release(int exitcode)
{
	usleep(200000);
	printf("recv: SIGINT\n");
	// 关闭PCM设备句柄
	snd_pcm_drain(record_handle);
	snd_pcm_close(record_handle);
}
static int record_audio_init(unsigned int s_rate, unsigned int s_channel)
{
	int ret, size, dir = 1;
	char *buffer;
	unsigned int rate = s_rate, channel = s_channel;

	snd_pcm_uframes_t periodsize;
	snd_pcm_hw_params_t *hw_params;	//硬件信息和PCM流配置

	//1. 打开PCM，最后一个参数为0意味着标准配置
	ret = snd_pcm_open(&record_handle, "default", SND_PCM_STREAM_CAPTURE, 0);
	if (ret < 0) {
		perror("snd_pcm_open");
		exit(1);
	}

	//2. 分配snd_pcm_hw_params_t结构体
	ret = snd_pcm_hw_params_malloc(&hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params_malloc");
		exit(1);
	}

	//3. 初始化hw_params
	ret = snd_pcm_hw_params_any(record_handle, hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params_any");
		exit(1);
	}

	//4. 初始化访问权限
	ret = snd_pcm_hw_params_set_access(record_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_access");
		exit(1);
	}

	//5. 初始化采样格式SND_PCM_FORMAT_U8, 8, 16位 
	ret = snd_pcm_hw_params_set_format(record_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_format");
		exit(1);
	}

	//rate = 8000;
	ret = snd_pcm_hw_params_set_rate_near(record_handle, hw_params, &rate, &dir);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_rate_near");
		exit(1);
	}

	//7. 设置通道数量
	//channel = 1;
	ret = snd_pcm_hw_params_set_channels(record_handle, hw_params, channel);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_channels");
		exit(1);
	}

	printf("sample rate: %d, channel: %d\n ", rate, channel);

	//frames = 160;
	frames = rate * 20 / 1000;
	frames = frames / channel * PERIOD_SIZE_MULT;
	ret = snd_pcm_hw_params_set_period_size_near(record_handle, hw_params, &frames, 0);
	if (ret < 0) 
	{
		printf("Unable to set period size %li : %s\n", frames,  snd_strerror(ret));
	}
	printf("set frames: %d\n ", frames);

	//periodsize = 800;
	periodsize = (rate / 1000) * 100 * PERIOD_SIZE_MULT;
	//periodsize = periodsize * 2;
	ret = snd_pcm_hw_params_set_buffer_size_near(record_handle, hw_params, &periodsize);
	if (ret < 0) 
	{
		printf("Unable to set buffer size %li : %s\n", periodsize, snd_strerror(ret));

	}
	printf("set periodsize: %d\n ", periodsize);

	//8. 设置hw_params
	ret = snd_pcm_hw_params(record_handle, hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params");
		exit(1);
	}


	snd_pcm_hw_params_get_period_size(hw_params, &periodsize, 0);

	snd_pcm_format_t fmt = SND_PCM_FORMAT_S16_LE;
	snd_pcm_hw_params_get_format(hw_params, &fmt);
	int bit_per_sample = snd_pcm_format_physical_width(fmt);
	int fmtbits = snd_pcm_format_width(fmt);
	int chunk_byte = periodsize * bit_per_sample * channel / 8;

	printf("capture.\n");
	printf("rate %u.\n", rate);
	printf("channel %d.\n", channel);
	printf("fmt:%s, bytes: %u, bits:%u.\n", snd_pcm_format_name(fmt), bit_per_sample/8, fmtbits);


	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);

	//size = frames * 2; /* 2 bytes/sample, 2 channels */
	//size = 5 * rate * 16 * channel / 8; /* 5 seconds, 1 channels */
	size = frames * channel * bit_per_sample / 8; /* 1 frames, 1 channels */
	buffer = (char *) malloc(size + 1);
	memset(buffer, 0x00, size+1);
	fprintf(stderr, "size: %d, chunk_byte: %d, periodsize = %d, frames = %d\n", size, chunk_byte, periodsize, frames);

	snd_pcm_prepare(record_handle);


	priv->buf = buffer;
	priv->buf_size = size;

	return 0;
}

snd_pcm_uframes_t frames_play;
snd_pcm_t *playback_handle;//PCM设备句柄pcm.h

static void alsa_play_release(int exitcode)
{

	usleep(200000);
	printf("play: SIGINT\n");
	// 关闭PCM设备句柄
	snd_pcm_drain(playback_handle);
	snd_pcm_close(playback_handle);

	//if (fp != NULL)
	//	fclose(fp);
}

static void null_alsa_error_handler (const char *file,
									 int line,
									 const char *function,
									 int err,
									 const char *fmt,
									 ...)
{
}
static int alsa_device_list()
{
	char **hints, **n;
	/* Enumerate sound devices */
	int err = snd_device_name_hint(-1, "pcm", (void***)&hints);
	if (err != 0)
		return -1;

	/* Set a null error handler prior to enumeration to suppress errors */
	snd_lib_error_set_handler(null_alsa_error_handler);

	n = hints;
	while (*n != NULL) {
		char *name = snd_device_name_get_hint(*n, "NAME");
		if (name != NULL) {
			printf("	alsa device: %s\n", name);
			//if (0 != strcmp("null", name))
			//	add_dev(af, name);
			free(name);
		}
		n++;
	}

	return 0;
}

static void alsa_pcm_setting_8000_1()
{
	int ret;
	//int buf[128];
	unsigned int val;
	unsigned int rate = 8000, channel = 1;
	int dir=0;
	//char *buffer;
	int size;
	snd_pcm_uframes_t periodsize_play;
	//snd_pcm_t *playback_handle;//PCM设备句柄pcm.h
	snd_pcm_hw_params_t *hw_params;//硬件信息和PCM流配置
	//if (argc != 2) {
	//	printf("error: alsa_play_test [music name]\n");
	//	exit(1);
	//}
	//printf("play song %s by wolf\n", argv[1]);
	//FILE *fp = fopen(argv[1], "rb");
	//fp = fopen(argv[1], "rb");
	//if(fp == NULL)
	//	return 0;
	//fseek(fp, 100, SEEK_SET);

	//1. 打开PCM，最后一个参数为0意味着标准配置
	//ret = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	ret = snd_pcm_open(&playback_handle, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0) {
		perror("snd_pcm_open");
		exit(1);
	}

	//2. 分配snd_pcm_hw_params_t结构体
	ret = snd_pcm_hw_params_malloc(&hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params_malloc");
		exit(1);
	}
	//3. 初始化hw_params
	ret = snd_pcm_hw_params_any(playback_handle, hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params_any");
		exit(1);
	}
	//4. 初始化访问权限
	ret = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_access");
		exit(1);
	}
	//5. 初始化采样格式SND_PCM_FORMAT_U8,8位
	ret = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_format");
		exit(1);
	}
	//6. 设置采样率，如果硬件不支持我们设置的采样率，将使用最接近的
	//val = 44100,有些录音采样频率固定为8KHz


	//val = 8000;
	//val = 44100;
	ret = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, &dir);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_rate_near");
		exit(1);
	}
	//7. 设置通道数量
	ret = snd_pcm_hw_params_set_channels(playback_handle, hw_params, channel);
	if (ret < 0) {
		perror("snd_pcm_hw_params_set_channels");
		exit(1);
	}
	printf("sample rate: %d, channel: %d\n ", rate, channel);

	//frames = 160;
	//frames_play = rate * 20 / 1000;
	frames_play = rate * 20 / 1000;
	frames_play = frames_play / channel * PERIOD_SIZE_MULT;
	ret = snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &frames_play, 0);
	if (ret < 0) 
	{
		printf("Unable to set period size %li : %s\n", frames_play,  snd_strerror(ret));
	}
	printf("set frames_play: %d\n ", frames_play);

	//periodsize = 800;
	periodsize_play = (rate / 1000) * 100 * PERIOD_SIZE_MULT;
	//periodsize_play = frames_play * 3;
	ret = snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &periodsize_play);
	if (ret < 0) 
	{
		printf("Unable to set buffer size %li : %s\n", periodsize_play, snd_strerror(ret));

	}
	printf("set periodsize_play: %d\n ", periodsize_play);

	//8. 设置hw_params
	ret = snd_pcm_hw_params(playback_handle, hw_params);
	if (ret < 0) {
		perror("snd_pcm_hw_params");
		exit(1);
	}

	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(hw_params, &frames_play, &dir);

	size = frames_play * 2; /* 2 bytes/sample, 2 channels */
	//buffer = (char *) malloc(size + 1);
	//memset(buffer, 0x0, size+1);
	fprintf(stderr, "size = %d\n", size);

	//signal(SIGINT, alsa_play_release);
	//signal(SIGTERM, alsa_play_release);


	snd_pcm_prepare(playback_handle);

}
#endif



//int buf[BUFSIZE * 2];
snd_pcm_t *playback_handle, *capture_handle;

//static unsigned int rate = 192000;
//static unsigned int format = SND_PCM_FORMAT_S32_LE;
//static unsigned int rate = 19200;
static unsigned int rate = 8000;
static unsigned int format = SND_PCM_FORMAT_S16_LE;
static unsigned int channel = 1;

unsigned int buffer_time = 300000;
unsigned int period_time = 100000;
snd_pcm_sframes_t buffer_size;
snd_pcm_sframes_t period_size;

static int open_stream(snd_pcm_t **handle, const char *name, int dir)
{
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	const char *dirname = (dir == SND_PCM_STREAM_PLAYBACK) ? "PLAYBACK" : "CAPTURE";
	int err;

	if ((err = snd_pcm_open(handle, name, dir, 0)) < 0) {
		fprintf(stderr, "%s (%s): cannot open audio device (%s)\n", 
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot allocate hardware parameter structure(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_any(*handle, hw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot initialize hardware parameter structure(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_access(*handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "%s (%s): cannot set access type(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_format(*handle, hw_params, format)) < 0) {
		fprintf(stderr, "%s (%s): cannot set sample format(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_rate_near(*handle, hw_params, &rate, NULL)) < 0) {
		fprintf(stderr, "%s (%s): cannot set sample rate(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_hw_params_set_channels(*handle, hw_params, channel)) < 0) {
		fprintf(stderr, "%s (%s): cannot set channel count(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}


	snd_pcm_hw_params_set_buffer_time_near(*handle, hw_params, &buffer_time, &dir);
	snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);

	snd_pcm_hw_params_set_period_time_near(*handle, hw_params, &period_time, &dir);
	snd_pcm_hw_params_get_period_size(hw_params, &period_size, &dir);


	if ((err = snd_pcm_hw_params(*handle, hw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot set parameters(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	snd_pcm_hw_params_free(hw_params);

	if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot allocate software parameters structure(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}
	if ((err = snd_pcm_sw_params_current(*handle, sw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot initialize software parameters structure(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

#if 1 

	snd_pcm_sw_params_set_start_threshold(*handle, sw_params, (buffer_size / period_size) * period_size);
	snd_pcm_sw_params_set_avail_min(*handle, sw_params, period_size);
	printf("period_size = %d, buffer_size = %d, dir = %d\n", period_size, buffer_size, dir);

#else

	if ((err = snd_pcm_sw_params_set_avail_min(*handle, sw_params, BUFSIZE)) < 0) {
		fprintf(stderr, "%s (%s): cannot set minimum available count(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_sw_params_set_start_threshold(*handle, sw_params, BUFSIZE)) < 0) {
		fprintf(stderr, "%s (%s): cannot set start mode(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}
#endif

	if ((err = snd_pcm_sw_params(*handle, sw_params)) < 0) {
		fprintf(stderr, "%s (%s): cannot set software parameters(%s)\n",
			name, dirname, snd_strerror(err));
		return err;
	}

	return 0;
}

pthread_mutex_t		voice_mutex;
pthread_cond_t		voice_cond;
static int alsa_pcm_init()
{
	int err;

	if ((err = open_stream(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK)) < 0)
		return err;

	if ((err = open_stream(&capture_handle, "default", SND_PCM_STREAM_CAPTURE)) < 0)
		return err;

	if ((err = snd_pcm_prepare(playback_handle)) < 0) {
		fprintf(stderr, "cannot prepare audio interface for use(%s)\n",
			snd_strerror(err));
		return err;
	}

	if ((err = snd_pcm_start(capture_handle)) < 0) {
		fprintf(stderr, "cannot prepare audio interface for use(%s)\n",
			snd_strerror(err));
		return err;
	}

	pthread_mutex_init(&voice_mutex, NULL);
	pthread_cond_init(&voice_cond, NULL);

	return 0;
}

//static int alsa_pcm_read(char *buffer)
//{
//	int ret = 0, avail;
//
//	avail = snd_pcm_avail_update(playback_handle);
//	if (avail > 0) {
//		if (avail > period_size)
//			avail = period_size;
//
//		ret = snd_pcm_writei(playback_handle, buffer, avail);
//		//printf("writei ret = %d\n", ret);
//	} else {
//		usleep(period_time);
//	}
//
//	return ret;
//}

static int alsa_pcm_write(char *buffer)
{
	int wait_time = 1000;
	int ret = 0, avail, err;

write_wait:
	if ((err = snd_pcm_wait(playback_handle, wait_time)) < 0) {
		fprintf(stderr, "poll failed(%s)\n", strerror(errno));
		return -1;
	}


	pthread_mutex_lock(&voice_mutex);
	avail = snd_pcm_avail_update(playback_handle);
	if (avail > 0) {
		if (avail < period_size) {
			pthread_mutex_unlock(&voice_mutex);
			printf("============= writei snd_pcm_wait = %d ============\n", wait_time);
			goto write_wait;
		}

		if (avail > period_size)
			avail = period_size;

		//double elapsed;
		struct timeval result, prev, next;	

		gettimeofday(&prev, 0);

		ret = snd_pcm_writei(playback_handle, buffer, avail);

		gettimeofday(&next, 0);
		printf("snd_pcm_writei ret = %d, avail = %d, prev(%dus), next(%dus)\n", ret, avail, prev.tv_usec, next.tv_usec);
		if (ret != period_size)
			printf("============= writei ret = %d ============\n", ret);
	} else {
		//usleep(2000);
	}

	pthread_cond_wait(&voice_cond, &voice_mutex);

	pthread_mutex_unlock(&voice_mutex);
	

	return ret;
}

static void alsa_pcm_play_8000_1(char *buffer, int len)
{
	int ret, avail, offset = 0;

play_back:

	while((ret = alsa_pcm_write(buffer+offset))<=0)
	//while((ret = snd_pcm_writei(playback_handle, buffer+offset, frames_play))<0)
	{
		//usleep(2000);
		printf("writei failed ret = %d, playback_handle = %p\n", ret, playback_handle);
		snd_pcm_prepare(playback_handle);
		if (ret == -EPIPE)
		{
			/* EPIPE means underrun */
			fprintf(stderr, "underrun occurred\n");
			//完成硬件参数设置，使设备准备好
			snd_pcm_prepare(playback_handle);
			continue;
		} 
		else if (ret < 0) 
		{
			fprintf(stderr, "error from writei: %s\n", snd_strerror(ret));
		}  
	}
	if (ret < period_size)
		printf("=========== snd_pcm_writei ret = %d, len = %d ===========\n", ret, len);

//	if (len/2 > ret) {
//		offset += ret;
//		len -= ret*2;
//		goto play_back;
//	}

	//usleep(period_time);
}

typedef struct _stream_header
{
	unsigned char		magic[4];		/* 0x30 0x31 0x63 0x64, 4 byte */
	unsigned char		rtp;			/* 1 byte */
	unsigned char		pt;				/* 1 byte */
	unsigned short		flowid;			/* packet id 2 byte */
	unsigned char		sim[6];			/* sim number bcd code, 6 byte */
	unsigned char		channel;		/* channel id, 1 byte */
	unsigned char		mtype;			/* media data type, 1 byte */
	unsigned char		timestamp[8];	/* data type, 8 byte */
	unsigned short		lifinterval;	/* last I frame interval, 2 byte */
	unsigned short		lfinterval;		/* last frame interval, 2 byte */
	unsigned short		bodylen;		/* data body length, 2 byte */

	unsigned long long simno;
} stream_header_t;

typedef struct audio_node_s {
	list_t				node;

	unsigned char*		data;
	unsigned int		size;

	//node_destroy		destroy;
} audio_node_t;

static void audio_mutex_init()
{
	PrivInfo* thiz = priv;
	//memset(thiz, 0, sizeof(PrivInfo));

	pthread_mutex_init(&thiz->list_mutex, NULL);
	pthread_cond_init(&thiz->cond_empty, NULL);

	INIT_LIST_HEAD(&thiz->audio_head);
}
static audio_node_t* pkt_node_create(void *data, int size)
{
	audio_node_t* pkt = (audio_node_t*)calloc(1, sizeof(audio_node_t) + size);

	if (pkt) {
		INIT_LIST_HEAD(&pkt->node);

		pkt->data = (char*)pkt + sizeof(audio_node_t);
		pkt->size = size;
		memcpy(pkt->data, data, size);
	} else {
		printf("node create failed, malloc null\n");
	}

	return pkt;
}
static int list_add_audio(void* p, void* data)
{
	PrivInfo* thiz = priv;
	audio_node_t* audio = (audio_node_t*)data;

	pthread_mutex_lock(&thiz->list_mutex);

	list_add_tail(&audio->node, &thiz->audio_head);
	if (thiz->list_size++ == 0)
		pthread_cond_broadcast(&thiz->cond_empty);

	pthread_mutex_unlock(&thiz->list_mutex);

	return 0;
}

static void* list_del_audio(void* p)
{
	PrivInfo* thiz = priv;
	audio_node_t* audio = NULL;

	pthread_mutex_lock(&thiz->list_mutex);

	if (list_empty(&thiz->audio_head)) {
		pthread_cond_wait(&thiz->cond_empty, &thiz->list_mutex);

		//pthread_mutex_unlock(&thiz->list_mutex);
		//return NULL;
	}

	if (thiz->list_size > 0) {
		audio = container_of(thiz->audio_head.next, audio_node_t, node);
		list_del(thiz->audio_head.next);
		thiz->list_size--;
		printf("======== list_size = %d\n", thiz->list_size);
		if (!audio)
			printf("audio node get failed, null\n");
		//else
		//if (thiz->list_size > 100)
		//	TIMA_LOGI("%lld [%d] list_size: %d", thiz->sim, nalu->cid, thiz->list_size);
	} else {
		printf("pthread_cond_wait end\n");
	}

	pthread_mutex_unlock(&thiz->list_mutex);

	return audio;
}

static inline size_t find_magic_code(const unsigned char *buf)
{
	if (buf[0] != 0x30 || buf[1] != 0x31 || buf[2] != 0x63 || buf[3] != 0x64) {
		int ii = 0;
		for (ii = 0; ii < 32; ii++)
		{
			printf("%02x ", buf[ii]);
		}
		printf("... data packet exception\n");
		return 0;
	}

	return 1;
}
int packet_jt1078_parse(unsigned char *packet, int length, stream_header_t *head, unsigned char **body)
{
	int pos = 0;

	if (!find_magic_code(packet))
		return -1;

	memcpy(head->magic, packet, 4);
	pos += 4;
	head->rtp	= packet[pos++];
	head->pt	= packet[pos++];

	//head->simno = simno_get(packet+8);
	//...

	head->channel	= packet[14];
	head->mtype		= packet[15];

	//int is_audio = ((packet[15]& 0xf0) == 0x30);
	//if (is_audio)
	//	head->bodylen = packet[28] * 2;
	//else
	head->bodylen = (packet[28] << 8) | packet[29];

	*body = packet + 30;

	if (head->bodylen > length - 30)	/* less than one packet */
		return 0;

	return head->bodylen + 30;	
}

void play_pcm()
{
	int ret = vpk_file_read();
	if (ret == -1) {        
		printf("========== end ==========\n");
		return;
	}
	alsa_pcm_play_8000_1(priv->buf, priv->buf_size);
}
void alsa_play(int client)
{
	int ret = 0;
	//int length = 0;
	//char recv_data[1024];

	//static char pcm_data[9632] = {0};

	//unsigned char *stream = NULL;
	//stream_header_t head = {0};
	//memset(recv_data, 0x00, sizeof(recv_data));


#if 0
	vpk_file_open(NULL, "./pcm_8000_1.pcm");
	printf("start send pcm data...\n");

	//pcm16_alaw_tableinit();
	char *g711_data = calloc(1, priv->buf_size/2);
	while (1)
	{
		////memset(encode_buff, 0, 1024);
		//ret = vpk_file_read();
		//if (ret == -1) {        
		//	printf("========== end ==========\n");
		//	break;        
		//}
		//////ret = jt1078_package(encode_buff, priv->buf, priv->buf_size);
		////pcm16_to_alaw(priv->buf_size, priv->buf, g711_data);
		////ret = jt1078_package(encode_buff, g711_data, priv->buf_size/2);
		////send(client, encode_buff, ret, 0);
		////usleep(5000);

		//alsa_pcm_play_8000_1(priv->buf, priv->buf_size);
		////usleep(1000);

		play_pcm();
		//sleep(5);
	}

#else
	usleep(1000);

	//while (1) {sleep(1);}
	while (1) {

		audio_node_t* audio = list_del_audio(NULL);
		alsa_pcm_play_8000_1(audio->data, audio->size);



		//ret = recv(client, recv_data+length, sizeof(recv_data)-length, 0);
		//length += ret;
		//printf("recv ret = %d, length = %d\n", ret, length);
		//while ( (ret = packet_jt1078_parse(recv_data, length, &head, &stream)) > 0)
		//{
		//	printf("ret = %d, head.bodylen = %d\n", ret, head.bodylen);
		//	alaw_to_pcm16(head.bodylen, stream, pcm_data);
		//	alsa_pcm_play_8000_1(pcm_data, head.bodylen*2);
		//	memmove(recv_data, recv_data + ret, length - ret);
		//	length -= ret;

		//}
		//	usleep(1000);
	}

#endif
}

void *vpk_test_play(void* arg)
{

	LOG_D("start play thread!");
	//alsa_device_list();
	//alsa_pcm_setting_8000_1();

	//sleep(1);

	while(1)
	{
		LOG_D("play thread run.");

		alsa_play(arg);

		sleep(2);
	}
}


void alsa_recv(int client)
{
	int ret = 0;
	int length = 0;
	char recv_data[1024];

	static int pcm_size = 0;
	static char pcm_data[9632] = {0};

	unsigned char *stream = NULL;
	stream_header_t head = {0};
	memset(recv_data, 0x00, sizeof(recv_data));

	while (1) {


		//ret = recv(client, recv_data, 830, 0);
		//alaw_to_pcm16(800, recv_data+30, pcm_data);
		//alsa_pcm_play_8000_1(pcm_data, 800*2);
		//memset(pcm_data, 0x00, sizeof(pcm_data));


		ret = recv(client, recv_data+length, sizeof(recv_data)-length, 0);
		length += ret;			
		
		struct timeval now;
		gettimeofday(&now, 0);
		printf("recv ret = %d, length = %d, now(%dus)\n", ret, length, now.tv_usec);

		while ( (ret = packet_jt1078_parse(recv_data, length, &head, &stream)) > 0)
		{
			printf("ret = %d, head.bodylen = %d, pcm_size = %d\n", ret, head.bodylen, pcm_size);
			alaw_to_pcm16(head.bodylen, stream, pcm_data);
			//alsa_pcm_play_8000_1(pcm_data, head.bodylen*2);

				
			audio_node_t* audio = pkt_node_create(pcm_data, period_size*2);
			list_add_audio(NULL, audio);
			
			//audio = pkt_node_create(pcm_data+period_size*2, period_size*2);
			//list_add_audio(NULL, audio);

			memmove(recv_data, recv_data + ret, length - ret);
			length -= ret;

		}
		//	usleep(1000);
	}

}

void *vpk_test_recv(void* arg)
{

	LOG_D("start recv thread!");

	audio_mutex_init();

	//sleep(1);
	while(1)
	{
		LOG_D("recv thread run.");
		//sleep(1);

		alsa_recv(arg);

		sleep(2);
	}
}

int main(int argc, char* argv[])
{
	struct sockaddr_in my_addr;
	int client;
	char encode_buff[1024];
	int ret = 0, avail;
	int err;

    char *str_ip = NULL;
    if (argc > 1)
        str_ip = argv[1];
    else {
        printf("args ip \n");
        fprintf(stderr, "Unable to server with ip null.\n");
        exit(1);
    }

	client = socket(AF_INET, SOCK_STREAM, 0);
	if (client == -1)
	{
		printf("socket error\n");
		return -1;
	}
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	//my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//my_addr.sin_addr.s_addr = inet_addr("172.17.13.222");
	my_addr.sin_addr.s_addr = inet_addr(str_ip);

	if (connect(client, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) 
	{
		printf("connect error\n");
		close(client);
		exit(1);
	}

#if 1

	//signal(SIGINT, alsa_release);
	//signal(SIGTERM, alsa_release);

	printf("start send capture data...\n");


	pcm16_alaw_tableinit();

	alaw_pcm16_tableinit();

	//vpk_test_play(client);
	alsa_pcm_init();

	

#if 0 

	char buf[1024];
	memset(buf, 0, sizeof(buf));

#define BUFSIZE (400 * 1)

	while (1) {
		int avail;
		//if ((err = snd_pcm_wait(playback_handle, 1000)) < 0) {
		if ((err = snd_pcm_wait(playback_handle, 1000)) < 0) {
			fprintf(stderr, "poll failed(%s)\n", strerror(errno));
			break;
		}	     

		avail = snd_pcm_avail_update(capture_handle);
		if (avail > 0) {
			if (avail > BUFSIZE)
				avail = BUFSIZE;

			ret = snd_pcm_readi(capture_handle, buf, avail);
			printf("readi ret = %d\n", ret);
		}

		avail = snd_pcm_avail_update(playback_handle);
		if (avail > 0) {
			if (avail > BUFSIZE)
				avail = BUFSIZE;

			ret = snd_pcm_writei(playback_handle, buf, avail);
			printf("writei ret = %d\n", ret);
		}
	}

#endif   




#if 1
	pthread_t pth_recv, pth_play;
	ret = pthread_create(&pth_play, NULL, vpk_test_play, (void*)client);
	if (ret != 0)
		printf("create thread \'vpk_test_play\' failed\n");


	//while(1)
	//{
	//	sleep(5);
	//}
	ret = pthread_create(&pth_recv, NULL, vpk_test_recv, (void*)client);
	if (ret != 0)
		printf("create thread \'vpk_test_recv\' failed\n");
#endif

//sleep(1);
	char *g711_data = calloc(1, period_size);

	//record_audio_init(8000, 1);

	unsigned int buf_size = period_size * 2;
	char* read_buffer = calloc(1, buf_size * 4);

usleep(100000);
	int frame_size = 0;
	unsigned int wait_time = period_time;

	while (1)
	{
		memset(encode_buff, 0, 1024);

		wait_time = period_time;
read_wait:
		if ((err = snd_pcm_wait(capture_handle, wait_time)) < 0) {
			fprintf(stderr, "poll failed(%s)\n", strerror(errno));
			break;
		}


		pthread_mutex_lock(&voice_mutex);

		avail = snd_pcm_avail_update(capture_handle);
		if (avail > 0) {
			if (avail < period_size) {
			
				pthread_mutex_unlock(&voice_mutex);
				wait_time = 1000;
				goto read_wait;
			}

		pthread_cond_broadcast(&voice_cond);
		
			if (avail > period_size)
				avail = period_size;

			//double elapsed;
			struct timeval prev, next;
			gettimeofday(&prev, 0);

			//ret = snd_pcm_readi(capture_handle, buf, avail);
			ret = snd_pcm_readi(capture_handle, read_buffer, avail);
			gettimeofday(&next, 0);
			printf("readi ret = %d, avail = %d, prev(%dus), next(%dus)\n", ret, avail, prev.tv_usec, next.tv_usec);
			//ret = snd_pcm_readi(capture_handle, priv->buf, frames);
			if (ret == -EPIPE) {
				printf("ca_thread_func: overrun!");
				snd_pcm_prepare (capture_handle);
				continue;

			} else if (ret > 0) {
			//printf("=========readi ret = %d, avail = %d\n", ret, avail);

#if 1
				if (ret != period_size)
					printf("record data size error, ret = %d, avail: %d, period_size: %d", ret, avail, period_size);
				pcm16_to_alaw(/*priv->buf_size*/ret*2, read_buffer, g711_data);
				ret = jt1078_package(encode_buff, g711_data, /*priv->buf_size/2*/ret);
				send(client, encode_buff, ret, 0);
#else
			audio_node_t* audio = pkt_node_create(read_buffer, ret*2);
			list_add_audio(NULL, audio);
#endif

				//frame_size += ret;
				//if (frame_size >= period_size) {
				//	printf("read frame_size = %d\n", frame_size);
				//	frame_size -= period_size;

				//	//if (ret*2 > priv->buf_size)
				//	//	printf("record data size error, ret = %d, avail: %d, buffer size: %d", ret, avail, priv->buf_size);
				//	//pcm16_to_alaw(/*priv->buf_size*/ret*2, priv->buf, g711_data);
				//	//ret = jt1078_package(encode_buff, g711_data, /*priv->buf_size/2*/ret);
				//	//send(client, encode_buff, ret, 0);

				//}


				////ret = jt1078_package(encode_buff, priv->buf, priv->buf_size);
				//if (ret*2 > priv->buf_size)
				//	printf("record data size error, ret = %d, avail: %d, buffer size: %d", ret, avail, priv->buf_size);
				//pcm16_to_alaw(/*priv->buf_size*/ret*2, priv->buf, g711_data);
				//ret = jt1078_package(encode_buff, g711_data, /*priv->buf_size/2*/ret);
				//send(client, encode_buff, ret, 0);

				//printf("snd_pcm_readi ret %d\n", ret);
			} else {
				printf("error snd_pcm_readi ret %d\n", ret);
				//break;
			}

		} else {

			//usleep(period_time);
		}


		pthread_mutex_unlock(&voice_mutex);

		//play_pcm();

		//audio_node_t* audio = list_del_audio(NULL);
		//if (audio) alsa_pcm_play_8000_1(audio->data, audio->size);
	}

	snd_pcm_drain(capture_handle);
	snd_pcm_close(capture_handle);

#elif 0

	vpk_file_open(NULL, "./pcm_8000_1.pcm");
	printf("start send pcm data...\n");

	pcm16_alaw_tableinit();
	char *g711_data = calloc(1, priv->buf_size/2);
	while (1)
	{
		memset(encode_buff, 0, 1024);
		ret = vpk_file_read();
		if (ret == -1) {        
			printf("========== end ==========\n");
			break;        
		}
		//ret = jt1078_package(encode_buff, priv->buf, priv->buf_size);
		pcm16_to_alaw(priv->buf_size, priv->buf, g711_data);
		ret = jt1078_package(encode_buff, g711_data, priv->buf_size/2);
		send(client, encode_buff, ret, 0);
		usleep(5000);
	}


#else

	//    char *sbuff = "hello wow!";
	char sbuff[255];
	char recv_data[255];

	printf("me: ");
	while (fgets(sbuff, sizeof(sbuff), stdin) != NULL)
	{
		memset(encode_buff, 0, 1024);
		ret = jt1078_package(encode_buff, sbuff, strlen(sbuff)-1);
		send(client, encode_buff, ret, 0);
		//send(client, sbuff, strlen(sbuff)-1, 0);
		if (strcmp(sbuff, "exit\n") == 0)
			break;

		//ret = recv(client, recv_data, sizeof(recv_data), 0);
		////fputs(recv_data, stdout);
		//if (ret > 0) {
		//    recv_data[ret] = 0x00;
		//    printf("recv: %s\n", recv_data);
		//}

		memset(sbuff, 0, sizeof(sbuff));
		memset(recv_data, 0, sizeof(recv_data));

		printf("me: ");
	}
#endif

	close(client);
	return 0;
}



