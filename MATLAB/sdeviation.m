clear %limpando as variáveis

t = tcpip('192.168.200.1', 9001, 'NetworkRole', 'client'); %criando o objeto tcpip com os parametros corretos para a comunicação com o rasp
fopen(t);%abrindo o  canal de comunicação
%zerando os vaoler das medias
avgGyroX = 0;
avgGyroY = 0;
avgGyroZ = 0;
avgTriadX = 0;
avgTriadY = 0;
avgTriadZ = 0;


N_LEITURAS = 5000;%amostragem

%descartando as primeiras leituras
for i = 1:10   
    data = fread(t, 72);%recebendo os dados 
    fwrite(t, "ok!");%mandano a confirmação para o server
    tdata = char(data');%colocando os caracteres recebidos em vetor linha
    
    pause(0.005)
end

%loop para tirar as medidas e calcular a media
for i = 1:N_LEITURAS
    
    data = fread(t, 72);%recebendo os dados 
    fwrite(t, "ok!");%mandano a confirmação para o server
    tdata = char(data');%colocando os caracteres recebidos em vetor linha
    
    %convertendo de char para floats
    datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');
    
    GyroX(i) = str2num(datanum{1});
    GyroY(i) = str2num(datanum{2});
    GyroZ(i) = str2num(datanum{3});
    TriadX(i) = str2num(datanum{4});
    TriadY(i) = str2num(datanum{5});
    TriadZ(i) = str2num(datanum{6});

    %incrementando o somatório
    avgGyroX =  avgGyroX  + GyroX(i);
    avgGyroY =  avgGyroY  + GyroY(i);
    avgGyroZ =  avgGyroZ  + GyroZ(i);
    avgTriadX = avgTriadX + TriadX(i);
    avgTriadY = avgTriadY + TriadY(i);
    avgTriadZ = avgTriadZ + TriadZ(i);

    %mostrando i na tela para saber qunto ainda falta rodas
    i
    
    %tempo de comunicação
    pause(0.005)
end

%tirando a média
avgGyroX = avgGyroX/N_LEITURAS;
avgGyroY = avgGyroY/N_LEITURAS;
avgGyroZ = avgGyroZ/N_LEITURAS;
avgTriadX = avgTriadX/N_LEITURAS;
avgTriadY = avgTriadY/N_LEITURAS;
avgTriadZ = avgTriadZ/N_LEITURAS;

%zerando a variancia
varGyroX = 0;
varGyroY = 0;
varGyroZ = 0;
varTriadX = 0;
varTriadY = 0;
varTriadZ = 0;


%loop para calcular o valar da variancia 
for i = 1:N_LEITURAS
    varGyroX = varGyroX + (avgGyroX - GyroX(i))^2;
    varGyroY = varGyroY + (avgGyroY - GyroY(i))^2;
    varGyroZ = varGyroZ + (avgGyroZ - GyroZ(i))^2;
    varTriadX = varTriadX + (avgTriadX - TriadX(i))^2;
    varTriadY = varTriadY + (avgTriadY - TriadY(i))^2;
    varTriadZ = varTriadZ + (avgTriadZ - TriadZ(i))^2;    
end


%calculando a variancia e mostrando tudo na tela 

varGyroX = varGyroX/N_LEITURAS
varGyroY = varGyroY/N_LEITURAS
varGyroZ = varGyroZ/N_LEITURAS
varTriadX = varTriadX/N_LEITURAS
varTriadY = varTriadY/N_LEITURAS
varTriadZ = varTriadZ/N_LEITURAS

fclose(t);%fechando; ;o canal de comunicação

%saida do código 8 de fevereiro de 2018
%
%avgdt =   0.0220
%
%
%avgGx =  7.1596e-04   +/-   4.3468e-04
%avgGy =  -0.0011      +/-   1.9492e-04
%avgGz = -1.4266e-04   +/-   4.3147e-04
%avgAx = 135.6624      +/-   0.0287
%avgAy =   3.7654      +/-   0.0430
%avgAz = -86.3204      +/-   0.0409
%avgMx =  79.1582      +/-   11.3044
%avgMy = -49.6738      +/-   4.6405
%avgMz = 170.7780      +/-   8.0265
%
%varGx =  4.3468e-04
%varGy =  1.9492e-04
%varGz =  4.3147e-04
%varAx =   0.0287
%varAy =   0.0430
%varAz =   0.0409
%varMx =  11.3044
%varMy =   4.6405
%varMz =   8.0265
