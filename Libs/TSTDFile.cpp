//  TSTDFile.cpp
//  TIMScript
//
//  Created by Romain Beguet on 14.05.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDMath.h"
#include "Instance.h"
#include "Interpreter.h"
#include <ctime>
#include <cmath>
#include <fstream>

namespace ts {
	namespace tstd {

		void importFile(cmplr::Compiler& cmp) {
			using namespace nodes;
			using namespace tools;

			ClassNode* FileClass = cmp.addClass("File");
			FileClass->extends(cmp.findClass("Object"));

			FileClass->setAttributes({new Argument("_fileptr", type::BasicType::Int)});

			cmp.setClassData(FileClass, "_new_", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				args[0].Instance->setAttr(0, TSDATA {.Ref = new std::fstream(((std::string*)args[1].Instance->getAttr(0).Ref)->c_str(),
						std::ios::in | std::ios::out | (args[2].Int == 1 ? std::ios::trunc : std::ios::in) )
													});
				return args[0];
			}, "(File, String, int)->File"));

			cmp.setClassData(FileClass, "delete", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				std::fstream* file = (std::fstream*)args[0].Instance->getAttr(0).Ref;
				if (file->is_open())
					file->close();
				delete file;
				return TSDATA();
			}, "(File)->void"));

			cmp.setClassData(FileClass, "good", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				return TSDATA {.Int = ((std::fstream*)args[0].Instance->getAttr(0).Ref)->good()};
			}, "(File)->bool"));

			cmp.setClassData(FileClass, "close", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				std::fstream* file = (std::fstream*)args[0].Instance->getAttr(0).Ref;
				if (file->is_open())
					file->close();
				return TSDATA();
			}, "(File)->void"));

			auto stringClass = cmp.getSymbolLocation("String");
			auto stringConstructor = cmp.getSymbolLocation("String._new_");
            cmp.setClassData(FileClass, "readAll", FunctionBuilder::Make([=](ExecutionContext * ctx, TSDATA * args) {

				std::fstream* file = (std::fstream*)args[0].Instance->getAttr(0).Ref;
				std::string* content = new std::string;

				while (file->good())
					content->operator+=(file->get());

				TSDATA str = ctx->newInstance(stringClass);
				return ctx->callFunction(stringConstructor, {str, TSDATA{.Ref = content}});

			}, "(File)->String"));

            cmp.setClassData(FileClass, "readLine", FunctionBuilder::Make([=](ExecutionContext * ctx, TSDATA * args) {

				std::fstream* file = (std::fstream*)args[0].Instance->getAttr(0).Ref;
				std::string* content = new std::string;

				while (file->good()) {
					char c = file->get();
					if (c == args[1].Int) {
						break;
					}
					content->operator+=(c);
				}

				TSDATA str = ctx->newInstance(stringClass);
				ctx->gc->store(str.Instance);
				return ctx->callFunction(stringConstructor, {str, TSDATA{.Ref = content}});

			}, "(File, int)->String"));

			cmp.setClassData(FileClass, "readChar", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {

				return TSDATA {.Int = ((std::fstream*)args[0].Instance->getAttr(0).Ref)->get()};

			}, "(File)->int"));

			cmp.setClassData(FileClass, "write", FunctionBuilder::Make([](ExecutionContext * ctx, TSDATA * args) {
				std::fstream& file(*((std::fstream*)args[0].Instance->getAttr(0).Ref));
				std::string& str(*((std::string*)args[1].Instance->getAttr(0).Ref));

				file << str;

				return args[0];
			}, "(File, String)->File"));
		}

	}
}
