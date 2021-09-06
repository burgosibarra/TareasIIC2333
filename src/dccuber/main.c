#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "../file_manager/manager.h"
#include <stdlib.h>
#include <math.h>

static int repartidores_creados;
static int cantidad_repartidores;
static int tiempo_entre_repartidores;
static int tiempo_entre_turnos;
static pid_t* repartidores;
static pid_t fabrica_pid;
static volatile sig_atomic_t operativo = 1;

int posicion_sem_1;
int posicion_sem_2;
int posicion_sem_3;
int posicion_bodega;
int other_id;
int semaforos_pid[3];

void fabrica_connect_sigaction(int sig, void (*handler)(int, siginfo_t *, void *))
{
    // Define the sigaction struct required to setup the handler
    struct sigaction action;

    // Setup the handler function
    action.sa_sigaction = handler;

    // Set the mask as empty
    sigemptyset(&action.sa_mask);

    // Set SA_SIGINFO as the primary flag
    // This tells sigaction that the handler receives three parameters instead
    // of one
    action.sa_flags = SA_SIGINFO;

    // Associate the signal to the handler
    sigaction(sig, &action, NULL);
}

void fabrica_crear_repartidor()
{
    printf("(%i) Fábrica: recibí SIGALRM, crearé al repartidor %i\n", getpid(), repartidores_creados + 1);
    pid_t pid = fork();
    if (pid >= 0)
    {
      repartidores_creados ++;
    }

    /* Si pid > 0 es fábrica*/
    if (pid > 0)
    {
        repartidores[repartidores_creados - 1] = pid;
        if (repartidores_creados == cantidad_repartidores)
        {
            printf("(%i) Fábrica: ya he creado a todos los repartidores\n", getpid());
            operativo = 0;
        }
    }
    /* Si pid == 0 es repartidor*/
    else if (pid == 0)
    {
      int digitos_1 = (int) floor(log10(abs(posicion_sem_1))) + 1;
      char char_int_1[digitos_1];
      sprintf(char_int_1, "%d", posicion_sem_1);
      int digitos_2 = (int) floor(log10(abs(posicion_sem_2))) + 1;
      char char_int_2[digitos_2];
      sprintf(char_int_2, "%d", posicion_sem_2);
      int digitos_3 = (int) floor(log10(abs(posicion_sem_3))) + 1;
      char char_int_3[digitos_3];
      sprintf(char_int_3, "%d", posicion_sem_3);
      int digitos_4 = (int) floor(log10(abs(posicion_bodega))) + 1;
      char char_int_4[digitos_4];
      sprintf(char_int_4, "%d", posicion_bodega);
      int digitos_5 = (int) floor(log10(abs(repartidores_creados))) + 1;
      char repartidores_creados_char[digitos_5];
      sprintf(repartidores_creados_char, "%d", repartidores_creados);
      int digitos_6 = (int) floor(log10(abs(tiempo_entre_turnos))) + 1;
      char tiempo_entre_turnos_char[digitos_6];
      sprintf(tiempo_entre_turnos_char, "%d", tiempo_entre_turnos);
      
      char *args_fabrica[] = {"repartidor", repartidores_creados_char, tiempo_entre_turnos_char,char_int_1, char_int_2, char_int_3, char_int_4, NULL};

      execv("./repartidor", args_fabrica);
      perror("exec failed");
      exit(EXIT_FAILURE);

    }
    /* Si pid < 0 es un fork() error*/
    else
    {
        perror("fork");
        exit(1);
    }
}

void fabrica_handle_sigabrt(int sigum)
{
    operativo = 0;
    printf("(%i) Fábrica: recibí SIGABRT, informaré a los repartidores que deben terminar\n", getpid());    
    for(int index = 0; index < cantidad_repartidores; index++)
    {
        if (repartidores[index] && repartidores[index] != -1)
        {
            printf("(%i) Fábrica: recibí SIGABRT, informando a repartidor %i\n", getpid(), index);    
            kill(repartidores[index], SIGABRT);
        }
    }
}

void fabrica_handle_sigint(int sigum)
{
  printf("(%i) Fábrica: recibí SIGINT, lo ignoro\n", getpid());
}

void fabrica_handle_sigusr1(int sigum, siginfo_t *siginfo, void *context)
{
    int semaforo_status = siginfo->si_value.sival_int;

    printf("(%i) Fábrica: recibí SIGUSR1 del semáforo %i, informaré a los repartidores\n", getpid(), abs(semaforo_status));

    for(int index = 0; index < cantidad_repartidores; index++)
    {
        if (repartidores[index] && repartidores[index] != -1)
        {
            printf("(%i) Fábrica: recibí SIGUSR1 del semáforo %i, informando a repartidor %i\n", getpid(), abs(semaforo_status), index + 1);
            send_signal_with_int(repartidores[index], semaforo_status);
        }
    }
}

void fabrica(int array[2])
{
    // Conectamos las señales
    signal(SIGABRT, fabrica_handle_sigabrt);
    signal(SIGINT, fabrica_handle_sigint);
    fabrica_connect_sigaction(SIGUSR1, fabrica_handle_sigusr1);

    printf("(%i) Fábrica: lista para trabajar\n", getpid());
    
    cantidad_repartidores = array[0];
    repartidores = calloc(cantidad_repartidores, sizeof(pid_t));
    tiempo_entre_repartidores = array[1];
    repartidores_creados = 0;

    while (repartidores_creados < cantidad_repartidores && operativo)
    {
      sleep(tiempo_entre_repartidores);
      if (operativo)
      {
        fabrica_crear_repartidor();
      }
    }

    for (int index = 0; index < repartidores_creados; index++)
    {
      while (wait(NULL) == -1)
          {
            // Nada
          }
    }

    free(repartidores);

    printf("(%i) Fábrica: todos los repartidores han terminado\n", getpid());
    printf("(%i) Fábrica: terminé mi trabajo\n", getpid());
    exit(0);
}

