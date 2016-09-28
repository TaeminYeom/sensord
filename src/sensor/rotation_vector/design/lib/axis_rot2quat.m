% axis_rot2quat
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

% Axis rotation to quaternion function
%
% - convert sensor rotation axis and angle values to quaternion
function q = axis_rot2quat(axis, angle)
	q0 = cos(angle/2);
	q1 = -axis(1)*sin(angle/2);
	q2 = -axis(2)*sin(angle/2);
	q3 = -axis(3)*sin(angle/2);
	q = [q0 q1 q2 q3];
end
