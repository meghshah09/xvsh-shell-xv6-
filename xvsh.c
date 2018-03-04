
/*------------------------- Header Files --------------------------*/
#include "types.h"
#include "user.h"
#include "fcntl.h"
/*--------------------- MACROS -----------------------*/
#define EXEC  1
#define BACK  5

#define MAXARGS 10
/*------------------------------- Global Variables and function declarations -----------------------*/
int found=-1;
char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";
struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  
  char *eargv[MAXARGS];
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);
void panic(char*);
struct cmd *parsecmd(char*);
struct cmd *parseline(char**, char*);
struct cmd *parseexechelper(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

/*---------------------------------------- RUNCMD ---------------------------------*/
void runcmd(struct cmd *cmd)
{
  int wpid;
  struct backcmd *bcmd;
  struct execcmd *ecmd;


  if(cmd == 0)
    exit();
  //int d=0;
  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
	//flag=-1;
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit();
  
		//printf(1,"I was in EXEC\n");
		//if(fork()==0)
			exec(ecmd->argv[0], ecmd->argv);
    	printf(2, "Cannot run this command %s\n", ecmd->argv[0]);
    
    break;
  
  case BACK:
	bcmd = (struct backcmd*)cmd;
	
	//printf(1,"I was in BACK\n");
	wpid = fork();
    if(wpid==0){
		runcmd(bcmd->cmd);
	}
	wait();
		
    break;
  }
  
   //printf(1,"runCMD Exit\n");
	exit();
}

/*-------------------------------- Gets the user Input --------------------------------*/
int getcmd(char *buf, int nbuf)
{	
										
  printf(1, "xvsh> ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0 ) // EOF
    return -1;
	
  return 0;
}

/*-------------------------- MAIN METHOD------------------*/
int main(void)
{
	/**/
  static char buf[100];
  //int pid;
	//int f=-1;
  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
   if(buf[0] == 'e' && buf[1] == 'x' && buf[2] == 'i' && buf[3] == 't' && buf[4] == '\n'){
      buf[strlen(buf)-1] = 0;
		while(wait()>0){}
      	exit();
    }
	found=-1;
	if(buf[0]=='\n')continue;
		for(int i=0;i<strlen(buf);i++){
			if(buf[i]=='&'){
				found=0;
				break;
			}
		}
	//int pid= fork1();

	if(found==0){
		//f=-1;
		int pid= fork1();
		if(pid==0){
				
			runcmd(parsecmd(buf));
		}
		printf(1,"[PID %d] runs as a background process\n",getpid());
		continue;
	}
	else{
	// when "&" is not found.
		//while(wait()>0){continue;}
		int pid= fork1();
		if(pid==0){
			runcmd(parsecmd(buf));
		}
		while(wait()>0){}
	}
	
  }
  //printf(1,"Main Exit\n");
	exit();
}
/*---------------------------------- UTIL FUNCTION----------------------*/
void panic(char *s)
{
  printf(2, "%s\n", s);
  exit();
}

int fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

struct cmd* execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd* backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}

/*------------------------------------- TOKENIZING -------------------------------*/

int gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
		
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;
  
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

/*-------------------------------- Function for seeing inside the string -------------*/
int peek(char **ps, char *es, char *toks)
{
  char *s;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}


/*------------------- PARSING the string given by the user -----------------*/

struct cmd* parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);

  nulterminate(cmd);
  //printf(1,"I am here\n");
  return cmd;
}

struct cmd* parseline(char **ps, char *es)
{
  struct cmd *cmd;
	//printf(1,"Before\n");
	//printf(1,"PS:%s\n",ps[0]);
  //printf(1,"ES:%s\n",es);
  cmd = parseexechelper(ps, es);
  //printf(1,"After\n");
  //printf(1,"PS:%s\n",ps[0]);
  //printf(1,"ES:%s\n",es);
  while(peek(ps, es, "&")){
	 // printf(1,"I am here\n");
    gettoken(ps, es, 0, 0);
	/*printf(1,"After1\n");
  printf(1,"PS:%s\n",ps[0]);
  printf(1,"ES:%s\n",es);*/
    cmd = backcmd(cmd);
  }

  return cmd;
}

struct cmd* parseexechelper(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);

  return cmd;
}

struct cmd* parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;

  while(!peek(ps, es, "|)&;")){
	//printf(1,"I am Parsinf\n");
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
		
		cmd->argv[argc] = q;
		//printf(1,"String:%s",cmd->argv[argc]);
		cmd->eargv[argc] = eq;
		argc++;

    if(argc >= MAXARGS)
      panic("too many args");
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

/*-----------------------------------NUL-terminate all the counted strings----------------------*/
struct cmd* nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;

  if(cmd == 0)
    return 0;
  
  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}
