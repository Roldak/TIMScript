//
//  Debugger.cpp
//  TIMScript
//
//  Created by Romain Beguet on 20.09.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Debugger.h"

namespace ts {

	void AbstractDebugger::setBreakPointBetween(size_t startOfLine, size_t endOfLine) {
		_breakPoints.push_back(std::pair<size_t, size_t>(startOfLine, endOfLine));
	}

	void AbstractDebugger::removeBreakPoint(size_t index) {
		_breakPoints.erase(_breakPoints.begin() + index);
	}

	bool AbstractDebugger::shouldBreak(size_t pos) {
		for (const auto& p : _breakPoints) {
			if (p.first < pos && p.second > pos)
				return true;
		}
		return false;
	}

}
