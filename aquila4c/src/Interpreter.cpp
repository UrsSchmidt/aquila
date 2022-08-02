#include "antlr4-runtime.h"
#include "Helper.h"
#include "Interpreter.h"

#include <sstream>

// XXX make this an option
#define STRICT true

#define POISON 0

#define TYPE_ANY  "Any"
#define TYPE_BOOL "Boolean"
#define TYPE_INT  "Integer"
#define TYPE_STR  "String"
#define TYPE_FUNC "Function"
#define TYPE_DICT "Dictionary"

#define isBool(x) ((x).has_value() && ((x).type() == typeid(Boolean)))
#define isInt(x)  ((x).has_value() && ((x).type() == typeid(Integer)))
#define isStr(x)  ((x).has_value() && ((x).type() == typeid(String)))
#define isFunc(x) ((x).has_value() && ((x).type() == typeid(Function)))
#define isDict(x) ((x).has_value() && ((x).type() == typeid(Dictionary)))

#define toBool(x) (std::any_cast<Boolean>(&(x)))
#define toInt(x)  (std::any_cast<Integer>(&(x)))
#define toStr(x)  (std::any_cast<String>(&(x)))
#define toFunc(x) (std::any_cast<Function>(&(x)))
#define toDict(x) (std::any_cast<Dictionary>(&(x)))

/* variables are shared between multiple instances
   to allow for function declarations to be used in different files */
std::stack<Dictionary*> variables;

std::string typeOf(Any o) {
    if (isBool(o)) {
        return TYPE_BOOL;
    } else if (isInt(o)) {
        return TYPE_INT;
    } else if (isStr(o)) {
        return TYPE_STR;
    } else if (isFunc(o)) {
        return TYPE_FUNC;
    } else if (isDict(o)) {
        return TYPE_DICT;
    } else {
        assert(0);
    }
}

void toBoolean(Boolean& result, String s, antlr4::ParserRuleContext* ctx) {
    result = strEquals(s, "true");
}

void toInteger(Integer& result, String s, antlr4::ParserRuleContext* ctx) {
    if (strIsNumeric(s)) {
        mpz_init_set_str(result, s.c_str(), 10);
    } else {
        mpz_init(result);
    }
    // FIXME when to call mpz_clear()?
}

void toString(String& result, Any o) {
    if (isBool(o)) {
        Boolean b = *toBool(o);
        result = b ? "true" : "false";
    } else if (isInt(o)) {
        String s(mpz_get_str(NULL, 10, *toInt(o)));
        result = s;
    } else if (isStr(o)) {
        String s = *toStr(o);
        result = s;
    } else if (isFunc(o)) {
        Function* f = toFunc(o);
        result = f->getText();
    } else if (isDict(o)) {
        Dictionary* d = toDict(o);
        std::stringstream ss;
        ss << "{" << std::endl;
        for (const auto& [key, value] : *d) {
            String keystr, valuestr;
            toString(keystr, key);
            ss << keystr << ": ";
            if (isStr(value)) {
                ss << "'" << *toStr(value) << "'";
            } else {
                toString(valuestr, value);
                ss << valuestr;
            }
            ss << "," << std::endl;
        }
        ss << "}";
        result = ss.str();
    } else {
        assert(0);
    }
}

void toDictionary(Dictionary& result, String s, antlr4::ParserRuleContext* ctx) {
    try {
        /* TODO
        AquilaLexer lexer = new AquilaLexer(CharStreams.fromString(s));
        AquilaParser parser = new AquilaParser(new CommonTokenStream(lexer));
        DictAggregateContext dictAggregate = parser.dictAggregate();
        if (parser.getNumberOfSyntaxErrors() == 0) {
            result = (Map) new Interpreter(new std::string[0]).visit(dictAggregate);
        }
        */
    } catch (...) {
        /* do nothing */
    }
    Dictionary d;
    result = d;
}

void error(std::string msg, antlr4::ParserRuleContext* ctx) {
    const auto token = ctx->getStart();
    std::cerr << "ERROR: " << msg << " @L" << token->getLine() << ":C" << token->getCharPositionInLine() << ":`" << ctx->getText() << "`" << std::endl;
    exit(1);
}

void exception(std::exception e, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "An exception occurred: `" << e.what() << "`";
    error(ss.str(), ctx);
}

/* TODO
void fileNotFound(File file, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "File not found! Expected: " << file;
    error(ss.str(), ctx);
}
*/

void missingKey(Dictionary* expectedMap, Any wasKey, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Missing key! Expected one of: ";
    for (Dictionary::iterator i = expectedMap->begin(); i != expectedMap->end(); i++) {
        String keystr;
        toString(keystr, i->first);
        ss << keystr << ", ";
    }
    String wasKeyStr;
    toString(wasKeyStr, wasKey);
    ss << "but was key `" << wasKeyStr << "`!";
    error(ss.str(), ctx);
}

void expectedNonVoidFunction(antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Expected non-void " << TYPE_FUNC << "!";
    error(ss.str(), ctx);
}

void typeMismatch(std::string expectedType, std::string wasType, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Type mismatch! Expected type `" << expectedType << "`, but was type `" << wasType << "`!";
    error(ss.str(), ctx);
}

void wrongKey(std::string expectedKey, Any wasKey, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    String wasKeyStr;
    toString(wasKeyStr, wasKey);
    ss << "Wrong key! Expected key `" << expectedKey << "`, but was key `" << wasKeyStr << "`!";
    error(ss.str(), ctx);
}

