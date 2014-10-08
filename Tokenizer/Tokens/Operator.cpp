//
//  Operator.cpp
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#include "Operator.h"

namespace ts {
    namespace tok{
        
        int Operator::getOperatorPrecedence(){
            return Operator::operatorsPrecedence[_opType];
        }
        
        std::unordered_map<std::string, OPERATOR_TYPE> createOperatorsMap(){
            std::unordered_map<std::string, OPERATOR_TYPE> map;
            map["+"]=PLUS;
            map["-"]=MINUS;
            map["*"]=TIMES;
            map["/"]=SLASH;
            map["%"]=MODULO;
            map["^"]=POWER;
            map["="]=EQUAL;
            map["=="]=EQUAL_EQUAL;
            map["!="]=NOT_EQUAL;
            map[">"]=SUP;
            map["<"]=INF;
            map[">="]=SUP_EQUAL;
            map["<="]=INF_EQUAL;
            map["is"]=ISTYPE;
            map["=>"]=RIGHT_ARROW;
            map["->"]=TINY_RIGHT_ARROW;
            map["?"]=TERNARY;
            map["||"]=OR;
            map["&&"]=AND;
            map["&"]=B_AND;
            map["|"]=B_OR;
            map["++"]=PLUS_PLUS;
            map["--"]=MINUS_MINUS;
            map["!"]=NOT;
            map["new"]=NEW;
            map["scoped"]=SCOPED;
            map["("]=PAREN_O;
            map[")"]=PAREN_C;
            map["["]=BRACKET_O;
            map["]"]=BRACKET_C;
            map["{"]=BRACE_O;
            map["}"]=BRACE_C;
            map["."]=DOT;
            map[","]=COMMA;
            map[":"]=COLON;
            map["::"]=DOUBLE_COLON;
            map[";"]=SEMICOLON;
            map["@"]=AT_SIGN;
            return map;
        }
        
        std::vector<int> createOperatorsPrecedencesTable(){
            std::vector<int> prec(NUMBER_OF_OPERATORS, -1);
            prec[PLUS]=20;
            prec[MINUS]=20;
            prec[TIMES]=40;
            prec[SLASH]=40;
            prec[MODULO]=40;
            prec[POWER]=50;
            prec[EQUAL]=2;
            prec[EQUAL_EQUAL]=9;
            prec[NOT_EQUAL]=9;
            prec[SUP]=10;
            prec[INF]=10;
            prec[SUP_EQUAL]=10;
            prec[INF_EQUAL]=10;
            prec[TERNARY]=2;
            prec[OR]=5;
            prec[AND]=6;
            prec[B_AND]=100;
            prec[PAREN_O]=100;
            prec[BRACKET_O]=100;
            prec[ISTYPE]=60;
            prec[DOT]=70;
            prec[COLON]=70;
            prec[DOUBLE_COLON]=70;
            return prec;
        }

        std::unordered_map<std::string, OPERATOR_TYPE> Operator::operators=createOperatorsMap();
        std::vector<int> Operator::operatorsPrecedence=createOperatorsPrecedencesTable();

        std::string Operator::toString(){
            switch (_opType) {
                case PLUS: return "+";
                case MINUS: return "-";
                case TIMES: return "*";
                case SLASH: return "/";
                case MODULO: return "%";
                case POWER: return "^";
                case EQUAL: return "=";
                case EQUAL_EQUAL: return "==";
                case NOT_EQUAL: return "!=";
                case SUP: return ">";
                case INF: return "<";
                case SUP_EQUAL: return ">=";
                case INF_EQUAL: return "<=";
                case ISTYPE: return "is";
                case RIGHT_ARROW: return "=>";
                case TINY_RIGHT_ARROW: return "->";
                case TERNARY: return "?";
                case OR: return "||";
                case AND: return "&&";
                case B_AND: return "&";
                case B_OR: return "|";
                case PLUS_PLUS: return "++";
                case MINUS_MINUS: return "--";
                case NOT: return "!";
                case NEW: return "new";
                case SCOPED: return "scoped";
                case PAREN_O: return "(";
                case PAREN_C: return ")";
                case BRACKET_O: return "[";
                case BRACKET_C: return "]";
                case BRACE_O: return "{";
                case BRACE_C: return "}";
                case COMMA: return ",";
                case DOT: return ".";
                case COLON: return ":";
                case DOUBLE_COLON: return "::";
                case SEMICOLON: return ";";
                case AT_SIGN: return "@";
                default: return "";
            }
        }

        Operator* Operator::getOperatorFromString(size_t pos, size_t len, const std::string& str){
            if(Operator::operators.count(str))
                return new Operator(pos, len, Operator::operators[str]);
            return NULL;
        }
        
        bool Operator::isValidOperator(const std::string& str){
            return Operator::operators.count(str);
        }

    }
}
