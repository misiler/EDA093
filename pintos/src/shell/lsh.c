
/* WEEE EEEEEEEE
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Submit the entire lab1 folder as a tar archive (.tgz).
 * Command to create submission archive: 
      $> tar cvf lab1.tgz lab1/
 *
 * All the best 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
void ExecuteCommand(Command *, Pgm *);

/* When non-zero, this global means the user is done using this program. */
int done = 0;

/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
  Command cmd;
  Command *cmdp;
  cmdp = &cmd;
  int n;
  char **pl;

  signal(SIGINT, SIG_IGN);

  while (!done) {

    char *line;
    line = readline("> ");

    if (!line) {
      /* Encountered EOF at top level */
      done = 1;
    }
    else {
      /*
       * Remove leading and trailing whitespace from the line
       * Then, if there is anything left, add it to the history list
       * and execute it.
       */
      stripwhite(line);

      if(*line) {
        add_history(line);
        /* execute it */
        n = parse(line, &cmd);
        PrintCommand(n, &cmd);
        pl = cmdp->pgm->pgmlist;
	if (!strcmp(*pl++, "cd")){
	  chdir(*pl);
        } else if (!strcmp(*cmdp->pgm->pgmlist,"exit")) {
          exit(0);
        } else {
         ExecuteCommand(&cmd, cmdp->pgm);
        }
      }
    }
    if(line) {
      free(line);
    }
  }
  return 0;
}


void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}

/*
* Name: ExecuteCommand
*
* Desc: Executes a given command by searching path.
*
*/
void
ExecuteCommand(Command *cmd, Pgm *p)
{
if(p == NULL){
        return;
}else{
 char **pl = p->pgmlist;
//recursice call:
ExecuteCommand(cmd, p->next);

 pid_t pid;
 int p[2]; //Pipe, Read=0, Write=1
 // Create the pipe, with error-checking
 if (pipe(p) == -1) {
        fprintf(stderr,"Pipe failed \n");
        return;
 }

 pid = fork();
 if (pid == 0){
  /*child process*/
  char *argv[20];
  int i = 0;
  close(p[0]);
  dup2(p[1],1);
  if (cmd->rstdout != NULL) {
   /*Write to file if > is used*/
   int fd = open(cmd->rstdout, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
   dup2(fd, 1);
  }
  if (cmd->rstdin != NULL){
   //Read from file if < is used
   int fd2 = open(cmd->rstdin, O_RDONLY);
   dup2(fd2, 0);
  }
  if (cmd->bakground != 0) {
    signal(SIGINT, SIG_IGN);
  }
  //Create argument array and execute
  while (*pl){
   argv[i] = *pl++;
   argv[i+1] = NULL;
   i++;
  }
  execvp(argv[0], argv);
  i = 0;
  } else if (pid < 0) {
   /*error forking*/
   perror("fork");
  } else {
  /*parent process*/
  close(p[1]);
  dup2(p[0],0);
  signal(SIGINT, SIG_IGN);
  if (cmd->bakground == 0) {
   wait(NULL);
  }
 }
}
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void
PrintPgm (Pgm *p)
{
  if (p == NULL) {
    return;
  }
  else {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p->next);
    printf("    [");
    while (*pl) {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void
stripwhite (char *string)
{
  register int i = 0;

  while (isspace( string[i] )) {
    i++;
  }

  if (i) {
    strcpy (string, string + i);
  }

  i = strlen( string ) - 1;
  while (i> 0 && isspace (string[i])) {
    i--;
  }

  string [++i] = '\0';
}
