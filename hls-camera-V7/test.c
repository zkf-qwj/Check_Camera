#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_TIMEOUT 30

int64_t gettime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

void check(char *dir)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        if(strcmp(".",entry->d_name) == 0 ||
                strcmp("..",entry->d_name) == 0)
                continue;
        if(gettime() - statbuf.st_atime > MAX_TIMEOUT )
        {
            printf("file:%s,access time:%d,cur_time:%lu\n",entry->d_name,statbuf.st_atime,gettime());
        }
        
    }
    chdir("..");
    closedir(dp);
}

int main(int argc, char *argv[])
{
    char *path = argv[1];
    check(path);
}
