extern "C" {
#include "fdevent.h"
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
}
#include <iostream>
#include <vector>
#include "check_camera.h"
#define MAX_TIMEOUT  30

Check_Camera *obj;//zlj addd 0711

static int g_balanceNum = 0;
static char g_balanceHost[64][32]={"0"};

enum CONN_STATUE
{
	CONNING = 0,
	CONNTED = 1,
	DISCONN = 2            
};
static handler_t Balance_recv_handle(void *s,void *ctx,int revents);

int64_t gettime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (int64_t)tv.tv_sec ;
}
Check_Camera::Check_Camera()
{
	ev = fdevent_init(1024, FDEVENT_HANDLER_LINUX_SYSEPOLL);    
	polltimeout =1;

	conn_statu = DISCONN;
}

Check_Camera::~Check_Camera()
{
	UnInit();
	fdevent_free(ev);
}
void Check_Camera::Init()
{
	sock = iosocket_init();
}

void Check_Camera::UnInit()
{
	iosocket_free(sock);
}


handler_t  balance_conn( struct sockaddr_in*addr,int *pfd)
{
	int fd =-1;
	if (-1 == (fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
		switch (errno) {
			case EMFILE:
				return HANDLER_WAIT_FOR_FD;
			default:
				return HANDLER_ERROR;
		}
	}

	(*pfd) = fd;
	fcntl(fd, F_SETFL, O_NONBLOCK | O_RDWR);
	if (-1 == connect(fd, (struct sockaddr *)addr, sizeof(struct sockaddr) )) {
		switch(errno) {
			case EINPROGRESS:
			case EALREADY:
			case EINTR:
				return HANDLER_WAIT_FOR_EVENT;
			default:
				close(fd);
				return HANDLER_ERROR;
		}
	}

	return HANDLER_GO_ON;
}


void Check_Camera::Trigger()
{
	//std::cout << "Entry Trigger"<<std::endl;
	switch(conn_statu)
	{

		case DISCONN:
			UnInit();
			Init();
			switch (balance_conn(&servaddr,&sock->fd))
			{
				case HANDLER_WAIT_FOR_FD:
					std::cout<<"HANDLER_WAIT_FOR_FD"<<std::endl;
					break;
				case HANDLER_WAIT_FOR_EVENT:  
					std::cout<<"***connecting,fdevent_register,sock->fd:"<<sock->fd <<std::endl;
					fdevent_register(ev, sock, Balance_recv_handle,NULL);
					fdevent_event_add(ev, sock, FDEVENT_IN | FDEVENT_HUP);                            
					conn_statu = CONNTED;   
					break; 
				case HANDLER_GO_ON: 
					std::cout<<"connection established"<<std::endl; 
					break;
				default:
					std::cout<<"connection error "<<std::endl;
					fresh_server_addr();
					break;
			}
			break;
		default:
			break;    
	} 
}

void del_dir(char *npath)
{
	char pathname[256]={'\0'};
	struct dirent *ent =NULL;
	struct stat statbuf;
	DIR * ptr;
	DIR *dpin;
	if ((ptr = opendir(npath)) == NULL){
		std::cout<<"cannot open del_dir_directory:"<<npath<<std::endl;
		return ;	
	}
	//	chmod(npath, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH|S_IXGRP|S_IXUSR);

	while((ent = readdir(ptr)) != NULL)
	{

		std::cout<<"ent->d_name:"<<ent->d_name<<std::endl;
		if(strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0)
			continue;

		lstat(ent->d_name,&statbuf);

		strcpy(pathname,npath);
		strcat(pathname,"/");
		strcat(pathname,ent->d_name);
		if(!S_ISDIR(statbuf.st_mode))
		{
			remove(pathname);
		}else{
			del_dir(pathname);
			remove(pathname);

		}
		if(rmdir(npath) == 0){
			std::cout<<"this is file dir del................. "<<npath<<std::endl;
		}
	}
}

//void check_dir(char *path,std::vector <std::string> *vec)
void check_dir(char *path)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	if((dp = opendir(path)) == NULL) {
		std::cout<<"cannot open directory:"<<path<<std::endl;
		return;
	}
	chdir(path);
	while((entry = readdir(dp)) != NULL) {
		std::cout<<"file_name: "<<entry->d_name<<std::endl;
		lstat(entry->d_name,&statbuf);
		if(strcmp(".",entry->d_name) == 0 ||
				strcmp("..",entry->d_name) == 0)
			continue;
		std::cout<<"cur file:"<<entry->d_name<<" access time:"<<statbuf.st_atime<<" cur_time:"<<gettime()<<std::endl;
		if(gettime() - statbuf.st_atime > MAX_TIMEOUT )
		{
			std::cout<<"file:"<<entry->d_name<<" access time:"<<statbuf.st_atime<<" cur_time:"<<gettime()<<std::endl;
			/*	if(S_ISDIR(statbuf.st_mode))
				{
				std::string node(entry->d_name);
			// vec->push_back(node);                
			del_dir(entry->d_name); 
			std::cout<<"delete file : " << entry->d_name << std::endl;
			remove(entry->d_name);
			char temp_path[1024]={'\0'};
			sprintf(temp_path,"%s/%s","/usr/local/nginx/html",entry->d_name);
			del_dir(temp_path); 
			}*/
			if(S_ISREG(statbuf.st_mode)){
				remove(entry->d_name);
				std::cout<<"rm reg file: " <<entry->d_name<<std::endl;
				break;
			}
			char path_old[512] = {0};
			char falg = 0;
			strcpy(path_old,obj->check_path);
			if (strcmp(path,obj->check_path) == 0){//防止删除/dev/shm/live目录
				sprintf(path,"%s/%s",path,entry->d_name);
				std::cout<<"path is ------:" <<path<<std::endl;	
				falg = 1;	
			}
			if( rmdir(path) ==0 )
			{
				std::cout<<"1delete:" <<path<<std::endl;		
				char temp_path[512]={'\0'};
				char temp_path_1[512] = {'\0'};
				char *p;
				p = rindex(path,'/');
				std::cout<<"p:" <<p<<std::endl;	
				//if(strcmp(p,"live")) return ;
				sprintf(temp_path,"%s%s","/usr/local/nginx/html",p);
				if (falg){
					strcpy(path,path_old);
					falg = 0;
				}
				std::cout<<"2delete:" <<temp_path<<std::endl;	
				
				sprintf(temp_path_1,"rm -rf %s",temp_path);
				int status = system(temp_path_1);
				if (status < 0){
					printf("cmd: %s\t error: %s", temp_path_1, strerror(errno));
				}
				system(temp_path_1);
				
				std::cout<<"del system:" <<temp_path_1<<std::endl;	

				del_dir(temp_path);
			}else{
				std::cout<<"rmdir() faild:"<<path<<std::endl;
			}
		}else if(S_ISDIR(statbuf.st_mode))
		{
			char temp_path[1024]={'\0'};
			sprintf(temp_path,"%s/%s",path,entry->d_name);
			check_dir(temp_path);
		}        
	}
	chdir("..");
	closedir(dp); 
}