void wrongNumberOfArguments(int expected, int was, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Wrong number of arguments! Expected " << expected << ", but was " << was << "!";
    error(ss.str(), ctx);
}

/* TODO struct DictComparator {
    bool operator() (const Any& a1, const Any& a2) const {
        if (isInt(a1) && isInt(a2)) {
            return *toInt(a1) < *toInt(a2);
        } else {
            String s1, s2;
            toString(s1, a1);
            toString(s2, a2);
            if (strIsNumeric(s1) && strIsNumeric(s2)) {
                return std::stoi(s1) < std::stoi(s2);
            } else {
                return s1.compare(s2) < 0;
            }
        }
    }
}; */

bool anyEquals(Any a, Any b) {
    return (isBool(a) && isBool(b) && *toBool(a) == *toBool(b))
        || (isInt(a)  && isInt(b)  && mpz_cmp(*toInt(a), *toInt(b)) == 0)
        || (isStr(a)  && isStr(b)  && *toStr(a) == *toStr(b))
        || (isFunc(a) && isFunc(b) && toFunc(a) == toFunc(b))
        || (isDict(a) && isDict(b) && toDict(a) == toDict(b));
}

void Interpreter::assign(Dictionary* map, Any key, Any value, antlr4::ParserRuleContext* ctx) {
#ifdef STRICT
    if (map->count(key)) {
        const std::string expected = typeOf(map->at(key));
        const std::string was = typeOf(value);
        if (!strEquals(expected, was)) {
            typeMismatch(expected, was, ctx);
            return;
        }
    }
#endif
    map->at(key) = value;
}

void Interpreter::handleLhs(AquilaParser::LhsContext* lhsc, std::function<void(Dictionary*, Any)> handler) {
    const std::string identifier = lhsc->Identifier()->getText();
    if (lhsc->lhsPart().empty()) {
        handler(variables.top(), identifier);
        return;
    }
    if (!variables.top()->count(identifier)) {
        Dictionary d;
        variables.top()->at(identifier) = d;
    }
    Any result = variables.top()->at(identifier);
    for (size_t i = 0; i < lhsc->lhsPart().size(); i++) {
        AquilaParser::LhsPartContext* lhspc = lhsc->lhsPart()[i];
        Any key;
        if (lhspc->Identifier()) {
            key = lhspc->Identifier()->getText();
        } else if (lhspc->expression()) {
            key = visit(lhspc->expression());
        } else {
            assert(0);
        }
        if (i < lhsc->lhsPart().size() - 1) {
            if (isDict(result)) {
                Dictionary* map = toDict(result);
                if (map->count(key)) {
                    result = map->at(key);
                } else {
                    missingKey(map, key, lhspc);
                    return;
                }
            } else {
                typeMismatch(TYPE_DICT, typeOf(result), lhspc);
                return;
            }
        } else {
            if (isDict(result)) {
                Dictionary* map = toDict(result);
                handler(map, key);
            } else {
                typeMismatch(TYPE_DICT, typeOf(result), lhspc);
                return;
            }
        }
    }
}

Any Interpreter::callFunction(AquilaParser::FunctionCallContext* callsite, Function* function, std::vector<Any> arguments) {
    std::vector<std::string> parameters;
    std::vector<std::string> types;
    for (AquilaParser::LambdaExpressionParameterContext* lepc : function->lambdaExpressionParameter()) {
        parameters.push_back(lepc->Identifier()->getText());
        types.push_back(lepc->Type() ? lepc->Type()->getText() : TYPE_ANY);
    }
    /* TODO
    if (!checkArgs(callsite, arguments, types.toArray(new std::string[0]))) {
        assert(0);
    }
    */
    Dictionary d(*variables.top());
    variables.push(&d);
    for (size_t i = 0; i < parameters.size(); i++) {
        const std::string parameter = parameters[i];
        Any argument = arguments[i];
        // TODO does this have to be fully qualified?
        variables.top()->at(parameter) = argument;
    }
    Any result;
    if (function->expression()) {
        result = visit(function->expression());
    } else if (function->block()) {
        visit(function->block());
        result = 0;
    } else {
        assert(0);
    }
    variables.pop();
    return result;
}

/* TODO
bool Interpreter::checkArgs(AquilaParser::FunctionCallContext* callsite, std::vector<Any> arguments, std::string... types) {
    return checkArgsHelper(callsite, arguments, true, types);
}

bool Interpreter::checkArgsNoFail(AquilaParser::FunctionCallContext* callsite, std::vector<Any> arguments, std::string... types) {
    return checkArgsHelper(callsite, arguments, false, types);
}

bool Interpreter::checkArgsHelper(AquilaParser::FunctionCallContext* callsite, std::vector<Any> arguments, bool failOnTypeMismatch, std::string... types) {
    if (arguments.size() != types.length) {
        wrongNumberOfArguments(types.length, arguments.size(), callsite);
        return false;
    }
    for (size_t i = 0; i < types.length; i++) {
        const std::string type = types[i];
        Any argument = arguments[i];
        if (!type.equals(TYPE_ANY) && !typeOf(argument).equals(type)) {
            if (failOnTypeMismatch) {
                typeMismatch(type, typeOf(argument), callsite->arguments[i]);
            }
            return false;
        }
    }
    return true;
}
*/

Any Interpreter::visitProgram(AquilaParser::ProgramContext *ctx) {
    for (AquilaParser::StatementContext* sc : ctx->statement()) {
        try {
            visit(sc);
        } catch (std::exception& e) {
            exception(e, sc);
            assert(0);
        }
    }
    return POISON;
}

Any Interpreter::visitStatement(AquilaParser::StatementContext *ctx) {
    return visitChildren(ctx);
}

