#include <stdlib.h>
#include <stdio.h>

#include "crms_API.h"

char* path = "mem.bin";
int PCB_SIZE = 4096;
int PCB_POINTER_SIZE = 256;
int POINTERS_AMOUNT = PCB_SIZE / PCB_POINTER_SIZE;
int PCB_SUBPOINTER_SIZE = 21;
int PAGE_TABLE_SIZE = 32;
int PROCESS_NAME = 12;
int SUBPOINTERS_AMOUNT = (PCB_POINTER_SIZE - 2 - PROCESS_NAME - PAGE_TABLE_SIZE)/PCB_SUBPOINTER_SIZE;
int PROCESS_FILE_NAME = 12;
int PROCESS_FILE_SIZE = 4;
int PROCESS_FILE_VIRTUAL_MEMORY = 4;



int name_coincidence(char* file_name_found, char* file_name)
{
    for (int index = 0; index < PROCESS_FILE_NAME; index++)
    {
        if ((char) atoi(file_name_found[index]) != file_name[index]) return 0;
    }
    return 1;
}

void crms_file_init(CrmsFile* crms_file, char process_id, char validation,
                    char* file_name, char* file_size, char* virtual_memory)
{
    // Completar
}

CrmsFile* cr_open(int process_id, char* file_name, char mode)
{
    
    FILE* memory;
    memory = fopen(path, "rb");  // r for read, b for binary

    //unsigned char pcb_table[PCB_SIZE];
    //fread(pcb_table, sizeof(pcb_table), 1, memory); // read PCB_SIZE bytes to our buffer

    char status;
    char process_id;
    for (int pointer = 0; pointer < POINTERS_AMOUNT; pointer++)
    {
        fread(&status, 1, 1, memory);
        fread(&process_id, 1, 1, memory);

        if (atoi(process_id) == process_id)
        {
            if (atoi(status) != 0x01)
            {
                // Manejar error
            }

            //char process_name[12];
            //fread(process_name, 1, PROCESS_NAME, memory);
            fseek(memory, PROCESS_NAME, SEEK_CUR);

            char validation;
            char file_name_found[PROCESS_FILE_NAME];
            char file_size[PROCESS_FILE_SIZE];
            char virtual_memory[PROCESS_FILE_VIRTUAL_MEMORY];
            switch(mode)
            {
                case 'r':
                    for (int subpointer = 0; subpointer < SUBPOINTERS_AMOUNT; subpointer++)
                    {
                        fread(&validation, 1, 1, memory);
                        fread(file_name_found, PROCESS_FILE_NAME, 1, memory);
                        fread(file_size, PROCESS_FILE_SIZE, 1, memory);
                        fread(virtual_memory, PROCESS_FILE_VIRTUAL_MEMORY, 1, memory);
                        if (name_coincidence(file_name_found, file_name))
                        {
                            // Verificar validez
                            if (atoi(validation) == 0x01)
                            {
                                // Crear el struct
                                CrmsFile crms_file;
                                // ojo que algunos parametros están en el stack
                                crms_file_init(*crms_file, process_id, validation,
                                               file_name, file_size, virtual_memory);
                                fclose(memory);
                                return crms_file;

                            }
                        }

                    }

                    // Manejar el error
                    fclose(memory);

                    break;
                
                case 'w':
                    for (int subpointer = 0; subpointer < SUBPOINTERS_AMOUNT; subpointer++)
                    {
                        fread(&validation, 1, 1, memory);
                        fread(file_name_found, PROCESS_FILE_NAME, 1, memory);
                        fread(file_size, PROCESS_FILE_SIZE, 1, memory);
                        fread(virtual_memory, PROCESS_FILE_VIRTUAL_MEMORY, 1, memory);
                        if (name_coincidence(file_name_found, file_name))
                        {
                            // Verificar validez
                            if (atoi(validation) == 0x01)
                            {
                                // Manejar el error
                                fclose(memory);

                            }
                        }

                    }

                    CrmsFile crms_file;
                    // ojo que algunos parametros están en el stack
                    // de donde salen todos los argumentos?
                    crms_file_init(*crms_file, process_id, validation,
                                    file_name, file_size, virtual_memory);
                    fclose(memory);
                    return crms_file;

                    break;
            }

        }
        else
        {
            fseek(memory, PCB_POINTER_SIZE - 2, SEEK_CUR);
        }
    }

    // Manejar error
    fclose(memory);



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
