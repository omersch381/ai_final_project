#include "GLUT.H"
#include <math.h>
#include <time.h>
#include "Node.h"
#include "Room.h"
#include <vector>
#include <iostream>
#include <queue>
#include <algorithm>
#include "CompareNodes.h"
#include "Bullet.h"
#include "Granade.h"
#include "Player.h"
#include "CompratorAstar.h"

using namespace std;
const int W = 800; // window width
const int H = 800; // window height
const int NUMOFEQUIPMENT = 5;

const int NUM_ROOMS = 12;
const int NUM_PLAYRES_IN_GROUP = 3;

bool run_Astar = false;

Node maze[MSZ][MSZ];
double map[MSZ][MSZ] = { 0 };
Room rooms[NUM_ROOMS];
int numExistingRooms = 0;

vector<Player> group1, group2;
vector <Node> medicines, bullets, granades;

double security_map [MSZ][MSZ] = { 0 };
Bullet* pb = nullptr;
Granade* pg = nullptr;

void killPlayer(vector<Player> &players, int index);
void fireBullet(Bullet* bullet, Player* enemy, int myValue);
void InitMaze();
void InitRooms();
void DigTunnels();
void createEquipment(vector <Node> &equipment,int value);
void createGroup(vector<Player> &group, const int color, int numPlayersInGroups);
void RestorePath(Node* first, int startValue, Player* player, bool isAstarBfs);
Node* BfsIteration(int start, int target, Player* player, bool isAstarBfs);
void resetAllGroups();
bool canFire(Player* p);
double distance(int x1, int y1, int y2, int x2);
bool arriveToEnemy(Bullet* bullet, Player* enemy, int column, int line, Player* me);
void DrawMaze();
void chooseTarget(vector<Player>& group, int myValue, int enemyValue);
void startGame();
void Astar(Player* startPlayer, Node* target);
void GenerateMap();
bool choosegGranade(Player* p, Player* enemy);
void fireGranade(Bullet* bullet, vector<Player>& enemyGroup, int indexEnemy, int myValue);
void AStarToWeapon(Player* me);
int findMyRoom(Node* nodePlayer);

void init()
{
	int r, c;
	srand(time(0)); // pseudo randomizer
	glClearColor(0.7, 0.7, 0.7, 0);

	glOrtho(-1, 1, -1, 1, -1, 1);

	InitMaze();
	InitRooms();

	for (int i = 0; i < 100; i++)
	{
		r = rand() % MSZ;
		c = rand() % MSZ;
		maze[r][c].SetValue(WALL);
	}

	DigTunnels();

	// Creating 2 groups and items
	createGroup(group1, GROUP1, NUM_PLAYRES_IN_GROUP);
	createGroup(group2, GROUP2, NUM_PLAYRES_IN_GROUP);
	createEquipment(bullets, BULLETS); // ORANGE COLOR
	createEquipment(granades, GRANADES); // PURPLE COLOR
	createEquipment(medicines, MEDICINE); // YELLOW COLOR
}

Room GenerateRoom()
{
	int w, h, ci, cj;
	Room* pr = nullptr;
	bool isOveralaping;

	do
	{
		delete pr;
		isOveralaping = false;
		w = 6 + rand() % 10;
		h = 6 + rand() % 10;

		ci = h / 2 + rand() % (MSZ - h);
		cj = w / 2 + rand() % (MSZ - w);

		pr = new Room(ci, cj, w, h);

		for (int i = 0; i < numExistingRooms && !isOveralaping; i++)
			if (rooms[i].CheckOverlapping(pr))
				isOveralaping = true;

	} while (isOveralaping);

	for (int i = pr->getLeftTop().getRow(); i <= pr->getRightBottom().getRow(); i++)
		for (int j = pr->getLeftTop().getCol(); j <= pr->getRightBottom().getCol(); j++)
			maze[i][j].SetValue(SPACE);

	return *pr;
}

// check if the node at row,col is white or gray that is better then the previous one
// and if so add it to pq
void AddNode(int row, int col, Node* pn, vector<Node> &gray, vector<Node> &black,
	priority_queue <Node*, vector<Node*>, CompareNodes> &pq)
{
	Point2D pt;
	Node* pn1;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
	double cost;

	pt.setRow(row);
	pt.setCol(col);
	if (maze[row][col].GetValue() == SPACE)
		cost = 0.1; // space cost
	else if (maze[row][col].GetValue() == WALL)
		cost = 3;
	// cost depends on is it a wall or a space
	pn1 = new Node(pt, pn->getTarget(), maze[pt.getRow()][pt.getCol()].GetValue(), pn->getG() + cost, pn);

	black_it = find(black.begin(), black.end(), *pn1);
	gray_it = find(gray.begin(), gray.end(), *pn1);
	if (black_it == black.end() && gray_it == gray.end()) // it is not black and not gray!
	{// i.e. it is white
		pq.push(pn1);
		gray.push_back(*pn1);
	}
}

