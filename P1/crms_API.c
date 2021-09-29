#include <stdlib.h>
#include <stdio.h>

#include "crms_API.h"

char* path = "mem.bin";
int PCB_SIZE = 4096;
int PCB_POINTER_SIZE = 256;
int PCB_SUBPOINTER_SIZE = 21;
int PAGE_TABLE_SIZE = 32;

void binchar(char character)
{
    char output[9];
    itoa(character, output, 2);
    printf("%s\n", output);
}


int name_coincidence(unsigned char* pcb_table, int position, char* file_name)
{
    for (int index = 0; index < 12; index++)
    {
        if ((char) atoi(pcb_table[position + index]) != file_name[index]) return 0;
    }
    return 1;
}


CrmsFile* cr_open(int process_id, char* file_name, char mode)
{
    
    FILE* memory;
    memory = fopen(path,"rb");  // r for read, b for binary

    unsigned char pcb_table[PCB_SIZE];
    fread(pcb_table, sizeof(pcb_table), 1, memory); // read 10 bytes to our buffer

    for (int pointer = 0; pointer < PCB_SIZE; pointer = pointer + PCB_POINTER_SIZE)
    {
        if (atoi(pcb_table[pointer + 1]) == process_id)
        {
            if (atoi(pcb_table[pointer]) != 0x01)
            {
                // Manejar error
            }

            switch(mode)
            {
                case 'r':
                    for (int subpointer = 14;
                         subpointer < PCB_POINTER_SIZE - PAGE_TABLE_SIZE;
                         subpointer = subpointer + PCB_SUBPOINTER_SIZE)
                    {
                        if (name_coincidence(pcb_table, pointer + subpointer + 1, file_name))
                        {
                            // Verificar validez
                            //Crear el struct
                        }
                    }

                    // Manejar error

                    break;

                case 'w':
                    for (int subpointer = 14;
                         subpointer < PCB_POINTER_SIZE - PAGE_TABLE_SIZE;
                         subpointer = subpointer + PCB_SUBPOINTER_SIZE)
                    {
                        if (name_coincidence(pcb_table, pointer + subpointer + 1, file_name))
                        {
                            // Verificar validez
                            //Crear el struct
                        }
                    }

                    // Manejar error
                    
                    break;
            }



        }
    }

    // Manejar error

}




'''

CrmsFile* cr_open(int process_id, char* file_name, char mode).
	Buscar en la tabla PCB el proceso con id process_id
	si no existe:
		error
	if mode = "r"
		Buscar en cada subentrada del PCB el archivo con nombre file_name
		if no existe:
			error
		extraer la informacion
		cmrsfile = {informacion}
	if mode = "w"
		Buscar en cada subentrada del PCB el archivo con nombre file_name
		if exista:
			error
		cmrsfile = {}
	return CmrsFile*


'''