void Check_Camera::Check()
{
	std::cout<<"Entry Check" <<std::endl;

	//std::vector <std::string> vec;
	//check_dir(check_path,&vec);
	check_dir(check_path);
	/* */ 
	   char temp[MAX_BUFFER_LEN];
	   char *p_temp = temp;
	   int buf_len =0,node_len=0;
	   for(std::vector<std::string>::iterator it  = vec.begin(); it != vec.end();it++)  
	   {
	   Json::Value root;
	   Json::FastWriter fter;
	   root["flag"] = "StopStream";	
	   root["DeviceId"] = (*it).c_str();
	   root["DeviceType"]=2;
	   node_len = strlen(fter.write(root).c_str());

	   if(buf_len + node_len +4 > MAX_BUFFER_LEN)
	   {
	   if( send(sock->fd,temp,buf_len,0) == buf_len)
	   {
	   memset(temp,0,MAX_BUFFER_LEN);
	   p_temp = temp;
	   buf_len =0;
	   }else
	   {
	   std::cout<<"send failed"<<std::endl;
	   }
	   }


	   memcpy(p_temp,&node_len,4);
	   p_temp +=4;
	   memcpy(p_temp,fter.write(root).c_str(),node_len);
	   p_temp += node_len;
	   buf_len += node_len +4;                
	   } 

	   if(buf_len < MAX_BUFFER_LEN && buf_len >0)
	   {
	   if( send(sock->fd,temp,buf_len,0) == buf_len)
	   {
	   std::cout<<"send success,buf_len:"<<buf_len<<std::endl;
	   }else
	   std::cout <<"send failed"<<std::endl;
	   }

	   vec.clear(); 
	     
}


void Check_Camera::Entry()
{

	fdevent_revents *revents = fdevent_revents_init();
	int poll_errno;
	int n;

	int64_t prevTs = gettime(), curTs;
	int64_t prevCTs =gettime();
	while(true)
	{        
		n = fdevent_poll(ev, this->polltimeout*1000);         
		poll_errno = errno;

		if (n > 0) 
		{
			size_t i;
			fdevent_get_revents(ev, n, revents);           
			for (i = 0; i < revents->used; i++) 
			{
				fdevent_revent *revent = revents->ptr[i];
				handler_t r;      

				switch (r = (*(revent->handler))(this, revent->context, revent->revents)) 
				{
					case HANDLER_FINISHED:
					case HANDLER_GO_ON:
					case HANDLER_WAIT_FOR_EVENT:
						break;
					case HANDLER_ERROR:
						/* should never happen */
						//Assert(false);
						break;
					default:
						std::cout<<"event loop returned:"<<(int) r<<std::endl;
						break;
				}
			}
		} 
		else if (n < 0 && poll_errno != EINTR) 
		{
			std::cout<<"event loop: fdevent_poll failed:"<<strerror(poll_errno)<<std::endl; 

		}


		curTs = gettime();
		/*
		   if(curTs >= prevTs + this->polltimeout ) 
		   {
		   Trigger();
		   prevTs = curTs;
		   }*/      

		if(curTs >= prevCTs + this->checktimeout)
		{
			Check();
			prevCTs = curTs;
		}

	}
	fdevent_revents_free(revents);     
}

