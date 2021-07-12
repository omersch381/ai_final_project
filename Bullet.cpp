#include "Bullet.h"
#include "GLUT.H"
#include <math.h>
#include <stdio.h>

const double PI = 3.14;

Bullet::Bullet()
{
}

Bullet::Bullet(double x, double y)
{
	double len;
	this->x = x;
	this->y = y;
	dirx = (rand() % 101)-50;
	diry = (rand() % 101)-50;
	len = sqrt(dirx*dirx + diry * diry);
	dirx /= len;
	diry /= len;
	isMoving = false;
}


Bullet::~Bullet()
{
}

void Bullet::setAngle(double angle)
{
	this->angle = angle;
}
double Bullet::getAngle() 
{
	return this->angle;
}

void Bullet::setX(double x)
{
	this->x = x;
}

void Bullet::setY(double y)
{
	this->y = y;
}

void Bullet::showMe()
{
	glColor3d(0, 0, 0);
	glBegin(GL_POLYGON);
	glVertex2d(x - 0.01, y);
	glVertex2d(x , y+ 0.01);
	glVertex2d(x + 0.01, y);
	glVertex2d(x, y - 0.01);
	glEnd();
}

void Bullet::SetIsMoving(bool move)
{
	isMoving = move;
}

bool Bullet::GetIsMoving()
{
	return isMoving;
}

void Bullet::move(Node maze[MSZ][MSZ])
{
	int i, j;
	i = MSZ * (y + 1) / 2;
	j = MSZ * (x + 1) / 2;

	if (isMoving && maze[i][j].GetValue()==SPACE)
	{
		x += 0.001*dirx;
		y += 0.001*diry;
	}
}

void Bullet::moveToTarget(Node maze[MSZ][MSZ], int myValue)
{
	int i, j;
	//0.00001 for save Accuracy of i and j 
	i = MSZ * (y + 1) / 2 + 0.00001;
	j = MSZ * (x + 1) / 2 + 0.00001;

	if (maze[i][j].GetValue() == SPACE || maze[i][j].GetValue() == myValue||
		maze[i][j].GetValue() == MEDICINE ||maze[i][j].GetValue() == BULLETS || maze[i][j].GetValue() == GRANADES)
	{
		x = 0.003 * dirx + x;
		y = 0.003 * diry + y;
	}
	else {
		SetIsMoving(false);
	}
}

double Bullet::getX()
{
	return x;
}

double Bullet::getY()
{
	return y;
}

void Bullet::SetDir(double angle)
{
	this->dirx = cos(angle);
	this->diry = sin(angle);
}

void Bullet::SimulateMotion(double map[MSZ][MSZ], Node maze[MSZ][MSZ])
{
	int i, j;
	i = MSZ * (y + 1) / 2;
	j = MSZ * (x + 1) / 2;

	while(maze[i][j].GetValue() == SPACE)
	{
		map[i][j] += DELTA;
		x += 0.001*dirx;
		y += 0.001*diry;
		i = MSZ * (y + 1) / 2;
		j = MSZ * (x + 1) / 2;
	}
}

bool Bullet::arrive(Node maze[MSZ][MSZ],int target,int myvalue)
{
	int i, j;
	i = MSZ * (y + 1) / 2 + 0.00001;
	j = MSZ * (x + 1) / 2 + 0.00001;
	while (maze[i][j].GetValue() == SPACE|| maze[i][j].GetValue()==myvalue|| 
		maze[i][j].GetValue()==MEDICINE|| maze[i][j].GetValue()==BULLETS|| maze[i][j].GetValue()== GRANADES)
	{
		x = 0.003 * dirx+x;
		y = 0.003 * diry+y;
		//0.00001 for save Accuracy of i and j 
		i = MSZ * (y + 1) / 2 +0.00001;
		j = MSZ * (x + 1) / 2 +0.00001;
	}
	return maze[i][j].GetValue() == target;
}

