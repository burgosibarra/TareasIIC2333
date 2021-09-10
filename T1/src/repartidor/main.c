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
static volatile sig_atomic_t operativo = 1;

void outputfile()
{
    char filename[27]; 
    sprintf( filename, "repartidor_%i.txt", id );
    FILE *output_file = fopen(filename, "w");
    fprintf(output_file, "%i,%i,%i,%i\n", turnos_a_semaforo_1, turnos_a_semaforo_2, turnos_a_semaforo_3, turnos_a_bodega);
    fclose(output_file);

    // Se cierra el archivo (si no hay leak)
    //fclose(output);
    
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
    printf("(%i) Repartidor %i: recibí SIGABRT, procedo a escribir mi archivo\n", getpid(), id);
    operativo = 0;
}

void repartidor_handle_sigint(int sigum)
{
    printf("(%i) Repartidor %i: recibí SIGINT, lo ignoro\n", getpid(), id);
}

void repartidor_handle_sigusr1(int sigum, siginfo_t *siginfo, void *context)
{
    int semaforo_id = siginfo->si_value.sival_int;
    cambio_pendiente[0] = abs(semaforo_id) - 1;

    if (semaforo_id < 0){
        cambio_pendiente[1] = 0;
        printf("(%i) Repartidor %i: recibí SIGUSR1, cambio el estado del semaforo %i a LUZ ROJA\n", getpid(), id, abs(semaforo_id));
    }
    else
    {
        cambio_pendiente[1] = 1;
        printf("(%i) Repartidor %i: recibí SIGUSR1, cambio el estado del semaforo %i a LUZ VERDE\n", getpid(), id, abs(semaforo_id));
    }
    modificar_estados_semaforo();
}

void avanzar()
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
        operativo = 0;
    }
    else
    {
        posicion ++;
        printf("(%i) Repartidor %i: avanzo a posicion %i\n", getpid(), id, posicion);
    }
}

int main(int argc, char *argv[])
{
    repartidor_connect_sigaction(SIGUSR1, repartidor_handle_sigusr1);
    signal(SIGABRT, repartidor_handle_sigabrt);
    signal(SIGINT, repartidor_handle_sigint);

    id = atoi(argv[1]);
    tiempo_entre_turnos = atoi(argv[2]);
    posicion_3semaforos_bodega[0] = atoi(argv[3]);
    posicion_3semaforos_bodega[1] = atoi(argv[4]);
    posicion_3semaforos_bodega[2] = atoi(argv[5]);
    posicion_3semaforos_bodega[3] = atoi(argv[6]);
    estado_semaforo[0] = atoi(argv[7]);
    estado_semaforo[1] = atoi(argv[8]);
    estado_semaforo[2] = atoi(argv[9]);
    printf("(%i) Repartidor %i: listo para trabajar\n", getpid(), id);

    while (operativo){
        sleep(tiempo_entre_turnos);
        if (operativo){
            avanzar();
        }
    }

    outputfile();
    printf("(%i) Repartidor %i: terminé mi trabajo\n", getpid(), id);
    return 0;

}
