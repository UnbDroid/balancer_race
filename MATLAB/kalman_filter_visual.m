%rodar o código para calcular as variacias ou pegar as varíavmeis no arquivo do repositório antes de rodar este script
clear
clc

%pegando dados passados
load('std_dev_14_feb_18.mat')

Lbd = 0.5

clear GyroX
clear GyroY
clear GyroZ
clear TriadX
clear TriadY
clear TriadZ
clear dt

%comunicação com o rasp
t = tcpip('192.168.200.1', 9002, 'NetworkRole', 'client'); %criando o objeto tcpip com os parametros corretos para a comunicação com o rasp
fopen(t);%abrindo o  canal de comunicação

%comunicação com processing
t2 = tcpip('127.0.0.1', 9003, 'NetworkRole', 'client'); %criando o objeto tcpip com os parametros corretos para a comunicação com o rasp
fopen(t2);%abrindo o  canal de comunicação


%fazendo a primeira leitura de tempo
data = fread(t, 71);%recebendo os dados 
fwrite(t, "ok!");%mandano a confirmação para o server
tdata = char(data');%colocando os caracteres recebidos em vetor linha
datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');

time = str2num(datanum{7})/1000000;
old_time = time;

%fazendo a primeira leitura
data = fread(t, 71);%recebendo os dados 
fwrite(t, "ok!");%mandano a confirmação para o server
tdata = char(data');%colocando os caracteres recebidos em vetor linha
datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');    
GyroX = str2num(datanum{1});
GyroY = str2num(datanum{2});
GyroZ = str2num(datanum{3});
TriadZ = str2num(datanum{4});
TriadY = str2num(datanum{5});
TriadX = str2num(datanum{6});

old_time = time;

time = str2num(datanum{7})/1000000;
while time < old_time
    time = time * 10;
end
dt = time - old_time;

%valores iniciais das matrize do filtro de kalma
T = eye(3)*dt;
Xk = [TriadX 0 0; 0 TriadY 0; 0 0 TriadZ];
R = [varTriadX 0 0; 0 varTriadY 0; 0 0 varTriadZ];
Pk = R;
Wk = [varGyroX 0 0; 0 varGyroY 0; 0 0 varGyroZ];
Qk = Wk + Lbd*eye(3);
Uk = [GyroX 0 0; 0 GyroY 0; 0 0 GyroZ];
K = eye(3);


while 1
    old_TriadX = TriadX;
    old_TriadY = TriadY;
    old_TriadZ = TriadZ;

    data = fread(t, 71);%recebendo os dados 
    fwrite(t, "ok!");%mandano a confirmação para o server
    tdata = char(data');%colocando os caracteres recebidos em vetor linha
    datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');    
    GyroX = str2num(datanum{1});
    GyroY = str2num(datanum{2});
    GyroZ = str2num(datanum{3});
    TriadZ = str2num(datanum{4});
    TriadY = str2num(datanum{5});
    TriadX = str2num(datanum{6});

    old_time = time;
    time = str2num(datanum{7})/1000000;

    while time < old_time
        time = time * 10;
    end

    dt = time - old_time;
    %leituras
    Uk_old = Uk;
    Uk = [GyroX 0 0; 0 GyroY 0; 0 0 GyroZ];
    Uk_med = (Uk+Uk_old)/2;
    Yk = [TriadX 0 0; 0 TriadY 0; 0 0 TriadZ];
    T = eye(3)*dt;

    %predição
    Xk = Xk + T*Uk_med;
    Pi = Pk + Qk*T^2;

	%correção
	if (old_TriadX ~= TriadX) | (old_TriadY ~= TriadY) | (old_TriadZ ~= TriadZ)
        K = Pi/(Pi + R);
        Xk = Xk + K*(Yk - Xk);
    	%atualizar covariancia
    	Pk = (eye(3)-K)*Pi*(eye(3)-K)' + K*R*K';
    else
        Pk = Pi;
    end
	%imprimindo a saída do filtro
    clc %limpando a tela
    Xk
    K

    %enviando dados para o processing fazer o plot
    mensagem = [sprintf('%0.5f',Xk(1,1)),';',sprintf('%0.5f',Xk(2,2)),';',sprintf('%0.5f',Xk(3,3)),';',sprintf('%f',dt)];
    fwrite(t2, mensagem);%mandano a confirmação para o server    
    datacache = fread(t2, 7);%recebendo o "abacaxi" (equivalente a recebendo o OK!, pergunta pro takashi pq ele mudou para abacaxi)

end


fclose(t);%fechando o canal de comunicação
fclose(t2);%fechando o canal de comunicação

