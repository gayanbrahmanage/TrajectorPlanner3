#include "trajectory_planner/Trajectory_Definitions.hpp"
#include "ros/ros.h"
#include "nav_msgs/Path.h"
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include "nav_msgs/OccupancyGrid.h"
/*
	Uses ros to subscibe to the /map and publishes a path to /path
	Author:Thomas Vy
	Date: June 12 2018
	Email: thomas.vy@ucalgary.ca
*/
ros::Publisher pub; //publisher to path
tf::TransformListener * plr; //transform listener
//sends the transform of the path relative to the map
void sendTransform()
{
	static tf::TransformBroadcaster broadcaster;
	broadcaster.sendTransform(
			tf::StampedTransform(
			tf::Transform(tf::Quaternion(0, 0, 0, 1), tf::Vector3(0, 0, 0.0)),   // map and path do need need to be tranformed.
				ros::Time::now(), "map", "path" ));
}
//publishes the path to /path
void publishInfo(const nav_msgs::OccupancyGrid::ConstPtr& msg)
{
	tf::StampedTransform transform;
  try{
    plr->lookupTransform("/map", "/base_link",
                             msg->info.map_load_time , transform); //looks for the transform between map and the robot
  }
  catch (tf::TransformException ex){
		return; //returns if there is an error in the transformation
  }
	int grid_x = (transform.getOrigin().x() - (int)msg->info.origin.position.x) / msg->info.resolution; //changes the robot real position to the grid
  int	grid_y = (transform.getOrigin().y() - (int)msg->info.origin.position.y) / msg->info.resolution;//changes the robot real position to the grid


	Pose start(grid_x,grid_y, tf::getYaw(transform.getRotation())); //the start position (the robot's current position)
	Pose goal(2151, 1628); //the goal position
	matrix original((int)msg->info.height, vector<double>((int)msg->info.width)); //the original map in a 2d vector
	for(int y =0 , k=0; y<(int)msg->info.height; y++)
	{
		for(int x=0; x<(int)msg->info.width ;x++)
		{
			//make start and end
			original[x][y] = msg->data[k++];
		}
	}
	if(original[goal.x][goal.y]!=0 ||original[start.x][start.y]!=0) // if the goal or start not in free space end the path finding
	{
		return;
	}
	Image img(original); //creates a converted image bsaed off original map
	img.insert_borders(); //creates cost map
	nav_msgs::Path path;
	static int num =0;
	path.header.seq = num++;
	path.header.stamp = ros::Time::now();
	path.header.frame_id = "path";
	if(img.planner(start, goal)) //plans the path if it find a path it publishes it
	{
		ROS_INFO("Found Path");
		path.poses = img.getPath();
		for(int i =0; i<path.poses.size();i++)
		{
			path.poses[i].pose.position.x = (path.poses[i].pose.position.x*msg->info.resolution+msg->info.origin.position.x);
			path.poses[i].pose.position.y = (path.poses[i].pose.position.y*msg->info.resolution+msg->info.origin.position.y);
		}
	}
	else
	{
		ROS_ERROR("Could NOT find path");
		return;
	}
	sendTransform();
	pub.publish(path);
}

int main(int argc, char ** argv)
{
	ros::init(argc, argv, "TrajectoryNode"); //init the node
	ros::NodeHandle n;
	pub = n.advertise<nav_msgs::Path>("path", 1000);// publishes to path
	ros::Subscriber sub = n.subscribe("map", 1000, publishInfo);// subscribes to map
	tf::TransformListener listener; //tf listener
	plr = &listener;
	ros::spin();
	return 0;
}