#pragma once
#include "Node.h"
const int MSZ = 100;


class Bullet
{
public:
	Bullet();
	Bullet(double x, double y);
	~Bullet();
	void showMe();
	void SetIsMoving(bool move);
	bool GetIsMoving();
	void move(Node maze[MSZ][MSZ]);
	double getX();
	double getY();
	void SetDir(double angle);
	void SimulateMotion(double map[MSZ][MSZ], Node maze[MSZ][MSZ]);
	bool arrive(Node maze[MSZ][MSZ], int target, int myvalue);
	void setX(double x);
	void setY(double y);
	void setAngle(double angle);
	double getAngle();
	void moveToTarget(Node maze[MSZ][MSZ], int myValue);

private:
	double x, y;
	double dirx, diry;
	double angle;
	bool isMoving;
};

