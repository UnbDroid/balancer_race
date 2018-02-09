%rodar o código para calcular as variacias ou pegar as varíavmeis no arquivo do repositório antes de rodar este script
clear

%pegando dados passados
load('std_dev_8_feb_18.mat')

A = eye(3);
B = eye(3);
Q = [varGx 0 0; 0 varGy 0; 0 0 varGz];
R = [varAx 0 0; 0 varAy 0; 0 0 varMz];


clear Gx
clear Gy
clear Gz
clear Ax
clear Ay
clear Az
clear Mx
clear My
clear Mz
clear dt

t = tcpip('192.168.200.1', 9001, 'NetworkRole', 'client'); %criando o objeto tcpip com os parametros corretos para a comunicação com o rasp
fopen(t);%abrindo o  canal de comunicação

%fazendo a primeira leitura de tempo
data = fread(t, 102);%recebendo os dados 
fwrite(t, "ok!");%mandano a confirmação para o server
tdata = char(data');%colocando os caracteres recebidos em vetor linha
datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');

time = str2num(datanum{10})/1000000;
old_time = time;

%fazendo a primeira leitura
data = fread(t, 102);%recebendo os dados 
fwrite(t, "ok!");%mandano a confirmação para o server
tdata = char(data');%colocando os caracteres recebidos em vetor linha
datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');    
Gx = str2num(datanum{1});
Gy = str2num(datanum{2});
Gz = str2num(datanum{3});
Ax = str2num(datanum{4});
Ay = str2num(datanum{5});
Az = str2num(datanum{6});
Mx = str2num(datanum{7});
My = str2num(datanum{8});
Mz = str2num(datanum{9});

old_time = time;
time = str2num(datanum{10})/1000000;

dt = time - old_time;

%valores iniciais das matrize do filtro de kalma
B = eye(3)*dt;
Xp = [Ax 0 0; 0 Ay 0; 0 0 Mz];
Pp = R;

while 1
    data = fread(t, 102);%recebendo os dados 
    fwrite(t, "ok!");%mandano a confirmação para o server
    tdata = char(data');%colocando os caracteres recebidos em vetor linha
    
    %convertendo de char para floats
    datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');
    
    Gx = str2num(datanum{1});
    Gy = str2num(datanum{2});
    Gz = str2num(datanum{3});
    Ax = str2num(datanum{4});
    Ay = str2num(datanum{5});
    Az = str2num(datanum{6});
    Mx = str2num(datanum{7});
    My = str2num(datanum{8});
    Mz = str2num(datanum{9});
    
    old_time = time;
    time = str2num(datanum{10})/1000000;
    dt = time - old_time;

    %predição
    B = eye(3)*dt;
    U = [Gx 0 0; 0 Gy 0; 0 0 Gz];
	Xp = A*Xp + B*U;
	Pp = A*Pp*(A') + Q;

	%correção
	K = Pp/(Pp + R);
	Y = [Ax 0 0; 0 Ay 0; 0 0 Mz];
	Xp = Xp + K*(Y - Xp);

	%atualizar covariancia
	Pp = (eye(3) - K)*Pp;

	%imprimindo a saída do filtro
    clc %limpando a tela
    Xp
    K
end


fclose(t);%fechando; ;o canal de comunicação

