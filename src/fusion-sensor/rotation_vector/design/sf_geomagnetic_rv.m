% sf_geomagnetic_rv
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

% Sensor Fusion Implementation for Determination of Geomagentic rotation Vector
%
% - Input Accelerometer and Magnetometer sensor data
% - Call estimate_geomagnetic_rotation
% - Plot results for geomagetic rotation on reference axis

addpath('lib');
clear
close all
clc

GRAVITY = 9.80665;

Max_Range_Accel = 39.203407; Min_Range_Accel = -39.204006; Res_Accel = 0.000598;
Max_Range_Magnetic = 1200; Min_Range_Magnetic = -1200; Res_Magnetic = 1;

Bias_Ax = 0.098586;
Bias_Ay = 0.18385;
Bias_Az = 10.084 - GRAVITY;

Bias_Mx = 0;
Bias_My = 0;
Bias_Mz = 0;

Sign_Mx = 1;
Sign_My = 1;
Sign_Mz = 1;

BUFFER_SIZE = 1095;

Accel_data = zeros(4,BUFFER_SIZE);
Mag_data =  zeros(4,BUFFER_SIZE);

Geo_RV = zeros(4,BUFFER_SIZE);
Orientation_RV = zeros(3,BUFFER_SIZE);

% Sensor Data simulating orientation motions

% get accel x,y,z axis data from stored file
Accel_data(1,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/accel.txt")(:,1))') - Bias_Ax)(1:BUFFER_SIZE);
Accel_data(2,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/accel.txt")(:,2))') - Bias_Ay)(1:BUFFER_SIZE);
Accel_data(3,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/accel.txt")(:,3))') - Bias_Az)(1:BUFFER_SIZE);
Accel_data(4,:) = ((dlmread("data/100ms/orientation/roll_pitch_yaw/accel.txt")(:,4))')(1:BUFFER_SIZE);

% get magnetometer x,y,z axis data from stored file
Mag_data(1,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/magnetic.txt")(:,1))') + Bias_Mx)(1:BUFFER_SIZE) * Sign_Mx;
Mag_data(2,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/magnetic.txt")(:,2))') + Bias_My)(1:BUFFER_SIZE) * Sign_My;
Mag_data(3,:) = (((dlmread("data/100ms/orientation/roll_pitch_yaw/magnetic.txt")(:,3))') + Bias_Mz)(1:BUFFER_SIZE) * Sign_Mz;
Mag_data(4,:) = ((dlmread("data/100ms/orientation/roll_pitch_yaw/magnetic.txt")(:,4))')(1:BUFFER_SIZE);

% estimate orientation
Geo_RV = estimate_geomagnetic_rv(Accel_data, Mag_data);

for i = 1:BUFFER_SIZE
	Orientation_RV(:,i) = quat2euler(Geo_RV(i,:));
end

hfig=(figure);
scrsz = get(0,'ScreenSize');
set(hfig,'position',scrsz);
% Geomagnetic Rotation Vector Plot Results
subplot(3,1,1)
UA = Orientation_RV(1,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'k');
legend(p1,'x-axis');
title(['Pitch']);
subplot(3,1,2)
UA = Orientation_RV(2,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'b');
legend(p1,'y-axis');
title(['Roll']);
subplot(3,1,3)
UA = Orientation_RV(3,:);
p1 = plot(1:length(UA),UA(1,1:length(UA)),'r');
legend(p1,'z-axis');
title(['Yaw']);