Any Interpreter::visitIfStatement(AquilaParser::IfStatementContext *ctx) {
    for (size_t i = 0; i < ctx->condition.size(); i++) {
        Any condition = visit(ctx->condition[i]);
        if (isBool(condition) && toBool(condition)) {
            visit(ctx->then[i]);
            return POISON;
        }
    }
    if (ctx->elseBlock) {
        visit(ctx->elseBlock);
    }
    return POISON;
}

Any Interpreter::visitSwitchStatement(AquilaParser::SwitchStatementContext *ctx) {
    Any switchHeadExpression = visit(ctx->switchHeadExpression);
    for (size_t i = 0; i < ctx->labels.size(); i++) {
        AquilaParser::SwitchStatementLabelsContext* sslc = ctx->labels[i];
        for (size_t j = 0; j < sslc->expression().size(); j++) {
            Any label = visit(sslc->expression()[j]);
            if (anyEquals(switchHeadExpression, label)) {
                visit(ctx->then[i]);
                return POISON;
            }
        }
    }
    if (ctx->defaultBlock) {
        visit(ctx->defaultBlock);
    }
    return POISON;
}

Any Interpreter::visitSwitchStatementLabels(AquilaParser::SwitchStatementLabelsContext *ctx) {
    assert(0);
}

Any Interpreter::visitLoopStatement(AquilaParser::LoopStatementContext *ctx) {
    while (true) {
        if (ctx->top) {
            visit(ctx->top);
        }
        Any condition = visit(ctx->expression());
        if (isBool(condition) && !toBool(condition)) {
            break;
        }
        if (ctx->bottom) {
            visit(ctx->bottom);
        }
    }
    return POISON;
}

Any Interpreter::visitForStatement(AquilaParser::ForStatementContext *ctx) {
    const std::string identifier = ctx->Identifier()->getText();
    Integer fromInt, toInt, stepInt, i;
    mpz_inits(fromInt, toInt, stepInt, i, NULL);
    /* from */
    Any from = visit(ctx->from);
    if (!isInt(from)) {
        typeMismatch(TYPE_INT, typeOf(from), ctx->from);
        assert(0);
    }
    mpz_set(fromInt, *toInt(from));
    /* to */
    Any to = visit(ctx->to);
    if (!isInt(to)) {
        typeMismatch(TYPE_INT, typeOf(to), ctx->to);
        assert(0);
    }
    mpz_set(toInt, *toInt(to));
    /* step */
    if (ctx->step) {
        Any step = visit(ctx->step);
        if (!isInt(step)) {
            typeMismatch(TYPE_INT, typeOf(step), ctx->step);
            assert(0);
        }
        mpz_set(stepInt, *toInt(step));
    } else {
        mpz_set_ui(stepInt, 1);
    }
    Any overriddenValue = variables.top()->at(identifier);
    for (mpz_set(i, fromInt); mpz_sgn(stepInt) >= 0 ? mpz_cmp(i, toInt) <= 0 : mpz_cmp(i, toInt) >= 0; mpz_add(i, i, stepInt)) {
        variables.top()->at(identifier) = i;
        visit(ctx->block());
    }
    variables.top()->at(identifier) = overriddenValue;
    mpz_clears(fromInt, toInt, stepInt, i, NULL);
    return POISON;
}

Any Interpreter::visitReadStatement(AquilaParser::ReadStatementContext *ctx) {
    std::string line;
    std::getline(std::cin, line);
    handleLhs(ctx->lhs(), [this, line, ctx](Dictionary* map, Any key) { assign(map, key, line, ctx); });
    return POISON;
}

Any Interpreter::visitWriteStatement(AquilaParser::WriteStatementContext *ctx) {
    Any rhs = visit(ctx->rhs);
    if (isStr(rhs)) {
        std::cout << toStr(rhs) << std::endl;
    } else {
        typeMismatch(TYPE_STR, typeOf(rhs), ctx->rhs);
        assert(0);
    }
    return POISON;
}

Any Interpreter::visitAssignStatement(AquilaParser::AssignStatementContext *ctx) {
    Any rhs = visit(ctx->rhs);
    handleLhs(ctx->lhs(), [this, rhs, ctx](Dictionary* map, Any key) { assign(map, key, rhs, ctx->rhs); });
    return POISON;
}

Any Interpreter::visitCallStatement(AquilaParser::CallStatementContext *ctx) {
    visit(ctx->functionCall());
    return POISON;
}

Any Interpreter::visitRemoveStatement(AquilaParser::RemoveStatementContext *ctx) {
    handleLhs(ctx->lhs(), [this, ctx](Dictionary* map, Any key) {
        if (map->count(key)) {
            map->erase(key);
        } else {
            missingKey(map, key, ctx);
        }
    });
    return POISON;
}

Any Interpreter::visitRunStatement(AquilaParser::RunStatementContext *ctx) {
    Any rhs = visit(ctx->rhs);
    if (isStr(rhs)) {
        /* TODO
        const File file = new File(scriptRoot, toStr(rhs));
        if (file.exists()) {
            try {
                // FIXME pass further arguments
                Main.run(new std::string[] { file.toString() });
            } catch (IOException e) {
                exception(e, ctx->rhs);
                assert(0);
            }
        } else {
            fileNotFound(file, ctx->rhs);
            assert(0);
        }
        */
    } else {
        typeMismatch(TYPE_STR, typeOf(rhs), ctx->rhs);
        assert(0);
    }
    return POISON;
}

