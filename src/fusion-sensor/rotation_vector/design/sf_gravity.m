% sf_gravity
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

% Sensor Fusion Implementation for Determination of Gravity
%
% - Input Accelerometer, Gyroscope and Magnetometer sensor data
% - Call estimate_gravity
% - Plot results for gravitational force on reference axis

addpath('lib');
clear
close all
clc

GRAVITY = 9.80665;

Max_Range_Accel = 39.203407; Min_Range_Accel = -39.204006; Res_Accel = 0.000598;
Max_Range_Gyro = 1146.862549; Min_Range_Gyro = -1146.880005; Res_Gyro = 0.017500;
Max_Range_Magnetic = 1200; Min_Range_Magnetic = -1200; Res_Magnetic = 1;

Bias_Ax = 0.098586;
Bias_Ay = 0.18385;
Bias_Az = 10.084 - GRAVITY;

Bias_Gx = -5.3539;
Bias_Gy = 0.24325;
Bias_Gz = 2.3391;

Bias_Mx = 0;
Bias_My = 0;
Bias_Mz = 0;

Sign_Mx = 1;
Sign_My = 1;
Sign_Mz = 1;

BUFFER_SIZE = 100;

Accel_data = zeros(4,BUFFER_SIZE);
Gyro_data = zeros(4,BUFFER_SIZE);
Mag_data =  zeros(4,BUFFER_SIZE);

OR_driv = zeros(3,BUFFER_SIZE);
OR_aid = zeros(3,BUFFER_SIZE);
OR_err = zeros(3,BUFFER_SIZE);

% Sensor Data simulating orientation motions

% get accel x,y,z axis data from stored file
Accel_data(1,:) = (((dlmread("data/100ms/gravity/single_roll_throw/accel.txt")(:,1))') - Bias_Ax)(1:BUFFER_SIZE);
Accel_data(2,:) = (((dlmread("data/100ms/gravity/single_roll_throw/accel.txt")(:,2))') - Bias_Ay)(1:BUFFER_SIZE);
Accel_data(3,:) = (((dlmread("data/100ms/gravity/single_roll_throw/accel.txt")(:,3))') - Bias_Az)(1:BUFFER_SIZE);
Accel_data(4,:) = ((dlmread("data/100ms/gravity/single_roll_throw/accel.txt")(:,4))')(1:BUFFER_SIZE);

% get gyro x,y,z axis data from stored file
Gyro_data(1,:) = (((dlmread("data/100ms/gravity/single_roll_throw/gyro.txt")(:,1))') - Bias_Gx)(1:BUFFER_SIZE);
Gyro_data(2,:) = (((dlmread("data/100ms/gravity/single_roll_throw/gyro.txt")(:,2))') - Bias_Gy)(1:BUFFER_SIZE);
Gyro_data(3,:) = (((dlmread("data/100ms/gravity/single_roll_throw/gyro.txt")(:,3))') - Bias_Gz)(1:BUFFER_SIZE);
Gyro_data(4,:) = ((dlmread("data/100ms/gravity/single_roll_throw/gyro.txt")(:,4))')(1:BUFFER_SIZE);

scale_Gyro = 575;
Gyro_data(1,:) = Gyro_data(1,:)/scale_Gyro;
Gyro_data(2,:) = Gyro_data(2,:)/scale_Gyro;
Gyro_data(3,:) = Gyro_data(3,:)/scale_Gyro;

% get magnetometer x,y,z axis data from stored file
Mag_data(1,:) = (((dlmread("data/100ms/gravity/single_roll_throw/magnetic.txt")(:,1))') + Bias_Mx)(1:BUFFER_SIZE) * Sign_Mx;
Mag_data(2,:) = (((dlmread("data/100ms/gravity/single_roll_throw/magnetic.txt")(:,2))') + Bias_My)(1:BUFFER_SIZE) * Sign_My;
Mag_data(3,:) = (((dlmread("data/100ms/gravity/single_roll_throw/magnetic.txt")(:,3))') + Bias_Mz)(1:BUFFER_SIZE) * Sign_Mz;
Mag_data(4,:) = ((dlmread("data/100ms/gravity/single_roll_throw/magnetic.txt")(:,4))')(1:BUFFER_SIZE);

% estimate orientation
Gravity = estimate_gravity(Accel_data, Gyro_data, Mag_data);

hfig=(figure);
scrsz = get(0,'ScreenSize');
set(hfig,'position',scrsz);
% Accelerometer Raw data
subplot(2,1,1)
UA = Accel_data(1,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'k');
hold on;
grid on;
UA = Accel_data(2,:);
p2 = plot(1:length(UA),UA(1,1:length(UA)),'b');
hold on;
grid on;
UA = Accel_data(3,:);
p3 = plot(1:length(UA),UA(1,1:length(UA)),'r');
title(['Raw Accelerometer Data']);
legend([p1 p2 p3],'x-axis', 'y-axis', 'z-axis');

% Gravity Plot Results
subplot(2,1,2)
UA = Gravity(1,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'k');
hold on;
grid on;
UA = Gravity(2,:);
p2 = plot(1:length(UA),UA(1,1:length(UA)),'b');
hold on;
grid on;
UA = Gravity(3,:);
p3 = plot(1:length(UA),UA(1,1:length(UA)),'r');
title(['Gravity']);
legend([p1 p2 p3],'x-axis', 'y-axis', 'z-axis');




