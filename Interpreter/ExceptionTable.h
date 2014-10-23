//
//  ExceptionTable.h
//  TIMScript
//
//  Created by Romain Beguet on 29.05.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__ExceptionTable__
#define __TIMScript__ExceptionTable__

#include <iostream>
#include <vector>
#include "Utils.h"

namespace ts {

	struct ExceptionTableCatch {
		const size_t localIndex;
		const size_t classIndex;
		const size_t catchEntryIP;
	};

	struct ExceptionTableTry {
		size_t tryBlockIPStart, tryBlockIPEnd;
		std::vector<ExceptionTableCatch> catches;
	};

	typedef std::vector<ExceptionTableTry> ExceptionTable;

}

#endif /* defined(__TIMScript__ExceptionTable__) */
