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

void replace_prefix(unsigned char* string)
{
	unsigned char* p=NULL;
	unsigned char tail_part[256]={0};

        //can be only one case

        if(p=strstr(string, VENDOR_TOKEN))
        {
	    strcpy(tail_part,p+11);
            strcpy(p,"data/asan/vendor");
            strcpy(p+16,tail_part);
        }
  
        
        if(p=strstr(string, SYSTEM_TOKEN))
        {
            strcpy(tail_part,p+6);
            strcpy(p,"data/asan/system");
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
  //FILE* fs=fdopen(fd,"r"); 

  int fd1=open("./default_duped.xml",O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR);

  int round=0,ret=0,ret1=0,ret2=0,ret3=0,rets=0;
  if (fd<0||fd1<0)
  {
     printf("error open any fd\n");
     return -1;
  }


    //<x-quic-distributable
    //    package="hy11,hy22,grease"
    //    path="&target-bin-prefix;/system/bin/qdss_qmi_helper" />

//    <x-quic-distributable path="&target-bin-prefix;/obj/include/libmdmdetect/inc/mdm_detect.h" />
  
    // fscanf(fs,"%s\0",buf);
//   printf("read count=%d, buf=%s\n",readline(fd,buf),buf);     
//  return -2; 


 while((ret1=readline(fd,buf))!=-1)
  {
//        printf("ret=%d, buf[0]=%d, readline=%s",ret1,buf[0],buf);//buf contains \n
        printf("%s",buf); 
       //if (ret1==
        if (ret<0)
	{
	       printf("error read fd\n");
	       return -1;
	}
	
        if (!strstr(buf,"<x-quic-distributable"))
          write(fd1,buf,ret1+1);
        else
        if ((p=strstr(buf,"<x-quic-distributable"))&&(*(p+21)==10))//multi-line
        {
           write(fd1,buf,ret1+1);
           while((ret2=readline(fd,bufw[0]))&&!strstr(bufw[0],">"))
           {
		
           strcat(bufs,bufw[0]);
           rets+=ret2;         
           //ret2=readline(fd,bufw[1]);
           //printf("readline=%s", bufw[0]);//buf contains \n
           //ret3=readline(fd,bufw[2]);
           //printf("ret=%d, readline=%s",ret3,bufw[2]);//buf contains \n
	   memset(bufw[0],0,256);
           round++;
	   }
           strcat(bufs,bufw[0]);
           rets+=ret2;
           //printf("%s",bufs);
           //printf("%s",buf);

           write(fd1,bufs,strlen(bufs));

           if (strstr(bufs,SO_TOKEN))
           {
            write(fd1,buf,ret1+1);
            replace_prefix(bufs);
	    write(fd1,bufs,strlen(bufs));
           }
 	   memset(bufs,0,8192);
           rets=0;
#if 0
           write(fd1,buf,ret1+1);
           write(fd1,bufw[1],ret2+1);
           write(fd1,bufw[2],ret3+1);


           write(fd1,buf,ret1+1);
           write(fd1,bufw[1],ret2+1);
           write(fd1,bufw[2],ret3+1);
#endif


           if (ret<0||ret1<0)
  	   {
     		printf("error write any fd\n");
     		return -1;
  	   }
           
        }
        else //1-line
           {
	           write(fd1,buf,ret1+1);
		   if (strstr(buf,SO_TOKEN))
                   {
			//printf("1-line with .so written: %s",buf);
			replace_prefix(buf);  
			//write(fd1,buf,ret1+1);
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
