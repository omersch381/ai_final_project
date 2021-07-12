#pragma once
#include "Node.h"
class CompratorAstar
{
public:
	CompratorAstar();
	~CompratorAstar();
	bool operator () (Node* pn1, Node* pn2)
	{
		return pn1->getG() + pn1->getH() + pn1->getSafeMap() > pn2->getG() + pn2->getH()+pn2->getSafeMap();
	}
};
