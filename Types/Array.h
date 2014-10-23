//
//  Array.h
//  TIMScript
//
//  Created by Romain Beguet on 27.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Array__
#define __TIMScript__Array__


#include <iostream>
#include <vector>
#include "Utils.h"
#include "Object.h"

namespace ts {
	namespace objects {

		class Array : public Object {
		public:
			Array(TSINT length, bool r);
			virtual ~Array();

			inline TSINT length() {
				return (TSINT)_array.size();
			}
			inline TSDATA* array() {
				return _array.data();
			}
			inline std::vector<TSDATA>* vector() {
				return &_array;
			}

			inline bool areRefs() {
				return _areRefs;
			}

			// GC

			virtual void gcPushRefs(std::vector<Object*>& objs);
			virtual const size_t size() {
				return sizeof(Array);
			}

		private:

			std::vector<TSDATA> _array;
			bool _areRefs;
		};

	}
}

#endif /* defined(__TIMScript__Array__) */
