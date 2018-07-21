#!/bin/bash 
echo "Compilando"
cd ESI
make clean
make all
cd ..
cd Coordinador
make clean
make all
cd ..
cd Instancia
make clean 
make all
cd ..
cd Planificador
make clean 
make all
cd ..
echo "Copiando instancias"
for i in {1,2}; do 
    cp -a Instancia ../Instancia$(cat /dev/urandom | tr -dc '_' | fold -w 1 | head -n 1 )$i; 
done
echo "Copiando esis"
for i in {1..6}; do 
    cp -a ESI ../ESI$(cat /dev/urandom | tr -dc '_' | fold -w 1 | head -n 1 )$i; 
done