//
//  Interpreter.h
//  TIMScript
//
//  Created by Romain Beguet on 28.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Interpreter__
#define __TIMScript__Interpreter__

#include <iostream>
#include "TSProgram.h"
#include "GC.h"

namespace ts {

	struct ExecutionContext {
		ExecutionContext(GarbageCollector& g, const TSDATA* cp, const TSDATA* np, const size_t_Array* lr)
			: gc(g), cPool(cp), nPool(np), locrefs(lr) {}

		objects::Instance* newInstance(size_t clssLocation);
		TSDATA callFunction(size_t funcLocation, const std::vector<TSDATA>& args);

		GarbageCollector& gc;
		const TSDATA* cPool;
		const TSDATA* nPool;
		const size_t_Array* locrefs;
	};

	class Interpreter2 {
	public:
		Interpreter2(Program* p);
		~Interpreter2();

		TSDATA run2();
		TSDATA run2(size_t funcLoc, const std::vector<TSDATA>& args);
		void computeConstantPool2();
		inline const TSDATA* constantPool2() {
			return _constantPool;
		}

		ExecutionContext* createExecutionContext2() {
			GarbageCollector* gc = new GarbageCollector(GC_INITIAL_MAXSIZE, NULL, NULL);
			ExecutionContext* ctx = new ExecutionContext(*gc, _constantPool, _prog->nativesPool(), _prog->localsRefs());
			return ctx;
		}

		void freeExecutionContext2(ExecutionContext* ctx) {
			delete &ctx->gc;
			delete ctx;
		}

		template<bool USE_GC, bool AS_THREAD>
		static TSDATA exec2(const TSDATA* cPool,
							const TSDATA* nPool,
							const size_t_Array* locrefs,
							const Procedure* proc,
							TSDATA* args = NULL) {
			GarbageCollector gc(GC_INITIAL_MAXSIZE, NULL, NULL); // initialize the garbage collector
			ExecutionContext ctx(gc, cPool, nPool, locrefs);
			return exec<USE_GC, AS_THREAD>(&ctx, proc, args);
		}

		template<bool USE_GC, bool AS_THREAD>
		static TSDATA exec(ExecutionContext* ctx, const Procedure* proc, TSDATA* args = NULL);

	private:

		Program* _prog;
		TSDATA* _constantPool;
		bool _cp_alrdyComputed;
	};

}

#endif /* defined(__TIMScript__Interpreter__) */
