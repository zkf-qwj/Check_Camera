#ifndef CHECK_CAMERA_H
#define CHECK_CAMERA_H
#define MAX_BUFFER_LEN 4096
#define BUF_LEN 1024
extern "C"
{
    #include "iosocket.h"
    #include <sys/socket.h>
    #include <netinet/in.h>
}
class Check_Camera
{
public: 
    Check_Camera();
    ~Check_Camera();
    bool Configure(char * pBindHost ,char*port,char *path,int timeout);
    int conn_statu;
    char recvBuf[MAX_BUFFER_LEN];    
    int recv_len;
    void Entry();
    fdevents *ev;
    iosocket *sock;
    void fresh_server_addr();
private:
    struct sockaddr_in servaddr;
    void Trigger();
    
    void Check();
     
    int polltimeout; 
    int checktimeout;
    void Init();
    void UnInit();
    char server_ip[BUF_LEN];
    
    
    char check_path[1024];
    char check_m_path[1024];
};
#endif
