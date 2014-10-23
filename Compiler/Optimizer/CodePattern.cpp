//
//  CodePattern.cpp
//  TIMScript
//
//  Created by Romain Beguet on 19.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "CodePattern.h"
#include "Bytecode.h"
#include <cmath>

#define P_LIT(x) new Lit((x))
#define P_VAR(x) new Var((#x))
#define P_FUN(x) new Func((x))
#define P_ONEOF(x) new OneOf((x))

namespace ts {
	namespace opt {

		// LIT

		bool Lit::matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const {
			return _lit == instr;
		}

		void Lit::generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const {
			code.push_back(_lit);
		}

		// VAR

		bool Var::matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const {
			auto it = vars.find(_name);
			if (it != vars.end())
				return it->second == instr;
			else
				vars[_name] = instr;
			return true;
		}

		void Var::generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const {
			code.push_back(vars[_name]);
		}

		// ONEOF

		bool OneOf::matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const {
			for (auto poss : _possibilities) {
				if (instr == poss) {
					vars[_name] = instr;
					return true;
				}
			}
			return false;
		}

		void OneOf::generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const {

		}

		// FUNC

		bool Func::matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const {
			return instr == _func(vars);
		}

		void Func::generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const {
			code.push_back(_func(vars));
		}

		// PATTERNS CREATION

		std::vector<CodePattern*> createCastOptimizationPatterns() {
			std::vector<CodePattern*> patterns;

			patterns.push_back(new CodePattern({
				P_LIT(PUSH_I), P_VAR(n),
				P_LIT(I_TO_R)
			}, {
				P_LIT(PUSH_R), P_FUN([](std::map<std::string, TSINSTR>& vars) {
					TSDATA x;
					x.Real = (TSINT)vars["n"];
					return x.Int;
				})
			}));

			patterns.push_back(new CodePattern({
				P_LIT(PUSH_R), P_VAR(n),
				P_LIT(R_TO_I)
			}, {
				P_LIT(PUSH_I), P_FUN([](std::map<std::string, TSINSTR>& vars) {
					TSDATA x;
					x.Real = instr_to_r(vars["n"]);
					return x.Real;
				})
			}));

			return patterns;
		}

		std::vector<CodePattern*> createIncrementationOptimizationPatterns() {
			std::vector<CodePattern*> patterns;

			patterns.push_back(new CodePattern({
				P_LIT(LOAD_N), P_VAR(n),
				P_LIT(PUSH_I), P_LIT(1),
				P_LIT(ADD_I),
				P_LIT(STORE_N), P_VAR(n),
			}, {
				P_LIT(INC_I_1), P_VAR(n),
				P_LIT(POP)
			}));

			patterns.push_back(new CodePattern({
				P_LIT(LOAD_N), P_VAR(n),
				P_LIT(PUSH_R), P_LIT(4607182418800017408), // 1 in real
				P_LIT(ADD_R),
				P_LIT(STORE_N), P_VAR(n),
			}, {
				P_LIT(INC_R_1), P_VAR(n),
				P_LIT(POP)
			}));

			patterns.push_back(new CodePattern({
				P_LIT(LOAD_N), P_VAR(n),
				P_LIT(DUP), P_LIT(1), // 1 in real
				P_LIT(GET_V), P_VAR(x),
				P_LIT(CALL_FUNC), P_LIT(1)
			}, {
				P_LIT(LD_CALL_V), P_VAR(n), P_VAR(x), P_LIT(1)
			}));

			return patterns;
		}

		std::vector<CodePattern*> createBranchesOptimizationsPatterns() {
			std::vector<CodePattern*> patterns;

			patterns.push_back(new CodePattern({
				P_LIT(CMP_I_L),
				P_LIT(IF_I_FALSE), P_VAR(n)
			}, {
				P_LIT(IFG_I), P_VAR(n)
			}));

			return patterns;
		}

		std::vector<CodePattern*> createConstantsOptimizationsPatterns() {
			std::vector<CodePattern*> patterns;

			patterns.push_back(new CodePattern({
				P_LIT(PUSH_I), P_VAR(a),
				P_LIT(PUSH_I), P_VAR(b),
				new OneOf("x", {ADD_I, SUB_I, MUL_I, DIV_I, MOD_I, POW_I})
			}, {
				P_LIT(PUSH_I), P_FUN([](std::map<std::string, TSINSTR>& vars) {
					switch (vars["x"]) {
						case ADD_I:
							return vars["a"] + vars["b"];
						case SUB_I:
							return vars["a"] - vars["b"];
						case MUL_I:
							return vars["a"] * vars["b"];
						case DIV_I:
							return vars["a"] / vars["b"];
						case MOD_I:
							return vars["a"] % vars["b"];
						case POW_I:
							return (TSINSTR)pow(vars["a"], vars["b"]);

						default:
							return vars["a"];
					}
				})
			}));
			return patterns;
		}

		std::vector<CodePattern*> createTautologyOptimizationsPatterns() {
			std::vector<CodePattern*> patterns;

			patterns.push_back(new CodePattern({
				P_LIT(PUSH_FALSE),
				P_LIT(IF_I_TRUE), P_VAR(n)
			}, {}));
			patterns.push_back(new CodePattern({
				P_LIT(PUSH_TRUE),
				P_LIT(IF_I_FALSE), P_VAR(n)
			}, {}));
			patterns.push_back(new CodePattern({
				P_LIT(PUSH_FALSE),
				P_LIT(IF_I_FALSE), P_VAR(n)
			}, {
				P_LIT(GOTO), P_VAR(n)
			}));
			patterns.push_back(new CodePattern({
				P_LIT(PUSH_TRUE),
				P_LIT(IF_I_TRUE), P_VAR(n)
			}, {
				P_LIT(GOTO), P_VAR(n)
			}));

			return patterns;
		}

		const std::vector<CodePattern*> CodePattern::CastOptimizationPatterns = createCastOptimizationPatterns();
		const std::vector<CodePattern*> CodePattern::IncrementationOptimizationPatterns = createIncrementationOptimizationPatterns();
		const std::vector<CodePattern*> CodePattern::BranchesOptimizations = createBranchesOptimizationsPatterns();
		const std::vector<CodePattern*> CodePattern::ConstantsOptimizations = createConstantsOptimizationsPatterns();
		const std::vector<CodePattern*> CodePattern::TautologyOptimizations = createTautologyOptimizationsPatterns();

		CodePattern::~CodePattern() {
			for (size_t i = 0; i < _code.size(); ++i)
				delete _code[i];
		}

		void CodePattern::generate(std::vector<TSINSTR>& code, std::map<std::string, TSINSTR>& vars) const {
			for (size_t i = 0; i < _replaceCode.size(); ++i) {
				_replaceCode[i]->generate(vars, code);
			}
		}

		bool CodePattern::tryMatchAndReplace(std::vector<TSINSTR>& prg, size_t ip) const {
			std::map<std::string, TSINSTR> vars;

			for (size_t i = 0; i < _code.size(); ++i)
				if (!_code[i]->matches(vars, prg[i + ip]))
					return false;

			std::vector<TSINSTR> newCode;
			generate(newCode, vars);

			prg.erase(prg.begin() + (long)ip, prg.begin() + (long)ip + (long)_code.size());
			prg.insert(prg.begin() + (long)ip, newCode.begin(), newCode.end());
			return true;
		}

	}
}