#include "Node.h"
#pragma once

const int FIGHT = 0;
const int ESCAPE = 1;
const int FILLING_HEALTH = 2;
const int FILLING_WEAPON = 3;
const int SQUIRE_INT = 2;

const int LIFE_SCORES = 25;
const int LIFE_HIT = 7;
const int ADD_BULLETS = 5;
const int ADD_GRANADES = 5;
const int FULL_LIFE = 100;
const int FULL_BULLETS = 20;
const int FULL_GRANADES = 5;

class Player
{
	
public:
	Player(int index);
	Player();
	~Player();

	void setState(int state);
	int getState();
	void add_life();
	void hit_life(double dist, int MSZ);
	void heal_life(double dist, int MSZ);
	int get_num_of_bullets();
	void add_num_of_bullets();
	int get_num_of_granade();
	void add_num_of_granades();
	void setNode(Node* node);
	Node* getNode();
	int getBehavior();
	void decisionTree();
	int getLife();
	void setLife(int life);
	void reduceBullet();
	int getIndex();
	void setIndex(int index);
	void printStatusPlayer();
	void reduceGranade();
	int getSecondState();
	void setSecondState(int secondState);

private:
	int state,secondState; 	//secondState is for fight case.
	int life;
	int num_of_bullets;
	int num_of_granades;
	Node* node;
	int behavior;
	int index;
	int isSquire;
};

