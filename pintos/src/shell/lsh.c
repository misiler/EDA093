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

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
void ExecuteCommand(Command *);
int SpawnProcess (int, int, char**);
/* When non-zero, this global means the user is done using this program. */
int done = 0;
int cntCmds = 0; //nof commands entered

/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
  Command cmd;
  int n;

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
	ExecuteCommand(&cmd);
	/*execl("/usr/bin/ls","ls",(char *) 0);*/
	//cleanup:
	cntCmds = 0;
      }
    }
    if(line) {
      free(line);
    }
  }
  return 0;
}

/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}
/* Name: ExecutePgm
* Desc: Recursive function to excute.
*/
void
ExecutePgm(Pgm *pgm){

int in = 0;
int fd[2];
char *argv[3];
char *argvtemp[3];

Pgm *pgmtemp = pgm;
//if(pgm == NULL){
//	return;
//} else {

//ExecutePgm(pgm->next);

char **pl = pgm->pgmlist;
argv[0] = *pl++;
argv[1] = *pl;
argv[2] = NULL;

for(int i = 0; i<cntCmds-1; i++){
//printf("curr: %s \n", *pl);//This row messess up something.
pgmtemp = pgmtemp->next;
pl = pgmtemp->pgmlist;
argvtemp[0] = *pl++;
argvtemp[1] = *pl;
argvtemp[2] = NULL;


      pipe(fd);
      /* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
      SpawnProcess(in, fd [1], argvtemp);

      /* No need for the write end of the pipe, the child will write here.  */
      close (fd [1]);

      /* Keep the read end of the pipe, the next child will read from there.  */
      in = fd [0];

/*
pid = fork();
if (pid == 0){
 //child process
     execvp(argv[0], argv);
} else if (pid < 0) {
 //error forking
 } else {
 //parent process
	if (cmd->bakground == 0) {
  		wait(NULL);
s 	}
//cmd->pgm=cmd->pgm->next;
*/
//pl = pgm->next->pgmlist; 

}
if (in != 0){
    dup2 (in, 0);
}
  /* Execute the last stage with the current process. */
 execvp(argv[0], argv);

//}
}
/*
* Name: ExecuteCommand
*
*/
void
ExecuteCommand(Command *cmd){
	ExecutePgm(cmd->pgm);
}

/*
* Name: SpawnProcess
*
* Desc: Starts a process and open/close the pipe to said process
*/
int SpawnProcess (int in, int out, char *argv[3])
{
  pid_t pid;

  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }
      return execvp (argv[0], argv);
    }
  return pid;
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
    cntCmds++;
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
