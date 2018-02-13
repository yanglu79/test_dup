#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//ssize_t read(int fd, void *buf, size_t count);

#include <string.h>
//buggy on more-than-3-line #define SO_TOKEN ".so\" />"
#define SO_TOKEN ".so\""
#define VENDOR_TOKEN "&partition;"
#define SYSTEM_TOKEN "system"
#define ETC_TOKEN "/etc"
#define LIB_TOKEN "/lib"
#define ASAN_VENDOR_TOKEN "data/asan/vendor"
#define ASAN_SYSTEM_TOKEN "data/asan/system"
#define MAIN_DIST_TOKEN "<x-quic-distributable"
#define PACKAGE_TOKEN "package="
#define HY22_TOKEN "\"hy22\""


void replace_prefix(unsigned char* string)
{
    unsigned char* p=NULL;
    unsigned char tail_part[256]={0};

    //can be only one case

    if(p=strstr(string, VENDOR_TOKEN))
    {
        strcpy(tail_part,p+11);
        strcpy(p,ASAN_VENDOR_TOKEN);
        strcpy(p+16,tail_part);
    }
    else if(p=strstr(string, SYSTEM_TOKEN))
    {
        strcpy(tail_part,p+6);
        strcpy(p,ASAN_SYSTEM_TOKEN);
        strcpy(p+16,tail_part);
    }

    return;
}

int readline(int fd, unsigned char* buf)
{
    int i=0,r=0;
    unsigned char bufc[1]={0};
    while((r=read(fd,bufc,1))&&(bufc[0]!=10))
    {
       //printf("bufc=%d ", bufc[0]);
        buf[i++]=bufc[0];
    }
    buf[i]=10;

    if (r==0)
        return -1;

    return i;//the actual(non-new-line) number of char; 0 means blank/new line; -1 means read file's end
}


int main()
{
  unsigned char* p=NULL;
  unsigned char buf[256]={0};
  unsigned char bufw[3][256]={{0,0},{0,0},{0,0}};
  unsigned char bufs[8192]={0};
  int fd=open("./default.xml",O_RDONLY);

  int fd1=open("./default_hy11_asan_gen.xml",O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);

  int round=0,ret=0,ret1=0,ret2=0,ret3=0,rets=0;
  if (fd<0||fd1<0)
  {
     printf("error open any fd\n");
     return -1;
  }
 
  while((ret1=readline(fd,buf))!=-1)
  {
//        printf("ret=%d, buf[0]=%d, readline=%s",ret1,buf[0],buf);//buf contains \n
        printf("%s",buf); 
        if (ret<0)
        {
          printf("error read fd\n");
          return -1;
        }
	
        if (!strstr(buf,MAIN_DIST_TOKEN))
          write(fd1,buf,ret1+1);
        else
        if ((p=strstr(buf,MAIN_DIST_TOKEN))&&(*(p+21)==10)&&(!strstr(buf,ETC_TOKEN)))//multi-line
        {
           write(fd1,buf,ret1+1);
           while((ret2=readline(fd,bufw[0]))&&!strstr(bufw[0],">"))
           {
		
            strcat(bufs,bufw[0]);
            rets+=ret2;         
            //printf("readline=%s", bufw[0]);//buf contains \n
            //printf("ret=%d, readline=%s",ret3,bufw[2]);//buf contains \n
            memset(bufw[0],0,256);
            round++;
           }
            strcat(bufs,bufw[0]);
            rets+=ret2;
            //printf("%s",bufs);
            //printf("%s",buf);
           
           write(fd1,bufs,strlen(bufs));//original

           if (strstr(bufs,SO_TOKEN)&&(!strstr(bufs,ETC_TOKEN)))
             if (strstr(bufs,VENDOR_TOKEN)&&(!strncmp(strstr(bufs,VENDOR_TOKEN)+11,LIB_TOKEN,4))&&((!strstr(bufs,PACKAGE_TOKEN HY22_TOKEN)))&&strstr(bufs,PACKAGE_TOKEN))
             {
                write(fd1,buf,ret1+1);
                replace_prefix(bufs);
write(fd1,bufs,strlen(bufs));
             }
             else if ((!strstr(bufs,VENDOR_TOKEN))&&(!strstr(bufs,PACKAGE_TOKEN HY22_TOKEN))&&strstr(bufs,PACKAGE_TOKEN))
                  {
                    write(fd1,buf,ret1+1);
                    replace_prefix(bufs);
                    write(fd1,bufs,strlen(bufs));
                  }
           
           memset(bufs,0,8192);
           rets=0;

           if (ret<0||ret1<0)
           {
             printf("error write any fd\n");
             return -1;
           }
          
        }
        else //1-line create and dup,replace
        {
          write(fd1,buf,ret1+1);
          if (strstr(buf,SO_TOKEN)&&(!strstr(buf,ETC_TOKEN))&&(!strstr(buf,PACKAGE_TOKEN HY22_TOKEN))&&strstr(buf,PACKAGE_TOKEN))
          {
             //printf("1-line with .so written: %s",buf);
             replace_prefix(buf);  
             write(fd1,buf,strlen(buf));
          }	
          if (ret<0||ret1<0)
          {
             printf("error write any fd\n");
             return -1;
          }
        }
        
          
      memset(buf,0,256);
      memset(bufw,0,256*3);  
  }

  return 0;  


}
