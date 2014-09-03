/*
 * Copr 1994. David W. Berry, All Rights Reserved
 */
#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <ctype.h>
#include <arpa/telnet.h>

#include "structs.h"
#include "db.h"
#include "utils.h"

extern char* crypt(const char*, const char*);

char*	dirPath = "builder";
char*	player_dir = "../players.d";

int	port = 4002;

int server(int port);
void run_fledit(int fd);
int validate(const char* user, const char* pwd);
void sigchld(int signo);
char* getstr(char* buf);

char echo_on[]  = {IAC, WONT, TELOPT_ECHO, 0 };
char echo_off[] = {IAC, WILL, TELOPT_ECHO, 0 };

int main(int argc, char**argv)
{
    extern int optind;
    extern char* optarg;
    int opt;

    while((opt = getopt(argc, argv, "d:p:")) != EOF)
    {
	switch(opt)
	{
	case 'd':
	    dirPath = optarg;
	    break;

	case 'p':
	    port = atoi(optarg);
	    break;

	default:
	    fprintf(stderr, "%s: unknown argument: '%c'", argv[0], opt);
	    exit(-1);
	    break;
	}
    }

    if(chdir(dirPath))
    {
	perror(dirPath);
	exit(-1);
    }

    return server(port);
}

int server(int port)
{
    extern int errno;
    int	socket;
    int	new_fd;
    struct sockaddr_in addr;
    int pid;
    
    signal(SIGCHLD, sigchld);
    
    socket = init_socket(port);

    while(1)
    {
	pid = sizeof(addr);
	
	if((new_fd = accept(socket, &addr, &pid)) < 0)
	{
	    if(errno == EINTR)
		continue;
	    
	    perror("flserver: accept failed");
	    exit(-1);
	}

	if((pid = fork()) < 0)
	{
	    perror("flserver: fork failed");
	    exit(-1);
	}

	if(pid == 0)
	{
	    /* this is the child */
	    run_fledit(new_fd);
	}
	else
	{
	    /* this is the parent */
	    close(new_fd);
	}
    }
}


int init_socket(int port)
{
    int s, done;
    int opt = -1, optlen;
    char hostname[256];
    struct sockaddr_in sa;
    struct hostent *hp;
    struct linger ld;

    bzero(&sa, sizeof(struct sockaddr_in));
    gethostname(hostname, 256);
    if(!(hp = gethostbyname(hostname)))
    {
	perror("gethostbyname");
	exit(1);
    }

    sa.sin_family = hp->h_addrtype;
    sa.sin_port	= htons(port);

    if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror("Init-socket");
	exit(1);
    }

#ifndef AUX
    optlen = sizeof(opt);
    if(getsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, &optlen) < 0)
    {
	perror("getsockopt REUSEADDR");
	exit(1);
    }
    if(!opt)
    {
	opt = 1;
	if(setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
	    perror ("setsockopt REUSEADDR");
	    exit (1);
	}
    }
#endif
    
    ld.l_onoff = 0;
    ld.l_linger = 1000;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0)	{
	perror("setsockopt LINGER");
	exit(1);
    }

    done = 6;
    while (1) {
	if (bind(s, &sa, sizeof(sa)) < 0)	{
	    perror("bind");
	    if(--done < 0)
	    {
		close(s);
		exit(1);
	    }
	    sleep(10);
	}
	else
	    break;
    }

    listen(s, 5);

    return(s);
}

void run_fledit(int fd)
{
    char	name[256];
    char	pwd[256];
    char	path[256];
    int i;
    
    if((dup2(fd, 0) < 0) ||
       (dup2(fd, 1) < 0) ||
       (dup2(fd, 2) < 0))
    {
	perror("flserver: can't dup file descriptor");
	exit(-1);
    }

    close(fd);

    setvbuf(stdout, NULL, _IOFBF, 8192);
    setvbuf(stderr, NULL, _IONBF, 8192);
    
    printf("Welcome to FLServer!\n");
    
    do
    {
	printf(echo_on);
	
	printf("Name:  ");
	if(!getstr(name))
	    exit(-1);

	for(i=0; i<strlen(name); i++)
	  *(name+i)=tolower(*(name+i));

	printf(echo_off);
	
	printf("Password: ");
	if(!getstr(pwd))
	    exit(-1);

	printf(echo_on);
    } while(!validate(name, pwd));

    mkdir(name, 0700);
    
    sprintf(path, "%s/fl", name);

    printf("Hello %s, launching %s \"%s\"\n", name, "fledit", path);

    fflush(stdout);
    
    execlp("fledit", "fledit", path, NULL);
    perror("fledit");
    exit(-1);
}

int validate(const char* name, const char* pwd)
{
    FILE* fp;
    struct char_data* ch;
    char buf[256];
    int ok = 0;
    
    sprintf(buf, "%s/%c/%s", player_dir, *name, name);

    if(!(fp = fopen(buf, "r")))
	return 0;

    CREATE(ch, struct char_data, 1);
    clear_char(ch);
    clear_aliases(ch);
    SpaceForSkills(ch);

    if(!read_player(ch, fp, READ_PLAYER))
    {
	if(!strncmp(ch->pwd, crypt(pwd, ch->pwd), 10))
	{
	    if(!IS_SET(ch->specials.flags, PLR_BUILDER))
	    {
		printf("Sorry, but you aren't a builder.\n");
		exit(1);
	    }
	    else
		ok = 1;
	}
    }

    free_char(ch);

    return ok;
}

void sigchld(int signo)
{
    int status;
    
    while(wait3(&status, WNOHANG, NULL) > 0)
	;
}

char* getstr(char* buf)
{
    char* ptr;
    int c;
    int telopt;
    
    fflush(stdout);
    
    for(ptr = buf, telopt = 0 ; (c = getchar()) != EOF ; )
    {
	switch(telopt)
	{
	case 0:
	    switch(c)
	    {
	    case '\r':
		break;

	    case '\n':
		*ptr = '\0';
		return buf;

	    case IAC:
		telopt = IAC;
		break;
	    
	    default:
		*ptr++ = c;
		break;
	    }
	    break;

	case IAC:
	    if(c == IAC)
	    {
		telopt = 0;
		*ptr++ = IAC;
	    }
	    else
		telopt = c;
	    break;
	    
	case DONT:
	case DO:
	case WONT:
	case WILL:
	    telopt = 0;
	    break;

	default:
	    printf("Unknown telnet escape sequence: IAC %d\n", c);
	    telopt = 0;
	    break;
	}
    }

    return NULL;
}
