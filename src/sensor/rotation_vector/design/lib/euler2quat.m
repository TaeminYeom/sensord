% euler2quat
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

% Euler Angles to quaternion conversion
%
% - Implementation to convert orientation in terms of euler angles to quaternion

function q = euler2quat(euler)
	theta = euler(1);
	phi = euler(2);
	psi = euler(3);
	R(1,1,1) = cos(psi)*cos(theta);
	R(1,2,1) = -sin(psi)*cos(phi) + cos(psi)*sin(theta)*sin(phi);
	R(1,3,1) = sin(psi)*sin(phi) + cos(psi)*sin(theta)*cos(phi);

	R(2,1,1) = sin(psi)*cos(theta);
	R(2,2,1) = cos(psi)*cos(phi) + sin(psi)*sin(theta)*sin(phi);
	R(2,3,1) = -cos(psi)*sin(phi) + sin(psi)*sin(theta)*cos(phi);

	R(3,1,1) = -sin(theta);
	R(3,2,1) = cos(theta)*sin(phi);
	R(3,3,1) = cos(theta)*cos(phi);

	q = rot_mat2quat(R);
end