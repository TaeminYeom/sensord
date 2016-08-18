% estimate_orientation
%
% Copyright (c) 2015 Samsung Electronics Co., Ltd.
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
% http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.

% Orientation Estimation function
%
% - Orientation and rotation vector Estimation using Gyroscope as the driving system and
%   Accelerometer+Geo-Magnetic Sensors as Aiding System.
% - Gaming rotation vector Estimation using Accelerometer and Gyroscope sensors.
% - Geomagnetic rotation vector Estimation using Accelerometer and Geomagnetic sensors.
% - Quaternion based approach
% - Estimation and correction of orientation errors and bias errors for gyroscope using Kalman filter

function [quat_driv, quat_aid, quat_error, gyro_bias]  = estimate_orientation(Accel_data, Gyro_data, Mag_data)
	PLOT_SCALED_SENSOR_COMPARISON_DATA = 1;
	PLOT_INDIVIDUAL_SENSOR_INPUT_DATA = 1;

	MAGNETIC_ALIGNMENT_FACTOR = -1;
	GYRO_DATA_DISABLED = 0;
	MAG_DATA_DISABLED = 0;

	if Gyro_data(4,1) == 0
		GYRO_DATA_DISABLED = 1;
	end

	if Mag_data(4,1) == 0
		MAG_DATA_DISABLED = 1;
	end

	GRAVITY = 9.80665;
	PI = 3.141593;
	ACCEL_THRESHOLD = 0.2;
	GYRO_THRESHOLD = 0.01;
	NON_ZERO_VAL = 0.1;
	PI_DEG = 180;

	MOVING_AVERAGE_WINDOW_LENGTH = 20;
	RAD2DEG = 57.2957795;
	DEG2RAD = 0.0174532925;
	US2S =  1.0 / 1000000.0;

	ZigmaW = 0.05 * DEG2RAD;
	TauW = 1000;

	BUFFER_SIZE = size(Accel_data,2);
	Ax = Accel_data(1,:);
	Ay = Accel_data(2,:);
	Az = Accel_data(3,:);
	ATime = Accel_data(4,:);

	mag_x = zeros(1,BUFFER_SIZE);
	mag_y = zeros(1,BUFFER_SIZE);
	mag_z = zeros(1,BUFFER_SIZE);
	MTime = zeros(1,BUFFER_SIZE);

	gyro_bias = zeros(3,BUFFER_SIZE);

	if MAG_DATA_DISABLED != 1
		Mx = Mag_data(1,:);
		My = Mag_data(2,:);
		Mz = Mag_data(3,:);
		MTime = Mag_data(4,:);
	end

	acc_x = zeros(1,BUFFER_SIZE);
	acc_y = zeros(1,BUFFER_SIZE);
	acc_z = zeros(1,BUFFER_SIZE);

	quat_aid = zeros(BUFFER_SIZE,4);
	quat_driv = zeros(BUFFER_SIZE,4);
	quat_gaming_rv = zeros(BUFFER_SIZE,4);
	quat_error = zeros(BUFFER_SIZE,4);

	acc_e = [0.0;0.0;1.0]; % gravity vector in earth frame
	mag_e = [0.0;MAGNETIC_ALIGNMENT_FACTOR;0.0]; % magnetic field vector in earth frame

	if GYRO_DATA_DISABLED != 1
		% Gyroscope Bias Variables
		Bx = 0; By = 0; Bz = 0;

		Gx = Gyro_data(1,:);
		Gy = Gyro_data(2,:);
		Gz = Gyro_data(3,:);
		GTime = Gyro_data(4,:);

		gyr_x = zeros(1,BUFFER_SIZE);
		gyr_y = zeros(1,BUFFER_SIZE);
		gyr_z = zeros(1,BUFFER_SIZE);

		% User Acceleration mean and Variance
		A_T = zeros(1,BUFFER_SIZE);
		G_T = zeros(1,BUFFER_SIZE);
		M_T = zeros(1,BUFFER_SIZE);
		var_roll = zeros(1,BUFFER_SIZE);
		var_pitch = zeros(1,BUFFER_SIZE);
		var_yaw = zeros(1,BUFFER_SIZE);
		var_Gx = zeros(1,BUFFER_SIZE);
		var_Gy = zeros(1,BUFFER_SIZE);
		var_Gz = zeros(1,BUFFER_SIZE);

		roll = zeros(1,BUFFER_SIZE);
		pitch = zeros(1,BUFFER_SIZE);
		yaw = zeros(1,BUFFER_SIZE);

		% system covariance matrix
		Q = zeros(6,6);

		% measurement covariance matrix
		R = zeros(6,6);

		A_T(1) = 100000;
		G_T(1) = 100000;
		M_T(1) = 100000;

		H = [eye(3) zeros(3,3); zeros(3,6)];
		x = zeros(6,BUFFER_SIZE);
		e = zeros(1,6);
		P = 1 * eye(6);% state covariance matrix

		quat_driv(1,:) = [1 0 0 0];
	end

	q = [1 0 0 0];

	% first order filtering
	for i = 1:BUFFER_SIZE
		% normalize accelerometer measurements
		norm_acc = 1/sqrt(Ax(i)^2 + Ay(i)^2 + Az(i)^2);
		acc_x(i) = norm_acc * Ax(i);
		acc_y(i) = norm_acc * Ay(i);
		acc_z(i) = norm_acc * Az(i);

		% gravity vector in body frame
		acc_b =[acc_x(i);acc_y(i);acc_z(i)];

		if MAG_DATA_DISABLED != 1
			% normalize magnetometer measurements
			norm_mag = 1/sqrt(Mx(i)^2 + My(i)^2 + Mz(i)^2);
			mag_x(i) = norm_mag * Mx(i);
			mag_y(i) = norm_mag * My(i);
			mag_z(i) = norm_mag * Mz(i);

			% Aiding System (Accelerometer + Geomagnetic) quaternion generation
			% magnetic field vector in body frame
			mag_b =[mag_x(i);mag_y(i);mag_z(i)];

			% compute measurement quaternion with TRIAD algorithm
			acc_b_x_mag_b = cross(acc_b,mag_b);
			acc_e_x_mag_e = cross(acc_e,mag_e);
			M_b = [acc_b acc_b_x_mag_b cross(acc_b_x_mag_b,acc_b)];
			M_e = [acc_e acc_e_x_mag_e cross(acc_e_x_mag_e,acc_e)];
			Rot_m = M_e * M_b';
			quat_aid(i,:) = rot_mat2quat(Rot_m);
		else
			axis = cross(acc_b, acc_e);
			angle = acos(dot(acc_b, acc_e));
			quat_aid(i,:) = axis_rot2quat(axis, angle);
		end

		if GYRO_DATA_DISABLED != 1
			gyr_x(i) = Gx(i) * PI;
			gyr_y(i) = Gy(i) * PI;
			gyr_z(i) = Gz(i) * PI;

			euler = quat2euler(quat_aid(i,:));
			roll(i) = euler(2);
			pitch(i) = euler(1);
			yaw(i) = euler(3);

			if i > 1
				A_T(i) = ATime(i) - ATime(i-1);
				G_T(i) = GTime(i) - GTime(i-1);
				M_T(i) = MTime(i) - MTime(i-1);
			end

			dt = G_T(i) * US2S;

			% Driving System (Gyroscope) quaternion generation
			% convert scaled gyro data to rad/s
			qDot = 0.5 * quat_prod(q, [0 gyr_x(i) gyr_y(i) gyr_z(i)]);

			% Integrate to yield quaternion
			q = q + qDot * dt * PI;

			% normalise quaternion
			quat_driv(i,:) = q / norm(q);

			% Kalman Filtering
			quat_error(i,:) = quat_prod(quat_aid(i,:), quat_driv(i,:));

			euler_e = quat2euler(quat_error(i,:));

			gyr_x(i) = gyr_x(i) - (Bx + euler_e(1));
			gyr_y(i) = gyr_y(i) - (By + euler_e(2));
			gyr_z(i) = gyr_z(i) - (Bz + euler_e(3));

			if i <= MOVING_AVERAGE_WINDOW_LENGTH
				var_Gx(i) = NON_ZERO_VAL;
				var_Gy(i) = NON_ZERO_VAL;
				var_Gz(i) = NON_ZERO_VAL;
				var_roll(i) = NON_ZERO_VAL;
				var_pitch(i) = NON_ZERO_VAL;
				var_yaw(i) = NON_ZERO_VAL;
			else
				var_Gx(i) = var(gyr_x((i-MOVING_AVERAGE_WINDOW_LENGTH):i));
				var_Gy(i) = var(gyr_y((i-MOVING_AVERAGE_WINDOW_LENGTH):i));
				var_Gz(i) = var(gyr_z((i-MOVING_AVERAGE_WINDOW_LENGTH):i));
				var_roll(i) = var(roll((i-MOVING_AVERAGE_WINDOW_LENGTH):i));
				var_pitch(i) = var(pitch((i-MOVING_AVERAGE_WINDOW_LENGTH):i));
				var_yaw(i) = var(yaw((i-MOVING_AVERAGE_WINDOW_LENGTH):i));
			end

			Qwn = [var_Gx(i) 0 0;0 var_Gy(i) 0;0 0 var_Gz(i);];
			Qwb = (2 * (ZigmaW^2) / TauW) * eye(3);

			% Process Noise Covariance
			Q = [Qwn zeros(3,3);zeros(3,3) Qwb];

			% Measurement Noise Covariance
			R = [[var_roll(i) 0 0;0 var_pitch(i) 0;0 0 var_yaw(i)] zeros(3,3); zeros(3,3) zeros(3,3)];

			% initialization for q
			if i == 1
				q = quat_aid(i,:);
			end

			F = [[0 gyr_z(i) -gyr_y(i);-gyr_z(i) 0 gyr_x(i);gyr_y(i) -gyr_x(i) 0] zeros(3,3); zeros(3,3) (-(1/TauW) * eye(3))];

			% Time Update
			if i > 1
				x(:,i) = F * x(:,i-1);
			end

			% compute covariance of prediction
			P = (F * P * F') + Q;

			x1 = euler_e(1)'/PI;
			x2 = euler_e(2)'/PI;
			x3 = euler_e(3)'/PI;

			if MAG_DATA_DISABLED != 1
				q = quat_prod(quat_driv(i,:), [1 x1 x2 x3]) * PI;
				q = q / norm(q);
			else
				euler_aid = quat2euler(quat_aid(i,:));
				euler_driv = quat2euler(quat_driv(i,:));
				if (acc_y(i)^2 < (ACCEL_THRESHOLD) && Gx(i)^2 < (GYRO_THRESHOLD))
					if (acc_x(i)^2 < (ACCEL_THRESHOLD) && Gy(i)^2 < (GYRO_THRESHOLD))
						if (Gz(i)^2 < (GYRO_THRESHOLD))
							q = euler2quat([euler_aid(2) euler_aid(1) euler_driv(3)]);
						end
					end
				end
			end

			if i > 1
				e = [x1 x2 x3 x(4,i) x(5,i) x(6,i)];
			end

			for j =1:6
				% compute Kalman gain
				K(:,j) = P(j ,:)./(P(j,j)+R(j,j));
				% update state vector
				x(:,i) = x(:,i) + K(:,j) * e(j);
				% update covariance matrix
				P = (eye(6) - (K(:,j) * H(j,:))) * P;
			end

			Bx = x(4,i);
			By = x(5,i);
			Bz = x(6,i);
			gyro_bias(:,i) = [x1; x2; x3] * PI_DEG + [Bx; By; Bz];
		end
	end

	if GYRO_DATA_DISABLED == 1
		quat_driv = quat_aid;
	end

	if PLOT_SCALED_SENSOR_COMPARISON_DATA == 1
		% Accelerometer/Gyroscope scaled Plot results
		hfig=(figure);
		scrsz = get(0,'ScreenSize');
		set(hfig,'position',scrsz);
		subplot(3,1,1)
		p1 = plot(1:BUFFER_SIZE,acc_x(1,1:BUFFER_SIZE),'r');
		hold on;
		grid on;
		if GYRO_DATA_DISABLED != 1
			p2 = plot(1:BUFFER_SIZE,Gx(1,1:BUFFER_SIZE),'b');
			if MAG_DATA_DISABLED != 1
				hold on;
				grid on;
				p3 = plot(1:BUFFER_SIZE,mag_x(1,1:BUFFER_SIZE),'k');
				title(['Accelerometer/Gyroscope/Magnetometer X-Axis Plot']);
				legend([p1 p2 p3],'Acc_X', 'Gyr_X', 'Mag_X');
			else
				title(['Accelerometer/Gyroscope X-Axis Plot']);
				legend([p1 p2],'Acc_X', 'Gyr_X');
			end
		else
			p2 = plot(1:BUFFER_SIZE,mag_x(1,1:BUFFER_SIZE),'k');
			title(['Accelerometer/Magnetometer X-Axis Plot']);
			legend([p1 p2],'Acc_X', 'Mag_X');
		end
		subplot(3,1,2)
		p1 = plot(1:BUFFER_SIZE,acc_y(1,1:BUFFER_SIZE),'r');
		hold on;
		grid on;
		if GYRO_DATA_DISABLED != 1
			p2 = plot(1:BUFFER_SIZE,Gy(1,1:BUFFER_SIZE),'b');
			if MAG_DATA_DISABLED != 1
				hold on;
				grid on;
				p3 = plot(1:BUFFER_SIZE,mag_y(1,1:BUFFER_SIZE),'k');
				title(['Accelerometer/Gyroscope/Magnetometer Y-Axis Plot']);
				legend([p1 p2 p3],'Acc_Y', 'Gyr_Y', 'Mag_Y');
			else
				title(['Accelerometer/Gyroscope Y-Axis Plot']);
				legend([p1 p2],'Acc_Y', 'Gyr_Y');
			end
		else
			p2 = plot(1:BUFFER_SIZE,mag_y(1,1:BUFFER_SIZE),'k');
			title(['Accelerometer/Magnetometer Y-Axis Plot']);
			legend([p1 p2],'Acc_X', 'Mag_Y');
		end
		subplot(3,1,3)
		p1 = plot(1:BUFFER_SIZE,acc_z(1,1:BUFFER_SIZE),'r');
		hold on;
		grid on;
		if GYRO_DATA_DISABLED != 1
			p2 = plot(1:BUFFER_SIZE,Gz(1,1:BUFFER_SIZE),'b');
			if MAG_DATA_DISABLED != 1
				hold on;
				grid on;
				p3 = plot(1:BUFFER_SIZE,mag_z(1,1:BUFFER_SIZE),'k');
				title(['Accelerometer/Gyroscope/Magnetometer Z-Axis Plot']);
				legend([p1 p2 p3],'Acc_Z', 'Gyr_Z', 'Mag_Z');
			else
				title(['Accelerometer/Gyroscope Z-Axis Plot']);
				legend([p1 p2],'Acc_Z', 'Gyr_Z');
			end
		else
			p2 = plot(1:BUFFER_SIZE,mag_z(1,1:BUFFER_SIZE),'k');
			title(['Accelerometer/Magnetometer Z-Axis Plot']);
			legend([p1 p2],'Acc_Z', 'Mag_Z');
		end
	end

	if PLOT_INDIVIDUAL_SENSOR_INPUT_DATA == 1
		% Accelerometer Raw (vs) filtered output
		hfig=(figure);
		scrsz = get(0,'ScreenSize');
		set(hfig,'position',scrsz);
		subplot(3,1,1)
		p1 = plot(1:BUFFER_SIZE,Ax(1,1:BUFFER_SIZE),'r');
		hold on;
		grid on;
		title(['Accelerometer X-Axis Plot']);
		subplot(3,1,2)
		p1 = plot(1:BUFFER_SIZE,Ay(1,1:BUFFER_SIZE),'r');
		hold on;
		grid on;
		title(['Accelerometer Y-Axis Plot']);
		subplot(3,1,3)
		p1 = plot(1:BUFFER_SIZE,Az(1,1:BUFFER_SIZE),'r');
		hold on;
		grid on;
		title(['Accelerometer Z-Axis Plot']);

		if GYRO_DATA_DISABLED != 1
			% Gyroscope Raw (vs) filtered output
			hfig=(figure);
			scrsz = get(0,'ScreenSize');
			set(hfig,'position',scrsz);
			subplot(3,1,1)
			p1 = plot(1:BUFFER_SIZE,Gx(1,1:BUFFER_SIZE),'r');
			hold on;
			grid on;
			title(['Gyroscope X-Axis Plot']);
			subplot(3,1,2)
			p1 = plot(1:BUFFER_SIZE,Gy(1,1:BUFFER_SIZE),'r');
			hold on;
			grid on;
			title(['Gyroscope Y-Axis Plot']);
			subplot(3,1,3)
			p1 = plot(1:BUFFER_SIZE,Gz(1,1:BUFFER_SIZE),'r');
			hold on;
			grid on;
			title(['Gyroscope Z-Axis Plot']);
		end

		if MAG_DATA_DISABLED != 1
			% Magnetometer Raw (vs) filtered output
			hfig=(figure);
			scrsz = get(0,'ScreenSize');
			set(hfig,'position',scrsz);
			subplot(3,1,1)
			p1 = plot(1:BUFFER_SIZE,Mx(1,1:BUFFER_SIZE),'r');
			hold on;
			grid on;
			title(['Magnetometer X-Axis Plot']);
			subplot(3,1,2)
			p1 = plot(1:BUFFER_SIZE,My(1,1:BUFFER_SIZE),'r');
			hold on;
			grid on;
			title(['Magnetometer Y-Axis Plot']);
			subplot(3,1,3)
			p1 = plot(1:BUFFER_SIZE,Mz(1,1:BUFFER_SIZE),'r');
			hold on;
			grid on;
			title(['Magnetometer Z-Axis Plot']);
		end
	end
end
