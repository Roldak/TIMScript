//
//  Identifier.h
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#ifndef __TimScript_5__Identifier__
#define __TimScript_5__Identifier__

#include <iostream>

#include "AbstractToken.h"

namespace ts {
	namespace tok {

		class Identifier : public AbstractToken {
		public:
			Identifier(size_t pos, size_t len, std::string n) : AbstractToken(pos, len), _name(n) {}
			virtual ~Identifier() {}

			inline std::string name() {
				return _name;
			}

			virtual TOKEN_TYPE getType() {
				return TK_IDENTIFIER;
			}
			virtual std::string toString() {
				return _name;
			}

		private:
			const std::string _name;
		};

	}
}

#endif /* defined(__TimScript_5__Identifier__) */
