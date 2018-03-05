%limpando a bagunça
clear
clc
close all

titulogeral = 'Slow Rotation';

data075 = load('plot_data_004');
titulo075R = 'Alpha = 0.75 - RM';
titulo075L = 'Alpha = 0.75 - LM';
tempo075 = data075(:,1);
LR075 = data075(:,2);
LT075 = data075(:,3);
RR075 = data075(:,4);
RT075 = data075(:,5);

data090 = load('plot_data_005');
titulo090R = 'Alpha = 0.90 - RM';
titulo090L = 'Alpha = 0.90 - LM';
tempo090 = data090(:,1);
LR090 = data090(:,2);
LT090 = data090(:,3);
RR090 = data090(:,4);
RT090 = data090(:,5);

data093 = load('plot_data_006');
titulo093R = 'Alpha = 0.93 - RM';
titulo093L = 'Alpha = 0.93 - LM';
tempo093 = data093(:,1);
LR093 = data093(:,2);
LT093 = data093(:,3);
RR093 = data093(:,4);
RT093 = data093(:,5);

data095 = load('plot_data_007');
titulo095L = 'Alpha = 0.95 - LM';
titulo095R = 'Alpha = 0.95 - RM';
tempo095 = data095(:,1);
LR095 = data095(:,2);
LT095 = data095(:,3);
RR095 = data095(:,4);
RT095 = data095(:,5);

figure(1);

subplot(4,2,1);
plot(tempo075,RR075);
hold on
plot(tempo075,RT075);
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');
title({titulogeral;titulo075R});

subplot(4,2,2);
plot(tempo075,LR075);
hold on
plot(tempo075,LT075);
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');
title(titulo075L);

subplot(4,2,3);
plot(tempo090,RR090);
hold on
plot(tempo090,RT090);
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');
title(titulo090R);

subplot(4,2,4);
plot(tempo090,LR090);
hold on
plot(tempo090,LT090);
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');
title(titulo090L);

subplot(4,2,5);
plot(tempo093,RR093);
hold on
plot(tempo093,RT093);
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');
title(titulo093R);

subplot(4,2,6);
plot(tempo093,LR093);
hold on
plot(tempo093,LT093);
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');
title(titulo093L);

subplot(4,2,7);
plot(tempo095,RR095);
hold on
plot(tempo095,RT095);
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');
title(titulo095R);

subplot(4,2,8);
plot(tempo095,LR095);
hold on
plot(tempo095,LT095);
xlabel('Tempo (us)');
ylabel('Vel (tcks/s)');
title(titulo095L);