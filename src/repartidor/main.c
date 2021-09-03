#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>


static int posicion_3semaforos_bodega[4] = {-1, -1, -1, -1};
static int estado_semaforo[3] = {-1, -1, -1};
static int cambio_pendiente[2] = {-1, -1};

static int id;
static int posicion = 0;
static int turnos = 0;
static int turnos_a_semaforo_1 = -1;
static int turnos_a_semaforo_2 = -1;
static int turnos_a_semaforo_3 = -1;
static int turnos_a_bodega = -1;
static int tiempo_entre_turnos;

static pid_t parent;
static pid_t child;

void outputfile()
{
    int cantidad = (int) floor(log10(abs(id))) + 1;
    char char_id[cantidad];
    sprintf(char_id, "%d", id);
    char nombre_archivo[] = "repartidor_";
    strcat(nombre_archivo, char_id);
    char extension[] = ".txt";
    strcat(nombre_archivo, extension);
    FILE *output_file = fopen(nombre_archivo, "w");
    fprintf(output_file, "%i,%i,%i,%i\n", turnos_a_semaforo_1, turnos_a_semaforo_2, turnos_a_semaforo_3, turnos_a_bodega);
    fclose(output_file);
    
    printf("(%i) Repartidor %i: ya escribí el archivo archivo\n", getpid(), id);
}

void repartidor_connect_sigaction(int sig, void (*handler)(int, siginfo_t *, void *))
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

void modificar_estados_semaforo()
{
    if (cambio_pendiente[0] != -1 && cambio_pendiente[1] != -1)
    {
        estado_semaforo[cambio_pendiente[0]] = cambio_pendiente[1];
        cambio_pendiente[0] = -1;
        cambio_pendiente[1] = -1;
    }
}

void repartidor_handle_sigabrt(int sigum)
{
    printf("(%i) Repartidor %i: recibí SIGABRT, proceda a escribir mi archivo\n", getpid(), id);
    outputfile();
    kill(child, SIGKILL);
}

void repartidor_handle_sigint(int sigum)
{
    printf("(%i) Repartidor %i: recibí SIGINT, lo ignoro\n", getpid(), id);
}

void repartidor_handle_sigusr1(int sigum, siginfo_t *siginfo, void *context)
{
    int semaforo_id = siginfo->si_value.sival_int;
    cambio_pendiente[0] = abs(semaforo_id);

    if (semaforo_id < 0){
        cambio_pendiente[1] = 0;
        printf("(%i) Repartidor %i: recibí SIGUSR1, cambio el estado del semaforo %i a LUZ ROJA\n", getpid(), id, semaforo_id);
    }
    else
    {
        cambio_pendiente[1] = 1;
        printf("(%i) Repartidor %i: recibí SIGUSR1, cambio el estado del semaforo %i a LUZ VERDE\n", getpid(), id, semaforo_id);
    }
    modificar_estados_semaforo();
}

void repartidor_handle_sigalarm(int sigum)
{

    int siguiente_semaforo1 = posicion + 1 == posicion_3semaforos_bodega[0];
    int siguiente_semaforo2 = posicion + 1 == posicion_3semaforos_bodega[1];
    int siguiente_semaforo3 = posicion + 1 == posicion_3semaforos_bodega[2];
    int siguiente_bodega = posicion + 1 == posicion_3semaforos_bodega[3];

    turnos++;
    printf("(%i) Repartidor %i: recibí SIGALRM, ejecutando turno n° %i\n", getpid(), id, turnos);

    if (siguiente_semaforo1){
        if (estado_semaforo[0])
        {
            posicion ++;
            turnos_a_semaforo_1 = turnos;
            printf("(%i) Repartidor %i: avanzo a semaforo 1\n", getpid(), id);
        }
        else
        {
            printf("(%i) Repartidor %i: NO puedo avanzar a semaforo 1\n", getpid(), id);
        }
    }
    else if (siguiente_semaforo2)
    {
        if (estado_semaforo[1])
        {
            posicion ++;
            turnos_a_semaforo_2 = turnos;
            printf("(%i) Repartidor %i: avanzo a semaforo 2\n", getpid(), id);
        }
        else
        {
            printf("(%i) Repartidor %i: NO puedo avanzar a semaforo 2\n", getpid(), id);
        }
    }
    else if (siguiente_semaforo3)
    {
        if (estado_semaforo[2])
        {
            posicion ++;
            turnos_a_semaforo_3 = turnos;
            printf("(%i) Repartidor %i: avanzo a semaforo 3\n", getpid(), id);
        }
        else
        {
            printf("(%i) Repartidor %i: NO puedo avanzar a semaforo 3\n", getpid(), id);
        }
    }
    else if (siguiente_bodega)
    {
        posicion ++;
        turnos_a_bodega = turnos;
        printf("(%i) Repartidor %i: avanzo a posicion %i\n", getpid(), id, posicion);
        printf("(%i) Repartidor %i: llegué a bodega\n", getpid(), id);
        kill(parent, SIGABRT);
    }
    else
    {
        posicion ++;
    }
    printf("(%i) Repartidor %i: avanzo a posicion %i\n", getpid(), id, posicion);
}

int main(int argc, char *argv[])
{
    signal(SIGALRM, repartidor_handle_sigalarm);
    signal(SIGABRT, repartidor_handle_sigabrt);
    signal(SIGINT, repartidor_handle_sigint);
    repartidor_connect_sigaction(SIGUSR1, repartidor_handle_sigusr1);


    id = atoi(argv[1]);
    tiempo_entre_turnos = atoi(argv[2]);
    posicion_3semaforos_bodega[0] = atoi(argv[3]);
    posicion_3semaforos_bodega[1] = atoi(argv[4]);
    posicion_3semaforos_bodega[2] = atoi(argv[5]);
    posicion_3semaforos_bodega[3] = atoi(argv[6]);

    printf("(%i) Repartidor %i: listo para trabajar\n", getpid(), id);

    /* Implementar avance*/
    parent = getpid();
    child = fork();
    if (child == 0)
    {
        while (1)
        {
            sleep(tiempo_entre_turnos);
            kill(parent, SIGALRM);
        }
    }
    else if (child > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("fork");
        exit(0); 
    }

    return 0;

}
