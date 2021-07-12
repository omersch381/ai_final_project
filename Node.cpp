#include "Node.h"
#include <math.h>

Node::Node()
{
	value = SPACE;
	parent = nullptr;
	g = 0;
}

Node::~Node()
{
}

Node::Node(Point2D &pt, Point2D* t, int v, double g, Node* pr) 
{
	point = pt;
	target = t;
	value = v;
	parent = pr;
	this->g = g;
	this->h = ComputeH();
	this->safeMap = 0;
}

void Node::SetValue( int value)
{
	this->value = value;
}

int Node::GetValue()
{
	return value;
}



double Node::ComputeH()
{
	return sqrt(pow(point.getRow()-target->getRow(),2)+
		pow(point.getCol() - target->getCol(), 2));
}

double Node::getF()
{
	return g+h;
}

Point2D Node::getPoint()
{
	return point;
}
void Node::SetParent(Node* p)
{
	parent = p;
}

Node * Node::getParent()
{
	return parent;
}

Point2D * Node::getTarget()
{
	return target;
}

void Node::SetLine(int line)
{
	this->line = line;
}


void Node::SetColumn(int col)
{
	this->column = col;
}

int Node::GetLine()
{
	return this->line;
}


int Node::GetColumn()
{
	return this->column;
}

double Node::getG()
{
	return g;
}


void Node::setG(double g) {
	this->g = g;
}

double Node::getH() 
{
	return h;
}

void Node::setH(double dist)
{
	this->h = dist;
}

double Node::getSafeMap()
{
	return safeMap;
}

void Node::setSafeMap(double safeMap)
{
	this->safeMap = safeMap*25; //*25 because increase safeMap values
}