Any Interpreter::visitBlock(AquilaParser::BlockContext *ctx) {
    for (AquilaParser::StatementContext* sc : ctx->statement()) {
        visit(sc);
    }
    return POISON;
}

Any Interpreter::visitLhs(AquilaParser::LhsContext *ctx) {
    assert(0);
}

Any Interpreter::visitLhsPart(AquilaParser::LhsPartContext *ctx) {
    assert(0);
}

Any Interpreter::visitExpression(AquilaParser::ExpressionContext *ctx) {
    return visitChildren(ctx);
}

Any Interpreter::visitIfExpression(AquilaParser::IfExpressionContext *ctx) {
    for (size_t i = 0; i < ctx->condition.size(); i++) {
        Any condition = visit(ctx->condition[i]);
        if (isBool(condition) && toBool(condition)) {
            return visit(ctx->then[i]);
        }
    }
    return visit(ctx->elseExpression);
}

Any Interpreter::visitLetExpression(AquilaParser::LetExpressionContext *ctx) {
    Dictionary d(*variables.top());
    variables.push(&d);
    for (AquilaParser::LetBindExpressionContext* lbec : ctx->letBindExpression()) {
        Any rhs = visit(lbec->rhs);
        handleLhs(lbec->lhs(), [this, rhs, lbec](Dictionary* map, Any key) { assign(map, key, rhs, lbec); });
    }
    Any result = visit(ctx->body);
    variables.pop();
    return result;
}

Any Interpreter::visitLetBindExpression(AquilaParser::LetBindExpressionContext *ctx) {
    assert(0);
}

Any Interpreter::visitSwitchExpression(AquilaParser::SwitchExpressionContext *ctx) {
    Any switchHeadExpression = visit(ctx->switchHeadExpression);
    for (size_t i = 0; i < ctx->labels.size(); i++) {
        AquilaParser::SwitchExpressionLabelsContext* selc = ctx->labels[i];
        for (size_t j = 0; j < selc->expression().size(); j++) {
            Any label = visit(selc->expression()[j]);
            if (anyEquals(switchHeadExpression, label)) {
                return visit(ctx->then[i]);
            }
        }
    }
    return visit(ctx->defaultExpression);
}

Any Interpreter::visitSwitchExpressionLabels(AquilaParser::SwitchExpressionLabelsContext *ctx) {
    assert(0);
}

Any Interpreter::visitLogicalOperation(AquilaParser::LogicalOperationContext *ctx) {
    Any result = visit(ctx->unaryLogicalOperation(0));
    for (size_t i = 0; i < ctx->logicalOperator().size(); i++) {
        AquilaParser::LogicalOperatorContext* loc = ctx->logicalOperator(i);
        AquilaParser::UnaryLogicalOperationContext* uloc = ctx->unaryLogicalOperation(i + 1);
        std::string op = loc->getText();
        Any operand = visit(uloc);
        if (isBool(result) && isBool(operand)) {
            if (strEquals(op, "and")) {
                result = *toBool(result) && *toBool(operand);
            } else if (strEquals(op, "or")) {
                result = *toBool(result) || *toBool(operand);
            } else if (strEquals(op, "xor")) {
                result = *toBool(result) ^ *toBool(operand);
            } else {
                assert(0);
            }
        } else {
            std::stringstream ss1, ss2;
            ss1 << TYPE_BOOL << " and " << TYPE_BOOL;
            ss2 << typeOf(result) << " and " << typeOf(operand);
            typeMismatch(ss1.str(), ss2.str(), ctx);
            assert(0);
        }
    }
    return result;
}

