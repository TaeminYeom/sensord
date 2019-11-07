% sf_orientation
%
% Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

% Sensor Fusion Implementation for Orientation Estimation
%
% - Input Accelerometer, Gyroscope and Magnetometer sensor data
% - Call estimate_orientation function
% - Plot results for orientation

addpath('lib');
clear
close all
clc

GRAVITY = 9.80665;
RAD2DEG = 57.2957795;

Max_Range_Accel = 39.203407; Min_Range_Accel = -39.204006; Res_Accel = 0.000598;
Max_Range_Gyro = 1146.862549; Min_Range_Gyro = -1146.880005; Res_Gyro = 0.017500;
Max_Range_Magnetic = 1200; Min_Range_Magnetic = -1200; Res_Magnetic = 1;

PITCH_PHASE_CORRECTION = -1;
ROLL_PHASE_CORRECTION = -1;
YAW_PHASE_CORRECTION = -1;

Bias_Ax = 0.098586;
Bias_Ay = 0.18385;
Bias_Az = 10.084 - GRAVITY;

Bias_Gx = -5.3539;
Bias_Gy = 0.24325;
Bias_Gz = 2.3391;

Bias_Mx = 0;
Bias_My = 0;
Bias_Mz = 0;

Sign_Ax = 1;
Sign_Ay = 1;
Sign_Az = 1;

Sign_Gx = 1;
Sign_Gy = 1;
Sign_Gz = 1;

Sign_Mx = 1;
Sign_My = 1;
Sign_Mz = 1;

BUFFER_SIZE = 1095;

Accel_data = zeros(4,BUFFER_SIZE);
Gyro_data = zeros(4,BUFFER_SIZE);
Mag_data =  zeros(4,BUFFER_SIZE);

OR_driv = zeros(3,BUFFER_SIZE);
OR_aid = zeros(3,BUFFER_SIZE);
OR_err = zeros(3,BUFFER_SIZE);

euler_driv = zeros(BUFFER_SIZE,3);
euler_aid = zeros(BUFFER_SIZE,3);
euler_err = zeros(BUFFER_SIZE,3);

% Sensor Data simulating orientation motions

% get accel x,y,z axis data from stored file
Accel_data(1,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/accel.txt")(:,1))') - Bias_Ax)(1:BUFFER_SIZE);
Accel_data(2,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/accel.txt")(:,2))') - Bias_Ay)(1:BUFFER_SIZE);
Accel_data(3,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/accel.txt")(:,3))') - Bias_Az)(1:BUFFER_SIZE);
Accel_data(4,:) = ((dlmread("data/100ms/orientation/roll_pitch_yaw/accel.txt")(:,4))')(1:BUFFER_SIZE);

% get gyro x,y,z axis data from stored file
Gyro_data(1,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/gyro.txt")(:,1))') - Bias_Gx)(1:BUFFER_SIZE);
Gyro_data(2,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/gyro.txt")(:,2))') - Bias_Gy)(1:BUFFER_SIZE);
Gyro_data(3,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/gyro.txt")(:,3))') - Bias_Gz)(1:BUFFER_SIZE);
Gyro_data(4,:) = ((dlmread("data/100ms/orientation/roll_pitch_yaw/gyro.txt")(:,4))')(1:BUFFER_SIZE);

scale_Gyro = 575;
Gyro_data(1,:) = Gyro_data(1,:)/scale_Gyro;
Gyro_data(2,:) = Gyro_data(2,:)/scale_Gyro;
Gyro_data(3,:) = Gyro_data(3,:)/scale_Gyro;

% get magnetometer x,y,z axis data from stored file
Mag_data(1,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/magnetic.txt")(:,1))') - Bias_Mx)(1:BUFFER_SIZE) * Sign_Mx;
Mag_data(2,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/magnetic.txt")(:,2))') - Bias_My)(1:BUFFER_SIZE) * Sign_My;
Mag_data(3,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/magnetic.txt")(:,3))') - Bias_Mz)(1:BUFFER_SIZE) * Sign_Mz;
Mag_data(4,:) = ((dlmread("data/100ms/orientation/roll_pitch_yaw/magnetic.txt")(:,4))')(1:BUFFER_SIZE);

% estimate orientation
[Quat_driv, Quat_aid, Quat_err, Gyro_bias]  = estimate_orientation(Accel_data, Gyro_data, Mag_data);

Gyro_bias(1,:) = Gyro_bias(1,:) + Bias_Gx;
Gyro_bias(2,:) = Gyro_bias(2,:) + Bias_Gy;
Gyro_bias(3,:) = Gyro_bias(3,:) + Bias_Gz;

for i = 1:BUFFER_SIZE
	euler_aid(i,:) = quat2euler(Quat_aid(i,:));
	OR_aid(1,i) = euler_aid(i,2)' * RAD2DEG;
	OR_aid(2,i) = euler_aid(i,1)' * RAD2DEG;
	OR_aid(3,i) = euler_aid(i,3)' * RAD2DEG;

	euler_driv(i,:) = quat2euler(Quat_driv(i,:));
	OR_driv(1,i) = ROLL_PHASE_CORRECTION * euler_driv(i,2)' * RAD2DEG;
	OR_driv(2,i) = PITCH_PHASE_CORRECTION * euler_driv(i,1)' * RAD2DEG;
	OR_driv(3,i) = YAW_PHASE_CORRECTION * euler_driv(i,3)' * RAD2DEG;

	if (OR_driv(3,i) < 0)
		OR_driv(3,i) = OR_driv(3,i) + 360;
	end

	if (OR_aid(3,i) < 0)
		OR_aid(3,i) = OR_aid(3,i) + 360;
	end
end

% Rotation Plot Results
hfig=(figure);
scrsz = get(0,'ScreenSize');
set(hfig,'position',scrsz);
subplot(3,1,1)
UA = OR_aid(2,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'r');
hold on;
grid on;
UA = OR_driv(2,:);
p2 = plot(1:length(UA),UA(1,1:length(UA)),'b');
hold on;
grid on;
UA = Gyro_bias(1,:);
p3 = plot(1:length(UA),UA(1,1:length(UA)),'g');
title(['Pitch']);
legend([p1 p2 p3],'Aiding System', 'Driving System', 'Gyroscope Bias error');
subplot(3,1,2)
UA = OR_aid(1,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'r');
hold on;
grid on;
UA = OR_driv(1,:);
p2 = plot(1:length(UA),UA(1,1:length(UA)),'b');
hold on;
grid on;
UA = Gyro_bias(2,:);
p3 = plot(1:length(UA),UA(1,1:length(UA)),'g');
title(['Roll']);
legend([p1 p2 p3],'Aiding System', 'Driving System', 'Gyroscope Bias error');
subplot(3,1,3)
UA = OR_aid(3,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'r');
hold on;
grid on;
UA = OR_driv(3,:);
p2 = plot(1:length(UA),UA(1,1:length(UA)),'b');
hold on;
grid on;
UA = Gyro_bias(3,:);
p3 = plot(1:length(UA),UA(1,1:length(UA)),'g');
title(['Yaw']);
legend([p1 p2 p3],'Aiding System', 'Driving System', 'Gyroscope Bias error');
