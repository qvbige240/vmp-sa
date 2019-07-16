#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//#define PORT  25341
#define PORT 9001
//#define PORT 4369

int jt1078_package(char *buffer, const char *data, unsigned short len)
{
	char recv_data[255];
	int ret = 0, i = 0;

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
// ./remote_play.demo 127.0.0.1 pcm_8000_1.pcm
int main(int argc, char* argv[])
{
    int client;
    int ret = 0;
    char encode_buff[1024];
    struct sockaddr_in my_addr;

    char *str_ip = NULL;
    if (argc > 2) {
        str_ip = argv[1];
        //vpk_file_open(NULL, "./pcm_8000_1.pcm");
        vpk_file_open(NULL, argv[2]);
    } else {
        printf("args need server ip and pcm file  \n");
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
        usleep(50000);
    }
    

#else
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



