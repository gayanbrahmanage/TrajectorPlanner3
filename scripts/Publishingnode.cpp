#include "ros/ros.h"
#include "nav_msgs/OccupancyGrid.h"
#include "nav_msgs/MapMetaData.h"
#include "geometry_msgs/Pose.h"
#include "geometry_msgs/Point.h"
#include "geometry_msgs/Quaternion.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "std_msgs/Header.h"
#include <string>
#include <iostream>
#include <fstream>
int main(int argc, char ** argv)
{
	ros::init(argc, argv, "PublishingNode");
	ros::NodeHandle n;
	ros::Rate loop_rate(0.05);
	ros::Publisher pub = n.advertise<nav_msgs::OccupancyGrid>("map", 1000);
	//read image
	nav_msgs::OccupancyGrid map;
	nav_msgs::MapMetaData mapData;
	std::string imageName = "/home/thomas/catkin_ws/src/trajectory_planner/scripts/path10.png";
	cv::Mat file = cv::imread(imageName, 0);
	if(!file.data)
	{
		std::cout<<"failed"<<std::endl;
		exit(1);
	}
	cv::threshold(file, file, 100, 255,cv::THRESH_BINARY);
	int length = file.rows*file.cols;
	std::vector<signed char> array(length);
	std::cout<<"reading "<<length<<" characters"<<std::endl;
	for(int i =0; i<file.rows;i++)
	{
		for(int j=0; j<file.cols;j++)
		{
			int t =100;
			if(file.at<uchar>(i,j))
			{
				t=0;
			}
			map.data.push_back(t);
		}
	}
	mapData.map_load_time = ros::Time::now();
	mapData.resolution = 4;
	mapData.width = file.rows;
	mapData.height = file.cols;
	map.info = mapData;
	int count =1;
	while(ros::ok())
	{
		std_msgs::Header header;
		header.seq = count++;
		header.stamp =  ros::Time::now();
		header.frame_id = "map";
		map.header = header;
		pub.publish(map);
		ros::spinOnce();
		std::cout<<"sent"<<std::endl;
		loop_rate.sleep();
	}
	std::cout<<"Shutting down"<<std::endl;
	return 0;
}
