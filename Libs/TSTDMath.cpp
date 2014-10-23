//
//  TSTDMath.cpp
//  TIMScript
//
//  Created by Romain Beguet on 23.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDMath.h"
#include <ctime>
#include <cmath>

namespace ts {
	namespace tstd {

		inline TSREAL rnd() {
			static long unsigned int _seed = 214013 * (unsigned long)time(NULL) + 2531011;
			_seed = (214013 * _seed + 2531011);
			return (TSREAL)((_seed >> 16) & 0x7FFF) / (TSREAL)0x8000;
		}

		inline TSINT fact(TSINT n) {
			TSINT res = 1;
			for (int i = 2; i <= n; ++i)
				res *= i;
			return res;
		}

		void importMath(cmplr::Compiler& cmp) {
			using namespace nodes;

			ClassNode* math = cmp.addClass("Math");

			cmp.setClassData(math, "sin", tools::FunctionBuilder::Get(sin));
			cmp.setClassData(math, "cos", tools::FunctionBuilder::Get(cos));
			cmp.setClassData(math, "tan", tools::FunctionBuilder::Get(tan));
			cmp.setClassData(math, "asin", tools::FunctionBuilder::Get(asin));
			cmp.setClassData(math, "acos", tools::FunctionBuilder::Get(acos));
			cmp.setClassData(math, "atan", tools::FunctionBuilder::Get(atan));

			cmp.setClassData(math, "hypot", tools::FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				args[0].Real = hypot(args[0].Real, args[1].Real);
				return args[0];
			}, "(real, real)->real"));

			cmp.setClassData(math, "toRadians", tools::FunctionBuilder::Get([](TSREAL r) {
				return r / 180 * M_PI;
			}));

			cmp.setClassData(math, "toDegrees", tools::FunctionBuilder::Get([](TSREAL r) {
				return r / M_PI * 180;
			}));

			cmp.setClassData(math, "log2", tools::FunctionBuilder::Get(log2));
			cmp.setClassData(math, "log10", tools::FunctionBuilder::Get(log10));
			cmp.setClassData(math, "log", tools::FunctionBuilder::Get(log));

			cmp.setClassData(math, "exp", tools::FunctionBuilder::Get(exp));
			cmp.setClassData(math, "exp2", tools::FunctionBuilder::Get(exp2));

			cmp.setClassData(math, "fact", tools::FunctionBuilder::Get(fact));

			cmp.setClassData(math, "sqrt", tools::FunctionBuilder::Get(sqrt));

			cmp.setClassData(math, "floor", tools::FunctionBuilder::Get(floor));
			cmp.setClassData(math, "ceil", tools::FunctionBuilder::Get(ceil));
			cmp.setClassData(math, "round", tools::FunctionBuilder::Get(round));

			cmp.setClassData(math, "rnd", tools::FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return TSDATA {.Real = rnd()};
			}, "()->real"));

			cmp.setClassData(math, "rndInt", tools::FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return TSDATA {.Int = (TSINT)floor(rnd() * args[0].Int)};
			}, "(int)->int"));

			cmp.setClassData(math, "rndFloat", tools::FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return TSDATA {.Real = rnd() * args[0].Real};
			}, "(real)->real"));

			cmp.setClassData(math, "abs", tools::FunctionBuilder::Get([](TSREAL r) {
				return r < 0 ? -r : r;
			}));


			cmp.setClassData(math, "clamp", tools::FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return args[0].Real < args[1].Real ? args[1] : (args[0].Real > args[2].Real ? args[2] : args[0]);
			}, "(real, real, real)->real"));

			cmp.setClassData(math, "min", tools::FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return args[0].Real < args[1].Real ? args[0] : args[1];
			}, "(real, real)->real"));
			cmp.setClassData(math, "max", tools::FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return args[0].Real < args[1].Real ? args[1] : args[0];
			}, "(real, real)->real"));

		}
	}
}
