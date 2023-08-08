#pragma once

//#include "Framework.h"
//#include <stdio.h>
//#include <iostream>
//#include <stack>
//
//using namespace std;

#include "Pattern/Command.h"
#include "Environment/Billboard.h"

//Concrete Command
class BillboardCommand : public Command
{
public:
	Billboard* billboard;

	BillboardCommand(Billboard* billboard)
	{
		this->billboard = billboard;
	}

	void Execute() override
	{
		billboard->Update();
		billboard->Render();
	}

	void Undo() override 
	{
		billboard->Undo(); 
	}

	//void Redo() override 
	//{
	//	billboard->Redo();
	//}
};