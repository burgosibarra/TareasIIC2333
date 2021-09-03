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

int posicion_sem_1;
int posicion_sem_2;
int posicion_sem_3;
int posicion_bodega;
int other_id;

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

void fabrica_handle_sigalarm(int signum)
{
    pid_t pid = fork();
    if (pid >= 0)
    {
      repartidores_creados ++;
    }

    /* Si pid > 0 es fábrica*/
    if (pid > 0)
    {
        repartidores[repartidores_creados] = pid;
        if (repartidores_creados < cantidad_repartidores)
        {
            alarm(tiempo_entre_repartidores);
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
    for(int index = 0; index < cantidad_repartidores; index++)
    {
        if (repartidores[index])
        {
            kill(repartidores[index], SIGABRT);
        }
    }
}

void fabrica_handle_sigint(int sigum)
{
    
}

void fabrica_handle_sigusr1(int sigum, siginfo_t *siginfo, void *context)
{
    int semaforo_status = siginfo->si_value.sival_int;

    for(int index = 0; index < cantidad_repartidores; index++)
    {
        if (repartidores[index])
        {
            union sigval sig = {};
            sig.sival_int = semaforo_status;
            sigqueue(repartidores[index], SIGUSR1, sig);
        }
    }
}

void fabrica(int array[2]) /* Ojo con el nombre */
{
    // Conectamos las señales
    signal(SIGALRM, fabrica_handle_sigalarm);
    signal(SIGABRT, fabrica_handle_sigabrt);
    signal(SIGINT, fabrica_handle_sigint);
    fabrica_connect_sigaction(SIGUSR1, fabrica_handle_sigusr1);

    /*comprobar con main.c*/
    cantidad_repartidores = array[0];
    repartidores = calloc(cantidad_repartidores, sizeof(pid_t));
    tiempo_entre_repartidores = array[1];
    repartidores_creados = 0;

    alarm(1);
    sleep(2); /* OJO con este valor */

    int STATUS;
    for (int index = 0; index < cantidad_repartidores; index++)
    {
        if (repartidores[index])
        {
          waitpid(repartidores[index], &STATUS, 0);
          printf("pid %i terminó\n", repartidores[index]);
        }
    }

    free(repartidores);

    exit(0);
}

void handle_sigusr1(
int sig, siginfo_t *siginfo, void *ucontext)
{
int valor_recibido = siginfo->si_value.sival_int;
printf("%i\n", valor_recibido);
}

int main(int argc, char *argv[])
{
  char* delay[3]; //arreglo de los delay de los semaforos
  int semaforos_pid[3]; //los pid de los procesos semaforo

  printf("I'm the DCCUBER process and my PID is: %i\n", getpid());

  char *filename = argv[1];
  InputFile *data_in = read_file(filename);

  printf("Leyendo el archivo %s...\n", filename);
  printf("- Lineas en archivo: %i\n", data_in->len);
  printf("- Contenido del archivo:\n");

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

  posicion_sem_1 = atoi(data_in->lines[0][0]);
  posicion_sem_2 = atoi(data_in->lines[0][1]);
  posicion_sem_3 = atoi(data_in->lines[0][2]);
  posicion_bodega = atoi(data_in->lines[0][3]);
  tiempo_entre_turnos = 1;
  tiempo_entre_repartidores = atoi(data_in->lines[1][0]);
  cantidad_repartidores = atoi(data_in->lines[1][1]);

  printf("\n");
  delay[0]  = data_in->lines[1][2];
  printf("%s\n",delay[0]);
  delay[1] = data_in->lines[1][3];
  printf("%s\n", delay[1]);
  delay[2] = data_in->lines[1][4];
  printf("%s\n",delay[2]);
  printf("Liberando memoria...\n");
  input_file_destroy(data_in);
  
  printf("Creando un proceso hijo\n");
  //connect_sigaction(SIGUSR1, handle_sigusr1);
  pid_t fabrica_pid = fork();
  if (fabrica_pid == 0)
  {
    int array[2] = {cantidad_repartidores, tiempo_entre_repartidores};
    fabrica(array);
  }
  
  for (int i = 0; i < 3; i++)
  {
    other_id = fork();
    semaforos_pid[i] = other_id;
    if (other_id == 0)
    {
      char id[20];
      char parent[20];
      sprintf(id,"%d", i+1);
      
      sprintf(parent,"%d", getppid());
      printf("hola\n");
      char *args[] = {"semaforo", id, delay[i], parent, NULL};
      execv("./semaforo", args);
      printf("No me debería leer\n");
    }
  }
  printf("sigue el papito\n");
  //sleep(30);
  //kill(semaforos_pid[1], SIGABRT);
  int status;
  waitpid(fabrica_pid, &status,0); //ojo
  kill(semaforos_pid[0], SIGABRT);
  kill(semaforos_pid[1], SIGABRT);
  kill(semaforos_pid[2], SIGABRT);
  return 0;
}