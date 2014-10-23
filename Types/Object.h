//
//  Object.h
//  TIMScript
//
//  Created by Romain Beguet on 05.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Object__
#define __TIMScript__Object__

#include <vector>
#include <iostream>

namespace ts {

	struct ExecutionContext;

	namespace objects {

		enum MARK_TYPE {UNMARKED = 0, MARKED = 1, TEMPORARY = 2};

		class Object {
		public:
			virtual ~Object() {}

			virtual void freeThis(ExecutionContext* ctx) {}

			// GC

			virtual void gcPushRefs(std::vector<Object*>& objs) = 0;
			virtual const size_t size() = 0;

			inline MARK_TYPE marked() {
				return _marked;
			}
			inline void mark() {
				_marked = (_marked == TEMPORARY ? TEMPORARY : MARKED);
			}
			inline void unmark() {
				_marked = (_marked == TEMPORARY ? TEMPORARY : UNMARKED);
			}

			inline void setTemporary() {
				_marked = TEMPORARY;
			}
			inline void unsetTemporary() {
				_marked = UNMARKED;
			}

			inline void setIndex(size_t i) {
				_indexInGcTemporaries = i;
			}
			inline size_t getIndex() {
				return _indexInGcTemporaries;
			}

		protected:

			MARK_TYPE _marked;
			size_t _indexInGcTemporaries;
		};

	}
}

#endif /* defined(__TIMScript__Object__) */
