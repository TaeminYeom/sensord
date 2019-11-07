% estimate_linear_acceleration
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

% Linear Acceleration Estimation function
%
% - Gravity estimation using the estimate_gravity function
% - compute linear acceleration based gravitational force computed for each axes

function [Linear_Acceleration]  = estimate_linear_acceleration(Accel_data, Gyro_data, Mag_data)

GRAVITY = 9.80665;

BUFFER_SIZE = size(Accel_data,2);

Gravity = zeros(3,BUFFER_SIZE);
Linear_Acceleration = zeros(3,BUFFER_SIZE);

% estimate orientation
Gravity  = estimate_gravity(Accel_data, Gyro_data, Mag_data);

Linear_Acceleration = Accel_data(1:3,:) - [Gravity(2,:); Gravity(1,:); Gravity(3,:);];

end
