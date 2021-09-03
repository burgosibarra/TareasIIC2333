#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../file_manager/manager.h"
int semaforo_id;
int changes;
int semaforo_pid;
int parent;
pid_t child_pid;

void handle_sigalrm(int sig){
    if (changes%2 == 0) 
    {
      printf("SEMAFORO %i EN VERDE\n", semaforo_id);
      send_signal_with_int(parent, semaforo_id);
    }
    else
    {
      printf("SEMAFORO %i EN ROJO\n", semaforo_id);
      send_signal_with_int(parent, -semaforo_id);
    }
    changes++;
}
void handle_sigabrt(int sig)
{
/* aqui va el código que procesa la señal */
  printf("ME LLEGÓ UN SIGABRT WTF\n");
  char *path;
  if (semaforo_id == 1)
  {
    path = "semaforo_1.txt";
  }else if (semaforo_id == 2)
  {
    path = "semaforo_2.txt";
  }else
  {
    path = "semaforo_3.txt";
  }
  // Abrimos un archivo en modo de lectura
  FILE *output = fopen(path, "w");
  fprintf(output,"%i", changes);

  // Se cierra el archivo (si no hay leak)
  fclose(output);

  printf("%s\n", path);
  
  kill(child_pid, SIGKILL);
}

int main(int argc, char const *argv[])
{
  signal(SIGALRM, handle_sigalrm);
  printf("I'm the SEMAFORO %s process and my PID is: %i\n",argv[1], getpid());
  semaforo_id = atoi(argv[1]);
  int delay = atoi(argv[2]);
  parent = atoi(argv[3]);

  printf("id: %i\n", semaforo_id);
  printf("delay: %i\n", delay);
  printf("parent pid: %i\n", parent);
  signal(SIGABRT,handle_sigabrt);
  changes = 0;
  semaforo_pid = getpid();
  child_pid = fork();
  if (child_pid < 0)
  {
    perror("fork");
    exit(0);
  }else if (child_pid == 0)
  {
    while (changes <= 10)
  {
    sleep(delay);
    kill(semaforo_pid, SIGALRM);
    changes++;
    printf("ALARM : %i\n",changes);
  }
  }
  wait(NULL);
  
  exit(0);
}