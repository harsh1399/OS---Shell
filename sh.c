#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include<dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"

int sh_launch(char **args){
   int status,pipe_index = 0,command_index = 0,pipe_status,flag=0,read_end;
   pid_t pid, wpid;
   int p[2],pip[2],previous_read_descriptor = 0;
   pid_t pipe_pid;
  //  char **hist[100];
  //  **hist[0] = args;
   if(strcmp(args[0],"cd")==0){
    chdir(args[1]);
    return 1;
   }
   else if(strcmp(args[0],"history")==0){

   }
   else{
    pid = fork();
    if(pid>0){
        pid = wait(&status);
        //printf("returned to parent process\n");  
        return 1;
    }
    else if(pid == 0){
          int i = 0;
          while(args[i]!=NULL){
              if(strcmp(args[i],">")==0){
                  int desc = open(args[i-1],O_RDONLY);
                  if(desc!=-1){
                    dup2(desc,0);
                  }else{
                    dup2(previous_read_descriptor,0);
                  }
                  close(1);   
                  open(args[i+1],O_WRONLY | O_CREAT,0777);
                  args[i] = NULL;
                  execvp(args[command_index],args+command_index);
              }
              else if(strcmp(args[i],"<")==0){
                  close(0);
                  open(args[i+1],O_RDONLY | O_CREAT,0777);
                  args[i] = NULL;
                  if(args[i+2]!=NULL && strcmp(args[i+2],">")==0){
                    close(1);
                    open(args[i+3],O_WRONLY | O_CREAT,0777);
                    i = i+3;
                  }
                  else if(args[i+2]!=NULL && strcmp(args[i+2],"|")==0){
                    flag = 1;
                    pipe(pip);
                    i = i+2;
                    int pid = fork();
                    if(pid==0){
                      dup2(pip[1],1);
                      close(pip[0]);
                      execvp(args[command_index],args+command_index);
                      exit(1);
                    }
                    else if(pid>0){
                      wait(&status);
                    }
                  }
                  if(flag!=1)
                    execvp(args[command_index],args+command_index);
              }
              else if(strcmp(args[i],"|")==0){
                pipe(p);
                pipe_pid = fork();
                if(pipe_pid == 0){
                  dup2(previous_read_descriptor,0);
                  dup2(p[1],1);
                  close(p[0]);
                  args[i] = NULL;
                  execvp(args[command_index],args+command_index);
                  exit(1);
                }
                else if(pipe_pid>0)
                {
                  wait(&pipe_status);
                  //printf("in parent of pipe\n");
                  close(p[1]);
                  previous_read_descriptor = p[0];
                  command_index = i+1;
                }
              }
              i+=1;
          }
          //indexes[number_of_commands] = NULL;
          if(args[i]==NULL){
            //dup2(stdout,1);
              if(flag==1){
                dup2(pip[0],0);
                close(pip[0]);
                close(pip[1]);
                execvp(args[i-1],args+i-1);  
              }
              dup2(previous_read_descriptor,0);
              execvp(args[command_index],args+command_index);
            
          }
    } 
    else{
      fprintf(stderr,"fork error\n");
      return -1;
    }
   }
}

int sh_execute(char **args)
{
  if (args[0] == NULL) {
    return 1;  // An empty command was entered.
  }
  return sh_launch(args);   // launch
}

char **sh_split_line(char *line)
{
  int bufsize = SH_TOK_BUFSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    // printf("%s\n",*(tokens+position));
    // //printf("\n");
    // printf("%d\n",*(*(tokens+position)));
    position++;

    if (position >= bufsize) {
      bufsize += SH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

char *sh_read_line(void)
{
  char *line = NULL;
  size_t bufsize = 0;  // have getline allocate a buffer for us

  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin))  // EOF@
    {
      fprintf(stderr, "EOF\n");
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "Value of errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }
  }
  return line;
}


void sh_loop(void)
{
  char *line;
  char **args;
  int status=1;
  //char *hist[100]={NULL};
  int i=0;
  char *hist[100] = {NULL};
  int count = 0;
  do {
    printf("utsh$ ");
    line = sh_read_line();
    hist[count] = strdup(line);
    args = sh_split_line(line);
    if(strcmp(*args,"history")==0){
      int number = 0;
      while(hist[number]!=NULL){
        char *arg = hist[number];
        int len = strlen(arg);
        printf("%d ",number+1);
        int i=0;
        while(*(arg+i)!='\0'){
          printf("%c",*(arg+i));
          i++;
        }
        printf("\n");
        number += 1;
      }
    }
    else if(strstr(hist[count],"&")!=NULL){
      int status;
      int pid = fork();
      if(pid==0){
        int i=0;
        while(args[i]!=NULL){
          if(strcmp(args[i],"&")==0){
            args[i] = NULL;
          }
          i++;
        }
        sh_execute(args);
        exit(1);
      }
      else if(pid>0){
        waitpid(0,&status,WNOHANG);
      }
      else{

      }
    }
    else if(strstr(hist[count],";")!=NULL){
      int i=0;
      int command_index = 0;
      while(args[i]!=NULL){
        if(strcmp(args[i],";")==0){
          args[i] = NULL;
          sh_execute(args+command_index);
          command_index = i+1;
        }
        i++;
      }
      sh_execute(args+command_index);
    }
    else
      status = sh_execute(args);

    free(line);
    free(args);
    count++;
  } while (status);
}

int main(int argc, char **argv)
{
  sh_loop();
  return EXIT_SUCCESS;
}