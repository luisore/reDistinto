#!/bin/bash
# SisOp 1C2018
# Instalador de ReDistinto
# DefaultName

ORIG_PATH=$(pwd)
TP_PATH=$ORIG_PATH"/tp-2018-1c-DefaultName"
USER="pablo092"
MAIL="elpabli09@gmail.com"

function mostrar() {
  echo "$1"
  echo "$2"
}

function avance() {
  if [ $CONTAME -lt 100 ]
  then
    printf "\r\e[37;41m \e[?25l ${1}...        (${CONTAME} of 100) \e[0m"
    CONTAME=$(($CONTAME + 20))
    sleep .5
  fi
}

function bajar(){
  mostrar "" "Ingrese sus credenciales de GIT:"
  read -p " GIT user name: " GUSER;
  read -p " GIT user email: " GEMAIL;
  if [ -z "$GUSER" ]
  then
    GUSER="$USER"
  fi
  if [ -z "$GEMAIL" ]
  then
    GEMAIL="$MAIL"
  fi
  git config --global user.name $GUSER
  git config --global user.email $GEMAIL
  git clone "https://${USER}@github.com/sisoputnfrba/tp-2018-1c-DefaultName.git"
  git clone "https://${USER}@github.com/sisoputnfrba/so-commons-library.git"
  mostrar "Todos los repositorios han sido clonados."
}

function instalarLibs(){
  mostrar
  cd $ORIG_PATH"/so-commons-library"
  sudo make
  sudo make install
  cd $ORIG_PATH

  mostrar "Todas las librerias han sido instaladas."
}

function instalar {
  echo ""
  echo " .:: El sistema sera compilado ::."
  echo ""
  declare -i CONTAME=19

  cd $TP_PATH"/Planificador"  
  sudo make clean
  sudo make
  cd ..
  avance "PLANIFICADOR compilado satisfactoriamente"

  cd $TP_PATH"/ESI"
  sudo make clean
  sudo make
  cd ..
  avance "ESI compilado satisfactoriamente"

  cd $TP_PATH"/Coordinador"
  sudo make clean
  sudo make
  cd ..
  avance "COORDINADOR compilado satisfactoriamente"

  cd $TP_PATH"/Instancia"
  sudo make clean
  sudo make
  cd ..
  avance "INSTANCIA compilado satisfactoriamente"

  mostrar

  mostrar "Todos los modulos han sido compilados."
  cd $TP_PATH
  sudo chmod -R a+rwX *
  cd ..
  chown -R utnso tp-2018-1c-DefaultName
}

function configurar {
  echo ""
  echo " .:: Complete los campos para el setup y luego instalar ::."
  echo " Los campos son opcionales. Dejar cualquier campo vacio para mantener la configuracion default."
  echo ""
  read -p " IP_COORDINADOR: " COORDINADOR;
  read -p " PUERTO_ESCUCHA_CONEXIONES_COORDINADOR: " COORDINADOR_P;

  read -p " IP_PLANIFICADOR: " PLANIFICADOR;
  read -p " PUERTO_ESCUCHA_CONEXIONES_PLANIFICADOR: " PLANIFICADOR_P;

  declare -i CONTAME=19
  mostrar
  ####################################### COORDINADOR #######################################
  avance "Creando archivo de configuracion para COORDINADOR"
  EL_FILE=$TP_PATH"/Coordinador/coordinador.config"gcc -L/home/utnso/workspace/library_commons -Wall -o test Planificador.c -llibrary_common
  agregarCoso "IP_COORDINADOR" $EL_FILE $COORDINADOR
  agregarCoso "PUERTO_ESCUCHA_CONEXIONES" $EL_FILE $COORDINADOR_P
  ####################################### PLANIFICADOR #######################################
  avance "Creando archivo de configuracion para PLANIFICADOR"
  EL_FILE=$TP_PATH"/Planificador/planificador.config"
  agregarCoso "IP_PLANIFICADOR" $EL_FILE $PLANIFICADOR
  agregarCoso "PUERTO_ESCUCHA_CONEXIONES" $EL_FILE $PLANIFICADOR_P
  agregarCoso "IP_COORDINADOR" $EL_FILE $COORDINADOR
  agregarCoso "PUERTO_COORDINADOR" $EL_FILE $COORDINADOR_P
  ####################################### INSTANCIA #######################################
  avance "Creando archivo de configuracion para ESI"
  EL_FILE=$TP_PATH"/Instancia/instancia.config"
  agregarCoso "IP_COORDINADOR" $EL_FILE $COORDINADOR
  agregarCoso "PUERTO_COORDINADOR" $EL_FILE $COORDINADOR_P
  ####################################### ESI #######################################
  avance "Creando archivo de configuracion para ESI"
  EL_FILE=$TP_PATH"/ESI/esi.config"
  agregarCoso "IP_PLANIFICADOR" $EL_FILE $PLANIFICADOR
  agregarCoso "PUERTO_ESCUCHA_CONEXIONES" $EL_FILE $PLANIFICADOR_P
  agregarCoso "IP_COORDINADOR" $EL_FILE $COORDINADOR
  agregarCoso "PUERTO_COORDINADOR" $EL_FILE $COORDINADOR_P
  #######################################################################################
  mostrar
  mostrar "Todos los archivos han sido configurados."
}

function borrar(){
  mostrar "" " .:: Se iniciara el proceso de borrado ::."
  echo ""
  read -p "Estas seguro que quieres desinstalar? [Y/N] " BLEH
  case $BLEH in
    [Yy]* ) otrave;;
    [Nn]* ) echo "Se ha cancelado la operacion";;
    * ) echo "No se ha eliminado ningun componente.";;
  esac
}

function otrave(){
  mostrar "" "Realmente, esto eliminara todo."
  read -p "Estas seguro? [Y/N] " RESP
  case $RESP in
  [Yy]* ) mostrar "Eliminando...."
        sudo rm -rf $TP_PATH
            sudo rm -rf $ORIG_PATH"/so-commons-library"
        mostrar "Se ha eliminado!";;
  [Nn]* ) echo "Se deshizo la operacion";;
  * ) echo "No se realizaron cambios";;
  esac
}

function agregarCoso(){
    if [ -n "$3" ]
    then
      sed -i "/${1}/ c\\${1}=${3}" $2
    fi
  }

function forceRoot(){
  if [[ $(id -un) != "root" ]]
  then
   echo "Si no eres root, no puede instalar el software."
   sudo $0
   exit
  fi
}

clear
forceRoot
echo ""
printf " \e[31;4;1m -== SisOp 1C2018 - ReDistinto ==- \e[0m"
mostrar
options=("Descargar - Clona los repositorios" \
    "Instalar libs & commons - Setup de las libs & commons libraries" \
    "Setup - Crea los archivos de configuracion" \
    "Instalar ReDistinto - Compila el sistema" \
    "Borrar - Elimina el sistema ReDistinto" "Quit")
PS3="Seleccione una opcion: "
select opt in "${options[@]}"; do 
    case "$REPLY" in
    1 ) bajar;;
    2 ) instalarLibs;;   
    3 ) configurar;;
    4 ) instalar;;
    5 ) borrar;;
    6 ) echo "#DefaultName - SisterCall"; break;;
    *) echo "Opcion invalida. Intenta con otra."; continue;;
    esac
done
