#include<arpa/inet.h>
#include<sys/epoll.h>
#define S(b) sizeof(b)
#define s struct
#define r typedef s
r sockaddr_in t;r epoll_event z;
int main(){int i,e,n,l,c,o;char b[0xFF]; t y={.sin_family = 2,.sin_port=
htons(0x1F90)} ,m;z x[0xF]; l=socket(2,1,0); bind(l,&y,S(y)); fcntl(l,2,
fcntl(l,1,0) | 04000); listen(l,16); e=epoll_create(1); epoll_ctl(e,1,l,
&((z){.events=2147483653,.data.fd=l})); o=S(m);for(;;){n=epoll_wait(e,x,
32,-1);for(i=0;i<n;i++){if(x[i].data.fd==l){c=accept(l,(s sockaddr *)&m,
&o); inet_ntop(2,&(m.sin_addr),b, S(m)); printf("[+] connected %s:%d\n",
b, ntohs(m.sin_port)); fcntl(c,2,fcntl(c,1,0) | 04000); epoll_ctl(e,1,c,
&((z){.events=2147491857,.data.fd=c}));}else if(x[i].events&0x001){bzero
(b,S(b));if(read(x[i].data.fd,b,S(b))<=0){break;}else{printf("[+] data:"
" %s\n",b);write(x[i].data.fd,b,strlen(b));}}if(x[i].events&8208){printf
("[+] connection closed\n"); epoll_ctl(e, 2, x[i].data.fd,0); close(x[i]
.data.fd);continue;}}}}
