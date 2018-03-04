%limpando a bagunça
clear
clc
close all

%pegando os dados
data = load("plot_data_005");
titulo = 'Controle Filmegem 03/03 18h31';

tempo = data(:,1);
gyro = data(:,2);
pot = data(:,3);
velL = data(:,4);
velR = data(:,5);

%tratando os valores do ecoder
% N = 7;
% leitura = zeros(1,N);

% for i = 1:size(velL)
% 	% não funcionou =[
% 	% j = N;
% 	% while j <= 2
% 	% 	leitura(j) = leitura(j-1);
% 	% 	j = j-1;
% 	% end
% 	%vou fazer na mão
% 	leitura(7) = leitura(6);
% 	leitura(6) = leitura(5);
% 	leitura(5) = leitura(4);
% 	leitura(4) = leitura(3);
% 	leitura(3) = leitura(2);
% 	leitura(2) = leitura(1);
% 	leitura(1) = velL(i);
% 	velL_t(i) = median(leitura);
% end

% leitura = zeros(1,N);

% for i = 1:size(velR)
% 	% não funcionou =[
% 	% j = N;
% 	% while j <= 2
% 	% 	leitura(j) = leitura(j-1);
% 	% 	j = j-1;
% 	% end
% 	%vou fazer na mão
% 	leitura(7) = leitura(6);
% 	leitura(6) = leitura(5);
% 	leitura(5) = leitura(4);
% 	leitura(4) = leitura(3);
% 	leitura(3) = leitura(2);
% 	leitura(2) = leitura(1);
% 	leitura(1) = velR(i);
% 	velR_t(i) = median(leitura);

% end



%plotando os valores obtidos
figure(1);

%subplot(2,1,1);
plot(tempo,gyro*(305/0.5));
hold on
plot(tempo,pot);
title(titulo);
xlabel('Tempo (us)');
ylabel('PWM (0 ~ 1023)');
legend('Gyro Integrat * Scale Factore','PWM(-1023 ~ 1023)');
%title('teste Input PWM (0 ~ 1023)');

