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
#include <dirent.h>
#include <libgen.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>
#include <glob.h>

extern int obtain_order();		/* See parser.y for description */
void mandato (char **argv, char **filev, int bg, int i, int argvc);
void secuencia (char **argv, char **filev,int bg, int argvc, char ***argvv, int i);
void redirec (char **filev, int i, int argvc);
void senal(int bg);
void man_cd(char **argv);
void man_umask(char **argv);

int main(void){

	char ***argvv = NULL;
	int argvc;
	char **argv = NULL;
	int argc;
	char *filev[3] = { NULL, NULL, NULL };
	int bg;
	int ret;

	setbuf(stdout, NULL);			/* Unbuffered */
	setbuf(stdin, NULL);
	
	struct sigaction act;
	act.sa_handler=SIG_IGN;
	act.sa_flags=SA_RESTART;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	
	while (1) {
		fprintf(stderr, "%s", "msh> ");	/* Prompt */
		ret = obtain_order(&argvv, filev, &bg);
		if (ret == 0) break;		/* EOF */
		if (ret == -1) continue;	/* Syntax error */
		argvc = ret - 1;		/* Line */
		if (argvc == 0) continue;	/* Empty line */
		int i;
		for (i = 0; (argv = argvv[i]); i++) {
		
			if(argvc > 1){//Ejecucion de secuencia
				secuencia(argv, filev, bg, argvc, argvv, i);
				
			}else{ //Ejecucion de un mandato
				/*Primero comprobamos si es un mandato interno*/
				if(strcmp(argv[0], "cd") == 0 || strcmp(argv[0], "umask") == 0){
					int fden;
					int fdsal;
					int fderr;
					int tmpen = dup(0);
					int tmpsal = dup(1);
					int tmperr = dup(2);
					/*Comprobamos las redirecciones*/
					if (filev[0] != NULL){
						fden = open(filev[0], O_RDONLY);
						if (fden < 0){
							perror("Error en la redireccion de entrada \n");
							continue;
						}else{
							dup2(fden, 0);
							close(fden);
						}
					}if (filev[1] != NULL){
						fdsal = creat(filev[1], 0666);
						if (fdsal < 0){
							perror("Error en la redireccion de salida \n");
							continue;
						}else{
							dup2(fdsal, 1);
							close(fdsal);
						}
					}if (filev[2] != NULL){
						fderr = creat(filev[2], 0666);
						if (fderr < 0){
							perror("Error en la redireccion de error \n");
							continue;
						}else{
							dup2(fderr, 2);
							close(fderr);
						}
					}
					
					if(strcmp(argv[0], "cd") == 0){//Si el mandato es "cd"
						man_cd(argv);
						
					}else if(strcmp(argv[0], "umask") == 0){//Si el mandato es "umask"
						man_umask(argv);
					}
					/*Restauramos las redirecciones*/
					if (filev[0] != NULL){
						dup2(tmpen, 0);
						close(tmpen);
					}
					if (filev[1] != NULL){
						dup2(tmpsal, 1);
						close(tmpsal);
					}
					if (filev[2] != NULL){
						dup2(tmperr, 2);
						close(tmperr);
					}
				
				}else{
					mandato(argv, filev, bg, i, argvc);
				}
			}	
		}			
	}
	exit(0);
	return 0;
}

/*Si tenemos un unico mandato*/
void mandato (char **argv, char **filev, int bg, int i, int argvc){
	char *nombre;
	pid_t pid = fork();
	int valor = pid;
	switch(pid){
		case -1:/*error de fork()*/
			perror("fork()");
			return 1;
		case 0:/*proceso hijo*/
			senal(bg);//Si el bit bg esta activado enmascaramos las señales
			redirec(filev, i, argvc);//Comprobamos si hay redirecciones
			execvp(argv[0], &argv[0]);
			perror("Error en el exec");
			exit (1);
			
		default:/*proceso padre*/
			if(!bg){//Si el bit bg no esta activado, esperamos a que termine
				waitpid(pid, NULL, 0);
			}else{//Si lo esta no esperamos y actualiza el valor de la variable bgpid
				nombre="bgpid";
				int lg = snprintf(NULL, 0, "%d", valor);
				char *con = malloc(lg + 1);
    				snprintf(con, lg + 1, "%d", valor);
				if(setenv(nombre, con, 1) < 0){
					perror("error");
					return 1;
				}
                		printf("[%d]\n", con);
			}
	}
}

