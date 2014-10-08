//
//  CodePattern.h
//  TIMScript
//
//  Created by Romain Beguet on 19.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__CodePattern__
#define __TIMScript__CodePattern__

#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include "Utils.h"

namespace ts{
    namespace opt{

        class Element{
        public:
            virtual ~Element(){}
            virtual bool matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const=0;
            virtual void generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const=0;
        };

        class Lit : public Element{
        public:
            Lit(TSINSTR lit) : _lit(lit) {}
            virtual ~Lit(){}

            virtual bool matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const;
            virtual void generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const;

        private:
            TSINSTR _lit;
        };

        class Var : public Element{
        public:
            Var(const std::string& name) : _name(name) {}
            virtual ~Var(){}

            virtual bool matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const;
            virtual void generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const;

        protected:
            const std::string _name;
        };

        class OneOf : public Var{
        public:
            OneOf(const std::string& name, const std::vector<TSINSTR>& possibilities) : Var(name), _possibilities(possibilities) {}
            virtual ~OneOf(){}

            virtual bool matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const;
            virtual void generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const;

        private:
            const std::vector<TSINSTR> _possibilities;
        };

        class Func : public Element{
        public:

            Func(std::function<TSINSTR(std::map<std::string, TSINSTR>&)> func) : _func(func) {}

            virtual ~Func(){}

            virtual bool matches(std::map<std::string, TSINSTR>& vars, TSINSTR instr) const;
            virtual void generate(std::map<std::string, TSINSTR>& vars, std::vector<TSINSTR>& code) const;

        private:

            std::function<TSINSTR(std::map<std::string, TSINSTR>&)> _func;
        };

        class CodePattern{
        public:

            CodePattern(const std::vector<Element*>& code, const std::vector<Element*>& replaceWith) : _code(code), _replaceCode(replaceWith) {}
            ~CodePattern();

            bool tryMatchAndReplace(std::vector<TSINSTR>& prg, size_t ip) const;
            size_t length() const{return _code.size();}
            size_t replaceLength() const{return _replaceCode.size();}

            static const std::vector<CodePattern*> CastOptimizationPatterns;
            static const std::vector<CodePattern*> IncrementationOptimizationPatterns;
            static const std::vector<CodePattern*> BranchesOptimizations;
            static const std::vector<CodePattern*> ConstantsOptimizations;
            static const std::vector<CodePattern*> TautologyOptimizations;

        private:

            void generate(std::vector<TSINSTR>& code, std::map<std::string, TSINSTR>& vars) const;

            std::vector<Element*> _code;
            std::vector<Element*> _replaceCode;

        };

    }
}

#endif /* defined(__TIMScript__CodePattern__) */
