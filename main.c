#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_INSTRUCCIONES 100
#define MAX_PROCESOS 10

// Estructura del proceso
typedef struct {
    int pid;
    int pc;
    int ax, bx, cx;
    int quantum;
    char estado[15];
    char instrucciones[MAX_INSTRUCCIONES][30];
    int num_instrucciones;
} Proceso;

Proceso procesos[MAX_PROCESOS];
int num_procesos = 0;

void limpiar_cadena(char* str) {
    while (*str) {
        if (*str == '\n' || *str == '\r') *str = '\0';
        str++;
    }
}

void cargar_instrucciones(Proceso* p) {
    char filename[20];
    sprintf(filename, "%d.txt", p->pid);
    FILE* archivo = fopen(filename, "r");
    if (!archivo) {
        printf("Error al abrir archivo de instrucciones: %s\n", filename);
        exit(1);
    }
    while (fgets(p->instrucciones[p->num_instrucciones], 30, archivo)) {
        limpiar_cadena(p->instrucciones[p->num_instrucciones]);
        p->num_instrucciones++;
    }
    fclose(archivo);
}

void cargar_procesos(const char* nombre_archivo) {
    FILE* archivo = fopen(nombre_archivo, "r");
    if (!archivo) {
        printf("No se pudo abrir el archivo de procesos.\n");
        exit(1);
    }
    char linea[100];
    while (fgets(linea, sizeof(linea), archivo)) {
        Proceso p = {0};
        p.ax = p.bx = p.cx = 0;
        char* token = strtok(linea, ",");
        while (token) {
            if (strncmp(token, "PID:", 4) == 0) p.pid = atoi(token + 4);
            else if (strncmp(token, "AX=", 3) == 0) p.ax = atoi(token + 3);
            else if (strncmp(token, "BX=", 3) == 0) p.bx = atoi(token + 3);
            else if (strncmp(token, "CX=", 3) == 0) p.cx = atoi(token + 3);
            else if (strncmp(token, "Quantum=", 8) == 0) p.quantum = atoi(token + 8);
            token = strtok(NULL, ",");
        }
        strcpy(p.estado, "Listo");
        cargar_instrucciones(&p);
        procesos[num_procesos++] = p;
    }
    fclose(archivo);
}

int obtener_valor(Proceso* p, const char* op) {
    if (strcmp(op, "AX") == 0) return p->ax;
    if (strcmp(op, "BX") == 0) return p->bx;
    if (strcmp(op, "CX") == 0) return p->cx;
    return atoi(op);
}

int* obtener_registro(Proceso* p, const char* op) {
    if (strcmp(op, "AX") == 0) return &p->ax;
    if (strcmp(op, "BX") == 0) return &p->bx;
    if (strcmp(op, "CX") == 0) return &p->cx;
    return NULL;
}

void ejecutar_instruccion(Proceso* p, char* instruccion) {
    char op[10], arg1[10], arg2[10];
    int n = sscanf(instruccion, "%s %[^,],%s", op, arg1, arg2);

    if (strcmp(op, "ADD") == 0 && n == 3) {
        int* reg = obtener_registro(p, arg1);
        int valor = obtener_valor(p, arg2);
        if (reg) {
            *reg += valor;
            printf(" -> Ejecutando: %s | Resultado: %s=%d\n", instruccion, arg1, *reg);
        }
    } else if (strcmp(op, "SUB") == 0 && n == 3) {
        int* reg = obtener_registro(p, arg1);
        int valor = obtener_valor(p, arg2);
        if (reg) {
            *reg -= valor;
            printf(" -> Ejecutando: %s | Resultado: %s=%d\n", instruccion, arg1, *reg);
        }
    } else if (strcmp(op, "MUL") == 0 && n == 3) {
        int* reg = obtener_registro(p, arg1);
        int valor = obtener_valor(p, arg2);
        if (reg) {
            *reg *= valor;
            printf(" -> Ejecutando: %s | Resultado: %s=%d\n", instruccion, arg1, *reg);
        }
    } else if (strcmp(op, "INC") == 0 && n == 2) {
        int* reg = obtener_registro(p, arg1);
        if (reg) {
            (*reg)++;
            printf(" -> Ejecutando: %s | Resultado: %s=%d\n", instruccion, arg1, *reg);
        }
    } else if (strcmp(op, "JMP") == 0 && n == 2) {
        int salto = atoi(arg1);
        if (salto >= 0 && salto < p->num_instrucciones) {
            p->pc = salto - 1;
            printf(" -> Ejecutando: %s | Salto a instrucción %d\n", instruccion, salto);
        }
    } else if (strcmp(op, "NOP") == 0) {
        printf(" -> Ejecutando: %s\n", instruccion);
    } else {
        printf(" -> Instrucción no reconocida: %s\n", instruccion);
    }
}

void intentar_desbloquear() {
    for (int i = 0; i < num_procesos; i++) {
        if (strcmp(procesos[i].estado, "Bloqueado") == 0) {
            if (rand() % 100 < 50) {
                strcpy(procesos[i].estado, "Listo");
                printf(" -> Proceso %d ha sido desbloqueado.\n", procesos[i].pid);
            }
        }
    }
}

void ejecutar_simulador() {
    int procesos_terminados = 0;
    while (procesos_terminados < num_procesos) {
        for (int i = 0; i < num_procesos; i++) {
            Proceso* p = &procesos[i];
            if (strcmp(p->estado, "Terminado") == 0 || strcmp(p->estado, "Bloqueado") == 0) continue;

            printf("\n[Cambio de contexto]\nCargando estado del Proceso %d\n", p->pid);

            int ejecutadas = 0;
            while (ejecutadas < p->quantum && p->pc < p->num_instrucciones) {
                if (rand() % 100 < 25) {
                    printf("!! Interrupción simulada en Proceso %d. Se bloquea antes de agotar quantum.\n", p->pid);
                    strcpy(p->estado, "Bloqueado");
                    break;
                }
                ejecutar_instruccion(p, p->instrucciones[p->pc]);
                p->pc++;
                ejecutadas++;
            }

            if (p->pc >= p->num_instrucciones) {
                strcpy(p->estado, "Terminado");
                procesos_terminados++;
                printf("Proceso %d terminado.\n", p->pid);
            } else if (strcmp(p->estado, "Bloqueado") != 0) {
                strcpy(p->estado, "Listo");
            }
        }
        printf("\n[Revisando procesos bloqueados...]\n");
        intentar_desbloquear();
    }
}

int main() {
    srand(time(NULL));
    freopen("simulacion.log", "w", stdout);
    cargar_procesos("procesos.txt");
    ejecutar_simulador();
    fclose(stdout);
    return 0;
}
