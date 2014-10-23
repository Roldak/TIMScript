//
//  TSTDPrimitive.cpp
//  TIMScript
//
//  Created by Romain Beguet on 13.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDPrimitive.h"
#include "Interpreter.h"

namespace ts {
	namespace tstd {

		void importPrimitive(cmplr::Compiler& cmp) {
			using namespace nodes;
			using namespace tools;

			// OBJECT

			ClassNode* object = cmp.addClass("Object");
			cmp.addClass("String");
			auto stringIndex = cmp.getSymbolLocation("String");


			cmp.setClassData(object, "_==_", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return TSDATA {.Int = (args[0].Ref == args[1].Ref)};
			}, "(Object, Object)->bool"));

			cmp.setClassData(object, "toString", FunctionBuilder::Make([stringIndex](ExecutionContext * ctx, TSDATA * args) {
				TSDATA str = ctx->newInstance(stringIndex);
				str.Instance->setAttr(0, TSDATA {.Ref = new std::string("<" + args[0].Instance->getClass()->name() + " at " + T_toString(args[0].Ref) + ">")});
				return str;
			}, "(Object)->String"));


			cmp.setClassData(object, "typeName", FunctionBuilder::Make([stringIndex](ExecutionContext * ctx, TSDATA * args) {
				TSDATA str = ctx->newInstance(stringIndex);
				str.Instance->setAttr(0, TSDATA {.Ref = new std::string(args[0].Instance->getClass()->name())});
				return str;
			}, "(Object)->String"));

			// INT

			ClassNode* Int = cmp.addClass("Int");
			Int->extends(object);
			Int->setAttributes({new Argument("_value", type::BasicType::Int)});

			auto IntLocation = cmp.getSymbolLocation("Int");

			cmp.setClassData(Int, "_new_", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				args[0].Instance->setAttr(0, args[1]);
				return args[0];
			}, "(Int, int)->Int"));

			cmp.setClassData(Int, "_==_", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				if (args[0].Instance->getClass() == args[1].Instance->getClass())
					if (args[0].Instance->getAttr(0).Int == args[1].Instance->getAttr(0).Int)
						return TSDATA {.Int = true};

				return TSDATA {.Int = false};
			}, "(Int, Object)->bool"));

			cmp.setClassData(Int, "_+_", FunctionBuilder::Make([IntLocation](ExecutionContext * ctx, TSDATA * args) {
				TSDATA toRet = ctx->newInstance(IntLocation);
				toRet.Instance->setAttr(0, TSDATA {.Int = args[0].Instance->getAttr(0).Int + args[1].Instance->getAttr(0).Int});
				return toRet;
			}, "(Int, Int)->Int"));

			cmp.setClassData(Int, "valueOf", FunctionBuilder::Make([stringIndex](ExecutionContext * ctx, TSDATA * args) {
				if (args[0].Instance->getClass() == ctx->cPool[stringIndex.index].Ref)
					return TSDATA {.Int = String_toT<TSINT>(*((std::string*)args[0].Instance->getAttr(0).Ref))};

				return TSDATA {.Int = 0};
			}, "(String)->int"));

			cmp.setClassData(Int, "toString", FunctionBuilder::Make([stringIndex](ExecutionContext * ctx, TSDATA * args) {
				TSDATA str = ctx->newInstance(stringIndex);
				str.Instance->setAttr(0, TSDATA {.Ref = new std::string(T_toString(args[0].Instance->getAttr(0).Int))});
				return str;
			}, "(Int)->String"));

			// REAL

			ClassNode* Real = cmp.addClass("Real");
			Real->extends(object);
			Real->setAttributes({new Argument("_value", type::BasicType::Real)});

			cmp.setClassData(Real, "_new_", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				args[0].Instance->setAttr(0, args[1]);
				return args[0];
			}, "(Real, real)->Real"));

			cmp.setClassData(Real, "_==_", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				if (args[0].Instance->getClass() == args[1].Instance->getClass())
					if (args[0].Instance->getAttr(0).Real == args[1].Instance->getAttr(0).Real)
						return TSDATA {.Int = true};

				return TSDATA {.Int = false};
			}, "(Real, Object)->bool"));

			cmp.setClassData(Real, "valueOf", FunctionBuilder::Make([stringIndex](ExecutionContext * ctx, TSDATA * args) {
				if (args[0].Instance->getClass() == ctx->cPool[stringIndex.index].Ref)
					return TSDATA {.Real = String_toT<TSREAL>(*((std::string*)args[0].Instance->getAttr(0).Ref))};

				return TSDATA {.Real = 0};
			}, "(String)->real"));

			cmp.setClassData(Real, "toString", FunctionBuilder::Make([stringIndex](ExecutionContext * ctx, TSDATA * args) {
				TSDATA str = ctx->newInstance(stringIndex);
				str.Instance->setAttr(0, TSDATA {.Ref = new std::string(T_toString(args[0].Instance->getAttr(0).Real))});
				return str;
			}, "(Real)->String"));
		}
	}
}
