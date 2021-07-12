#include "Granade.h"

const double PI = 3.14;


Granade::Granade()
{
}

Granade::Granade(double x, double y)
{
	int i;
	double alpha, delta = 2*PI/NUM_BULLETS;
	this->x = x;
	this->y = y;
	for (i = 0,alpha = 0; i < NUM_BULLETS; i++,alpha+=delta)
	{
		bullets[i] = new Bullet(x, y);
		bullets[i]->SetDir(alpha);
	}

}

Granade::Granade(double x, double y,double angle)
{
	int i;
	double alpha, delta = 2 * PI / NUM_BULLETS;
	this->x = x;
	this->y = y;
	for (i = 0, alpha = angle; i < NUM_BULLETS; i++, alpha += delta)
	{
		bullets[i] = new Bullet(x, y);
		bullets[i]->SetDir(alpha);
	}
}


Granade::~Granade()
{
}

void Granade::explode()
{
	for (int i = 0; i < NUM_BULLETS; i++)
		bullets[i]->SetIsMoving(true);
}

void Granade::showMe()
{
	for (int i = 0; i < NUM_BULLETS; i++)
		bullets[i]->showMe();
}

void Granade::moveBullets(Node maze[MSZ][MSZ])
{
	for (int i = 0; i < NUM_BULLETS; i++)
		bullets[i]->move(maze);
}

void Granade::SimulateExplosion(double map[MSZ][MSZ], Node maze[MSZ][MSZ])
{
	for (int i = 0; i < NUM_BULLETS; i++)
	{
		bullets[i]->SetIsMoving(true);
		bullets[i]->SimulateMotion(map, maze);
	}

}
void Granade::moveToTarget(Node maze[MSZ][MSZ], int myValue)
{
	for (int i = 0; i < NUM_BULLETS; i++) 
	{
		bullets[i]->moveToTarget(maze, myValue);
	}
	this->showMe();
}

bool Granade::isMoving()
{
	int numBulletsThatStop = 0;
	for (int i = 0; i < NUM_BULLETS; i++)
	{
		if (!bullets[i]->GetIsMoving())
			numBulletsThatStop++;
	}
	return numBulletsThatStop < NUM_BULLETS;
}

Bullet* Granade:: getBulletByIndex(int i)
{
	return bullets[i];
}