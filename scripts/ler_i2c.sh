if [ $# -lt 1 ]; then
	echo "Porta padrao 0x68"
	gate="0x68"
else
	gate=${1}
fi

while [ 1 ]
do clear
i2cdump -y 1 ${gate}
sleep 0.2
done

