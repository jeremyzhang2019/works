#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<sys/wait.h>

#include "socket.h"
#include "jobprotocol.h"
// #ifndef SERVER
    #define SERVER "[SERVER]"

#define QUEUE_LENGTH 5

#ifndef JOBS_DIR
    #define JOBS_DIR "jobs/"
#endif

struct jobs{
  char* command;
  int sock_fd;

};
struct task{
  int pid;
  struct task *next;
};

int main(void) {
    // This line causes stdout and stderr not to be buffered.
    // Don't change this! Necessary for autotesting.
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    struct sockaddr_in *self = init_server_addr(PORT);
    int listenfd = setup_server_socket(self, QUEUE_LENGTH);

    struct jobs joblines[QUEUE_LENGTH];
    for(int index = 0;index<QUEUE_LENGTH;index++){
           joblines[index].sock_fd = -1;
           joblines[index].command = NULL;
    }

    struct task *head = malloc(sizeof(struct task));
    head->pid = 0;
    head->next = NULL;

int max_fd = listenfd;
fd_set all_fds, listen_fds;
FD_ZERO(&all_fds);
FD_SET(listenfd,&all_fds);

while(1){
        listen_fds = all_fds;
        int ready = select(max_fd+1,&listen_fds,NULL,NULL,NULL);
        if(ready==-1){
          perror("select");
          exit(1);
        }


        if(FD_ISSET(listenfd,&listen_fds)){
        //accept_connection
        int client_socket = accept_connection(listenfd);
        for(int i=0;i<QUEUE_LENGTH;i++){
          if(joblines[i].sock_fd == -1){
            joblines[i].sock_fd = client_socket;
            break;
          }
        }
        if(client_socket<0){
          continue;
        }
        if(client_socket>max_fd){
          max_fd = client_socket;
        }
        FD_SET(client_socket,&all_fds);
        printf("Accepted connection\n");
        }

//connection established and do jobs for clients




        char *commad = malloc(BUFSIZE);
        for(int i= 0 ;i<QUEUE_LENGTH;i++){
        if(joblines[i].sock_fd != -1 && FD_ISSET(joblines[i].sock_fd,&listen_fds)){
        int client_socket = joblines[i].sock_fd;
        int read_num = read(client_socket,commad,BUFSIZE);
        if(read_num<0){
          perror("read");
          exit(1);
        }
        //selected and test commands
        //commands
        commad[read_num] = '\0';
        char* job = "jobs\n";
        char* run = "run ";
        // char* kill = "kill";
        // char* watch = "watch";
        // char* exit1 = "exit";
        // printf("processed comparement\n");
        if(strcmp(commad,job)==0){
          //client typed jobs;
          char buf[BUFSIZE];
        //job list maintain 
        struct tast* t1 = head;
        int status;
        while(head->next!=NULL){
            if(waitpid(head->next->pid,&status,WNOHANG)!= 0){
              head->next = head->next->next;
            }
            else{
              head = head->next;
            }

        }
        head = t1;





        if(head->pid == 0){
            snprintf(buf,BUFSIZE,"%s No currently running jobs\n",SERVER);
            int num_written = write(client_socket,buf,strlen(buf));
            if(num_written<0){
              perror("write");
              exit(1);
            }

        }
        else{
          snprintf(buf,BUFSIZE,"%s",SERVER);
          int num_written = write(client_socket,buf,strlen(buf));
          if(num_written<0){
            perror("write");
            exit(1);
           }
           struct task * temp = head;
           memset(&buf[0], 0, sizeof(buf));
           while(temp->next != NULL){
               snprintf(buf,BUFSIZE," %d",temp->next->pid);
               int num_written = write(client_socket,buf,strlen(buf));
               if(num_written<0){
                 perror("write");
                 exit(1);
               }
               memset(&buf[0], 0, sizeof(buf));
               temp = temp->next;

           }
         }
//jobs finished
   }
// do run JOBS_DIR
if(strncmp(run,commad,4)==0){
int f = fork();
struct task* curr = malloc(sizeof(struct task));
curr->pid = f;
curr->next = NULL;

struct task * temp = head;
while(head->next!=NULL){
  head = head->next;
}
head->next = curr;
head = temp;



if(f<0){
  perror("fork");
  exit(1);
}
if(f>0){
  continue;
}
if(f == 0){
  int fd[2];
  if(pipe(fd)<0){
    perror("pipe");
    exit(1);
  }

  char job_name[BUFSIZE];
  char job[BUFSIZE];
  char args[BUFSIZE];
  memset(&job_name[0], 0, sizeof(job_name));
  memset(&job[0], 0, sizeof(job));
  memset(&args[0], 0, sizeof(args));
  // printf("commmad is %s\n",commad);
  for(int i=4;i<strlen(commad);i++){
     job_name[i-4] = commad[i];
     if(commad[i] == '\0'){break;}
  }
int flag = 0;
for(int i=0;i<strlen(job_name);i++){
  if(job_name[i] ==' '){
    job[i] = '\0';
    flag = i;
    break;
  }else
   job[i] = job_name[i];
}

for(int k = flag+1;k<strlen(job_name);k++){
  if(job_name[k]=='\n'){
      args[k-(flag+1)] = '\0';
      break;
  }
  args[k-(flag+1)]=job_name[k];
  if(job_name[k]=='\0'){
    break;
  }
}

int f2 = fork();



if(f2<0){
  perror("fork");
  exit(1);
}

if(f2==0){
  if(close(fd[0])<0){
    perror("close");
    exit(1);
  }
  char exe_file[BUFSIZE];
  sprintf(exe_file, "%s/%s", JOBS_DIR, job);
  dup2(fd[1],STDOUT_FILENO);
  dup2(fd[1],STDERR_FILENO);
  if(execl(exe_file,job,args,NULL)<0){
    perror("execl");
    exit(1);
  }
  if(close(fd[1])<0){
    perror("close");
    exit(1);
  }
  return 0;

}
if(f2>0){
  close(fd[1]);
  char buf2[BUFSIZE]={'\0'};
  sprintf(buf2, "%s job %d created\n", SERVER, f2);
  int writnum = write(fd[1],buf2,BUFSIZE);
  char result[BUFSIZE]={'\0'};
  while(1){
  int num_read = read(fd[0],result,BUFSIZE);
  if(num_read<0){
    perror("read");
    exit(1);
  }
  int num_written = write(client_socket,result,strlen(result));
  memset(&result[0], 0, sizeof(result));
  if(num_read == 0){break;}
  if(num_written<0){
    perror("write");
    exit(1);
  }
  }
  wait(0);
  close(fd[0]);
  return 0;



}





}














}





      }
    }
  }
// #######################################################################
//
// #######################################################################




    /* TODO: Initialize job and client tracking structures, start accepting
     * connections. Listen for messages from both clients and jobs. Execute
     * client commands if properly formatted. Forward messages from jobs
     * to appropriate clients. Tear down cleanly.
     */

    /* Here is a snippet of code to create the name of an executable to execute:
     * char exe_file[BUFSIZE];
     * sprintf(exe_file, "%s/%s", JOBS_DIR, <job_name>);
     */


    free(self);
    close(listenfd);
    return 0;
}
