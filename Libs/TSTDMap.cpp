//
//  TSTDMap.cpp
//  TIMScript
//
//  Created by Romain Beguet on 02.04.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDVector.h"
#include "Interpreter.h"
#include "Array.h"
#include <map>

namespace ts {
	namespace objects {
		class MapInstance : Object {
		public:

			MapInstance(ExecutionContext* ctx, TSDATA fn) : map(TSDATAComparator(ctx, fn)) {
				func = (Function*)fn.Ref;
			}

			virtual ~MapInstance() {}

			// GC

			virtual void gcPushRefs(std::vector<Object*>& objs) {
				objs.push_back(func);
				for (auto it = map.begin(), end = map.end(); it != end; ++it) {
					objs.push_back((*it).first.Instance);
					objs.push_back((*it).second.Instance);
					(*it).first.Instance->gcPushRefs(objs);
					(*it).second.Instance->gcPushRefs(objs);
				}
			}

			virtual const size_t size() {
				return sizeof(MapInstance);
			}

			struct TSDATAComparator {
				TSDATAComparator(ExecutionContext* ctx, TSDATA fn) : ctx(ctx), func((Function*)fn.Ref) {

				}

				bool operator()(TSDATA a, TSDATA b) {
					std::vector<TSDATA> data(2);
					data[0] = a;
					data[1] = b;
					return FunctionCaller(ctx, func).call(data).Int;
				}

				ExecutionContext* ctx;
				Function* func;
			};

			std::map<TSDATA, TSDATA, TSDATAComparator> map;
			Function* func;
		};
	}
	namespace tstd {



		void importMap(cmplr::Compiler& cmp) {
			using namespace nodes;
			using namespace tools;

			ClassNode* mapClass = cmp.addClass("Map");
			InterfaceNode* iterableInterface = cmp.findInterface("Iterable");
			ClassNode* objectClass = cmp.findClass("Object");

			mapClass->extends(objectClass);
			mapClass->addInterface(iterableInterface);
			mapClass->setAttributes({
				new Argument("_map", objectClass->typeClass())
			});

			cmp.setClassData(mapClass, "_new_", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				TSDATA map;
				map.Ref = new objects::MapInstance(ctx, args[1]);
				args[0].Instance->setAttr(0, map);
				ctx->gc->trace(map.Instance);
				ctx->gc->store(map.Instance);
				return args[0];
			}, "(Map, (Object, Object)->bool)->Map"));

			cmp.setClassData(mapClass, "set", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				((objects::MapInstance*)args[0].Instance->getAttr(0).Ref)->map[args[1]] = args[2];
				return TSDATA();
			}, "(Map, Object, Object)->void"));

			cmp.setClassData(mapClass, "get", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return ((objects::MapInstance*)args[0].Instance->getAttr(0).Ref)->map[args[1]];
			}, "(Map, Object)->Object"));

			cmp.setClassData(mapClass, "_[_", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return ((objects::MapInstance*)args[0].Instance->getAttr(0).Ref)->map[args[1]];
			}, "(Map, Object)->Object"));

			cmp.setClassData(mapClass, "contains", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				auto& map(((objects::MapInstance*)args[0].Instance->getAttr(0).Ref)->map);
				auto it = map.find(args[1]);
				return TSDATA {.Int = (it != map.end())};
			}, "(Map, Object)->bool"));

			auto stringClass = cmp.getSymbolLocation("String");
			size_t toStringOffset = objectClass->getDefinitionIndex("toString");

            cmp.setClassData(mapClass, "toString", FunctionBuilder::Make([=](ExecutionContext * ctx, TSDATA * args) {
				std::string* c = new std::string("{");
				auto& map(((objects::MapInstance*)args[0].Instance->getAttr(0).Ref)->map);

				std::vector<TSDATA> toStringArgs(1);

				for (auto it = map.begin(), end = map.end(); it != end; ++it) {
					toStringArgs[0] = (*it).first;
					TSDATA tsString = ctx->callVirtual(toStringOffset, toStringArgs);
					c->append(*((std::string*)tsString.Instance->getAttr(0).Ref));
					ctx->gc->store(tsString.Instance);

					c->append(" : ");

					toStringArgs[0] = (*it).second;
					tsString = ctx->callVirtual(toStringOffset, toStringArgs);
					c->append(*((std::string*)tsString.Instance->getAttr(0).Ref));
					ctx->gc->store(tsString.Instance);

					c->append(", ");
				}

				if (c->size() > 2)
					c->resize(c->size() - 2);

				c->append("}");

				TSDATA theString = ctx->newInstance(stringClass);
				theString.Instance->setAttr(0, TSDATA {.Ref = c});

				return theString;
			}, "(Map)->String"));

			cmp.setClassData(mapClass, "iterator", tools::FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return TSDATA();
			}, "(Map)->Iterator"));

			cmp.setClassData(mapClass, "StrCmp", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				std::string* a((std::string*)args[0].Instance->getAttr(0).Ref);
				std::string* b((std::string*)args[1].Instance->getAttr(0).Ref);

				return TSDATA {.Int = (*a < *b)};
			}, "(Object, Object)->bool"));

			cmp.setClassData(mapClass, "IntCmp", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return TSDATA {.Int = (args[0].Instance->getAttr(0).Int < args[1].Instance->getAttr(0).Int)};
			}, "(Object, Object)->bool"));
		}
	}
}
