%limpando a bagunça
clear
clc
close all

%pegando os dados
data = load("plot_data_003");
titulo = 'Right Motor - OnRev';

tempo = data(:,1);
pwm = data(:,2);
vel = data(:,3);

%plotando os valores obtidos
figure(1);
subplot(2,1,1);
plot(tempo,pwm);
title({titulo;'Input PWM'});
xlabel('Tempo (us)');
ylabel('PWM (0 ~ 1023)');
%title('teste Input PWM (0 ~ 1023)');
subplot(2,1,2); 
plot(tempo,vel);
title('Vel econder');
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');

%tratando os dados do ecoder
N = 7;
leitura = zeros(1,N);

for i = 1:size(vel)
	% não funcionou =[
	% j = N;
	% while j <= 2
	% 	leitura(j) = leitura(j-1);
	% 	j = j-1;
	% end
	%vou fazer na mão
	leitura(7) = leitura(6);
	leitura(6) = leitura(5);
	leitura(5) = leitura(4);
	leitura(4) = leitura(3);
	leitura(3) = leitura(2);
	leitura(2) = leitura(1);
	leitura(1) = vel(i);
	vel_t(i) = median(leitura);

end

% figure(2);
% plot(tempo,vel);
% hold on
% plot(tempo,vel_t);

%plotando os dados tratados
figure(2);
subplot(2,1,1);
plot(tempo,vel);
title({titulo;'Raw'});
xlabel('Tempo (us)');
ylabel('PWM (0 ~ 1023)');
%title('teste Input PWM (0 ~ 1023)');
subplot(2,1,2); 
plot(tempo,vel_t);
title('Treated');
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');

figure(3);
plot(tempo,(1024/2500)*vel_t);
title({titulo;'Dead Zone'});
xlabel('Tempo (us)');
hold on;
plot(tempo,pwm);
legend('Vel * Scale Factore','PWM (0 ~ 1023)')
datacursormode on