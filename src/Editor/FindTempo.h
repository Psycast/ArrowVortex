﻿#pragma once

#include <Core/Vector.h>

namespace Vortex {

struct TempoResult
{
	double bpm, offset, fitness;
};

class TempoDetector
{
public:
	static TempoDetector* New(double time, double len);
	virtual ~TempoDetector() {}

	virtual const char* getProgress() const = 0;
	virtual bool hasResult() const = 0;
	virtual const Vector<TempoResult>& getResult() const = 0;
};

}; // namespace Vortex