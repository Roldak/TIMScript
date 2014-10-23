//
//  NativeData.h
//  TIMScript
//
//  Created by Romain Beguet on 19.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__NativeData__
#define __TIMScript__NativeData__

#include <iostream>
#include <functional>
#include "Utils.h"
#include "Object.h"
#include "ClassNode.h"

namespace ts {
	struct ExecutionContext;

	namespace objects {

		class NativeData : public Object {
		public:

		};

		class NativeFunction : public NativeData {
		public:
			NativeFunction() {}
			NativeFunction(const std::function<TSDATA(ExecutionContext*, TSDATA*)>& func) : _funcPtr(func) {}

			void setFunction(const std::function<TSDATA(ExecutionContext*, TSDATA*)>& func) {
				_funcPtr = func;
			}

			inline TSDATA call(ExecutionContext* ctx, TSDATA* argv) const {
				return _funcPtr(ctx, argv);
			}

			virtual void gcPushRefs(std::vector<Object*>& objs) {}
			virtual const size_t size() {
				return sizeof(NativeFunction);
			}

		private:

			std::function<TSDATA(ExecutionContext*, TSDATA*)> _funcPtr;
		};

	}
}

#endif /* defined(__TIMScript__NativeData__) */
