# Simulador de Planificación de Procesos (C)

**Autores:** Viviana Arango, Andrés Echeverri, David Quintero  
**Asignatura:** Sistemas Operativos (ST0257)  
**Proyecto:** Simulador Round-Robin con interrupciones y archivos por PID  

---

## 1. Resumen del Proyecto
Este proyecto implementa un **simulador educativo de planificación de procesos en C** bajo Linux. Emula un planificador **Round-Robin** que:

- Asigna un quantum a cada proceso.
- Guarda y restaura el estado (PC, registros AX/BX/CX).
- Lee instrucciones desde un archivo por proceso (ej: `1.txt`, `2.txt`...).
- Ejecuta instrucciones: `ADD`, `SUB`, `MUL`, `INC`, `JMP`, `NOP`.
- Simula **interrupciones** (bloqueo) y **desbloqueos**.
- Registra toda la ejecución en `simulacion.log`.

---

## 2. Especificaciones Técnicas
- **Arquitectura:** x86_64  
- **CPU OP-MODE(S):** 32-bit, 64-bit  
- **Modelo CPU:** Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz  
- **RAM total del sistema:** 15 GB  
- **Máxima RAM utilizada por el simulador:** 976 KB  
- **Compilador usado:** gcc (Ubuntu 13.2.0)  

---

## 3. Requisitos
- Sistema Linux (nativo, WSL o OnlineGDB).
- `gcc` instalado.
- (Opcional) `/usr/bin/time` para medir consumo de memoria.

**Instalación de GCC en Debian/Ubuntu:**
```bash
sudo apt update
sudo apt install build-essential
```

---

## 4. Estructura del proyecto

```graphql
/simulador/
 ├─ main.c             # Código fuente
 ├─ procesos.txt       # Lista de procesos
 ├─ 1.txt              # Instrucciones para PID 1
 ├─ 2.txt              # Instrucciones para PID 2
 ├─ ...                # Más archivos según cantidad de procesos
 └─ simulacion.log     # Salida de la simulación
```
---

## 5. Formato del archivo procesos.txt

Cada línea define un proceso:

```plaintext
PID:1,AX=3,BX=2,CX=1,Quantum=3
PID:2,AX=4,BX=2,CX=0,Quantum=2
```
Campos obligatorios:
- **PID:<n>** → Identificador único del proceso.
- **Quantum=<n>** → Quantum asignado.
- **Campos opcionales (por defecto en 0):** AX, BX, CX.
- **Formato:** sin espacios innecesarios, separados por comas.

---

## 6. Archivo de instrucciones

Cada proceso debe tener un archivo n.txt con sus instrucciones, por ejemplo:

**1.txt**
```plaintext
ADD AX,BX
INC AX
SUB CX,1
NOP
```

**2.txt**
```plaintext
MUL AX,2
SUB AX,BX
INC CX
NOP
```
---

## 7. Código fuente (main.c)

```c
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
```
---

## 8. Compilación (paso a paso en la terminal de Linux)

A continuaciòn la guia exacta de los comandos a seguir para ejecutar el programa:

- Abrir la terminal (Ctrl+Alt+T en Ubuntu/Linux Mint).
- Ir a la carpeta del proyecto (ejemplo: ~/simulador_contexto):

```bash
cd ~/simulador_contexto
```

- Si no tiene la carpeta creada, la puede crear con:

```bash
mkdir ~/simulador_contexto
```

- Luego, para crear cada archivo dentro de la carpeta, usa el editor de archivos *nano* de esta forma:

**1. Crear el archivo fuente main.c**
```bash
nano main.c
```
- Copia y pega el codigo anteriormente dado en el numeral 7.
- Para guardar Ctrl+O, enter y para salir Ctrl+X.

**2. Crear el archivo de procesos.txt**
```bash
nano procesos.txt
```
- Copia y pega la informacion anteriormente dada en el numeral 5.
- Para guardar Ctrl+O, enter y para salir Ctrl+X.

**3. Crear archivo de proceso 1.txt y proceso 2.txt**
```bash
nano 1.txt
nano 2.txt
```
- Copia y pega la informacion anteriormente dada en el numeral 6.
- Para guardar Ctrl+O, enter y para salir Ctrl+X.

