/*-
 * main.c
 * Minishell C source
 * Shows how to use "obtain_order" input interface function.
 *
 * Copyright (c) 1993-2002-2019, Francisco Rosales <frosal@fi.upm.es>
 * Todos los derechos reservados.
 *
 * Publicado bajo Licencia de Proyecto Educativo Práctico
 * <http://laurel.datsi.fi.upm.es/~ssoo/LICENCIA/LPEP>
 *
 * Queda prohibida la difusión total o parcial por cualquier
 * medio del material entregado al alumno para la realización
 * de este proyecto o de cualquier material derivado de este,
 * incluyendo la solución particular que desarrolle el alumno.
 *
 * DO NOT MODIFY ANYTHING OVER THIS LINE
 * THIS FILE IS TO BE MODIFIED
 */

#include <stddef.h>			/* NULL */
#include <stdio.h>			/* setbuf, printf */
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#include <sys/resource.h>

#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <glob.h>
//#include <times.h>

extern int obtain_order();		/* See parser.y for description */

int main(void)
{
	char ***argvv = NULL;
	int argvc;
	char **argv = NULL;
	int argc;
	char *filev[3] = { NULL, NULL, NULL };
	int bg;
	int ret;

	setbuf(stdout, NULL);			/* Unbuffered */
	setbuf(stdin, NULL);

	while (1) {
		fprintf(stderr, "%s", "msh> ");	/* Prompt */
		ret = obtain_order(&argvv, filev, &bg);
		if (ret == 0) break;		/* EOF */
		if (ret == -1) continue;	/* Syntax error */
		argvc = ret - 1;		/* Line */
		if (argvc == 0) continue;	/* Empty line */
#if 1


			
		if(argvc = 1){
			mandato(argvc);
			
		}
		if(argvc > 1){
			secuencia(argvc);
		}	
					
						
			
			
			
							
				
#endif
	}
	exit(0);
	return 0;
}

void mandato (argvc *argv){
	pid_t pid;
	pid = fork();
	if(pid < 0){
	switch(pid){
		case -1:/*error de fork()*/
			perror("fork()");
			return 1;
		case 0:/*hijo*/
			execvp(argv[1], &argv[1]);
			perror("exec");
			return 2;
		default:/*padre*/
			break;
	}
}

void secuencia (argv, *argv){
	int fd[2];
	int ent;
	int sal;
	pid_t pid;
	int i;
	int N;
	char c;
	
	N = atoi(argv[1]);
	if (pipe(fd) < 0){
		perror("Error al crear el pipe\n");
		return 0;
	}
	
	ent = fd[0];
	sal = fd[1];
	write(sal, &c, 1);
			
	for(i=0; i<N; i++){
		if (i != N-1)
		if(pipe(fd) < 0){
			perror("Error al crear el pipe\n");
			return 0;
		}
		
		pid = fork();
		switch(pid){
			case-1:/*error del fork()*/
				return 1;
			case 0: /*proceso hijo*/
				execvp(argv[0],argv);	
				if(i !=N-1){
					close(sal);
					sal= dup (fd[1]);
					close(fd[0]);
					close(fd[1]);
				}
				i = N;
				break;
						
			default:/*proceso padre*/
				if( i==N-1)
					return 0;
				else{
					close(ent);
					close(fd[1]);
					ent =fd[0];
				}
						
						
		}
	}
}

void redent (argv, *argv){
	int fdent;
	fdent = open(filev[0], O_RDONLY);
	if(fdent < 0){
		perror("open");
		exit(1);
	}
	dup2(fdent, 0);
	}
	
void redsal (argv, *argv){
	int fdsal;
	fdsal = open(filev[1], O_WRONLY|O_CREAT|O_TRUNC,0666);
	if(fdsal < 0){
		perror("open");
		exit(1);
	}
	dup2(fdsal, 1);
}

void rederr (argv, *argv){
	int fderr;
	fderr = open(filev[2], O_WRONLY|O_CREAT|O_TRUNC,0666);
	if(fderr < 0){
		perror("open");
		exit(1);
	}
	dup2(fderr, 2);
}
	

















