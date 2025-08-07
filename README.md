Simulador de Planificación de Procesos (C)
Autor: Viviana Arango,Andres Echeverri ,David Quintero 
Asignatura: Sistemas Operativos (ST0257)
Proyecto: Simulador Round-Robin con interrupciones y archivos por PID

##1. Resumen del proyecto
Este proyecto implementa un simulador educativo de planificación de procesos en C. Emula un planificador Round-Robin donde cada proceso:

-Tiene un PID, registros simulados AX, BX, CX, un contador PC y un Quantum.

-Lee su lista de instrucciones desde un archivo separado (ej. 1.txt, 2.txt, ...).

-Soporta instrucciones: ADD, SUB, MUL, INC, JMP, NOP.

-Presenta cambios de contexto (guardado/carga de estado), interrupciones simuladas (bloqueo) y desbloqueo (reintento).

-Registra toda la salida en simulacion.log.

##2. Requisitos
Sistema Linux (o WSL) o usar OnlineGDB.

-gcc instalado.

(Opcional) /usr/bin/time para medir memoria máxima usada.

----Instalación de gcc (Debian/Ubuntu):-----
sudo apt update
sudo apt install build-essential
3. Estructura de archivos del proyecto
/simulador/
 ├─ main.c             # Código fuente (simulador)
 ├─ procesos.txt       # Archivo con la lista de procesos ¿
 ├─ 1.txt              # Instrucciones para PID 1
 ├─ 2.txt              # Instrucciones para PID 2
 ├─ ...                # más archivos PID.txt según sea necesario
 └─ simulacion.log     # Archivo de salida (se genera al ejecutar)
##4. Formato del archivo procesos.txt (ES OBLIGATORIO)
Cada línea describe un proceso y sus parámetros con pares clave=valor separados por comas. Ejemplo:
PID:1,AX=3,BX=2,CX=1,Quantum=3
PID:2,AX=4,BX=2,CX=0,Quantum=2
PID:<numero> — Obligatorio. Identificador del proceso (entero).

AX=<numero>, BX=<numero>, CX=<numero> — Registros iniciales (opcionales; por defecto 0 si no están).
Quantum=<numero> — Quantum del proceso (obligatorio).

Notas:
-Use comas , para separar campos.
-No deje espacios antes/después de claves (el parser tolera, pero mantén formato consistente).
-El orden puede variar, pero PID y Quantum deben estar presentes.

##5. Formato de archivos de instrucciones (por proceso)
Por cada PID:n en procesos.txt debe haber un archivo n.txt con las instrucciones del proceso, una instrucción por línea. Ejemplos de instrucciones válidas:

ADD AX,BX    # Ax = Ax + Bx
ADD AX,5     # Ax = Ax + 5
SUB AX,BX
MUL AX,2
INC AX
JMP 0        # Salta a la instrucción número 0 (la primera)
NOP          # No hace nada, gasta quantum
Reglas:

-JMP k salta a la instrucción con índice k (la primera instrucción es índice 0).

-INC R incrementa el registro R (AX/BX/CX).

-Los registros válidos: AX, BX, CX.

-Un archivo n.txt puede terminar con o sin newline final.

##6. Compilar en Linux (paso a paso)
Abre una terminal y ve a la carpeta del proyecto:

cd /simulador
Compilar (recomendado con codificación UTF-8):

gcc -fexec-charset=UTF-8 main.c -o procplanner
-fexec-charset=UTF-8 es opcional pero evita problemas de codificación en la salida (simulacion.log) si usas caracteres acentuados.

##7. Ejecutar el simulador
El programa espera que le pases el archivo de procesos por -f:

./procplanner -f procesos.txt
Nota: El programa redirige su salida a simulacion.log (por diseño), por lo que no verás nada en pantalla; la salida estará en simulacion.log.

##Ver el log después de ejecutar:

cat simulacion.log

Si prefieres ver la salida en tiempo real mientras se escribe al log:

tail -f simulacion.log
# en otra terminal corre:
./procplanner -f procesos.txt

##8. Si no aparece nada o el log está vacío: checklist de solución
¿Existe procesos.txt y está en la misma carpeta? ls para comprobar.

¿Existen 1.txt, 2.txt, ... con instrucciones válidas (sin espacios extraños)? ls y cat 1.txt.

¿Ejecutaste ./procplanner -f procesos.txt? Asegúrate de usar -f.

¿Tu procesos.txt contiene ceros por error? Ábrelo con nano procesos.txt y comprueba valores.

¿Tienes permisos de escritura para generar simulacion.log? ls -l y chmod si es necesario.

Si simulacion.log existe pero está vacío, intenta ejecutar el binario y luego ls -l simulacion.log para verificar timestamp.

##9. Ejemplo mínimo (copiar/pegar)
procesos.txt

PID:1,AX=3,BX=2,CX=1,Quantum=2
PID:2,AX=4,BX=2,CX=0,Quantum=2

1.txt
ADD AX,BX
INC AX
SUB CX,1
NOP

