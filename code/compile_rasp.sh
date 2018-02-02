#!/bin/sh
if [ $# -lt 2 ]; then
	echo "ERROR: Falta argumentos, parça!"
	echo "Arg1 = Nome do Arquivo C"
	echo "Arg2 = Nome do Arquivo Objeto de Saída"
	echo "Arg3 = IP do Raspberry"
else
	if [ $# -lt 3 ]; then
		echo "Warning: IP Address as default (192.168.200.1)!"
		ip_rasp="192.168.200.1"
	else
		ip_rasp=${3}
	fi

	nome_codigo=${1}
	nome_binario=${2}


	login_rasp="pi"
	#password="godroidgo"
	dst_folder_rasp="/home/pi/ccdir"

	../gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc -o ${nome_binario} ${nome_codigo} -I. -L. ./libwiringPi.so -lm

	#arm-linux-gnueabi-gcc -o ${nome_binario} ${nome_codigo} -I. -L. ./libwiringPi.so -lm
	#./libm.so.6

	# encrypted_password="$(perl -e 'printf("%s\n", crypt($ARGV[0], "password"))' "${password}")"

	scp ./${nome_binario} ${login_rasp}@${ip_rasp}:${dst_folder_rasp}
	rm ${nome_binario}
fi

