
#include<ros/ros.h>
#include<nav_msgs/OccupancyGrid.h>
#include<geometry_msgs/PoseWithCovarianceStamped.h>
#include<tf/tf.h>
#include<math.h>
#include<serial/serial.h>
#include<string>
#include<vector>

#include"MapDataStruct.h"
#include"ControlDataStruct.h"


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

#include <sys/ioctl.h>



// This header in.h constains constant and structures needed for internet domain
// address.
#include <netinet/in.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h>

using namespace std;

typedef boost::shared_ptr<MapDataPart> MapDataPartPtr;
typedef vector<boost::shared_ptr<MapDataPart> > MapDataPartArray;
typedef boost::shared_ptr<MapData> MapDataPtr;

// Socket for communication with remote PC
int sockfd, newsockfd, cmdsockfd, socket_port;


const int SOCKET_ERROR_NOPROVIDE_PORT   = 0x00;
const int SOCKET_ERROR_EVENT_ONCREATE   = 0x01;
const int SOCKET_ERROR_EVENT_ONACCEPT   = 0x02;
const int SOCKET_ERROR_EVENT_ONWRITE    = 0x03;
const int SOCKET_ERROR_EVENT_ONREAD     = 0x04;
const int SOCKET_ERROR_EVENT_ONBINDING  = 0x05;
struct MapData* amap;
nav_msgs::OccupancyGrid::ConstPtr pMsg;

bool is_received_map(false);
bool is_need_map(false);

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
        ROS_WARN("OMG, please provide Port\n");
        break;
    default:
        ROS_ERROR("ERROR, on ??\n");
        break;
    }
}

void callback_map(const nav_msgs::OccupancyGrid::ConstPtr &msg) {

    pMsg = msg;
    is_received_map = true;
    ROS_INFO("Received topic!");

    amap->info.res = msg->info.resolution;
    amap->info.height = msg->info.height;
    amap->info.width = msg->info.width;

    amap->info.origin_x = msg->info.origin.position.x;
    amap->info.origin_y = msg->info.origin.position.y;

    tf::Matrix3x3 m(tf::Quaternion(
                         msg->info.origin.orientation.x,
                         msg->info.origin.orientation.y,
                         msg->info.origin.orientation.z,
                         msg->info.origin.orientation.w));

    double roll, pitch, yaw;
    m.getRPY(roll,pitch,yaw);


    amap->info.origin_yaw = yaw;

    // Get the map data
    memcpy(amap->data,pMsg->data.data(),SIZE_MAP);
}

void porcessRec(ros::NodeHandle* ptr_node);

bool is_sending_data(false);
bool is_connect_rst(false);
int last_sending_stamp(0);

ros::Publisher pub;

