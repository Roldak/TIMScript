//
//  TSProgram.h
//  TIMScript
//
//  Created by Romain Beguet on 04.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__TSProgram__
#define __TIMScript__TSProgram__

#include <iostream>
#include <vector>
#include "Procedure.h"
#include "Scope.h"

namespace ts {
	class Program {
	public:
		Program(std::vector<Procedure*>& cp, TSDATA* np, size_t mainRefMap, size_t mainExTable, Procedure* pr) :
			_constantPool(cp), _nativesPool(np), _main_refmap_index(mainRefMap), _main_extable_index(mainExTable), _main_proc(pr) {}

		inline const std::vector<Procedure*>& constantPool() {
			return _constantPool;
		}
		inline const TSDATA* nativesPool() {
			return _nativesPool;
		}
		inline const size_t mainRefMapIndex() {
			return _main_refmap_index;
		}
		inline const size_t mainExTableIndex() {
			return _main_extable_index;
		}
		inline Procedure* main() {
			return _main_proc;
		}

	private:
		std::vector<Procedure*> _constantPool;
		TSDATA* _nativesPool;
		size_t _main_refmap_index;
		size_t _main_extable_index;
		Procedure* _main_proc;
	};
}

#endif /* defined(__TIMScript__TSProgram__) */
