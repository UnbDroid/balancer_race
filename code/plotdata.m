%limpando a bagunça
clear
clc
close all

%system('rm -f ../MATLAB/plots/fastplot/*');
%system('sshpass -p godroidgo scp pi@192.168.200.1:/home/pi/datalog/plot_data_* ../MATLAB/plots/fastplot/');

i = 0;
fname = sprintf('../MATLAB/plots/fastplot/plot_data_%03d', i);
while(exist(fname))
	data = load(fname);
	n_measurements = size(data, 1);
	n_vals = size(data, 2);
	t = data(2:end, 1);
	figure(i+1);
	clf
	hold on;
	for j = 2:n_vals;
		d = data(2:end, j);
		plot(t, d);
	end
	xlabel('Time (us)');
	ylabel('Angle (°)');
	titulo = sprintf('file %03d', i);
	title(titulo);
	legend();
	i = i+1;
	fname = sprintf('../MATLAB/plots/fastplot/plot_data_%03d', i);
end

%

%a =
%
%   1.000000000000000  -1.968427786938518   1.735860709208886  -0.724470829507362   0.120389599896245
%
%>> b
%
%b =
%
%   0.010209480791203   0.040837923164813   0.061256884747219   0.040837923164813   0.010209480791203
