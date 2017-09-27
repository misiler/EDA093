
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

void countPgm(Pgm *);
void stripwhite(char *);
void ExecuteCommand(Command *, Pgm *);

/* When non-zero, this global means the user is done using this program. */
int done = 0;
int nofCmds = 0;
int currCmd = 0;
int p[20][2];//pipe-array, maximum of 20 piped commands.

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
  } else {
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
    countPgm(cmdp->pgm);
    currCmd = nofCmds;
    pl = cmdp->pgm->pgmlist;
    if (!strcmp(*pl++, "cd")){
     chdir(*pl);
    } else if (!strcmp(*cmdp->pgm->pgmlist,"exit")) {
     exit(0);
    } else {
     //CreatePipes();
     ExecuteCommand(&cmd, cmdp->pgm);
     nofCmds = 0;
    }
   }
  }
  if(line) {
   free(line);
  }
 }
 return 0;
}


/*
 *Name: ExecutePgm
 *
 *Desc: Executes programs, is called by a child process and forks more children
 *and creates pipes between them if nessesary
 *
*/
void
ExecutePgm(Command *cmd, Pgm *pgm, int writer, int listener)
{

 if(pgm == NULL){
  return;
 }else if (pgm->next != NULL){
  currCmd--;
  pid_t pid;
  if (pipe(p[currCmd]) == -1) {//Error-check
   //Creates a pipe
   return;
  }
  //forks a "grandchild" of lsh
  pid = fork();
  if (pid == 0){
   //Child process
   //Recursive calls if not last remaning (or only) command
   if(currCmd > 1 && currCmd != nofCmds){
    ExecutePgm(cmd, pgm->next, 1, 1);
   } else {
    ExecutePgm(cmd, pgm->next, 1, 0);
   }
  } else if (pid < 0) {
   /*error forking*/
   perror("fork");
  } else {
   /*parent process*/
   wait(NULL);
  }
 }

 //First command
 if (writer == 1 && listener == 0) {
  dup2(p[currCmd][1],1); // output to pipe
  close(p[currCmd][0]);
  close(p[currCmd][1]);
  if (cmd->rstdin != NULL){
   //Read from file if < is used
   int fd2 = open(cmd->rstdin, O_RDONLY);
   dup2(fd2, 0);
   close(fd2);
  }
  if (cmd->bakground != 0) {
   signal(SIGINT, SIG_IGN);
  }
 //Last command
 } else if (writer == 0 && listener == 1) {
   dup2(p[currCmd][0],0); // take input from pipe
   close(p[currCmd][1]);
   close(p[currCmd][0]);
   if (cmd->rstdout != NULL) {
    //Write to file if > is used
    int fd = open(cmd->rstdout, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    dup2(fd, 1);
    close(fd);
   }
 //Middle command
 } else if(writer == 1 && listener == 1){
   dup2(p[currCmd][0],0); // take input from pipe
   dup2(p[currCmd+1][1],1); // send output to next pipe
   close(p[currCmd+1][1]);
   close(p[currCmd+1][0]);
   close(p[currCmd][0]);
   close(p[currCmd][1]);
 } else { //if no piping occurs we may still need file IO
   if (cmd-> rstdout != NULL) {
    int fd = open(cmd->rstdout, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    dup2(fd, 1);
    close(fd);
   }
   if (cmd-> rstdin != NULL) {
    int fd2 = open(cmd->rstdin, O_RDONLY);
    dup2(fd2, 0);
    close(fd2);
   }
 }
 //Execution stage:
 char **pl = pgm->pgmlist;
 char *argv[20];
 int i = 0;
 //Create argument array and execute
 while (*pl){
  argv[i] = *pl++;
  argv[i+1] = NULL;
  i++;
 }
 i = 0;
 execvp(argv[0], argv);
}

/*
* Name: ExecuteCommand
*
* Desc: Executes a given command by searching path.
*
*/
void
ExecuteCommand(Command *cmd, Pgm *pgm)
{
 pid_t pid;
 pid = fork();
 if (pid == 0){
   if (nofCmds > 1){//start piping and forking more children if more than 1 command is used
   ExecutePgm(cmd, pgm, 0, 1);
   }else{
   ExecutePgm(cmd, pgm, 0, 0);
   }
  } else if (pid < 0) {
   /*error forking*/
   perror("fork");
  } else {
  /*parent process*/
  signal(SIGINT, SIG_IGN);
  if (cmd->bakground == 0) {
   wait(NULL);
  }
 }
}

/*
 *Name: countPgm
 *
 *Desc: counts how many commands that are to be executed
 *
 */
void
countPgm(Pgm *p){
 if (p == NULL) {
  return;
 } else {
 countPgm(p->next);
 nofCmds++;
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
