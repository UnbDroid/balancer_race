function plot(filename, graphname)
	load filename
	n_measurements = size(filename)(1);
	n_vals = size(filename)(2);
	t = filename(:, 1);
	figure(1);
	hold on;
	for i = 2:n_vales;
		d = filename(:, i);
		plot(t, d, 'linewidth', 1.5);
	end
	legend('Gyro Integration', 'Raw Accel', 'Filtered Accel');
	xlabel('Time (us)');
	ylabel('Angle (°)');
	title(graphname);
end