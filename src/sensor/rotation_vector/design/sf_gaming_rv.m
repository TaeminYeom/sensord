% sf_gaming_rv
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

% Sensor Fusion Implementation for Determination of Gaming rotation Vector
%
% - Input Accelerometer and Gyroscope sensor data
% - Call estimate_gaming_rotation
% - Plot results for gaming rotation on reference axis

addpath('lib');
clear
close all
clc

GRAVITY = 9.80665;
RAD2DEG = 57.2957795;

Max_Range_Accel = 39.203407; Min_Range_Accel = -39.204006; Res_Accel = 0.000598;
Max_Range_Gyro = 1146.862549; Min_Range_Gyro = -1146.880005; Res_Gyro = 0.017500;

PITCH_PHASE_CORRECTION = -1;
ROLL_PHASE_CORRECTION = -1;
YAW_PHASE_CORRECTION = -1;

Bias_Ax = 0.098586;
Bias_Ay = 0.18385;
Bias_Az = 10.084 - GRAVITY;

Bias_Gx = -5.3539;
Bias_Gy = 0.24325;
Bias_Gz = 2.3391;

BUFFER_SIZE = 1095;

Accel_data = zeros(4,BUFFER_SIZE);
Gyro_data = zeros(4,BUFFER_SIZE);

Game_RV = zeros(4,BUFFER_SIZE);
Orientation_RV = zeros(3,BUFFER_SIZE);

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

% estimate orientation
Game_RV = estimate_gaming_rv(Accel_data, Gyro_data);

for i = 1:BUFFER_SIZE
	Orientation_RV(:,i) = quat2euler(Game_RV(i,:));
	Orientation_RV(1,i) = ROLL_PHASE_CORRECTION * Orientation_RV(1,i) * RAD2DEG;
	Orientation_RV(2,i) = PITCH_PHASE_CORRECTION * Orientation_RV(2,i) * RAD2DEG;
	Orientation_RV(3,i) = YAW_PHASE_CORRECTION * Orientation_RV(3,i) * RAD2DEG;
end

hfig=(figure);
scrsz = get(0,'ScreenSize');
set(hfig,'position',scrsz);
% Gaming Rotation Vector Plot Results
subplot(3,1,1)
UA = Orientation_RV(1,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'k');
legend(p1,'x-axis');
subplot(3,1,2)
UA = Orientation_RV(2,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'b');
legend(p1,'y-axis');
subplot(3,1,3)
UA = Orientation_RV(3,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'r');
legend(p1,'z-axis');
title(['Gaming Rotation Vector']);