void AddNeighbours(Node* pn, vector<Node>& gray, vector<Node>& black,
	priority_queue <Node*, vector<Node*>, CompareNodes>& pq)
{
	// try down
	if (pn->getPoint().getRow() < MSZ - 1)
		AddNode(pn->getPoint().getRow() + 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try up
	if (pn->getPoint().getRow() > 0)
		AddNode(pn->getPoint().getRow() - 1, pn->getPoint().getCol(), pn, gray, black, pq);
	// try left
	if (pn->getPoint().getCol() > 0)
		AddNode(pn->getPoint().getRow(), pn->getPoint().getCol() - 1, pn, gray, black, pq);
	// try right
	if (pn->getPoint().getCol() < MSZ - 1)
		AddNode(pn->getPoint().getRow(), pn->getPoint().getCol() + 1, pn, gray, black, pq);
}

void resetAllEquipments(Node* next, Player* player,bool isAstarBfs) {
	int i;
	for (i = 0; i < medicines.size(); i++)
	{
		if (&maze[next->GetLine()][next->GetColumn()] != &maze[medicines[i].GetLine()][medicines[i].GetColumn()]|| isAstarBfs)
			maze[medicines[i].GetLine()][medicines[i].GetColumn()].SetValue(MEDICINE);
		else
		{
			maze[medicines[i].GetLine()][medicines[i].GetColumn()].SetValue(SPACE);
			medicines.erase(medicines.begin() + i);
			player->add_life();
		}
	}
	for (i = 0; i < granades.size(); i++) {
		if (&maze[next->GetLine()][next->GetColumn()] != &maze[granades[i].GetLine()][granades[i].GetColumn()] || isAstarBfs)
			maze[granades[i].GetLine()][granades[i].GetColumn()].SetValue(GRANADES);
		else
		{
			maze[granades[i].GetLine()][granades[i].GetColumn()].SetValue(SPACE);
			granades.erase(granades.begin() + i);
			player->add_num_of_granades();
		}
	}
	for (i = 0; i < bullets.size(); i++) {
		if (&maze[next->GetLine()][next->GetColumn()] != &maze[bullets[i].GetLine()][bullets[i].GetColumn()] || isAstarBfs)
			maze[bullets[i].GetLine()][bullets[i].GetColumn()].SetValue(BULLETS);
		else
		{
			maze[bullets[i].GetLine()][bullets[i].GetColumn()].SetValue(SPACE);
			bullets.erase(bullets.begin() + i);
			player->add_num_of_bullets();
		}
	}
}

void resetValues(vector<Node*> grayNode)
{
	Node* node;
	while (!grayNode.empty()) {
		node = grayNode.front();
		grayNode.erase(grayNode.begin());
		node->SetValue(SPACE);
	}
}

void resetAllGroups()
{
	int i;
	for (i = 0; i < group1.size(); i++)
		maze[group1[i].getNode()->GetLine()][group1[i].getNode()->GetColumn()].SetValue(GROUP1);

	for (i = 0; i < group2.size(); i++)
		maze[group2[i].getNode()->GetLine()][group2[i].getNode()->GetColumn()].SetValue(GROUP2);

}

void RestorePath(Node* first, int startValue, Player* player,bool isAstarBfs)
{
	Node* current = first;
	Node* next = first;
	while (current->GetValue() != startValue)
	{
		next = current;
		current = current->getParent();
	}

	//reset all positions:
	resetAllGroups();
	resetAllEquipments(next, player, isAstarBfs);
	//if not in same team so replace position(move player) and is not astar bfs
	if (maze[next->GetLine()][next->GetColumn()].GetValue() != startValue && !isAstarBfs)
	{
		maze[current->GetLine()][current->GetColumn()].SetValue(SPACE);
		maze[next->GetLine()][next->GetColumn()].SetValue(startValue);
		player->setNode(&(maze[next->GetLine()][next->GetColumn()]));
	}
}

Node* BfsIteration(int start, int target, Player* player, bool isAstarBfs) 
{
	/*
	General BFS iteration for any path finding via BFS.
	*/

	Node* current,*nodeTarget;
	vector<Node*> queueBfs, grayNode;
	queueBfs.push_back(player->getNode());
	bool run_bfs = true;
	nodeTarget = player->getNode();
	while (run_bfs == true) 
	{
		if (queueBfs.empty()) // there is no way to target
		{
			resetValues(grayNode);
			run_bfs = false;
		}
		else
		{
			current = queueBfs.front();
			queueBfs.erase(queueBfs.begin());
			// check down 
			if (maze[current->GetLine()][current->GetColumn()].GetValue() != start)
				maze[current->GetLine()][current->GetColumn()].SetValue(BLACK); // visited
			if (maze[current->GetLine() - 1][current->GetColumn()].GetValue() == target)// found
			{	
				resetValues(grayNode);
				nodeTarget = &maze[current->GetLine() - 1][current->GetColumn()];
				RestorePath(current, start,player, isAstarBfs);
				run_bfs = false;
				nodeTarget->SetParent(current); //this line is for "restore path" in fight mode
			}
			//if is equipment and in the same team but not me
			else if ((maze[current->GetLine() - 1][current->GetColumn()].GetValue() == SPACE ||
				maze[current->GetLine() - 1][current->GetColumn()].GetValue() == MEDICINE ||
				maze[current->GetLine() - 1][current->GetColumn()].GetValue() == BULLETS ||
				maze[current->GetLine() - 1][current->GetColumn()].GetValue() == GRANADES||
				(maze[current->GetLine() - 1][current->GetColumn()].GetValue() == start && !isAstarBfs) )&& 
				(&maze[current->GetLine() - 1][current->GetColumn()] !=player->getNode()))
			{
				maze[current->GetLine() - 1][current->GetColumn()].SetValue(GRAY); // paint it gray
				maze[current->GetLine() - 1][current->GetColumn()].SetParent(current);
				grayNode.push_back(&(maze[current->GetLine() - 1][current->GetColumn()]));
				queueBfs.push_back(&(maze[current->GetLine() - 1][current->GetColumn()]));
			}
			if (run_bfs) // the target wasn't found yet
			{
				// check UP
				if (maze[current->GetLine() + 1][current->GetColumn()].GetValue() == target) // found
				{
					resetValues(grayNode);
					RestorePath(current, start, player, isAstarBfs);
					run_bfs = false;
					nodeTarget = &maze[current->GetLine() + 1][current->GetColumn()];
					nodeTarget->SetParent(current); //this line is for "restore path" in fight mode
				}
				else if ((maze[current->GetLine() + 1][current->GetColumn()].GetValue() == SPACE ||
					maze[current->GetLine() + 1][current->GetColumn()].GetValue() == MEDICINE ||
					maze[current->GetLine() + 1][current->GetColumn()].GetValue() == BULLETS ||
					maze[current->GetLine() + 1][current->GetColumn()].GetValue() == GRANADES||
					(maze[current->GetLine() + 1][current->GetColumn()].GetValue() == start && !isAstarBfs) )
					&& (&maze[current->GetLine() + 1][current->GetColumn()] != player->getNode()))
				{
					maze[current->GetLine() + 1][current->GetColumn()].SetValue(GRAY); // paint it gray
					maze[current->GetLine() + 1][current->GetColumn()].SetParent(current);
					grayNode.push_back(&(maze[current->GetLine() + 1][current->GetColumn()]));
					queueBfs.push_back(&(maze[current->GetLine() + 1][current->GetColumn()]));
				}
			}
			if (run_bfs) // the target wasn't found yet
			{
				// check LEFT
				if (maze[current->GetLine()][current->GetColumn() - 1].GetValue() == target) // found
				{
					resetValues(grayNode);
					RestorePath(current, start, player, isAstarBfs);
					run_bfs = false;
					nodeTarget = &maze[current->GetLine()][current->GetColumn() - 1];
					nodeTarget->SetParent(current); //this line is for "restore path" in fight mode
				}
				else if ((maze[current->GetLine()][current->GetColumn() - 1].GetValue() == SPACE ||
					maze[current->GetLine()][current->GetColumn() - 1].GetValue() == MEDICINE ||
					maze[current->GetLine()][current->GetColumn() - 1].GetValue() == BULLETS ||
					maze[current->GetLine()][current->GetColumn() - 1].GetValue() == GRANADES||
					(maze[current->GetLine()][current->GetColumn() - 1].GetValue() == start && !isAstarBfs))
					&& (&maze[current->GetLine()][current->GetColumn() - 1] != player->getNode()))
				{
					maze[current->GetLine()][current->GetColumn() - 1].SetValue(GRAY); // paint it gray
					maze[current->GetLine()][current->GetColumn() - 1].SetParent(current);
					grayNode.push_back(&(maze[current->GetLine()][current->GetColumn() - 1]));
					queueBfs.push_back(&(maze[current->GetLine()][current->GetColumn() - 1]));
				}
			}
			if (run_bfs) // the target wasn't found yet
			{
				// check RIGHT
				if (maze[current->GetLine()][current->GetColumn() + 1].GetValue() == target) // found
				{
					resetValues(grayNode);
					RestorePath(current, start, player, isAstarBfs);
					run_bfs = false;
					nodeTarget = &maze[current->GetLine()][current->GetColumn() + 1];
					nodeTarget->SetParent(current); //this line is for "restore path" in fight mode
				}
				else if ((maze[current->GetLine()][current->GetColumn() + 1].GetValue() == SPACE ||
					maze[current->GetLine()][current->GetColumn() + 1].GetValue() == MEDICINE ||
					maze[current->GetLine()][current->GetColumn() + 1].GetValue() == BULLETS ||
					maze[current->GetLine()][current->GetColumn() + 1].GetValue() == GRANADES||
					(maze[current->GetLine()][current->GetColumn() + 1].GetValue() == start && !isAstarBfs))
					&& (&maze[current->GetLine()][current->GetColumn() + 1] !=player->getNode()))
				{
					maze[current->GetLine()][current->GetColumn() + 1].SetValue(GRAY); // paint it gray
					maze[current->GetLine()][current->GetColumn() + 1].SetParent(current);
					grayNode.push_back(&(maze[current->GetLine()][current->GetColumn() + 1]));
					queueBfs.push_back(&(maze[current->GetLine()][current->GetColumn() + 1]));
				}
			}
		}
	}
	return nodeTarget;
}

void bestFirstSearchForEscapeState(Player* player, Node *enemyPlayer)
{
	/*
	This method finds an escape node, using BFS.
	
	Every time there is a need for escaping in the game, this method gets it.
	*/

	Node* current, * bestEscapeTarget;
	vector<Node*> queueBfs, grayNode;
	bool run_bfs = true;
	int i, lineEnemy, columnEnemy, line, column;
	double dist;
	int startValue = player->getNode()->GetValue();
	bestEscapeTarget = player->getNode();
	queueBfs.push_back(player->getNode());

	lineEnemy = enemyPlayer->GetLine();
	columnEnemy = enemyPlayer->GetColumn();

	//set G and safeMap values to start:
	bestEscapeTarget->setG(0);
	bestEscapeTarget->setSafeMap(map[bestEscapeTarget->GetLine()][bestEscapeTarget->GetColumn()]);
	bestEscapeTarget->setH(distance(lineEnemy,columnEnemy, bestEscapeTarget->GetLine(), bestEscapeTarget->GetColumn()));
	
	//start algorithm
	while (run_bfs)
	{
		if (queueBfs.empty()) // there is no way to target
		{
			resetValues(grayNode);
			run_bfs = false;
		}
		else
		{
			current = queueBfs.front();
			queueBfs.erase(queueBfs.begin());
			
			//enemy distance
			// compare with G and safeMap Values
			if (-current->getSafeMap() + current->getH() >
				-bestEscapeTarget->getSafeMap() + bestEscapeTarget->getH())
			{
				bestEscapeTarget = current;
			}

			if (maze[current->GetLine()][current->GetColumn()].GetValue() != startValue)
				maze[current->GetLine()][current->GetColumn()].SetValue(BLACK); // visited
			
			// check neighbors.
			for (i = 0; i < 4; i++) 
			{
				line = current->GetLine() + (i - 1) % 2;
				column = current->GetColumn() + (i - 2) % 2;
				
				if (maze[line][column].GetValue() != WALL && maze[line][column].GetValue() != enemyPlayer->GetValue()
					&& maze[line][column].GetValue() != player->getNode()->GetValue() && maze[line][column].GetValue() != GRAY 
					&& maze[line][column].GetValue() != BLACK)
				{
					maze[line][column].SetValue(GRAY); // paint it gray
					maze[line][column].SetParent(current);
					maze[line][column].setG(current->getG() + 1);
					maze[line][column].setSafeMap(map[line][column]);
					maze[line][column].setH(distance(lineEnemy, columnEnemy, line, column));
					grayNode.push_back(&(maze[line][column]));
					queueBfs.push_back(&(maze[line][column]));
				}
			}
		}
	}
	resetValues(grayNode);
	RestorePath(bestEscapeTarget, startValue, player, false);
}

void restoreQueue(queue<Node*> queue) 
{
	Node* node;
	while (!queue.empty()) 
	{
		node = queue.front();
		queue.pop();
		maze[node->GetLine()][node->GetColumn()].SetValue(SPACE);
	}
}

void Astar(Player* startPlayer, Node* target)
{
	bool findTarget = false;
	Node* current, *next, *start;
	int i, j, enemyValue;
	start = startPlayer->getNode();
	queue<Node*> nodeBlack, nodeGray;
	priority_queue <Node*, vector<Node*>, CompratorAstar> pq;
	pq.push(start);
	enemyValue = start->GetValue() == GROUP1 ? GROUP1 : GROUP2;

	start->setG(0);
	while (!findTarget) //There is not path to target
	{
		if (pq.empty())
		{
			restoreQueue(nodeBlack);
			restoreQueue(nodeGray);
			return;
		}
		current = pq.top();
		pq.pop();
		if (current == target)
		{
			restoreQueue(nodeBlack);
			restoreQueue(nodeGray);
			//restore value and move one step to target
			RestorePath(current,start->GetValue(), startPlayer,false);
			//reset the pq for next players
			return;
		}
		// insert all node except me to black queue
		if (current != start)
		{
			current->SetValue(BLACK);
			nodeBlack.push(current);
		}
		//check all neighbors: 
		for (i = -1;i < 2;i = i + 2) {
			next = &maze[current->GetLine() - i][current->GetColumn()];
			//no target/wall/visited/queue so add to queue
			// visit black and gray is node that queue contain 
			if (next->GetValue() != WALL && next->GetValue() != start->GetValue() && next->GetValue() != BLACK && next->GetValue() != GRAY&& next->GetValue()!=enemyValue) 
			{
				//add to gray and pq; Update H and G values.
				nodeGray.push(next);
				next->SetValue(GRAY);
				next->setH(distance(next->GetLine(),next->GetColumn(), target->GetLine(),target->GetColumn()));
				next->setG(1 + current->getG());
				next->setSafeMap(map[next->GetLine()][next->GetColumn()]);
				next->SetParent(current);
				pq.push(next);
			}
		}
		for (j = -1;j < 2;j = j + 2) {
			next = &maze[current->GetLine()][current->GetColumn() - j];
			if (next->GetValue() != WALL && next->GetValue() != start->GetValue() && next->GetValue() != BLACK && next->GetValue()!= GRAY && next->GetValue() != enemyValue)
			{
				//add to gray and pq; Update H and G values.
				nodeGray.push(next);
				next->SetValue(GRAY);
				next->setH(distance(next->GetLine(), next->GetColumn(), target->GetLine(), target->GetColumn()));
				next->setG(1 + current->getG());
				next->setSafeMap(map[next->GetLine()][next->GetColumn()]);
				next->SetParent(current);
				pq.push(next);
			}
		}
	}
}

void killPlayer(vector<Player>& group, int index)
{
	Player p = group[index];
	if (index == SQUIRE_INT)
	{
		group.clear();
	}else
	{
		if (group.size() > 0)
		{
			p.printStatusPlayer();
			printf("\nplayer Dead");
			maze[p.getNode()->GetLine()][p.getNode()->GetColumn()].SetValue(SPACE);
			group.erase(group.begin() + index);
		}
	}
}

void startGame()
{
	if (group1.size() > 0)
		chooseTarget(group1, GROUP1, GROUP2);
	else 
	{
		//Game Over
		run_Astar = false;
		printf("\n\n-----------Game Over--------------\n");
		return;
	}
	if (group2.size() >0)
		chooseTarget(group2, GROUP2, GROUP1);
	else 
		//Game Over
		run_Astar = false;

	if (!run_Astar)
		printf("\n\n-----------Game Over--------------\n");
}

bool isNearPlayerInTheRoom(Node* target)
{
	return findMyRoom(target) > -1;
}

Player* getPlayerFromNode(Node *playerNode,int groupValue) 
{
	int i;
	vector<Player>* myGroup;
	myGroup = groupValue == GROUP2 ? &group2 : &group1;

	for (i = 0; i < myGroup->size(); i++)
		if ((*myGroup)[i].getNode() == playerNode)
			return &(*myGroup)[i];
		
	return nullptr;
}


void fillingHealth(int myValue, Player* me)
{
	Node* target;
	int enemyValue;
	// if there are no more medicines, fill again
	if (medicines.size() == 0)
		createEquipment(medicines, MEDICINE);
	//choose target
	target = BfsIteration(myValue, MEDICINE, me, true);//bfs
	
	if (target == me->getNode()) //if no find medicene - do escape
	{
		enemyValue = myValue == GROUP1 ? GROUP2 : GROUP1;
		target = BfsIteration(myValue, enemyValue, me, true);
		bestFirstSearchForEscapeState(me, target);
	}
	else //go to medicine
		Astar(me, target);
}

void fightMode(Player *me,int enemyValue)
{
	Node* target;
	Player* playerEnemy;
	if (!canFire(me) && me->getIndex() != SQUIRE_INT)
	{
		me->setSecondState(FIGHT);
		target = BfsIteration(me->getNode()->GetValue(), enemyValue, me, true);
		playerEnemy = getPlayerFromNode(target, enemyValue);
		//if i can't fight my enemy , so i go search weapon:
		if (playerEnemy != nullptr && target->getParent()->GetValue() != me->getNode()->GetValue() && playerEnemy->getState() != FIGHT)
		{
			BfsIteration(me->getNode()->GetValue(), enemyValue, me, false);
			me->setSecondState(FIGHT);
		}
		else if (target == me->getNode() || (!isNearPlayerInTheRoom(target) && playerEnemy->getSecondState() == FIGHT))
		{
			me->setSecondState(FILLING_WEAPON);
			AStarToWeapon(me);
		}
		else
		{
			BfsIteration(me->getNode()->GetValue(), enemyValue, me, false);
			me->setSecondState(FIGHT);
		}
	}
}

void chooseTarget(vector<Player>& group ,int myValue, int enemyValue)
{
	int i, j, state;
	Node* target;

	for (i = 0;i < group.size();i++)
	{
		for (j = 0;j < 10000000;j++); 

		if (group1.size() == 0 || group2.size() == 0)
		{
			//game over
			run_Astar = false;
			return;
		}
		else
		{
			group[i].decisionTree();
			state = group[i].getState();
			switch (state)
			{
			case FIGHT:
				fightMode(&(group[i]), enemyValue);
				break;

			case ESCAPE:
				group[i].setSecondState(ESCAPE);
				target = BfsIteration(myValue, enemyValue, &(group[i]), true);
				bestFirstSearchForEscapeState(&(group[i]), target);
				break;

			case FILLING_HEALTH:
				group[i].setSecondState(FILLING_HEALTH);
				fillingHealth(myValue, &(group[i]));
				break;

			case FILLING_WEAPON:
				group[i].setSecondState(FILLING_WEAPON);
				AStarToWeapon(&(group[i]));
				break;
			}
		}
	}
}

void AStarToWeapon(Player * me)
{
	int myValue, enemyValue;
	Node* target;
	myValue = me->getNode()->GetValue();
	// if there are no more weapons, fill again
	if (granades.size() == 0 && bullets.size() == 0)
	{
		createEquipment(granades, GRANADES);
		createEquipment(bullets, BULLETS);
	}
		//choose target
	if (granades.size() > 0 && me->get_num_of_granade() < FULL_GRANADES) 
	{
		target = BfsIteration(myValue, GRANADES, me, true);
		if (target == me->getNode() && bullets.size())
			target = BfsIteration(myValue, BULLETS, me, true);
	}
	else //must to be bullets on maze
	{
		target = BfsIteration(myValue, BULLETS, me, true);
	}
	//if not find granade and bullets, do escape
	if (target == me->getNode())
	{
		if (myValue == GROUP1)
			enemyValue = GROUP2;
		else
			enemyValue = GROUP1;
		target = BfsIteration(myValue, enemyValue, me, true);
		bestFirstSearchForEscapeState(me, target);
	}
	else //if find bullet or grand, do astar
		Astar(me, target);
}

double distance(int x1, int y1, int x2, int y2) 
{
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

//this method check if we can arrive to enemy with shot
bool arriveToEnemy(Bullet* bullet,Player* enemy,int column,int line,Player *me) 
{
	int enemyLine, enemyColumn;
	double  distanceY, angle, distanceX;

	enemyLine = enemy->getNode()->GetLine();
	enemyColumn = enemy->getNode()->GetColumn();

	distanceY = 1.0 * line - enemyLine;
	distanceX = 1.0 * column - enemyColumn;
	angle = atan2(distanceY, distanceX)+ 3.14159265;
	bullet->SetDir(angle);
	bullet->setAngle(angle);
	bullet->SetIsMoving(true);
	if (bullet->arrive(maze, enemy->getNode()->GetValue(), me->getNode()->GetValue()) == true)
		return true;
	
	return false;
}

void fireGranade(Bullet* bullet, vector<Player> &enemyGroup, int indexEnemy, int myValue)
{
	int rowG, colG, rowB, colB, i, j , explode_dist = 0;
	double dist;
	Granade *g;

	//caculate granade position on maze
	rowG = MSZ * (bullet->getY() + 1) / 2 + 0.00001;
	colG = MSZ * (bullet->getX() + 1) / 2 + 0.00001;

	dist = distance(rowG, colG, enemyGroup[indexEnemy].getNode()->GetLine(),enemyGroup[indexEnemy].getNode()->GetColumn());
	
	//throw one bullet before explode
	while (bullet->GetIsMoving() && dist > GRANADE_DISTANCE_FROM_BOOM) 
	{
		glClear(GL_COLOR_BUFFER_BIT);
		DrawMaze();
		bullet->moveToTarget(maze, myValue);
		bullet->showMe();
		glutSwapBuffers();
		rowG = MSZ * (bullet->getY() + 1) / 2 + 0.00001;
		colG = MSZ * (bullet->getX() + 1) / 2 + 0.00001;
		dist = distance(rowG, colG, enemyGroup[indexEnemy].getNode()->GetLine(), enemyGroup[indexEnemy].getNode()->GetColumn());

	}
	//explode the granade and draw on maze
	g = new Granade( bullet->getX(), bullet->getY(), bullet->getAngle() );
	g->explode();

	while (g->isMoving() && explode_dist < 2*DISTANCE_GRANADE) 
	{
		explode_dist ++;
		glClear(GL_COLOR_BUFFER_BIT);
		DrawMaze();
		g->moveToTarget(maze,myValue);
		glutSwapBuffers();
	}

	//check if enemy hit
	for (i = 0;i < NUM_BULLETS ; i++)
	{
		bullet = g->getBulletByIndex(i);
		rowB = MSZ * (bullet->getY() + 1) / 2 + 0.00001;
		colB = MSZ * (bullet->getX() + 1) / 2 + 0.00001;

		for (j = 0; j < enemyGroup.size() ; j++)
		{
			if (&maze[rowB][colB] == enemyGroup[j].getNode()) //if hit on enemy
			{
				dist = distance(rowG, colG, enemyGroup[j].getNode()->GetLine(), enemyGroup[j].getNode()->GetColumn());
				enemyGroup[j].hit_life(dist, MSZ);
				if (enemyGroup[j].getLife() <= 0)
					killPlayer(enemyGroup, j);
			}
		}
	}
	//draw maze
	glClear(GL_COLOR_BUFFER_BIT);
	DrawMaze();
	glutSwapBuffers();
}

void fireBullet(Bullet* bullet, Player* enemy, int myValue)
{
	while (bullet->GetIsMoving()) {
		glClear(GL_COLOR_BUFFER_BIT);
		DrawMaze();
		bullet->moveToTarget(maze, myValue);
		bullet->showMe();
		glutSwapBuffers();
	}
	glClear(GL_COLOR_BUFFER_BIT);
	DrawMaze();
	glutSwapBuffers();
}

Granade* createGranade(int line,int column) 
{
	double xx, yy;
	Granade* g;
	xx = (((column + 0.5) * 2.0) / MSZ) - 1.0;
	yy = (((line + 0.5) * 2.0) / MSZ) - 1.0;
	g = new Granade(xx,yy);
	return g;
}

Bullet* createBullet(int line,int column) 
{
	double xx, yy;
	Bullet* bullet;
	//calculate bullete position
	xx = (((column+0.5) * 2.0) / MSZ) - 1;
	yy = (((line+0.5) * 2.0) / MSZ) - 1;
	bullet = new Bullet(xx, yy);
	return bullet;
}

//Choose granade if num granade > 0 else choose bullet
bool choosegGranade(Player *p,Player *enemy)
{
	double dist;
	
	dist = distance( p->getNode()->GetLine(), p->getNode()->GetColumn(), enemy->getNode()->GetLine(), enemy->getNode()->GetColumn());
	//if there is granade and distanse is small than DISTANCE_GRANADE
	if (p->get_num_of_granade() > 0 && dist < DISTANCE_GRANADE)
		return true;

	return false;
}

int findMyRoom(Node* nodePlayer) 
{
	int indexRoom, lineNode, colNode;
	lineNode = nodePlayer->GetLine();
	colNode = nodePlayer->GetColumn();
	for (indexRoom = 0; indexRoom < NUM_ROOMS; indexRoom++)
	{
		if (rooms[indexRoom].getLeftTop().getRow() <= lineNode
			&& rooms[indexRoom].getRightBottom().getRow() >= lineNode
			&& rooms[indexRoom].getLeftTop().getCol() <= colNode
			&& rooms[indexRoom].getRightBottom().getCol() >= colNode) {
			return indexRoom;
		}
	}
	return -1;
}

bool checkInSameRoom(Player* me, Player* enemy) 
{
	int myRoom, enemyRoom;
	myRoom = findMyRoom(me->getNode());
	enemyRoom = findMyRoom(enemy->getNode());
	if (myRoom == -1 || enemyRoom == -1)
		return false;
	return myRoom == enemyRoom;
}

bool canFire(Player* p)
{
	int i, column, line, myGroup;
	Bullet* bullet;
	double xx, yy;
	vector<Player> *enemyGroup;
	myGroup = p->getNode()->GetValue();
	line = p->getNode()->GetLine();
	column = p->getNode()->GetColumn();
	bullet = createBullet(line, column);
	xx = bullet->getX();
	yy = bullet->getY();

	if (myGroup == GROUP1) 
		enemyGroup = &group2;
	else
		enemyGroup = &group1;
	
	//for loop group 1:
	for (i = 0;i < enemyGroup->size() ; i++)
	{
		//set bullet to start:
		bullet->setX(xx);
		bullet->setY(yy);
		//&& checkInSameRoom(p, &(*enemyGroup)[i])
		if (arriveToEnemy(bullet, &(*enemyGroup)[i], column, line, p) && checkInSameRoom(p, &(*enemyGroup)[i]))
		{
			//set bullet to start:
			bullet->setX(xx);
			bullet->setY(yy);

			//start fire:
			if (choosegGranade(p, &(*enemyGroup)[i]))
			{
				fireGranade(bullet, *enemyGroup, i, myGroup); //i = enemy index
				p->reduceGranade();
			}
			else if (p->get_num_of_bullets() > 0)//choose bullets
			{
				fireBullet(bullet, &(*enemyGroup)[i] , myGroup);
				(*enemyGroup)[i].hit_life(distance(line, column, (*enemyGroup)[i].getNode()->GetLine(), (*enemyGroup)[i].getNode()->GetColumn()), MSZ);
				if ((*enemyGroup)[i].getLife() <= 0)
					killPlayer((*enemyGroup), i);
				p->reduceBullet();
			}
			return true;
		}
	}
	return false;
}

// runs A* from room i to room j
void PaveWay(Point2D start, Point2D target)
{
	vector<Node> grays;
	vector<Node> blacks;
	priority_queue <Node*, vector<Node*>, CompareNodes> pq;

	Node *pn;
	bool stop = false;
	vector<Node>::iterator gray_it;
	vector<Node>::iterator black_it;
	double wall_cost = 10;
	double space_cost = 0.2;
	pn = new Node(start, &target, maze[start.getRow()][start.getCol()].GetValue(), 0, nullptr);
	pq.push(pn);
	grays.push_back(*pn);
	while (!pq.empty() && !stop)
	{
		// take the best node from pq
		pn = pq.top();
		// remove top Node from pq
		pq.pop();
		if (pn->getPoint() == target) // the path has been found
		{
			stop = true;
			// restore path to dig tunnels
			// set SPACE instead of WALL on the path
			while (!(pn->getPoint() == start))
			{
				maze[pn->getPoint().getRow()][pn->getPoint().getCol()].SetValue(SPACE);
				pn = pn->getParent();
			}
			return;
		}
		else // pn is not target
		{
			// remove Node from gray and add it to black
			gray_it = find(grays.begin(), grays.end(), *pn); // operator == must be implemented in Node
			if (gray_it != grays.end())
				grays.erase(gray_it);
			blacks.push_back(*pn);
			// check the neighbours
			AddNeighbours(pn, grays, blacks, pq);
		}
	}
}

void DigTunnels()
{
	int i, j;

	for (i = 0; i < NUM_ROOMS; i++)
	{
		cout << "Path from " << i << endl;
		for (j = i + 1; j < NUM_ROOMS; j++)
		{
			cout << " to " << j << endl;
			PaveWay(rooms[i].getCenter(), rooms[j].getCenter());
		}
	}
}

void InitMaze()
{
	int i, j;

	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			maze[i][j].SetValue(WALL);
			maze[i][j].SetColumn(j);
			maze[i][j].SetLine(i);
		}
}

void InitRooms()
{
	for (numExistingRooms = 0; numExistingRooms < NUM_ROOMS; numExistingRooms++)
		rooms[numExistingRooms] = GenerateRoom();
}


void createEquipment(vector<Node> &equipment, int value) {
	int x, y, i;
	for (i = 0; i < NUMOFEQUIPMENT; i++)
	{
		do {
			x = rand() % MSZ;
			y = rand() % MSZ;
		} while (maze[x][y].GetValue() != SPACE);
		maze[x][y].SetValue(value);
		equipment.push_back(maze[x][y]);
	}
}

void createGroup(vector<Player>& group, const int color, int numPlayersInGroups)
{
	int i, x, y;
	Player* p;
	for (i = 0; i < numPlayersInGroups; i++)
	{
		do 
		{
			x = rand() % MSZ;
			y = rand() % MSZ;
		} while (maze[x][y].GetValue() != SPACE);
		p = new Player(i);
		p->setNode(&(maze[x][y]));
		p->getNode()->SetValue(color);
		group.push_back(*p);
	}
}

void DrawMaze()
{
	int i, j;
	double sz, x, y;

	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			// set color
			switch (maze[i][j].GetValue())
			{
			case SPACE:
				glColor3d(1, 1, 1); // white
				break;
			case WALL:
				glColor3d(0.4, 0, 0); // dark red
				break;
			case GRAY:
				glColor3d(0, 0.5, 0.5); // light green
				break;
			case BLACK:
				glColor3d(0, 0.7, 0.3); // dark green
				break;
			case PATH:
				glColor3d(1, 0, 1); // magenta
				break;
			case GROUP1:
				glColor3d(0, 0, 1); // light blue
				break;
			case GROUP2:
				glColor3d(1, 0, 0); // red
				break;
			case MEDICINE:
				glColor3d(0, 1, 0); // turkies
				break;
			case BULLETS:
				glColor3d(1, 0.5, 0); // red special
				break;
			case GRANADES:
				glColor3d(1, 0.5, 0.7); //  
				break;
			}
			
			// draw rectangle
			sz = 2.0 / MSZ;
			x = j * sz - 1;
			y = i * sz - 1;
			glBegin(GL_POLYGON);
			glVertex2d(x, y);
			glVertex2d(x + sz, y);
			glVertex2d(x + sz, y + sz);
			glVertex2d(x, y + sz);
			glEnd();
		}
}

