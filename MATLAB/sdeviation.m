clear %limpando as variáveis

t = tcpip('192.168.200.1', 9001, 'NetworkRole', 'client'); %criando o objeto tcpip com os parametros corretos para a comunicação com o rasp
fopen(t);%abrindo o  canal de comunicação
%zerando os vaoler das medias
avgGx = 0;
avgGy = 0;
avgGz = 0;
avgAx = 0;
avgAy = 0;
avgAz = 0;
avgMx = 0;
avgMy = 0;
avgMz = 0;
avgdt = 0;

N_LEITURAS = 50000;%amostragem

%descartando as primeiras leituras
for i = 1:10   
    data = fread(t, 102);%recebendo os dados 
    fwrite(t, "ok!");%mandano a confirmação para o server
    tdata = char(data');%colocando os caracteres recebidos em vetor linha
    
    datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');
    time = str2num(datanum{10})/1000000;
    pause(0.005)
end

%loop para tirar as medidas e calcular a media
for i = 1:N_LEITURAS
    
    data = fread(t, 102);%recebendo os dados 
    fwrite(t, "ok!");%mandano a confirmação para o server
    tdata = char(data');%colocando os caracteres recebidos em vetor linha
    
    %convertendo de char para floats
    datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');
    
    Gx(i) = str2num(datanum{1});
    Gy(i) = str2num(datanum{2});
    Gz(i) = str2num(datanum{3});
    Ax(i) = str2num(datanum{4});
    Ay(i) = str2num(datanum{5});
    Az(i) = str2num(datanum{6});
    Mx(i) = str2num(datanum{7});
    My(i) = str2num(datanum{8});
    Mz(i) = str2num(datanum{9});
    %dt(i) = str2num(datanum{10})/1000000;
    old_time = time;
    time = str2num(datanum{10})/1000000;

    dt(i) = time - old_time;

    %printaio = time - old_time

    %incrementando o somatório
    avgdt = avgdt + dt(i);
    avgGx = avgGx + Gx(i)*dt(i);
    avgGy = avgGy + Gy(i)*dt(i);
    avgGz = avgGz + Gz(i)*dt(i);
    avgAx = avgAx + Ax(i);
    avgAy = avgAy + Ay(i);
    avgAz = avgAz + Az(i);
    avgMx = avgMx + Mx(i);
    avgMy = avgMy + My(i);
    avgMz = avgMz + Mz(i);
    
    %mostrando i na tela para saber qunto ainda falta rodas
    i
    
    %tempo de comunicação
    pause(0.005)
end

%tirando a média
avgGx = avgGx/N_LEITURAS;
avgGy = avgGy/N_LEITURAS;
avgGz = avgGz/N_LEITURAS;
avgAx = avgAx/N_LEITURAS;
avgAy = avgAy/N_LEITURAS;
avgAz = avgAz/N_LEITURAS;
avgMx = avgMx/N_LEITURAS;
avgMy = avgMy/N_LEITURAS;
avgMz = avgMz/N_LEITURAS;
avgdt = avgdt/N_LEITURAS;

%zerando a variancia
varGx = 0;
varGy = 0;
varGz = 0;
varAx = 0;
varAy = 0;
varAz = 0;
varMx = 0;
varMy = 0;
varMz = 0;


%loop para calcular o valar da variancia 
for i = 1:N_LEITURAS

    varGx = varGx + (avgGx - Gx(i)*dt(i))^2;
    varGy = varGy + (avgGy - Gy(i)*dt(i))^2;
    varGz = varGz + (avgGz - Gz(i)*dt(i))^2;
    varAx = varAx + (avgAx - Ax(i))^2;
    varAy = varAy + (avgAy - Ay(i))^2;
    varAz = varAz + (avgAz - Az(i))^2;
    varMx = varMx + (avgMx - Mx(i))^2;
    varMy = varMy + (avgMy - My(i))^2;
    varMz = varMz + (avgMz - Mz(i))^2;     
    
end


%calculando a variancia e mostrando tudo na tela 

varGx = varGx/N_LEITURAS
varGy = varGy/N_LEITURAS
varGz = varGz/N_LEITURAS
varAx = varAx/N_LEITURAS
varAy = varAy/N_LEITURAS
varAz = varAz/N_LEITURAS
varMx = varMx/N_LEITURAS
varMy = varMy/N_LEITURAS
varMz = varMz/N_LEITURAS




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
