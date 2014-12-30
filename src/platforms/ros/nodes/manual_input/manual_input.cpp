/****************************************************************************
 *
 *   Copyright (c) 2014 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file manual_input.cpp
 * Reads from the ros joystick topic and publishes to the px4 manual control input topic.
 *
 * @author Thomas Gubler <thomasgubler@gmail.com>
*/

#include "manual_input.h"

#include <px4/manual_control_setpoint.h>
#include <platforms/px4_middleware.h>

ManualInput::ManualInput() :
	_n(),
	joy_sub(_n.subscribe("joy", 10, &ManualInput::JoyCallback, this)),
	_man_ctrl_sp_pub(_n.advertise<px4::manual_control_setpoint>("manual_control_setpoint", 10))
{
	/* Load parameters, default values work for Microsoft XBox Controller */
	_n.param("map_x", param_axes_map[0], 4);
	_n.param("scale_x", param_axes_scale[0], 1.0);
	_n.param("off_x", param_axes_offset[0], 0.0);

	_n.param("map_y", param_axes_map[1], 3);
	_n.param("scale_y", param_axes_scale[1], -1.0);
	_n.param("off_y", param_axes_offset[1], 0.0);

	_n.param("map_z", param_axes_map[2], 2);
	_n.param("scale_z", param_axes_scale[2], -0.5);
	_n.param("off_z", param_axes_offset[2], 0.5);

	_n.param("map_r", param_axes_map[3], 0);
	_n.param("scale_r", param_axes_scale[3], -1.0);
	_n.param("off_r", param_axes_offset[3], 0.0);
}

void ManualInput::JoyCallback(const sensor_msgs::JoyConstPtr& msg)
{
	px4::manual_control_setpoint msg_out;

	/* Fill px4 manual control setpoint topic with contents from ros joystick */
	/* Map sticks to x, y, z, r */
	MapAxis(msg, param_axes_map[0], param_axes_scale[0], param_axes_offset[0], msg_out.x);
	MapAxis(msg, param_axes_map[1], param_axes_scale[1], param_axes_offset[1], msg_out.y);
	MapAxis(msg, param_axes_map[2], param_axes_scale[2], param_axes_offset[2], msg_out.z);
	MapAxis(msg, param_axes_map[3], param_axes_scale[3], param_axes_offset[3], msg_out.r);
	//ROS_INFO("x: %1.4f y: %1.4f z: %1.4f r: %1.4f", msg_out.x, msg_out.y, msg_out.z, msg_out.r);

	/* Map buttons to switches */
	//XXX todo
	/* for now just publish switches in position for manual flight */
	msg_out.mode_switch = 0;
	msg_out.return_switch = 0;
	msg_out.posctl_switch = 0;
	msg_out.loiter_switch = 0;
	msg_out.acro_switch = 0;
	msg_out.offboard_switch = 0;

	msg_out.timestamp = px4::get_time_micros();

	_man_ctrl_sp_pub.publish(msg_out);
}

void ManualInput::MapAxis(const sensor_msgs::JoyConstPtr& msg, int map_index, double scale, double offset, float &out)
{
	out = (float)(msg->axes[map_index] * scale + offset);
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "manual_input");
  ManualInput m;
  ros::spin();
  return 0;
}