2.txt
MUL AX,2
SUB AX,BX
INC CX
NOP
Compilar y ejecutar:

gcc -fexec-charset=UTF-8 main.c -o procplanner
./procplanner -f procesos.txt
cat simulacion.log

##10. Explicación detallada de la salida (simulacion.log) — línea a línea
El simulacion.log contiene registros human-readable del comportamiento. Ejemplo de salida esperada y su significado:

[Cambio de contexto]
Cargando estado del Proceso 1
 -> Ejecutando: ADD AX,BX | Resultado: AX=5
 -> Ejecutando: INC AX | Resultado: AX=6
!! Interrupción simulada en Proceso 1. Se bloquea antes de agotar quantum.
Explicación:

[Cambio de contexto] — se inició el turno para un proceso (cambio entre procesos).

Cargando estado del Proceso 1 — el proceso 1 pasa a Ejecutando.

-> Ejecutando: ADD AX,BX | Resultado: AX=5 — se ejecutó la instrucción y muestra el registro modificado (AX ahora vale 5).

!! Interrupción simulada ... — durante el quantum, con cierta probabilidad el proceso quedó bloqueado (simulación de E/S o evento externo).

Más bloques:

[Revisando procesos bloqueados...]
 -> Proceso 1 ha sido desbloqueado.
Después de cada ciclo completo se revisan procesos bloqueados; algunos son desbloqueados (regresan a Listo).

Terminación:

nginx
Copiar
Editar
Proceso 2 terminado.
Cuando PC alcanza el número de instrucciones (PC == num_instrucciones) el proceso cambia a Terminado y no volverá a ejecutarse.

##11. Detalles técnicos importantes 
PC (Program Counter): inicia en 0 y se incrementa después de ejecutar cada instrucción. Si un proceso tiene 3 instrucciones, al terminar PC quedará en 3 (indicando que consumió 3 instrucciones).

Quantum: número máximo de instrucciones que puede ejecutar por turno.

Interrupciones: implementadas con probabilidad (variable , actualmente la pusimos en 25%) — el proceso se marca Bloqueado.

Desbloqueo: tras cada ciclo, cada proceso Bloqueado tiene 50% de probabilidad de volver a Listo.

JMP: JMP k ajusta PC para que la próxima instrucción ejecutada sea la k (la primera instrucción es 0).

NOP: ocupa ciclo pero no cambia registros.

Registro de resultados: el programa imprime el resultado de las operaciones para facilitar la trazabilidad pedagógica.

##12. Hacer la ejecución determinista
Para quitar la aleatoriedad y reproducir la misma ejecución cada vez, abre main.c y busca:

srand(time(NULL));
y reemplázalo por una semilla fija, por ejemplo:

srand(12345);
Luego recompila. Con esto las interrupciones y desbloqueos serán reproducibles.

##13. Ver uso de memoria y CPU del programa
Para medir la memoria máxima usada por la ejecución:

/usr/bin/time -v ./procplanner -f procesos.txt
Busca la línea:

Maximum resident set size (kbytes): <valor>
Ejemplo : Maximum resident set size (kbytes): 976 → 976 KB.

Comprobar CPU / info del procesador:
lscpu
Información de memoria total del sistema:
free -h
##14. Errores comunes y soluciones rápidas
Mensaje "Uso: ./procplanner -f archivo_procesos.txt"
→ significa que no pasaste -f <archivo>. Ejecuta ./procplanner -f procesos.txt.

simulacion.log vacío o solo ceros en registros
→ revisa procesos.txt (valores iniciales) y que existan PID.txt correctos.

Error al abrir n.txt
→ nombre del archivo debe coincidir con PID (por ejemplo PID:3 → 3.txt).

Permisos
→ chmod +x procplanner para dar permiso de ejecución (si corresponde).

Compilador no encontrado
→ instalar gcc (sudo apt install build-essential).

##15. Cómo ejecutar en OnlineGDB (paso a paso)
Ir a https://www.onlinegdb.com/online_c_compiler

Crear/pegar main.c en el editor.

En el árbol de archivos (panel izquierdo) crear procesos.txt, 1.txt, 2.txt y pegar los ejemplos.

En la parte inferior buscar la opción Command line arguments (o un cuadro de entrada para "Arguments") y poner:

-f procesos.txt
Click en Run.

En el panel izquierdo aparecerá simulacion.log cuando termine; haz click para abrirlo.

(Si tu instancia de OnlineGDB no tiene el campo de argumentos, puedes simularlo editando main.c temporalmente para pasar "procesos.txt" como nombre fijo o colocar argv manualmente. Pero OnlineGDB suele tener campo de argumentos).


##16. Conclusión
El simulador demuestra cómo un planificador Round-Robin administra el uso del CPU entre procesos, muestra la importancia del quantum, permite estudiar interrupciones y bloqueos, y sirve como herramienta didáctica para comprender la estructura interna del cambio de contexto y los efectos sobre registros. El consumo de recursos es mínimo (976 KB en un sistema con 15 GB), lo que permite escalar la simulación para experimentos adicionales.





