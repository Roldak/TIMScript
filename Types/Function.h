//
//  Function.h
//  TIMScript
//
//  Created by Romain Beguet on 05.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Function__
#define __TIMScript__Function__

#include <iostream>
#include <vector>
#include "Object.h"
#include "Procedure.h"
#include "NativeData.h"

namespace ts {
	namespace objects {

		class Function : public Object {
		public:
			Function(NativeFunction* native, const std::string& name);
			Function(const std::vector<TSDATA>& cap, const Procedure& proc);
			virtual ~Function();

			inline const std::vector<TSDATA>& captures() const {
				return _captures;
			}
			inline size_t_Array* getLocRefTable() const {
				return _proc.getRefMap();
			}
			inline const Procedure& getProc() const {
				return _proc;
			}
			inline const Procedure* getProcPtr() const {
				return &_proc;
			}

			inline const NativeFunction* getNative() const {
				return _native;
			}

			TSDATA call(ExecutionContext* ctx, const std::vector<TSDATA>& args) const;

			// GC

			virtual void gcPushRefs(std::vector<Object*>& objs);
			virtual const size_t size() {
				return sizeof(Function);
			}

		private:

			const std::vector<TSDATA> _captures;
			const Procedure _proc;
			NativeFunction* _native;
		};

	}
}

#endif /* defined(__TIMScript__Function__) */
