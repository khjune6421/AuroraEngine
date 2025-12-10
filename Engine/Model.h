#pragma once
#include "ComponentBase.h"

class Model : public ComponentBase
{
public:
	Model() = default;
	~Model() override = default;
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) = delete;
	Model& operator=(Model&&) = delete;

protected:
	void Begin() override;
};