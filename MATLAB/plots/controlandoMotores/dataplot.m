%limpando a bagunça
clear
clc
close all

titulogeral = 'Velocidade do motor com controle PID';

data0 = load('plot_data_000');
titulo0 = 'KP = 1000, KI = 0, KD = 0';
tempo0 = data0(:,1);
ref0 = data0(:,2);
spdL0 = data0(:,3);
spdR0 = data0(:,4);

data1 = load('plot_data_001');
titulo1 = 'KP = 6000, KI = 0, KD = 0';
tempo1 = data1(:,1);
ref1 = data1(:,2);
spdL1 = data1(:,3);
spdR1 = data1(:,4);

data2 = load('plot_data_002');
titulo2 = 'KP = 6000, KI = 6000, KD = 0';
tempo2 = data2(:,1);
ref2 = data2(:,2);
spdL2 = data2(:,3);
spdR2 = data2(:,4);


data3 = load('plot_data_003');
titulo3 = 'KP = 6000, KI = 6000, KD = 75';
tempo3 = data3(:,1);
ref3 = data3(:,2);
spdL3 = data3(:,3);
spdR3 = data3(:,4);

figure(1);

subplot(4,1,1);
plot(tempo0,ref0);
hold on
plot(tempo0,spdL0);
plot(tempo0,spdR0);
xlabel('Tempo (us)');
ylabel('Vel (m/s)');
title({titulogeral;titulo0});
legend('Reference','Vell LM','Vell RM');

subplot(4,1,2);
plot(tempo1,ref1);
hold on
plot(tempo1,spdL1);
plot(tempo1,spdR1);
xlabel('Tempo (us)');
ylabel('Vel (m/s)');
title(titulo1);
legend('Reference','Vell LM','Vell RM');

subplot(4,1,3);
plot(tempo2,ref2);
hold on
plot(tempo2,spdL2);
plot(tempo2,spdR2);
xlabel('Tempo (us)');
ylabel('Vel (m/s)');
title(titulo2);
legend('Reference','Vell LM','Vell RM');

subplot(4,1,4);
plot(tempo3,ref3);
hold on
plot(tempo3,spdL3);
plot(tempo3,spdR3);
xlabel('Tempo (us)');
ylabel('Vel (m/s)');
title(titulo3);
legend('Reference','Vell LM','Vell RM');