#!/bin/sh
if [ $# -lt 1 ]; then
	echo "ERROR: Falta argumentos, parça!"
	echo "Arg1 = Nome do Arquivo C"
else
	nome_codigo=${1}
	nome_binario=compilation_test

	../gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc -o ${nome_binario} ${nome_codigo} -I. -L. ./libwiringPi.so -lm
	
	# encrypted_password="$(perl -e 'printf("%s\n", crypt($ARGV[0], "password"))' "${password}")"

	# scp ./${nome_binario} ${login_rasp}@${ip_rasp}:${dst_folder_rasp}
	rm ./${nome_binario}
fi

