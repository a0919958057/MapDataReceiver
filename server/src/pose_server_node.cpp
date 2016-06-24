

#include<ros/ros.h>
#include<sensor_msgs/Imu.h>
#include<sensor_msgs/LaserScan.h>
#include<tf/tf.h>
#include<tf/transform_listener.h>
#include<geometry_msgs/PoseWithCovarianceStamped.h>
#include<angles/angles.h>
#include<math.h>
#include<serial/serial.h>
#include<string>

/******************************/

// This header contains declearations used in most input and output and is typicall
// included in all C program
#include <stdio.h>
#include <stdlib.h>

// This header socket.h include a number of definition of stucture need for socket
#include <sys/socket.h>
// This header file contains definition of number of data type used in system
// calls. These types are used in the next two include files.
#include <sys/types.h>



// This header in.h constains constant and structures needed for internet domain
// address.
#include <netinet/in.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h>

/***************************/

#include "PoseDataStruct.h"

// Pose data
PoseData pose_data;

// Socket for communication with remote PC
int sockfd, newsockfd, socket_port;


const int SOCKET_ERROR_NOPROVIDE_PORT   = 0x00;
const int SOCKET_ERROR_EVENT_ONCREATE   = 0x01;
const int SOCKET_ERROR_EVENT_ONACCEPT   = 0x02;
const int SOCKET_ERROR_EVENT_ONWRITE    = 0x03;
const int SOCKET_ERROR_EVENT_ONREAD     = 0x04;
const int SOCKET_ERROR_EVENT_ONBINDING  = 0x05;


void errormsg_print(const int _event) {

    switch (_event) {
    case SOCKET_ERROR_EVENT_ONCREATE:
        ROS_ERROR("ERROR, on Create Socket\n");
        break;
    case SOCKET_ERROR_EVENT_ONBINDING:
        ROS_ERROR("ERROR, on binding socket to server\n");
        break;
    case SOCKET_ERROR_EVENT_ONACCEPT:
        ROS_ERROR("ERROR, on accept remote request\n");
        break;
    case SOCKET_ERROR_EVENT_ONREAD:
        ROS_ERROR("ERROR, on read message from socket\n");
        break;
    case SOCKET_ERROR_EVENT_ONWRITE:
        ROS_ERROR("ERROR, on write message to socket\n");
        break;
    case SOCKET_ERROR_NOPROVIDE_PORT:
        ROS_ERROR("OMG, please provide Port\n");
        break;
    default:
        ROS_ERROR("ERROR, on ??\n");
        break;
    }
}

void callback_amcl_pose(const geometry_msgs::PoseWithCovarianceStampedPtr &msg) {
    // Copy the covariance from topic
    for(int i=0;i<36;i++) {
        pose_data.pose_cov[i] = msg->pose.covariance[i];
    }
}

int main(int argc, char** argv) {

    // Initial the pose data
    memset(&pose_data, 0, sizeof(PoseData));

    // Initial ROS
    ros::init(argc,argv,"ocare_controller_node");

    // Create the ROS node
    ros::NodeHandle privata_node("~");

    tf::TransformListener listener;

    if(!privata_node.hasParam("socket_port"))
        ROS_WARN("Usage : rosrun robot_controller pose_server_node _socket_port:=<socket_port>");
    privata_node.param<int>("socket_port",socket_port,25652);

    ros::NodeHandle node;
    ros::Subscriber sub = node.subscribe("/amcl_pose", 100, callback_amcl_pose);


    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;


    if (argc < 2) errormsg_print(SOCKET_ERROR_NOPROVIDE_PORT);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Check is there socket create error
    if(sockfd < 0) {
        fprintf(stdout, "ERROR opening socket\n");
    } else {
        fprintf(stdout, "SUCCESSFUL opening socket:%d\n",sockfd);
    }

    bzero((char*) &serv_addr, sizeof(serv_addr));


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(socket_port);

    // SET SOCKET REUSE Address
    int sock_opt = 1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void*)&sock_opt, sizeof(sock_opt)) == -1) {
        ROS_ERROR("ERROR ON SETUP SOCKET CONFIG\n");
        return -1;
    } else {
        ROS_INFO("SUCCESSFUL ON SETUP SOCKET CONFIG\n");
    }

    ROS_INFO("Binding socket at port:%d\n",socket_port);
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        errormsg_print(SOCKET_ERROR_EVENT_ONBINDING);
    else
        ROS_INFO("SUCCESSFUL on binding");


    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    RECONNECT:
    newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);

    if(newsockfd < 0) errormsg_print(SOCKET_ERROR_EVENT_ONACCEPT);


    bzero(buffer,256);

    n = read(newsockfd, buffer,255);


    if( n < 0) errormsg_print(SOCKET_ERROR_EVENT_ONREAD);
    printf("Here is the message: %s \n", buffer);






    ros::Rate rate(20.0);
    while (privata_node.ok()){
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

      // Copy the (x, y, z, x_axis_rotation, y_axis_rotation, z_axis_rotation)
      // from newest topic data to local pose_data
      pose_data.pose_position[0] = pos_x;
      pose_data.pose_position[1] = pos_y;
      pose_data.pose_position[2] = 0;
      pose_data.pose_orientation[0] = roll;
      pose_data.pose_orientation[1] = pitch;
      pose_data.pose_orientation[2] = yaw;

      ROS_INFO("x:[%f], y:[%f], yaw:[%f]", pos_x, pos_y, yaw);

      int cbLeft(sizeof(PoseData));
      int cbDataSended(0);
      int currentSended(0);

      do{
          currentSended = write(newsockfd,(char*)&pose_data+cbDataSended,cbLeft);
          cbLeft -= currentSended;
          cbDataSended += currentSended;

          if(currentSended == -1) {
              currentSended = 0;
              ROS_ERROR("Pose server: Socket Send fail");
              ROS_ERROR("Pose server: RESET CONNECT, PLEASE RECONNECT!");
              close(newsockfd);

              goto RECONNECT;
          }


      }while(cbLeft > 0);

      ros::spinOnce();
      rate.sleep();
    }

    close(newsockfd);
    close(sockfd);
}