void DrawMap()
{
	int i, j;
	double sz, xx, yy;

	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			if (maze[i][j].GetValue() == SPACE)
			{
				double c;
				c = 1 - map[i][j];// 1(white) is very safe, 0(black) is very dangerous
				glColor3d(c, c, c);
				// draw rectangle
				sz = 2.0 / MSZ;
				xx = (j * sz - 1);
				yy = i * sz - 1;

				glBegin(GL_POLYGON);
				glVertex2d(xx, yy);
				glVertex2d(xx + sz, yy);
				glVertex2d(xx + sz, yy + sz);
				glVertex2d(xx, yy + sz);

				glEnd();
			}
		}
}

void GenerateMap()
{
	const int NUMBER_RETRIES = 5000;
	int i;
	int col, row;
	Granade* pg = nullptr;

	for (i = 0; i < NUMBER_RETRIES; i++)
	{
		do
		{
			col = rand() % (MSZ-2)+1;
			row = rand() % (MSZ-2)+1;
		} while (maze[row][col].GetValue() != SPACE);

		pg = createGranade(row, col);
		pg->SimulateExplosion(map, maze);
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT); // clean frame buffer
	DrawMaze();
	glutSwapBuffers();// show what was drawn in "frame buffer"
}

void displayMap()
{
	glClear(GL_COLOR_BUFFER_BIT); // clean frame buffer

	DrawMaze();
	DrawMap();

	glutSwapBuffers();// show what was drawn in "frame buffer"
}

// checks if dx,dy is on SPACE in maze
bool CheckIsSpace(double dx, double dy)
{
	int i, j;

	i = MSZ * (dy + 1) / 2;
	j = MSZ * (dx + 1) / 2;
	return  maze[i][j].GetValue() == SPACE;
}

void idle()
{
	if (run_Astar)
		startGame();
	
	glutPostRedisplay();// calls indirectly to display
}

void Menu(int choice)
{
	if (choice == 1)
	{
		GenerateMap();
		run_Astar = true;
	}
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("Dungeon");

	glutDisplayFunc(display); // refresh function
	glutIdleFunc(idle); // idle: when nothing happens

	// menu
	glutCreateMenu(Menu);
	glutAddMenuEntry("Start Game", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	init();

	glutMainLoop();
	return 0;
}