handler_t Balance_recv_handle(void *s,void *ctx,int revents)
{
	Check_Camera *sobj = (Check_Camera *)s;
	// res_data_t *context = (res_data_t*)ctx;

	int ret =0;
	if (revents & FDEVENT_IN)
	{
		memset(sobj->recvBuf,0,MAX_BUFFER_LEN);
		ret = recv(sobj->sock->fd,sobj->recvBuf,MAX_BUFFER_LEN,0);
		if(ret >0)
		{
			std::cout<<"recv:"<<sobj->recvBuf<<std::endl;
		}
	}

	if((ret <=0) || (revents & FDEVENT_HUP) || (revents & FDEVENT_ERR))
	{
		std::cout <<"connet to balance failed" <<std::endl;
		fdevent_event_del(sobj->ev, sobj->sock);        
		fdevent_unregister(sobj->ev, sobj->sock);
		sobj->conn_statu = DISCONN;  
		sobj->fresh_server_addr();
	}
	return HANDLER_GO_ON;
}

void Check_Camera:: fresh_server_addr()
{
	if(g_balanceNum <=1)
		return ;
	int i=0;
	char host[32]={'\0'};

	char *p_port = NULL;
	int port = 0;
	for(i=0;i<g_balanceNum;i++)
	{
		char *IpAddr = g_balanceHost[i];
		if(strstr(IpAddr,server_ip))
		{
			continue;
		}
		p_port = strstr(IpAddr,":");
		if(p_port)
		{
			memcpy(host,IpAddr,p_port -IpAddr );
			p_port += 1;
			port = atoi(p_port);
		}        
	}
	if(strlen(host) && port >0)
	{
		std::cout<<"new host:"<<host<<",port:"<<port<<std::endl;
		servaddr.sin_addr.s_addr = inet_addr(host);
		servaddr.sin_port = htons(port);
		strcpy(server_ip,host);

	}
}

void get_host_port(char *IpAddr,char *host,char*port)
{
	char *p_port = NULL;
	p_port = strstr(IpAddr,":");
	if(p_port)
	{
		memcpy(host,IpAddr,p_port -IpAddr );
		p_port += 1;
		strcpy(port,p_port);
	}        
}

int GetBalance(char *host)
{
	char delims[] = "#";
	char *result = NULL;
	result = strtok( host, delims );   	

	while( result != NULL ) {

		strcpy(g_balanceHost[g_balanceNum],result);
		g_balanceNum ++;		
		result = strtok( NULL, delims );
	} 
}


bool Check_Camera::Configure(char * pBindHost ,char*port,char *path,int timeout)
{
	int i=0;
	bzero( (void *)&servaddr, sizeof(servaddr) );
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(port));
	servaddr.sin_addr.s_addr = inet_addr(pBindHost);    
	strncpy(server_ip,pBindHost,sizeof(server_ip));
	strcpy(check_path,path);
	if(timeout >0)
		checktimeout = timeout;
	return true;
}
void del_hls_dir(const char *path)
{
	char *child_dir;
	char *rm;
	struct dirent *entry;
	DIR *dp;
	DIR*dp1;
	child_dir = (char *)malloc(sizeof(char)*100);
	rm = (char *)malloc(sizeof(char)*100);
	if((dp = opendir(path)) == NULL) {
		printf("open not dir %s\n",path);
		return;
	}
	while((entry = readdir(dp)) != NULL){
		if ((strcmp(entry->d_name,".") == 0) || (strcmp(entry->d_name,"..") == 0)){
			continue;
		}
		sprintf(child_dir,"%s/%s",path,entry->d_name);
		if((dp1 = opendir(child_dir)) != NULL){
			sprintf(rm,"rm -rf %s/%s",path,entry->d_name);
			 system(rm);
		printf("fist del:  %s\n",rm);
		}
	}
	free(child_dir);
	free(rm);
	closedir(dp);
	return;
}

int main(int argc,char *argv[])
{
	char BackEnd[MAX_BUFFER_LEN]={'\0'};
	char path[BUF_LEN]={'\0'};
	char m3u8_path[BUF_LEN]={'\0'};
	int  check_interval =0;
	int ch; 
	while( (ch  =  getopt( argc, argv, ":p:b:t:"))!=  -1){
		switch(ch){
			case 'p':
				strncpy(path,optarg,BUF_LEN);
				break;
			case 'b':
				strncpy(BackEnd,optarg,MAX_BUFFER_LEN);
				break;
			case 't':
				check_interval = atoi(optarg);
				break;
		}
	}
	if(strlen(BackEnd))
	{
		GetBalance(BackEnd);
	}
	//del_hls_dir("/usr/local/nginx/html");
	if(g_balanceNum >0)
	{
		obj = new Check_Camera();
		char host[16]={'0'};
		char port[16] ={'\0'};
		get_host_port(g_balanceHost[0],host,port);
		obj->Configure(host,port,path,check_interval);
		obj->Entry();
	}
}