int main(int argc, char** argv) {

    amap = new MapData;

    // Initial ROS
    ros::init(argc,argv,"map_server_node");

    // Create the ROS node
    ros::NodeHandle node("~");

    ros::NodeHandle node_main;
    pub = node_main.advertise<geometry_msgs::PoseWithCovarianceStamped>
           ("/initialpose",10, true);
    ros::Subscriber sub_map = node_main.subscribe("/map",10,callback_map);

    if(!node.hasParam("socket_port"))
        ROS_WARN("Usage : rosrun robot_controller robot_controller_node _socket_port:=<socket_port> , default : 25651");
    node.param<int>("socket_port",socket_port, 25651);


    // Create a subscriber, listen to map topic


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
    cmdsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
    if(newsockfd < 0) errormsg_print(SOCKET_ERROR_EVENT_ONACCEPT);
    else ROS_INFO("CMD tunnel Connect successful");

    listen(sockfd, 5);

    newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
    if(newsockfd < 0) errormsg_print(SOCKET_ERROR_EVENT_ONACCEPT);
    else ROS_INFO("Map tunnel Connect successful");


    // Setup buffer size
    int nSendBuf = 4*1024*1024;
    int err = setsockopt(newsockfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));

    bzero(buffer,256);

    n = read(newsockfd, buffer,255);


    if( n < 0) errormsg_print(SOCKET_ERROR_EVENT_ONREAD);
    printf("Here is the message: %s \n", buffer);







    // Initial the MapMetaInfo struct
    amap->info.map_stamp = 0;
    amap->info.res = 0.05;
    amap->info.height = 4000;
    amap->info.width = 4000;
    amap->info.origin_x = 0;
    amap->info.origin_y = 0;
    amap->info.origin_yaw = 0.0;

    ros::Rate rate(5);
    while (node.ok()){

      porcessRec(&node);

      if( is_received_map && is_need_map) {


          // Package the META data for MAP
          ROS_INFO("Map Metadata: Res = ");


          volatile int cbLeft;
          volatile int cbDataSended;
          volatile int currentSended;

          // Repackage the Map data;
          MapDataPart apart;
          apart.info.map_stamp = amap->info.map_stamp;
          apart.info.res = amap->info.res;
          apart.info.height = amap->info.height;
          apart.info.width = amap->info.height;
          apart.info.origin_x = amap->info.origin_x;
          apart.info.origin_y = amap->info.origin_y;
          apart.info.origin_yaw = amap->info.origin_yaw;


          for(int i=0;i<SIZE_MAP;i+= MAP_PER_CUT_SIZE) {


              porcessRec(&node);

              if(is_sending_data == true && is_connect_rst == true) {
                  ROS_WARN("MAP SERVER :RESTORE STAMP %d",last_sending_stamp);
                  i = last_sending_stamp;
                  is_connect_rst = false;
              }
              // Record the part stamp and copy the part data into MapDataPart
              apart.info.part_stamp = i;
              memcpy(apart.data, amap->data + i, MAP_PER_CUT_SIZE);

              // Sleep for a while
              usleep(50000);

              cbLeft = sizeof(MapDataPart);
              cbDataSended = 0;
              currentSended = 0;
              last_sending_stamp = i;
              ROS_INFO("Map Part : Stamp:[%d]",i);
              do{

                  //ROS_INFO("SEND MESSAGE: left:%d  currentsended:%d",cbLeft, currentSended);

                  if(cbLeft > 4000) {
                  currentSended = write(newsockfd,(char*)&apart+cbDataSended,4000);
                  } else {
                  currentSended = write(newsockfd,(char*)&apart+cbDataSended,cbLeft);
                  }

                  is_sending_data = true;


                  if(currentSended == -1) {
                      currentSended = 0;
                      ROS_ERROR("MAP SERVER: Socket Send fail");
                      ROS_ERROR("MAP SERVER: RESET CONNECT, PLEASE RECONNECT!");
                      close(newsockfd);
                      is_connect_rst = true;

                      goto RECONNECT;
                  }

                  cbLeft -= currentSended;
                  cbDataSended += currentSended;

                  //ROS_INFO("SENDED MESSAGE: left:%d  currentsended:%d",cbLeft, currentSended);

              }while(cbLeft > 0);


              if(i == 0) {
                  last_sending_stamp = 0;
              } else {
                  last_sending_stamp = i-1;
              }

              is_sending_data = false;

          }

          is_need_map = false;
      } else {
          //ROS_INFO("NO MAP RECEIVED");
      }




      rate.sleep();
      ros::spinOnce();
    }

    close(newsockfd);
    close(sockfd);

    delete amap;
}

void porcessRec(ros::NodeHandle* ptr_node) {
    // Read the command from MFC Client
    int bytes_available(0);
    if(ioctl(cmdsockfd, FIONREAD, &bytes_available) < 0) {
        ROS_ERROR("Read Socket Read buffer failed, error= %d\n ", errno);
    }
    ControlMsg cmd_msg;
    if(bytes_available > 0) {
        read(cmdsockfd, &cmd_msg,sizeof(ControlMsg));
        ROS_INFO("Read CMD Message! %d",cmd_msg.map_msg);
        bytes_available = 0;
        switch(cmd_msg.map_msg) {
        case SET_INIT_POSE: {

            // Create a empty PoseWithCovarianceStamped message
            geometry_msgs::PoseWithCovarianceStamped initial_pose;

            // Setup the Header
            initial_pose.header.frame_id = "/map";
            initial_pose.header.stamp = ros::Time::now();

            // copy the position data
            initial_pose.pose.pose.position.x = cmd_msg.init_pose_position[0];
            initial_pose.pose.pose.position.y = cmd_msg.init_pose_position[1];
            initial_pose.pose.pose.position.z = cmd_msg.init_pose_position[2];

            // copy the orientation data
            tf::Quaternion q = tf::createQuaternionFromYaw(cmd_msg.init_pose_orientation[2]);
            geometry_msgs::Quaternion q_msg;
            tf::quaternionTFToMsg(q,q_msg);
            initial_pose.pose.pose.orientation = q_msg;

            // copy the covariace data
            for(int i=0;i<36;i++) {
                initial_pose.pose.covariance[i] = cmd_msg.init_pose_cov[i];
            }


            // publish the initial pose topic
            pub.publish(initial_pose);
            ROS_INFO("Initial pose set ! ");
            break;
        }
        case GET_MAP_LIST: {
            ControlMsg report_msg;
            memcpy(&report_msg,&cmd_msg,sizeof(ControlMsg));
            for(int i=0;i<5;i++) {
                report_msg.data[i] = 10-i;
            }

            write(cmdsockfd,&report_msg,sizeof(ControlMsg));

            break;
        }
        case SEL_MAP:

            break;
        case LOAD_MAP:

            break;

        case GET_MAP:
            is_need_map = true;
            break;
        }
    }




}
