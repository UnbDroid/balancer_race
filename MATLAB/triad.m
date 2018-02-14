%rodar o código para calcular as variacias ou pegar as varíavmeis no arquivo do repositório antes de rodar este script
clear

t = tcpip('192.168.200.1', 9001, 'NetworkRole', 'client'); %criando o objeto tcpip com os parametros corretos para a comunicação com o rasp
fopen(t);%abrindo o  canal de comunicação
data = fread(t, 102);%recebendo os dados 
fwrite(t, "ok!");%mandano a confirmação para o server
tdata = char(data');%colocando os caracteres recebidos em vetor linha
datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');


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

u = [Ax, Ay, Az];
u = u/norm(u);

v = [Mx, My, Mz];
v = v/norm(v);

i_n = (u+v)/(norm(u+v));
j_n = cross(i_n,u-v)/norm(cross(i_n,u-v));
k_n = cross(i_n,j_n);



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
    
    %old_time = time;
    %time = str2num(datanum{10})/1000000;
    %dt = time - old_time;

    u = [Ax, Ay, Az];
	u = u/norm(u);

	v = [Mx, My, Mz];
	v = v/norm(v);

	i_b = (u+v)/(norm(u+v));
	j_b = cross(i_b,u-v)/norm(cross(i_b,u-v));
	k_b = cross(i_b,j_b);
   
   	Mrot = ([i_b; j_b; k_b]')*[i_n; j_n; k_n];

    clc %limpando a tela
    
    %Yaw = atan2d(-Mrot(2,1),Mrot(1,1))
    %Pitch = atan2d(Mrot(3,1),sqrt(Mrot(3,2)^2+Mrot(3,3)^2))
    %Roll = atan2d(Mrot(3,2),Mrot(3,3))
    
    pause(0.1)

end


fclose(t);%fechando; ;o canal de comunicação
