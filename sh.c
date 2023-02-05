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
   int status;
   pid_t pid, wpid;
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
                close(1);   
                open(args[i+1],O_WRONLY | O_CREAT,0777);
                args[i] = NULL;
                execvp(args[0],args);
            }
            else if(strcmp(args[i],"<")==0){
                close(0);
                open(args[i+1],O_RDONLY | O_CREAT,0777);
                execvp(args[0],args+1);
            }
            i+=1;
        }
        if(args[i]==NULL)
            execvp(args[0],args);
    
   } 
   else{
    fprintf(stderr,"fork error\n");
    return -1;
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
  int status;

  do {
    printf("utsh$ ");
    line = sh_read_line();
    args = sh_split_line(line);
    status = sh_execute(args);
    // int i=0;
    // while(args[i]!=NULL){
    //     printf("%s",args[i]);
    //     i+=1;
    // }
    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  sh_loop();
  //printf("hello");
  return EXIT_SUCCESS;
}