Any Interpreter::visitLogicalOperator(AquilaParser::LogicalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitUnaryLogicalOperation(AquilaParser::UnaryLogicalOperationContext *ctx) {
    Any result = visit(ctx->relation());
    if (ctx->unaryLogicalOperator()) {
        if (isBool(result)) {
            std::string op = ctx->unaryLogicalOperator()->getText();
            if (strEquals(op, "not")) {
                result = !*toBool(result);
            } else {
                assert(0);
            }
        } else {
            typeMismatch(TYPE_BOOL, typeOf(result), ctx->relation());
            assert(0);
        }
    }
    return result;
}

Any Interpreter::visitUnaryLogicalOperator(AquilaParser::UnaryLogicalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitRelation(AquilaParser::RelationContext *ctx) {
    Any result = visit(ctx->addition(0));
    if (ctx->relationalOperator() && ctx->addition().size() == 2) {
        AquilaParser::RelationalOperatorContext* roc = ctx->relationalOperator();
        AquilaParser::AdditionContext* ac = ctx->addition(1);
        std::string op = roc->getText();
        Any operand = visit(ac);
        if (strEquals(op, ":")) {
            if (isDict(operand)) {
                Dictionary* d = toDict(operand);
                result = false;
                for (Dictionary::iterator i = d->begin(); i != d->end(); i++) {
                    if (anyEquals(i->second, result)) {
                        result = true;
                        break;
                    }
                }
            } else {
                typeMismatch(TYPE_DICT, typeOf(operand), ac);
                assert(0);
            }
        } else if (strEquals(op, "<=") || strEquals(op, "<>") || strEquals(op, "<") || strEquals(op, "=") || strEquals(op, ">=") || strEquals(op, ">")) {
            if (isInt(result) && isInt(operand)) {
                if (strEquals(op, "<=")) {
                    result = mpz_cmp(*toInt(result), *toInt(operand)) <= 0;
                } else if (strEquals(op, "<>")) {
                    result = mpz_cmp(*toInt(result), *toInt(operand)) != 0;
                } else if (strEquals(op, "<")) {
                    result = mpz_cmp(*toInt(result), *toInt(operand)) < 0;
                } else if (strEquals(op, "=")) {
                    result = mpz_cmp(*toInt(result), *toInt(operand)) == 0;
                } else if (strEquals(op, ">=")) {
                    result = mpz_cmp(*toInt(result), *toInt(operand)) >= 0;
                } else if (strEquals(op, ">")) {
                    result = mpz_cmp(*toInt(result), *toInt(operand)) > 0;
                } else {
                    assert(0);
                }
            } else {
                std::stringstream ss1, ss2;
                ss1 << TYPE_INT << " and " << TYPE_INT;
                ss2 << typeOf(result) << " and " << typeOf(operand);
                typeMismatch(ss1.str(), ss2.str(), ctx);
                assert(0);
            }
        } else if (strEquals(op, "eq") || strEquals(op, "ew") || strEquals(op, "in") || strEquals(op, "ne") || strEquals(op, "sw")) {
            if (isStr(result) && isStr(operand)) {
                if (strEquals(op, "eq")) {
                    result = strEquals(*toStr(result), *toStr(operand));
                } else if (strEquals(op, "ew")) {
                    result = strEndsWith(*toStr(result), *toStr(operand));
                } else if (strEquals(op, "in")) {
                    result = strContains(*toStr(operand), *toStr(result));
                } else if (strEquals(op, "ne")) {
                    result = !strEquals(*toStr(result), *toStr(operand));
                } else if (strEquals(op, "sw")) {
                    result = strStartsWith(*toStr(result), *toStr(operand));
                } else {
                    assert(0);
                }
            } else {
                std::stringstream ss1, ss2;
                ss1 << TYPE_STR << " and " << TYPE_STR;
                ss2 << typeOf(result) << " and " << typeOf(operand);
                typeMismatch(ss1.str(), ss2.str(), ctx);
                assert(0);
            }
        } else {
            assert(0);
        }
    }
    return result;
}

Any Interpreter::visitRelationalOperator(AquilaParser::RelationalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitAddition(AquilaParser::AdditionContext *ctx) {
    Any result = visit(ctx->multiplication(0));
    for (size_t i = 0; i < ctx->additionalOperator().size(); i++) {
        AquilaParser::AdditionalOperatorContext* aoc = ctx->additionalOperator(i);
        AquilaParser::MultiplicationContext* mc = ctx->multiplication(i + 1);
        std::string op = aoc->getText();
        Any operand = visit(mc);
        if (strEquals(op, "&")) {
            if (isStr(result) && isStr(operand)) {
                std::stringstream ss;
                ss << *toStr(result) << *toStr(operand);
                result = ss.str();
            } else {
                std::stringstream ss1, ss2;
                ss1 << TYPE_STR << " and " << TYPE_STR;
                ss2 << typeOf(result) << " and " << typeOf(operand);
                typeMismatch(ss1.str(), ss2.str(), ctx);
                assert(0);
            }
        } else if (strEquals(op, "+") || strEquals(op, "-")) {
            if (isInt(result) && isInt(operand)) {
                Integer r;
                mpz_init(r);
                if (strEquals(op, "+")) {
                    mpz_add(r, *toInt(result), *toInt(operand));
                } else if (strEquals(op, "-")) {
                    mpz_sub(r, *toInt(result), *toInt(operand));
                } else {
                    assert(0);
                }
                result = r;
                mpz_clear(r);
            } else {
                std::stringstream ss1, ss2;
                ss1 << TYPE_INT << " and " << TYPE_INT;
                ss2 << typeOf(result) << " and " << typeOf(operand);
                typeMismatch(ss1.str(), ss2.str(), ctx);
                assert(0);
            }
        } else {
            assert(0);
        }
    }
    return result;
}

Any Interpreter::visitAdditionalOperator(AquilaParser::AdditionalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitMultiplication(AquilaParser::MultiplicationContext *ctx) {
    Any result = visit(ctx->unaryAddition(0));
    for (size_t i = 0; i < ctx->multiplicationOperator().size(); i++) {
        AquilaParser::MultiplicationOperatorContext* moc = ctx->multiplicationOperator(i);
        AquilaParser::UnaryAdditionContext* uac = ctx->unaryAddition(i + 1);
        std::string op = moc->getText();
        Any operand = visit(uac);
        if (isInt(result) && isInt(operand)) {
            Integer r;
            mpz_init(r);
            if (strEquals(op, "*")) {
                mpz_mul(r, *toInt(result), *toInt(operand));
            } else if (strEquals(op, "/")) {
                mpz_fdiv_q(r, *toInt(result), *toInt(operand));
            } else if (strEquals(op, "mod")) {
                // FIXME double check fdiv_r() behavior
                mpz_fdiv_r(r, *toInt(result), *toInt(operand));
            } else if (strEquals(op, "rem")) {
                // FIXME double check mod() behavior
                mpz_mod(r, *toInt(result), *toInt(operand));
            } else {
                assert(0);
            }
            result = r;
            mpz_clear(r);
        } else {
            std::stringstream ss1, ss2;
            ss1 << TYPE_INT << " and " << TYPE_INT;
            ss2 << typeOf(result) << " and " << typeOf(operand);
            typeMismatch(ss1.str(), ss2.str(), ctx);
            assert(0);
        }
    }
    return result;
}

Any Interpreter::visitMultiplicationOperator(AquilaParser::MultiplicationOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitUnaryAddition(AquilaParser::UnaryAdditionContext *ctx) {
    Any result = visit(ctx->factorial());
    if (ctx->unaryAdditionalOperator()) {
        if (isInt(result)) {
            std::string op = ctx->unaryAdditionalOperator()->getText();
            Integer r;
            mpz_init(r);
            if (strEquals(op, "+")) {
                /* do nothing */
            } else if (strEquals(op, "-")) {
                mpz_neg(r, *toInt(result));
            } else {
                assert(0);
            }
            result = r;
            mpz_clear(r);
        } else {
            typeMismatch(TYPE_INT, typeOf(result), ctx->factorial());
            assert(0);
        }
    }
    return result;
}

Any Interpreter::visitUnaryAdditionalOperator(AquilaParser::UnaryAdditionalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitFactorial(AquilaParser::FactorialContext *ctx) {
    Any result = visit(ctx->factor());
    if (ctx->factorialOperator()) {
        if (isInt(result)) {
            std::string op = ctx->factorialOperator()->getText();
            Integer r;
            mpz_init(r);
            if (strEquals(op, "!")) {
                mpz_fac_ui(r, mpz_get_ui(*toInt(result)));
            } else {
                assert(0);
            }
            result = r;
            mpz_clear(r);
        } else {
            typeMismatch(TYPE_INT, typeOf(result), ctx->factor());
            assert(0);
        }
    }
    return result;
}

Any Interpreter::visitFactorialOperator(AquilaParser::FactorialOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitFactor(AquilaParser::FactorContext *ctx) {
    if (ctx->bracketExpression()) {
        return visit(ctx->bracketExpression());
    } else if (ctx->absExpression()) {
        return visit(ctx->absExpression());
    } else if (ctx->dictAggregate()) {
        return visit(ctx->dictAggregate());
    } else if (ctx->lambdaExpression()) {
        return visit(ctx->lambdaExpression());
    } else if (ctx->literal()) {
        return visit(ctx->literal());
    } else if (ctx->functionCall()) {
        Any result = visit(ctx->functionCall());
        /* TODO if (result == POISON) {
            expectedNonVoidFunction(ctx->functionCall());
            assert(0);
        } */
        return result;
    } else if (ctx->accessExpression()) {
        return visit(ctx->accessExpression());
    } else {
        assert(0);
    }
}

Any Interpreter::visitBracketExpression(AquilaParser::BracketExpressionContext *ctx) {
    return visit(ctx->expression());
}

Any Interpreter::visitAbsExpression(AquilaParser::AbsExpressionContext *ctx) {
    Any result = visit(ctx->expression());
    if (isInt(result)) {
        Integer r;
        mpz_init(r);
        mpz_abs(r, *toInt(result));
        result = r;
        mpz_clear(r);
    } else {
        typeMismatch(TYPE_INT, typeOf(result), ctx->expression());
        assert(0);
    }
    return result;
}

Any Interpreter::visitDictAggregate(AquilaParser::DictAggregateContext *ctx) {
    Dictionary d;
    for (size_t i = 0; i < ctx->dictAggregatePair().size(); i++) {
        AquilaParser::DictAggregatePairContext* mapc = ctx->dictAggregatePair(i);
        Any key;
        if (mapc->key) {
            key = visit(mapc->key);
        } else {
            Integer one;
            mpz_init_set_ui(one, (unsigned long int) i);
            key = one;
            // FIXME when to call mpz_clear()?
        }
        Any value = visit(mapc->value);
        d.at(key) = value;
    }
    return d;
}

Any Interpreter::visitDictAggregatePair(AquilaParser::DictAggregatePairContext *ctx) {
    assert(0);
}

Any Interpreter::visitDictAggregateKey(AquilaParser::DictAggregateKeyContext *ctx) {
    if (ctx->Identifier()) {
        return ctx->Identifier()->getText();
    } else if (ctx->expression()) {
        return visit(ctx->expression());
    } else {
        assert(0);
    }
}

Any Interpreter::visitLambdaExpression(AquilaParser::LambdaExpressionContext *ctx) {
    return ctx;
}

Any Interpreter::visitLambdaExpressionParameter(AquilaParser::LambdaExpressionParameterContext *ctx) {
    assert(0);
}

Any Interpreter::visitLiteral(AquilaParser::LiteralContext *ctx) {
    if (ctx->String()) {
        return strDropQuotes(ctx->String()->getText());
    } else if (ctx->Integer()) {
        std::string s = ctx->Integer()->getText();
        int radix = 10;
        if (!strIsNumeric(s)) {
            if (strStartsWith(s, "0b")) {
                radix = 2;
            } else if (strStartsWith(s, "0o")) {
                radix = 8;
            } else if (strStartsWith(s, "0x")) {
                radix = 16;
            } else {
                assert(0);
            }
            s = s.substr(2);
        }
        Integer i;
        mpz_init_set_str(i, s.c_str(), radix);
        return i;
        // FIXME when to call mpz_clear()?
    } else if (ctx->False()) {
        return false;
    } else if (ctx->True()) {
        return true;
    } else {
        assert(0);
    }
}

Any Interpreter::visitFunctionCall(AquilaParser::FunctionCallContext *ctx) {
    const std::string identifier = ctx->Identifier()->getText();
    std::vector<Any> arguments;
    for (AquilaParser::ExpressionContext* ec : ctx->arguments) {
        arguments.push_back(visit(ec));
    }
    Any result;
    /* TODO
    switch (identifier) {
    case "bool2str": {
        if (!checkArgs(ctx, arguments, TYPE_BOOL)) {
            assert(0);
        }
        const Boolean* arg1 = toBool(arguments.get(0));
        toString(result, arg1);
    }   break;
    case "boolean":
        result = checkArgsNoFail(ctx, arguments, TYPE_BOOL);
        break;
    case "char2ord": {
        if (!checkArgs(ctx, arguments, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        result = BigInteger.valueOf((long) arg1.charAt(0));
    }   break;
    case "charat": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const Integer* arg2 = toInt(arguments.get(1));
        result = Character.toString(arg1.charAt(arg2.intValueExact()));
    }   break;
    case "dict2str": {
        if (!checkArgs(ctx, arguments, TYPE_DICT)) {
            assert(0);
        }
        const Dictionary* arg1 = toDict(arguments.get(0));
        toString(result, arg1);
    }   break;
    case "dictionary":
        result = checkArgsNoFail(ctx, arguments, TYPE_DICT);
        break;
    case "error": {
        if (!checkArgs(ctx, arguments, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        error(arg1, ctx);
        result = arg1;
    }   break;
    case "exists": {
        if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
            assert(0);
        }
        const Dictionary* arg1 = toDict(arguments.get(0));
        const Function* arg2 = toFunc(arguments.get(1));
        const std::vector<std::any> localArguments;
        result = false;
        for (std::any key : arg1.keySet()) {
            const std::any value = arg1[key];
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            if (Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                result = true;
                break;
            }
        }
    }   break;
    case "exit": {
        if (!checkArgs(ctx, arguments, TYPE_INT)) {
            assert(0);
        }
        const Integer* arg1 = toInt(arguments.get(0));
        result = arg1;
        System.exit(arg1.intValueExact());
    }   break;
    case "filter": {
        if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
            assert(0);
        }
        const Dictionary* arg1 = toDict(arguments.get(0));
        const Function* arg2 = toFunc(arguments.get(1));
        const std::vector<std::any> localArguments;
        const Dictionary resultMap = new TreeMap<>(DICT_COMPARATOR);
        for (std::any key : arg1.keySet()) {
            const std::any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            if (Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                resultMap[key] = value;
            }
        }
        result = resultMap;
    }   break;
    case "findleft": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const String* arg2 = toStr(arguments.get(1));
        const Integer* arg3 = toInt(arguments.get(2));
        result = BigInteger.valueOf(arg1.indexOf(arg2, arg3.intValueExact()));
    }   break;
    case "findright": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const String* arg2 = toStr(arguments.get(1));
        const Integer* arg3 = toInt(arguments.get(2));
        result = BigInteger.valueOf(arg1.lastIndexOf(arg2, arg3.intValueExact()));
    }   break;
    case "fold": {
        if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_ANY, TYPE_FUNC)) {
            assert(0);
        }
        const Dictionary* arg1 = toDict(arguments.get(0));
        const Any arg2 = arguments.get(1);
        const Function* arg3 = toFunc(arguments.get(2));
        const std::vector<std::any> localArguments;
        result = arg2;
        for (std::any key : arg1.keySet()) {
            const std::any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(result);
            localArguments.push_back(value);
            result = callFunction(ctx, arg3, localArguments);
        }
    }   break;
    case "forall": {
        if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
            assert(0);
        }
        const Dictionary* arg1 = toDict(arguments.get(0));
        const Function* arg2 = toFunc(arguments.get(1));
        const std::vector<std::any> localArguments;
        result = true;
        for (std::any key : arg1.keySet()) {
            const std::any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            if (!Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                result = false;
                break;
            }
        }
    }   break;
    case "foreach": {
        if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
            assert(0);
        }
        const Dictionary* arg1 = toDict(arguments.get(0));
        const Function* arg2 = toFunc(arguments.get(1));
        const std::vector<std::any> localArguments;
        for (std::any key : arg1.keySet()) {
            const std::any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            callFunction(ctx, arg2, localArguments);
        }
        result = null;
    }   break;
    case "function":
        result = checkArgsNoFail(ctx, arguments, TYPE_FUNC);
        break;
    case "gcd": {
        if (!checkArgs(ctx, arguments, TYPE_INT, TYPE_INT)) {
            assert(0);
        }
        const Integer* arg1 = toInt(arguments.get(0));
        const Integer* arg2 = toInt(arguments.get(1));
        result = arg1.gcd(arg2);
    }   break;
    case "head": {
        if (!checkArgs(ctx, arguments, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        result = arg1.isEmpty() ? "" : arg1.substr(0, 1);
    }   break;
    case "int2str": {
        if (!checkArgs(ctx, arguments, TYPE_INT)) {
            assert(0);
        }
        const Integer* arg1 = toInt(arguments.get(0));
        toString(result, arg1);
    }   break;
    case "integer":
        result = checkArgsNoFail(ctx, arguments, TYPE_INT);
        break;
    case "join": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_DICT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const Dictionary* arg2 = toDict(arguments.get(1));
        result = std::string.join(arg1, arg2.values());
    }   break;
    case "left": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const Integer* arg2 = toInt(arguments.get(1));
        result = arg1.substr(0, arg2.intValueExact());
    }   break;
    case "length": {
        if (!checkArgs(ctx, arguments, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        result = BigInteger.valueOf(arg1.length());
    }   break;
    case "map": {
        if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
            assert(0);
        }
        const Dictionary* arg1 = toDict(arguments.get(0));
        const Function* arg2 = toFunc(arguments.get(1));
        const std::vector<std::any> localArguments;
        const Dictionary resultMap = new TreeMap<>(DICT_COMPARATOR);
        for (std::any key : arg1.keySet()) {
            const std::any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            const std::any newValue = callFunction(ctx, arg2, localArguments);
            resultMap[key] = newValue;
        }
        result = resultMap;
    }   break;
    case "mid": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const Integer* arg2 = toInt(arguments.get(1));
        const Integer* arg3 = toInt(arguments.get(2));
        result = arg1.substr(arg2.intValueExact(), arg2.intValueExact() + arg3.intValueExact());
    }   break;
    case "ord2char": {
        if (!checkArgs(ctx, arguments, TYPE_INT)) {
            assert(0);
        }
        const Integer* arg1 = toInt(arguments.get(0));
        result = Character.toString((char) arg1.intValueExact());
    }   break;
    case "pow": {
        if (!checkArgs(ctx, arguments, TYPE_INT, TYPE_INT)) {
            assert(0);
        }
        const Integer* arg1 = toInt(arguments.get(0));
        const Integer* arg2 = toInt(arguments.get(1));
        result = arg1.pow(arg2.intValueExact());
    }   break;
    case "repeat": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const Integer* arg2 = toInt(arguments.get(1));
        result = arg1.repeat(arg2.intValueExact());
    }   break;
    case "replace": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const String* arg2 = toStr(arguments.get(1));
        const String* arg3 = toStr(arguments.get(2));
        result = arg1.replace(arg2, arg3);
    }   break;
    case "right": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const Integer* arg2 = toStr(arguments.get(1));
        result = arg1.substr(arg1.length() - arg2.intValueExact());
    }   break;
    case "sgn": {
        if (!checkArgs(ctx, arguments, TYPE_INT)) {
            assert(0);
        }
        const Integer* arg1 = toInt(arguments.get(0));
        result = BigInteger.valueOf(arg1.signum());
    }   break;
    case "size": {
        if (!checkArgs(ctx, arguments, TYPE_DICT)) {
            assert(0);
        }
        const Dictionary* arg1 = toDict(arguments.get(0));
        result = BigInteger.valueOf(arg1.size());
    }   break;
    case "sleep": {
        if (!checkArgs(ctx, arguments, TYPE_INT)) {
            assert(0);
        }
        const Integer* arg1 = toInt(arguments.get(0));
        try {
            Thread.sleep(arg1.longValueExact());
            result = true;
        } catch (InterruptedException e) {
            result = false;
        }
    }   break;
    case "split": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const String* arg2 = toStr(arguments.get(1));
        const String*[] parts = arg2.split(arg1, -1);
        const Dictionary resultMap = new TreeMap<>(DICT_COMPARATOR);
        for (size_t i = 0; i < parts.length; i++) {
            resultMap[BigInteger.valueOf(i)] = parts[i];
        }
        result = resultMap;
    }   break;
    case "sqrt": {
        if (!checkArgs(ctx, arguments, TYPE_INT)) {
            assert(0);
        }
        const Integer* arg1 = toInt(arguments.get(0));
        result = arg1.sqrt();
    }   break;
    case "str2bool": {
        if (!checkArgs(ctx, arguments, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        toBoolean(result, arg1, ctx);
    }   break;
    case "str2dict": {
        if (!checkArgs(ctx, arguments, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        toDictionary(result, arg1, ctx);
    }   break;
    case "str2int": {
        if (!checkArgs(ctx, arguments, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        toInteger(result, arg1, ctx);
    }   break;
    case "string":
        result = checkArgsNoFail(ctx, arguments, TYPE_STR);
        break;
    case "substring1": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const Integer* arg2 = toInt(arguments.get(1));
        result = arg1.substr(arg2.intValueExact());
    }   break;
    case "substring2": {
        if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT, TYPE_INT)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        const Integer* arg2 = toInt(arguments.get(1));
        const Integer* arg3 = toInt(arguments.get(2));
        result = arg1.substr(arg2.intValueExact(), arg3.intValueExact());
    }   break;
    case "tail": {
        if (!checkArgs(ctx, arguments, TYPE_STR)) {
            assert(0);
        }
        const String* arg1 = toStr(arguments.get(0));
        result = arg1.isEmpty() ? "" : arg1.substr(1);
    }   break;
    default:
    */
        if (variables.top()->count(identifier)) {
            Any object = variables.top()->at(identifier);
            if (isFunc(object)) {
                Function* function = toFunc(object);
                result = callFunction(ctx, function, arguments);
            } else {
                typeMismatch(TYPE_FUNC, typeOf(object), ctx);
                assert(0);
            }
        } else {
            missingKey(variables.top(), identifier, ctx);
            assert(0);
        }
    //}
    return result;
}

Any Interpreter::visitAccessExpression(AquilaParser::AccessExpressionContext *ctx) {
    const std::string identifier = ctx->Identifier()->getText();
    if (!variables.top()->count(identifier)) {
        Dictionary d;
        variables.top()->at(identifier) = d;
    }
    Any result = variables.top()->at(identifier);
    for (AquilaParser::AccessExpressionPartContext* aepc : ctx->accessExpressionPart()) {
        Any key;
        if (aepc->Identifier()) {
            key = aepc->Identifier()->getText();
        } else if (aepc->expression()) {
            key = visit(aepc->expression());
        } else {
            assert(0);
        }
        if (isDict(result)) {
            Dictionary* map = toDict(result);
            if (map->count(key)) {
                result = map->at(key);
            } else {
                missingKey(map, key, aepc);
                assert(0);
            }
        } else {
            typeMismatch(TYPE_DICT, typeOf(result), aepc);
            assert(0);
        }
    }
    return result;
}

Any Interpreter::visitAccessExpressionPart(AquilaParser::AccessExpressionPartContext *ctx) {
    assert(0);
}
