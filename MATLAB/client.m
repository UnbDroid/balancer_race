t = tcpip('192.168.200.1', 9001, 'NetworkRole', 'client'); %criando o objeto tcpip com os parametros corretos para a comunicação com o rasp
fopen(t);%abrindo o  canal de comunicação

while true
    
    data = fread(t, 102);%recebendo os dados 
    fwrite(t, "ok!");%mandano a confirmação para o server
    tdata = char(data');%colocando os caracteres recebidos em vetor linha
    
    %convertendo de char para floats
    datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');
    
    gyroX = str2num(datanum{1});
    gyroY = str2num(datanum{2});
    gyroZ = str2num(datanum{3});
    acellX = str2num(datanum{4});
    acellY = str2num(datanum{5});
    acellZ = str2num(datanum{6});
    magX = str2num(datanum{7});
    magY = str2num(datanum{8});
    magZ = str2num(datanum{9});
    dt = str2num(datanum{10});
    
    pause(0.05)
end

fclose(t);%fechando o canal de comunicação