/*Si tenemos varios mandatos*/
void secuencia (char **argv, char **filev,int bg, int argvc, char ***argvv, int i){
	
	pid_t pid;
	int fd[2];
	int fd2[2];
	int status;
	char *nombre;
	if(i==0){// Proceso padre para el primer hijo
		if(pipe(fd) < 0){
			perror("Error en la tuberia");
			return 1;
		}
	} else if(i==argvc-1){// Proceso padre para el ultimo hijo
		close(fd[1]);
	}else{// Proceso padre para el resto de hijos
		if(pipe(fd2) < 0){
		 	perror("Error");
		 	return 1;
		}
		dup2(fd2[1], fd[1]);
		close(fd2[1]);
		
	}
	pid=fork();
	int valor = pid;
	switch(pid){
		case-1:/*error del fork()*/
			perror("Error en el fork");
			return 1;
		case 0: /*proceso hijo*/
			senal(bg);//Comprobamos si se ejecuta en bg
			redirec(filev, i, argvc);//Comprobamos si hay redirecciones
			if(i==0){//primer hijo
				close(fd[0]);
				dup2(fd[1], 1);
				close(fd[1]);
			}else if((i==argvc-1)){//ultimo hijo
				dup2(fd[0], 0);
				close(fd[0]);	
			}else{//resto hijo
				dup2(fd[0], 0);
				dup2(fd[1], 1);
				close(fd2[0]);
				close(fd[0]);
				close(fd[1]);
			}
			execvp(argv[0], argv);
			perror("Error en el exec");
			exit (1);	
		default:/*proceso padre*/
			
			if(i>0 && i<argvc-1){//Para el resto de hijos
				dup2(fd2[0], fd[0]);
				close(fd2[0]);
			}
			if(i==argvc-1 && !bg){//Para el ultimo hijo si no tiene activado el bit bg
			close(fd[0]);
			waitpid(pid, &status, 0);		
			}
			if(bg && (i==argvc-1)) {//Para el ultimo hijo si tiene activado el bit bg
                		nombre="bgpid";
				int lg = snprintf(NULL, 0, "%d", valor);
				char *con = malloc(lg + 1);
    				snprintf(con, lg + 1, "%d", valor);
				if(setenv(nombre, con, 1) < 0){
					perror("error");
					return 1;
				}
                		printf("[%d]\n", con);
            		}	
		}
}


/*Comprueba las redirecciones de entrada*/
void redirec (char **filev, int i, int argvc){
	int fdent, fdsal, fderr;
	if(filev[0] && (i==0)){//Redireccion de entrada
		fdent = open(filev[0], O_RDONLY);
				
	if(fdent < 0){
		perror("Error en la redireccion de entrada \n");
		exit (1);
	}
	dup2(fdent, 0);
	close(fdent);
	}
	
	if(filev[1] && (i==argvc-1)){//Redireccion de salida
		fdsal = creat(filev[1], 0666);
		
		
	if(fdsal < 0){
		perror("Error en la redireccion de salida \n");
		exit (1);
	}
	dup2(fdsal, 1);
	close(fdsal);
	}	

	if(filev[2]){//Redireccion de error
	fderr = creat(filev[2], 0666);
	
	
	if(fderr < 0){
		perror("Error en la redireccion de error \n");
		exit (1);
	}
	dup2(fderr, 2);
	close(fderr);
	}
}

/*Comprueba la señaes y el bit bg*/
void senal(int bg){
	struct sigaction act;
	act.sa_handler=SIG_IGN;
	act.sa_flags=SA_RESTART;
	
	if(!bg){
	act.sa_handler=SIG_DFL;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	}
}

/*Mandato cd*/
void man_cd(char **argv){
	
	if(argv[2]){//Se llama a cd con dos directorios
		perror("No se puede cambiar a dos directorios \n");
		return 1;
	}
	int aux = 0;
	char buffer[2048];
	if(argv[1]){//Se llama a cd con un directorio
		char *directorio = argv[1];
		if(opendir(directorio)){
			aux = chdir(directorio);
		}else{
			perror("El directorio no existe \n");
			return 1;
		}
	}else{//Se llama a cd sin directorio, por defecto va al HOME
		aux = chdir(getenv("HOME"));
	}
	if(aux < 0){
		perror("Error al cambiar de directorio \n");
		return 1;
	}
	if(getcwd(buffer, sizeof(buffer)) != NULL){
		printf("%s\n", buffer);
	}else{
		perror("Error obteniendo directorio");
	}
}

/*Mandato umask*/		
void man_umask(char **argv) {

	if(argv[2]){//Se llama a umask con un numero incorrecto de argumentos
		perror("No se puede cambiar la mascara de dos ficheros \n");
		return 1;
	}
	mode_t mask;
	if(!argv[1]){//Se llama a umask sin argumentos
		mask = umask(0);
		printf("%o\n", mask);
	}else{//Se llama a umask con un argumento
		int cero = 0;
		int num = atoi(argv[1]);
		if(num == 0){
			cero = 1;
		}
		long valor = strtol(argv[1], NULL, 8);
		if(valor == 0 && !cero){//Comprobamos si la mascara se sale de rango
			perror("Numero fuera de rango \n");
            		return 1;
        	}else{
			mask = valor;
        		umask(mask);
        		printf("%o\n", mask);
        	}
    	}
    	
}
