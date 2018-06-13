#ifndef TRAJECTCONST
#define TRAJECTCONST
#include <vector>
#include "geometry_msgs/PoseStamped.h"
/**
  The constants for the trajectory planner
  Author: Thomas Vy(thomas.vy@ucalgary.ca)
  Date: June 12, 2018
**/

//prototypes for classes and namespace
using namespace std;
class Image;
class Position;
struct Wall;
struct Pose;

//Constants
const int PRECISION = 100; //The number of dots for the bspline curve
const int UNKNOWN =-2; //The unknown space value
const int WALL=-1; //The wall space value
const int EMPTY_SPACE =1; //The empty space value
const int RESOLUTION = 10; //The distance from wall to calculate(for cost)
const float LENGTH = 2; //The distance between points(higher number = fast computation but inaccurate path)
const float CURVATURE = 0.2; //The curvature the robot takes

//The typedefs
typedef vector< vector<double> > matrix; //Holds the map cells
typedef vector<geometry_msgs::PoseStamped> pathMessage;//The path message
typedef vector<Position> listOfPositions; //list of positions
typedef priority_queue<Position,vector<Position>, std::greater<Position> > positionPriorityQueue; //the priority queue
typedef vector<unique_ptr<Position> > vectorOfPointers; //vector of Pointer
typedef list<Position> positionLinkedList; //Linked list for position
typedef vector<Pose> poseVector; //vector of poses
typedef vector<double> doubleVector; //vector of doubles

//Wall class for wall pixels
typedef struct Wall{
	int x; //The x positon of the wall
	int y; //The y postion of the wall
	std::vector<bool> direction; // the free space direction
	Wall(int x, int y, vector<bool> direction):x(x), y(y), direction(direction){}; //constructor
}Wall;

//pose to hold the x, y, and angle of the postition
typedef struct Pose{
	double x; //The x postion
	double y; // y position
	double radian; // angle of the robot
	Pose():x(0),y(0),radian(0){}; //Constructor
	Pose(double x, double y, double radian):x(x),y(y),radian(radian) {}; //Constructor
	Pose(double x, double y):x(x),y(y),radian(0){}; //Constructor
	Pose(const Pose & rhs); //Copy Constructor
	Pose& operator=(const Pose & rhs); //Assignment operator
	Pose endPose(float curvature, float length); //Calculating end position for a path

}Pose;
//The position class for points on the map
class Position{
	private:
		float cost; //The cost of that position
		float total_cost; //The total cost of moving to that position
		Pose pose; //The pose of that spot
	public:
		Position * prePosition; //The previous spot of this position
		Position(Pose & pose, float total_cost, float cost, Position * prePosition);//constructor
		Position(Pose & pose, float cost, Position * prePosition);//constructor
		Position(const Position & rhs);//copy constructor
		Position():cost(0),total_cost(0){} //default constructor
		Position& operator=(const Position & rhs);//assignment operator
		bool operator>(Position const& right) const {return total_cost > right.total_cost;}// comparing for the priority queue
		listOfPositions getNeighbours (matrix & walls, Pose & goal);// gets the neighbours of this position
		bool checkNeighbour (Pose & current, Pose & next, matrix & walls);// checks the neighbour if it is a valid spot(doesn't cross through a wall)
		Pose & getPoint(){return pose;} //returns the pose of the position
};

//Hold the image grids and do calculations for them
class Image{
	private:
		pathMessage path;// the message to be broadcasted
		matrix convertedImage;//the Wall, Unknown, and Empty space located
		matrix arena;// the cost grid
		void dilation(Wall & wall);//turns all the empty space cells next to a wall into a certain cost
	public:
		Image(const matrix & oriImage);//constructor. Turns the original image into the converted Image
		void insert_borders ();//creates the arena by locating all wall cells
		vector<bool> checkSpace (int i, int j);//checks spaces around a wall to see which direction is open space
		bool planner (Pose & start, Pose & goal);//plans the path for the map
		void Bezier (poseVector & points, int num =PRECISION);// creates the bspline curve
		void multipleVectors(doubleVector & berst, Pose point);//multiples two Vectors together
		doubleVector Berstein(doubleVector & arr, int n, int k);//Does some math
		double binomialCoeff(int n, int k);//calculates binomial coefficient
		const pathMessage & getPath ();//gets the path message
};
#endif