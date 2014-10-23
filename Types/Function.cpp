//
//  Function.cpp
//  TIMScript
//
//  Created by Romain Beguet on 05.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Function.h"
#include "Interpreter.h"

namespace ts {
	namespace objects {

		// FUNCTION

		Function::Function(NativeFunction* native, const std::string& name) : _native(native), _proc(name) {

		}

		Function::Function(const std::vector<TSDATA>& cap, const Procedure& proc) : _captures(cap), _proc(proc), _native(NULL) {

		}

		Function::~Function() {

		}

		TSDATA Function::call(ExecutionContext* ctx, const std::vector<TSDATA>& args) const {
			return FunctionCaller(ctx, this).call(args);
		}

		// GC

		void Function::gcPushRefs(std::vector<Object*>& objs) {
			if (_native)
				return;

			size_t_Array refsLocation = *_proc.getRefMap();

			for (size_t i = 0; i < _captures.size(); ++i) {
				if (refsLocation._array[i] != std::string::npos) {

					Object* obj = ((Object*)_captures[i].Ref);

					if (obj && !obj->marked())
						objs.push_back(obj);
				}
			}
		}


	}
}
