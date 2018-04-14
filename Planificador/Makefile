# Como nos conviene armar un makefile?
# Primero podemos definir algunas constantes que vamos a usar muchas veces
# Como por ejemplo, bibliotecas, flags, el compilador (el "CC")

TARGET = planificador 
LIBS = -lcrypto -lcommons -lreadline
CC = gcc 
CFLAGS = -g -Wall 

# Ahora creemos las tareas del makefile
# Cuando hagamos make sin argumentos, se ejecutará la tarea por default
# Mientras que si le pasamos un argumento, ejecutará la tarea con ese nombre
# como por ejemplo make all o make clean

.PHONY: default all clean
# Esta regla .PHONY es para decirle a make que si hay archivos con nombres similares
# a alguna regla, que entonces no asuma que el argumento es el archivo, sino que es la regla
# dado que va a tratar de tomar el archivo y no la regla por defecto

default: $(TARGET) # la regla por defecto (tp0)
all: default # Si ejecutan all, que ejecute la default

# Definamos un par de constantes mas, OBJECTS (que vamos a usar para generar los .o)
# y HEADERS, para incluir los headers que definamos.
# Nota: wildcard implica que seleccione todos los que cumplen con la regex (osea todos los que terminen con .h)

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS) # Las variables que le pasamos a la tarea, le decimos que use todos los .c y todos los .h
	$(CC) $(CFLAGS) -c $< -o $@ # Creamos los objects sin linkear

.PRECIOUS: $(TARGET) $(OBJECTS) # Nos permite mantener los archivos temporales terminados si matan a make con Cntl-C

$(TARGET): $(OBJECTS) 
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@ 

clean: # Finalmente creamos una tarea para eliminar todos los .o y el binario
	-rm -f *.o
	-rm -f $(TARGET)