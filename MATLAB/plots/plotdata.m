filename = "magnetTest/plot_data_012"
grname = "Off Motor"
fname = load(filename);
n_measurements = size(fname, 1);
n_vals = size(fname, 2);
t = fname(2:end, 1);
offset = fname(2, 3);
figure(1);
clf
hold on;
d = fname(2:end, 2);
plot(t, d);
for i = 3:n_vals;
	d = fname(2:end, i) - offset;
	plot(t, d);
end
%legend("Gyro Integration", "Raw Accel", "Filtered Accel");
xlabel("Time (us)");
ylabel("Angle (°)");
title(grname);
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
