% estimate_geomagnetic_rv
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

% Geomagnetic Rotation Vector Estimation function
%
% - Input orientation using only Accel and Geomagnetic sensors
% - Orientation Estimation using estimate_orientation function
% - Output aiding system quaternion as geomagnetic rotation vector

function [Quat_aid]  = estimate_geomagnetic_rv(Accel_data, Mag_data)

BUFFER_SIZE = size(Accel_data,2);

Gyro_data = zeros(4,BUFFER_SIZE);

Quat_driv = zeros(4,BUFFER_SIZE);
Quat_aid = zeros(4,BUFFER_SIZE);
Quat_err = zeros(4,BUFFER_SIZE);

% estimate orientation
[Quat_driv, Quat_aid, Quat_err]  = estimate_orientation(Accel_data, Gyro_data, Mag_data);

end
