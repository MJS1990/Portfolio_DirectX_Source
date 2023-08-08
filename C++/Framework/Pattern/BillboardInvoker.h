#pragma once

#include "Framework.h"
#include "Command.h"
#include "Environment/Billboard.h"
#include "Pattern/Command.h"

//인보커 -> 클라이언트에서 실행을 주문받는 객체
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
