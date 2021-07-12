#pragma once
#include "Bullet.h"
#include "Node.h"
const int NUM_BULLETS = 8;
const int DISTANCE_GRANADE = 10;
const double GRANADE_DISTANCE_FROM_BOOM =1.5;

class Granade
{
public:
	Granade();
	Granade(double x, double y);
	Granade(double x, double y, double angle);
	~Granade();
	void explode();
	void showMe();
	void moveBullets(Node maze[MSZ][MSZ]);
	void SimulateExplosion(double map[MSZ][MSZ], Node maze[MSZ][MSZ]);
	void moveToTarget(Node maze[MSZ][MSZ], int myValue);
	bool isMoving();
	Bullet* getBulletByIndex(int i);
	
private:
	double x, y;
	Bullet* bullets[NUM_BULLETS];
};

