#!/bin/sh
if [ $# -lt 2 ]; then
	echo "ERROR: Falta argumentos, parça!"
	echo "Arg1 = Nome do Arquivo C"
	echo "Arg2 = Nome do Arquivo Objeto de Saída"
else
	nome_codigo=${1}
	nome_binario=${2}

	arm-linux-gnueabi-gcc -o ${nome_binario} ${nome_codigo} -I. -L. ./libwiringPi.so

	# encrypted_password="$(perl -e 'printf("%s\n", crypt($ARGV[0], "password"))' "${password}")"

	# scp ./${nome_binario} ${login_rasp}@${ip_rasp}:${dst_folder_rasp}
	rm ./${nome_binario}
fi

