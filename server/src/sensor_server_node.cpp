
#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <turtlesim/Spawn.h>

int main(int argc, char** argv){
  ros::init(argc, argv, "my_tf_listener");

  ros::NodeHandle node;

  tf::TransformListener listener;

  ros::Rate rate(10.0);
  while (node.ok()){
    tf::StampedTransform transform;
    try{

      listener.lookupTransform("/base_link",
                               "/map", ros::Time(0), transform);
    }
    catch (tf::TransformException ex){
      ROS_ERROR("%s",ex.what());
      ros::Duration(1.0).sleep();
    }

    double pos_x = transform.getOrigin().x();
    double pos_y = transform.getOrigin().y();
    tf::Matrix3x3 m(transform.getRotation());
    double roll, pitch, yaw;
    m.getRPY(roll,pitch,yaw);

    ROS_INFO("x:[%f], y:[%f], yaw:[%f]", pos_x, pos_y, yaw);
    rate.sleep();
  }
  return 0;
};
