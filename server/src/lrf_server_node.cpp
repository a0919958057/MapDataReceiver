#include<ros/ros.h>
#include<sensor_msgs/Imu.h>
#include<sensor_msgs/LaserScan.h>
#include<math.h>
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

#define LRF_DATA_LENGH 510
/***************************/

std::string laser_topic;

bool is_rst_connect(false);

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

void callback_laser(const sensor_msgs::LaserScan::ConstPtr& msg) {
    ROS_INFO("Laser Seq: [%d]", msg->header.seq);
    const float* aRLFdata = msg->ranges.data();

    int cbLeft(LRF_DATA_LENGH);
    int cbDataSended(0);
    int currentSended(0);

    do{
        currentSended = write(newsockfd,(char*)aRLFdata+cbDataSended,cbLeft);

        if(currentSended == -1) {
            currentSended = 0;
            ROS_ERROR("Socket Send fail");
            ROS_ERROR("RESET CONNECT, PLEASE RECONNECT!");
            close(newsockfd);
            is_rst_connect = true;

            return;
        }

        cbLeft -= currentSended;
        cbDataSended += currentSended;



    }while(cbLeft > 0);
}

int main(int argc, char** argv) {

    // Initial ROS
    ros::init(argc,argv,"lrf_server_node");

    // Create the ROS node
    ros::NodeHandle node("~");

    if(!node.hasParam("laser_topic"))
        ROS_WARN("Usage : rosrun robot_controller robot_controller_node _laser_topic:=<laser_topic>");
    node.param<std::string>("laser_topic",laser_topic,"/scan");

    if(!node.hasParam("socket_port"))
        ROS_WARN("Usage : rosrun robot_controller robot_controller_node _socket_port:=<socket_port>");
    node.param<int>("socket_port",socket_port,25650);

    // Subscribe the Topic of sensor_msgs/IMU
    ros::Subscriber sub_lrf = node.subscribe(laser_topic,100,callback_laser);

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




    // Into the Message loop, while there is a topic be subscribed exist then callback.
    ros::Rate r(10);
    while(ros::ok()) {
        if(is_rst_connect) {
            is_rst_connect = false;
            goto RECONNECT;
        }
        r.sleep();
        ros::spinOnce();
    }

    close(newsockfd);
    close(sockfd);
}
