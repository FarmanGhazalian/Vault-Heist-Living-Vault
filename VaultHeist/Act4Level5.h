#pragma once
#include "Act4Level3.h"

class Act4Level5 : public Act4Level3
{
protected:
	bool building;

public:
	Act4Level5();
	virtual ~Act4Level5() {}

	bool initLevel(const string& jsonPath) override;

	ActionResult processAction(const PlayerAction& action) override;

	bool isBuilding() const;
};
