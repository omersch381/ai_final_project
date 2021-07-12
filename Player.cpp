#include "Player.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

Player::Player(int index)
{
	life = FULL_LIFE;
	num_of_bullets = FULL_BULLETS;
	num_of_granades = FULL_GRANADES;
	behavior = rand()%2; // 0 - defence , 1- attack
	state = -1;
	this->index = index;
	secondState = FIGHT;
}

Player::Player()
{
	life = FULL_LIFE;
	num_of_bullets = FULL_BULLETS;
	num_of_granades = FULL_GRANADES;
	behavior = rand()%2; // 0 - defence , 1- attack
	state = (behavior == 0) ? ESCAPE : FIGHT;
	this->index = 0;
	secondState = FIGHT;
}

Player::~Player()
{
}

void Player::setIndex(int index)
{
	this->index = index;
}

int Player::getIndex()
{
	return this->index;
}

void Player::setSecondState(int secondState)
{

	this->secondState = secondState;
}


int Player::getSecondState()
{
	return this->secondState;
}

void Player::setState(int state)
{
	if (this->state != state) {
		this->state = state;
		this->printStatusPlayer();
	}
	this->state = state;
	
}



int Player::getState()
{
	return this->state;
}

int Player::getLife() {
	return this->life;
}
void Player::setLife(int life) {
	this->life = life;
}
void Player::add_life()
{
	life += LIFE_SCORES;
	if (life > 100)
		life = 100;
}

void Player::hit_life(double dist, int MSZ) 
{
	double hitPercent = 1 - (dist / MSZ);
	this->life = this->life - (LIFE_HIT * hitPercent);
}

void Player::reduceBullet()
{
	this->num_of_bullets = this->num_of_bullets- 1;
}

void Player::reduceGranade()
{
	this->num_of_granades = this->num_of_granades - 1;
}

int Player::get_num_of_bullets()
{
	return this->num_of_bullets;
}

void Player::add_num_of_bullets()
{
	
	this->num_of_bullets += ADD_BULLETS;
	if (this->num_of_bullets > FULL_BULLETS)
		this->num_of_bullets = FULL_BULLETS;
}

int Player::get_num_of_granade()
{
	return this->num_of_granades;
}


void Player::add_num_of_granades()
{
	this->num_of_granades += ADD_GRANADES;
	if (this->num_of_granades > FULL_GRANADES)
		this->num_of_granades = FULL_GRANADES;
}

void Player::setNode(Node * node)
{
	this->node = node;
}

Node * Player::getNode()
{
	return node;
}

int Player::getBehavior()
{
	return behavior;
}

void Player::decisionTree()
{
	int numOfWeapon = get_num_of_bullets() + get_num_of_granade();
	
	if (getBehavior() == 0) // defense behavior
	{
		if (life < 35)
			setState(ESCAPE);
		else if (life < 60)
			setState(FILLING_HEALTH);
		else if (numOfWeapon < (FULL_BULLETS + FULL_GRANADES) / 2)
			setState(FILLING_WEAPON);
		else 
			setState(FIGHT);
	}
	else // attack behave
	{
		if (life < 25)
			setState(ESCAPE);
		else if (life < 45)
			setState(FILLING_HEALTH);
		else if (numOfWeapon <=  FULL_GRANADES)
			setState(FILLING_WEAPON);
		else
			setState(FIGHT);
	}
}

void Player::printStatusPlayer()
{
	char* state1 = "";
	switch (this->getState())
	{
	case 0:
		state1 = "fight";
		break;
	case 1:
		state1 = "escape";
		break;
	case 2:
		state1 = "filling health";
		break;
	case 3:
		state1 = "filling weapon";
		break;
	}

	char* state2 = "";
	switch (this->getSecondState())
	{
	case 0:
		state2 = "fight";
		break;
	case 1:
		state2 = "escape";
		break;
	case 2:
		state2 = "filling health";
		break;
	case 3:
		state2 = "filling weapon";
		break;
	}

	char* behaveior = "";
	switch (this->getBehavior())
	{
	case 0:
		behaveior = "defence";
		break;
	case 1:
		behaveior = "attack";
		break;
	}

	printf("\n\n-----\nplayer status:");
	printf("\nplayer number %d, from team %d\n", this->getIndex(), this->getNode()->GetValue());
	printf("behavior = %s, state = %s, second_state=%s \nlife = %d, bullets = %d, granades =%d",
		behaveior, state1, state2, this->getLife(), this->get_num_of_bullets(),
		this->get_num_of_granade());
	printf("\n-----");
}
