#pragma once

#include "Framework.h"
#include "Command.h"
#include "Environment/Billboard.h"
#include "Pattern/Command.h"

//�κ�Ŀ -> Ŭ���̾�Ʈ���� ������ �ֹ��޴� ��ü
class BillboardInvoker
{
public:
	Command* slot;

	BillboardInvoker() {}
	~BillboardInvoker() {}

	void SetCommand(Command* command) { slot = command; }
	void DrawBillboard() { slot->Execute(); }
	void BillboardUndo() { slot->Undo(); }
};
