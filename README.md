 Simulador de Planificación de Procesos en C

Este proyecto implementa un simulador educativo de planificación de procesos en lenguaje C. Permite observar el comportamiento de un sistema operativo al ejecutar múltiples procesos usando planificación con quantum fijo (round-robin), interrupciones simuladas y gestión de estados de proceso (listo, bloqueado, terminado).

## 📂 Características principales

- Lectura de procesos desde un archivo `procesos.txt`.
- Instrucciones de cada proceso almacenadas en archivos separados (`1.txt`, `2.txt`, etc.).
- Planificador tipo **Round-Robin** con quantum configurable por proceso.
- Simulación de **interrupciones aleatorias** y **desbloqueo posterior**.
- Cambios de contexto y resultados detallados por instrucción.
- Toda la salida se redirige a un archivo `simulacion.log`.

## Ejecución

Compilar:
```bash
gcc -fexec-charset=UTF-8 main.c -o procplanner
