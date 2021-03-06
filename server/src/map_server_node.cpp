
#include<ros/ros.h>
#include<nav_msgs/OccupancyGrid.h>
#include<tf/tf.h>
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

#define MAP_SIZE_X (400)
#define MAP_SIZE_Y (400)

#define SIZE_MAP (MAP_SIZE_X * MAP_SIZE_Y)
/***************************/

struct MapMetaInfo{
    float res;
    unsigned int height;
    unsigned int width;
    float origin_x;
    float origin_y;
    float origin_yaw;
};

struct MapData{
    struct MapMetaInfo info;
    uint8_t data[SIZE_MAP];
};

// Socket for communication with remote PC
int sockfd, newsockfd, socket_port;


const int SOCKET_ERROR_NOPROVIDE_PORT   = 0x00;
const int SOCKET_ERROR_EVENT_ONCREATE   = 0x01;
const int SOCKET_ERROR_EVENT_ONACCEPT   = 0x02;
const int SOCKET_ERROR_EVENT_ONWRITE    = 0x03;
const int SOCKET_ERROR_EVENT_ONREAD     = 0x04;
const int SOCKET_ERROR_EVENT_ONBINDING  = 0x05;

struct MapData* amap;
nav_msgs::OccupancyGrid::ConstPtr pMsg;

bool is_received_map(false);

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



int main(int argc, char** argv) {

    amap = new MapData;

    // Initial ROS
    ros::init(argc,argv,"ocare_controller_node");

    // Create the ROS node
    ros::NodeHandle node("~");

    ros::NodeHandle node_main;
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
    newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);

    if(newsockfd < 0) errormsg_print(SOCKET_ERROR_EVENT_ONACCEPT);


    // Setup buffer size
    int nSendBuf = 4*1024*1024;
    int err = setsockopt(newsockfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));

    bzero(buffer,256);

    n = read(newsockfd, buffer,255);


    if( n < 0) errormsg_print(SOCKET_ERROR_EVENT_ONREAD);
    printf("Here is the message: %s \n", buffer);







    // Initial the MapMetaInfo struct
    amap->info.res = 0.05;
    amap->info.height = 4000;
    amap->info.width = 4000;
    amap->info.origin_x = 0;
    amap->info.origin_y = 0;
    amap->info.origin_yaw = 0.0;

    ros::Rate rate(5);
    while (node.ok()){
      if( is_received_map == true) {


          // Package the META data for MAP
          ROS_INFO("Map Metadata: Res = ");


          volatile int cbLeft;
          volatile int cbDataSended;
          volatile int currentSended;

          cbLeft = sizeof(MapData);
          cbDataSended = 0;
          currentSended = 0;

          do{

              ROS_INFO("SEND MESSAGE: left:%d  currentsended:%d",cbLeft, currentSended);

              if(cbLeft > 4000) {
                  currentSended = write(newsockfd,(char*)amap+cbDataSended,4000);
              } else {
                  currentSended = write(newsockfd,(char*)amap+cbDataSended,cbLeft);
              }
              //sleep(1);
/*
              printf("Press any key to continue\n");
              while(std::cin.get()!='\n');
              if(currentSended == -1) {
                  currentSended = 0;
                  ROS_ERROR("Socket Send fail");
              }

              */
              cbLeft -= currentSended;
              cbDataSended += currentSended;

              ROS_INFO("SENDED MESSAGE: left:%d  currentsended:%d",cbLeft, currentSended);

          }while(cbLeft > 0);
      }




      rate.sleep();
      ros::spinOnce();
    }

    close(newsockfd);
    close(sockfd);

    delete amap;
}