Ahora, para **compilar** el codigo C, se debe usar el siguiente comando:
```bash
gcc -fexec-charset=UTF-8 main.c -o procplanner
```
Donde:
- **gcc** → es el compilador de C de GNU.
- **-fexec-charset=UTF-8** → asegura que el conjunto de caracteres usado en las cadenas del ejecutable sea UTF-8, evitando problemas con acentos o caracteres especiales.
- **main.c** → es el archivo de código fuente que se quiere compilar.
- **-o procplanner** → le dice a gcc que el ejecutable generado se llame procplanner.

Lo siguiente es la **ejecucion del simulador**, se debe usar el siguiente comando:
```bash
./procplanner -f procesos.txt
```
Donde:
- **./** → indica que quieres ejecutar un programa que está en el directorio actual.
- **procplanner** → es el ejecutable que se compilo en el paso anterior.
- **-f procesos.txt** → es un argumento que el programa espera, y que en este caso le indica qué archivo de procesos debe cargar para la simulación.

Para la **Visualizacion del log**:
```bash
cat simulacion.log
```
Donde:
- **cat** → comando que muestra el contenido de un archivo en la terminal.
- **simulacion.log** → es el archivo de texto donde el simulador guardó toda la ejecución paso a paso.

---

## 9. Explicacion de simulacion.log

Aqui se tiene todo el registro de la simulacion que aparece en pantalla:

```plaintext
[Cambio de contexto]
Cargando estado del Proceso 1
 -> Ejecutando: ADD AX,BX | Resultado: AX=5
 -> Ejecutando: INC AX | Resultado: AX=6
 -> Ejecutando: SUB CX,1 | Resultado: CX=0

[Cambio de contexto]
Cargando estado del Proceso 2
 -> Ejecutando: MUL AX,2 | Resultado: AX=8
!! Interrupción simulada en Proceso 2. Se bloquea antes de agotar quantum.

[Revisando procesos bloqueados...]
 -> Proceso 2 ha sido desbloqueado.

[Cambio de contexto]
Cargando estado del Proceso 1
!! Interrupción simulada en Proceso 1. Se bloquea antes de agotar quantum.

[Cambio de contexto]
Cargando estado del Proceso 2
 -> Ejecutando: SUB AX,BX | Resultado: AX=6
!! Interrupción simulada en Proceso 2. Se bloquea antes de agotar quantum.

[Revisando procesos bloqueados...]

[Revisando procesos bloqueados...]
 -> Proceso 1 ha sido desbloqueado.

[Cambio de contexto]
Cargando estado del Proceso 1
 -> Ejecutando: NOP
Proceso 1 terminado.

[Revisando procesos bloqueados...]

[Revisando procesos bloqueados...]
 -> Proceso 2 ha sido desbloqueado.

[Cambio de contexto]
Cargando estado del Proceso 2
 -> Ejecutando: INC CX | Resultado: CX=1
!! Interrupción simulada en Proceso 2. Se bloquea antes de agotar quantum.

[Revisando procesos bloqueados...]
 -> Proceso 2 ha sido desbloqueado.

[Cambio de contexto]
Cargando estado del Proceso 2
 -> Ejecutando: NOP
Proceso 2 terminado.

[Revisando procesos bloqueados...]
```

Significado:
- [Cambio de contexto]: se cambia el proceso activo.
- Cargando estado: el proceso pasa a "Ejecutando".
- Ejecutando: se muestra la instrucción y el valor resultante.
- Interrupción simulada: el proceso se bloquea antes de agotar quantum.

---

## 10. Detalles de implementacion

- **PC (Program Counter):** inicia en 0, incrementa tras cada instrucción.
- **Quantum:** máximo de instrucciones por turno.
- **Interrupciones:** probabilidad ~25%.
- **Desbloqueo:** probabilidad ~50% después de cada ciclo.
- **NOP:** consume ciclo sin modificar registros.
- **Registro de operaciones:** todas las modificaciones se guardan para trazabilidad.


## 11. Medicion de recursos

Los siguientes son los comandos a usar, ejecutando en la terminal para conocer la informacion del hardware:

- *Medir memoria usada*
```bash
/usr/bin/time -v ./procplanner -f procesos.txt
```

- *Ver informacion del CPU*
```bash
lscpu
```

- *Ver RAM total*
```bash
free -h
```

---

## 12. Conclusion

El simulador permite visualizar cómo un planificador Round-Robin administra el CPU entre procesos, mostrando el impacto del quantum, interrupciones y bloqueos. Es una herramienta didáctica para entender el cambio de contexto y su efecto sobre registros, con consumo mínimo de recursos (976 KB en un sistema con 15 GB de RAM).















