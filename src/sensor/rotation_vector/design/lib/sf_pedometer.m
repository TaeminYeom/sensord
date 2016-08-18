% sf_pedometer
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

% Sensor Fusion Implementation for Pedometer Estimation
%
% - Input Accelerometer, Gyroscope and Magnetometer sensor data
% - Plot results for Pedometer

addpath('lib');
clear
close all
clc

GRAVITY = 9.80665;

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

BUFFER_SIZE = 8700;

Accel_data = zeros(4,BUFFER_SIZE);
Gyro_data = zeros(4,BUFFER_SIZE);
Mag_data =  zeros(4,BUFFER_SIZE);

% Sensor Data simulating pedometer motions

% get accel x,y,z axis data from stored file
Accel_data(1,:) = (((dlmread("accel")(:,1))') - Bias_Ax)(1:BUFFER_SIZE);
Accel_data(2,:) = (((dlmread("accel")(:,2))') - Bias_Ay)(1:BUFFER_SIZE);
Accel_data(3,:) = (((dlmread("accel")(:,3))') - Bias_Az)(1:BUFFER_SIZE);
Accel_data(4,:) = ((dlmread("accel")(:,4))')(1:BUFFER_SIZE);

% get gyro x,y,z axis data from stored file
Gyro_data(1,:) = (((dlmread("gyro")(:,1))') - Bias_Gx)(1:BUFFER_SIZE);
Gyro_data(2,:) = (((dlmread("gyro")(:,2))') - Bias_Gy)(1:BUFFER_SIZE);
Gyro_data(3,:) = (((dlmread("gyro")(:,3))') - Bias_Gz)(1:BUFFER_SIZE);
Gyro_data(4,:) = ((dlmread("gyro")(:,4))')(1:BUFFER_SIZE);

scale_Gyro = 1150;
Gyro_data(1,:) = Gyro_data(1,:)/scale_Gyro;
Gyro_data(2,:) = Gyro_data(2,:)/scale_Gyro;
Gyro_data(3,:) = Gyro_data(3,:)/scale_Gyro;

% get magnetometer x,y,z axis data from stored file
Mag_data(1,:) = (((dlmread("magnetic")(:,1))') - Bias_Mx)(1:BUFFER_SIZE) * Sign_Mx;
Mag_data(2,:) = (((dlmread("magnetic")(:,2))') - Bias_My)(1:BUFFER_SIZE) * Sign_My;
Mag_data(3,:) = (((dlmread("magnetic")(:,3))') - Bias_Mz)(1:BUFFER_SIZE) * Sign_Mz;
Mag_data(4,:) = ((dlmread("magnetic")(:,4))')(1:BUFFER_SIZE);

hfig=(figure);
scrsz = get(0,'ScreenSize');
set(hfig,'position',scrsz);

subplot(3,1,1)
UA = Accel_data(1,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'r');
hold on;
grid on;
UA = Accel_data(2,:);
p2 = plot(1:length(UA),UA(1,1:length(UA)),'b');
hold on;
grid on;
UA = Accel_data(3,:);
p3 = plot(1:length(UA),UA(1,1:length(UA)),'g');
title(['Accel']);

subplot(3,1,2)
UA = Gyro_data(1,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'r');
hold on;
grid on;
UA = Gyro_data(2,:);
p2 = plot(1:length(UA),UA(1,1:length(UA)),'b');
hold on;
grid on;
UA = Gyro_data(3,:);
p3 = plot(1:length(UA),UA(1,1:length(UA)),'g');
title(['Gyro']);

subplot(3,1,3)
UA = Mag_data(1,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'r');
hold on;
grid on;
UA = Mag_data(2,:);
p2 = plot(1:length(UA),UA(1,1:length(UA)),'b');
hold on;
grid on;
UA = Mag_data(3,:);
p3 = plot(1:length(UA),UA(1,1:length(UA)),'g');
title(['Magnetic']);