void handle_sigint(int sigum){
  printf("(%i) Principal: he recibido un SIGINT, terminando procesos...\n", getpid());
  printf("(%i) Principal: he recibido un SIGINT, procedo a terminar la Fábrica\n", getpid());
  
  int status;
  
  kill(fabrica_pid, SIGABRT);
  while (waitpid(fabrica_pid, &status, 0) == -1)
  {

  }
  printf("(%i) DCCUBER: proceso Fábrica ha terminado, procedo a retirar los procesos semáforo\n", getpid());
  
  printf("(%i) DCCUBER: retirando semáforo 1\n", getpid());
  kill(semaforos_pid[0], SIGABRT);
  while (waitpid(semaforos_pid[0], &status, 0) == -1)
  {

  }
  
  printf("(%i) DCCUBER: retirando semáforo 2\n", getpid());
  kill(semaforos_pid[1], SIGABRT);
  while (waitpid(semaforos_pid[1], &status, 0) == -1)
  {

  }
  
  printf("(%i) DCCUBER: retirando semáforo 3\n", getpid());
  kill(semaforos_pid[2], SIGABRT);
  while (waitpid(semaforos_pid[2], &status, 0) == -1)
  {

  }
  exit(0);
}

void handle_sigusr1(int sig, siginfo_t *siginfo, void *ucontext)
{
  int valor_recibido = siginfo->si_value.sival_int;
  printf("%i\n", valor_recibido);
}

int main(int argc, char *argv[])
{
  signal(SIGINT, handle_sigint);
  char* delay[3]; //arreglo de los delay de los semaforos

  printf("(%i) DCCUBER: Hi! I'm the DCCUBER process\n", getpid());

  char *filename = argv[1];
  InputFile *data_in = read_file(filename);

  printf("(%i) DCCUBER: leyendo el archivo %s...\n", getpid(), filename);
  printf("(%i) DCCUBER: el archivo tiene %i lineas\n", getpid(), data_in->len);
  printf("(%i) DCCUBER: el contenido del archivo es\n", getpid());

  printf("\t- ");
  for (int i = 0; i < 4; i++)
  {
    printf("%s, ", data_in->lines[0][i]);
  }
  printf("\n");

  printf("\t- ");
  for (int i = 0; i < 5; i++)
  {
    printf("%s, ", data_in->lines[1][i]);
  }
  printf("\n");

  posicion_sem_1 = atoi(data_in->lines[0][0]);
  posicion_sem_2 = atoi(data_in->lines[0][1]);
  posicion_sem_3 = atoi(data_in->lines[0][2]);
  posicion_bodega = atoi(data_in->lines[0][3]);
  tiempo_entre_turnos = 1;
  tiempo_entre_repartidores = atoi(data_in->lines[1][0]);
  cantidad_repartidores = atoi(data_in->lines[1][1]);

  delay[0]  = data_in->lines[1][2];
  delay[1] = data_in->lines[1][3];
  delay[2] = data_in->lines[1][4];
  
  printf("(%i) DCCUBER: liberando el archivo de la memoria\n", getpid());
  input_file_destroy(data_in);
  
  printf("(%i) DCCUBER: creando el proceso fábrica\n", getpid());
  fabrica_pid = fork();
  if (fabrica_pid == 0)
  {
    int array[2] = {cantidad_repartidores, tiempo_entre_repartidores};
    fabrica(array);
  }
  
  printf("(%i) DCCUBER: creando los procesos semáforos\n", getpid());
  for (int i = 0; i < 3; i++)
  {
    other_id = fork();
    semaforos_pid[i] = other_id;
    if (other_id == 0)
    {
      char id[20];
      char parent[20];
      sprintf(id,"%d", i+1);
      
      sprintf(parent,"%d", fabrica_pid);
      char *args[] = {"semaforo", id, delay[i], parent, NULL};
      execv("./semaforo", args);
    }
  }
  
  int status;
  while (waitpid(fabrica_pid, &status, 0) == -1)
  {

  }
  printf("(%i) DCCUBER: proceso Fábrica ha terminado, procedo a retirar los procesos semáforo\n", getpid());
  
  printf("(%i) DCCUBER: retirando semáforo 1\n", getpid());
  kill(semaforos_pid[0], SIGABRT);
  while (waitpid(semaforos_pid[0], &status, 0) == -1)
  {

  }
  
  printf("(%i) DCCUBER: retirando semáforo 2\n", getpid());
  kill(semaforos_pid[1], SIGABRT);
  while (waitpid(semaforos_pid[1], &status, 0) == -1)
  {

  }
  
  printf("(%i) DCCUBER: retirando semáforo 3\n", getpid());
  kill(semaforos_pid[2], SIGABRT);
  while (waitpid(semaforos_pid[2], &status, 0) == -1)
  {

  }

  printf("(%i) DCCUBER: todos han terminado, hasta aquí llega mi trabajo\n", getpid());
  return 0;
}