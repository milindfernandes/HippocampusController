#ifndef CARROT_H
#define CARROT_H

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/TwistStamped.h>
#include <geometry_msgs/Quaternion.h>
#include <math.h>
#include <Eigen/Eigen>

#include <AbstractHippocampusController.h>

using namespace std;
using namespace Eigen;

class CarrotControl: public AbstractHippocampusController{
private:

    Vector2f loiterOrigin;
    float radius;
    float gain;
    float lambda;

    float roll, pitch, yaw, thrust;
    float yawMax;

    void algorithm();
    void loadParameters();
public:
    CarrotControl(const ros::NodeHandle& nh, const ros::NodeHandle& nh_private, double frequency);
    AttitudeSetpoint generateSetpoint();
};

CarrotControl::CarrotControl(const ros::NodeHandle& nh, const ros::NodeHandle& nh_private, double frequency):
    //Call super class constructor
    AbstractHippocampusController(nh, nh_private, frequency){
    _pose_sub = _nh.subscribe<geometry_msgs::PoseStamped> ("mavros/local_position/pose", 10, &CarrotControl::poseCallback, dynamic_cast<AbstractHippocampusController*>(this));
    CarrotControl::loadParameters();
    roll = 0;
}


AttitudeSetpoint CarrotControl::generateSetpoint(){
    AttitudeSetpoint sp;
    CarrotControl::loadParameters();
    CarrotControl::algorithm();

    sp.set(roll, pitch, yaw, thrust);
    return sp;
}

void CarrotControl::algorithm(){
    //calculate cross track error d
    Vector2f p;
    p[0] = _pose_NED[0];
    p[1] = -_pose_NED[1];
    float norm = pow(loiterOrigin[0] - p[0], loiterOrigin[1] - p[1]);
    float d = norm - radius;


    //calculate s
    Vector2f s;
    float theta = atan2(p[1] - loiterOrigin[1], p[0] - loiterOrigin[0]);
    s[0] = radius * cos(theta + lambda);
    s[1] = radius * sin(theta + lambda);
    ROS_INFO("d: %.2f;    s: %.2f; %.2f", d, s[0], s[1]);

    //calculate desired heading
    float yaw_d = atan2(s[1] - p[1], s[0] - p[0]);
    float delta_yaw = yaw_d + _attitude_NED[2];

    /**
    if (delta_yaw > yawMax) {
        delta_yaw = yawMax;
    } else if (delta_yaw < -yawMax){
        delta_yaw = -yawMax;
    }*/

    //calculate input
    yaw = -gain * delta_yaw;
    ROS_INFO("yaw: %.2f;  yaw_d: %.2f;  delta_yaw: %.2f", _attitude_NED[2], yaw_d, delta_yaw);

}

void CarrotControl::loadParameters(){
    _nh_private.param<float>("thrust", thrust, 0.0);
    _nh_private.param<float>("yawMax", yawMax, 0.0);

    _nh_private.param<float>("origin_x", loiterOrigin[0], 0.0);
    _nh_private.param<float>("origin_y", loiterOrigin[1], 0.0);
    _nh_private.param<float>("gain", gain, 0.0);
    _nh_private.param<float>("lambda", lambda, 0.0);
    _nh_private.param<float>("radius", radius, 0.0);
}

#endif


