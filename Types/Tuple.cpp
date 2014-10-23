//
//  Tuple.cpp
//  TIMScript
//
//  Created by Romain Beguet on 06.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Tuple.h"
#include <cstdlib>

namespace ts {
	namespace objects {

		Tuple::Tuple(size_t n, size_t_Array** refsLocations) : _refsLocations(refsLocations) {
			_elems = (TSDATA*)calloc(n, sizeof(TSDATA));
		}

		Tuple::Tuple(TSDATA* tab, size_t_Array** refsLocations) : _refsLocations(refsLocations) {
			_elems = tab;
		}

		Tuple::~Tuple() {
			free(_elems);
		}

		void Tuple::gcPushRefs(std::vector<Object*>& objs) {
			size_t_Array refsLocation = **_refsLocations;

			for (size_t curCount = 0, i = 0; curCount < refsLocation._count; ++i) {
				if (refsLocation._array[i] != std::string::npos) {
					++curCount;
					Object* obj = ((Object*)_elems[i].Ref);

					if (obj && !obj->marked())
						objs.push_back(obj);
				}
			}
		}

	}
}
