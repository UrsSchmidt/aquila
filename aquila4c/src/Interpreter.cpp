#include "antlr4-runtime.h"
#include "Helper.h"
#include "Interpreter.h"

#include <sstream>

//#define DEBUG true

#ifdef DEBUG
#define DEBUG_HR()     do { std::cerr << "-------------------------" << std::endl; } while (0)
#define DEBUG_LOG(s)   do { std::cerr << "LOG " << __FUNCTION__ << ": " << s << std::endl; } while (0)
#define DEBUG_CTX()    do { std::cerr << "CTX " << __FUNCTION__ << ": " << toString(*variables.back()) << std::endl; } while (0)
#define DEBUG_BGN()    do { std::cerr << "BGN " << __FUNCTION__ <<  "()" << std::endl; } while (0)
#define DEBUG_BGN_P(p) do { std::cerr << "BGN " << __FUNCTION__ <<  "(" << p << ")" << std::endl; } while (0)
#define DEBUG_END()    do { std::cerr << "END " << __FUNCTION__ << std::endl; } while (0)
#define DEBUG_END_R(r) do { std::cerr << "END " << __FUNCTION__ << " (result=" << r << ")" << std::endl; } while (0)
#else
#define DEBUG_HR()     /*empty*/
#define DEBUG_LOG(s)   /*empty*/
#define DEBUG_CTX()    /*empty*/
#define DEBUG_BGN()    /*empty*/
#define DEBUG_BGN_P(p) /*empty*/
#define DEBUG_END()    /*empty*/
#define DEBUG_END_R(r) /*empty*/
#endif

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
std::vector<Dictionary*> variables;

std::string typeOf(const Any& o) {
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
        return "<err:invalid-type>";
    }
}

Boolean toBoolean(const String& s, antlr4::ParserRuleContext* ctx) {
    return strEquals(s, "true");
}

Integer toInteger(const String& s, antlr4::ParserRuleContext* ctx) {
    Integer result;
    if (strIsNumeric(s)) {
        mpz_init_set_str(result.i, s.c_str(), 10);
    } else {
        mpz_init(result.i);
    }
    return result;
}

String toString(const Any& o) {
    String result;
    if (!o.has_value()) {
        result = "<err:no-value>";
    } else if (isBool(o)) {
        Boolean b = *toBool(o);
        result = b ? "true" : "false";
    } else if (isInt(o)) {
        String s(mpz_get_str(NULL, 10, toInt(o)->i));
        result = s;
    } else if (isStr(o)) {
        String s = *toStr(o);
        result = s;
    } else if (isFunc(o)) {
        Function* f = toFunc((Any&) o);
        result = f->getText();
    } else if (isDict(o)) {
        Dictionary d = *toDict((Any&) o);
        std::stringstream ss;
        ss << "{" << std::endl;
        for (const auto& [key, value] : d) {
            ss << key << ": ";
            if (isStr(value)) {
                ss << "'" << *toStr(value) << "'";
            } else {
                ss << toString(value);
            }
            ss << "," << std::endl;
        }
        ss << "}";
        result = ss.str();
    } else {
        result = "<err:invalid-type>";
    }
    return result;
}

Dictionary toDictionary(const String& s, antlr4::ParserRuleContext* ctx) {
    Dictionary result;
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
    return result;
}

bool DictComparator::operator() (const std::any& a1, const std::any& a2) const {
    bool result;
    if (isInt(a1) && isInt(a2)) {
        result = mpz_cmp(toInt(a1)->i, toInt(a2)->i) < 0;
    } else {
        String s1 = toString(a1);
        String s2 = toString(a2);
        if (strIsNumeric(s1) && strIsNumeric(s2)) {
            result = std::stoi(s1) < std::stoi(s2);
        } else {
            result = s1.compare(s2) < 0;
        }
    }
    return result;
}

bool anyEquals(const Any& a, const Any& b) {
    DEBUG_BGN();
    bool result =
           (isBool(a) && isBool(b) && *toBool(a) == *toBool(b))
        || (isInt(a)  && isInt(b)  && mpz_cmp(toInt(a)->i, toInt(b)->i) == 0)
        || (isStr(a)  && isStr(b)  && strEquals(*toStr(a), *toStr(b)))
        || (isFunc(a) && isFunc(b) && toFunc(a) == toFunc(b))
        || (isDict(a) && isDict(b) && toDict(a) == toDict(b));
    DEBUG_END_R(result);
    return result;
}

void error(const std::string& msg, antlr4::ParserRuleContext* ctx) {
    const auto token = ctx->getStart();
    std::cerr << "ERROR: " << msg << " @L" << token->getLine() << ":C" << token->getCharPositionInLine() << ":`" << ctx->getText() << "`" << std::endl;
    exit(1);
}

void exception(const std::exception& e, antlr4::ParserRuleContext* ctx) {
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

void missingKey(const Dictionary& expectedMap, const std::string& wasKey, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Missing key! Expected one of: ";
    for (const auto& [key, value] : expectedMap) {
        ss << key << ", ";
    }
    ss << "but was key `" << wasKey << "`!";
    error(ss.str(), ctx);
}

void expectedNonVoidFunction(antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Expected non-void " << TYPE_FUNC << "!";
    error(ss.str(), ctx);
}

void typeMismatch(const std::string& expectedType, const Any& wasValue, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Type mismatch! Expected type `" << expectedType << "`, but was `" << toString(wasValue) << "` which is of type `" << typeOf(wasValue) << "`!";
    error(ss.str(), ctx);
}

void wrongKey(const std::string& expectedKey, const std::string& wasKey, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Wrong key! Expected key `" << expectedKey << "`, but was key `" << wasKey << "`!";
    error(ss.str(), ctx);
}

void wrongNumberOfArguments(int expected, int was, antlr4::ParserRuleContext* ctx) {
    std::stringstream ss;
    ss << "Wrong number of arguments! Expected " << expected << ", but was " << was << "!";
    error(ss.str(), ctx);
}

void assign(Dictionary* d, const std::string& key, const Any& value, antlr4::ParserRuleContext* ctx) {
    DEBUG_BGN_P("d=" << toString(*d) << ", key=" << key << ", value=" << toString(value));
#ifdef STRICT
    if (d->count(key)) {
        DEBUG_LOG("map.count(key)");
        const std::string expected = typeOf((*d)[key]);
        const std::string was = typeOf(value);
        if (!strEquals(expected, was)) {
            typeMismatch(expected, value, ctx);
            return;
        }
    }
#endif
    (*d)[key] = value;
    DEBUG_LOG("d=" << toString(*d));
    DEBUG_END();
}

Interpreter::Interpreter(int argc, char* argv[]) {
    DEBUG_BGN_P("argc=" << argc);
    // TODO scriptRoot = argc > 0 ? argv[0] : "";
    variables.push_back(new Dictionary());
    /* passing command line arguments */
    /* TODO
    Map argsMap = new TreeMap<>(DICT_COMPARATOR);
    for (int i = 0; i < args.length; i++) {
        argsMap.put(BigInteger.valueOf(i), args[i]);
    }
    variables.peek().put("args", argsMap);
    */
    /* passing environment variables */
    /* TODO
    Map envMap = new TreeMap<>(DICT_COMPARATOR);
    final Map<String, String> env = System.getenv();
    for (String envName : env.keySet()) {
        envMap.put(envName, env.get(envName));
    }
    variables.peek().put("env", envMap);
    */
    DEBUG_CTX();
    DEBUG_END();
}

Interpreter::~Interpreter() {
    DEBUG_BGN();
    DEBUG_CTX();
    delete variables.back();
    variables.pop_back();
    DEBUG_END();
}

void Interpreter::handleLhs(AquilaParser::LhsContext* lhsc, std::function<void(Dictionary*, std::string)> handler) {
    DEBUG_BGN();
    String identifier = lhsc->Identifier()->getText();
    DEBUG_LOG("identifier=" << identifier);
    if (lhsc->lhsPart().empty()) {
        DEBUG_LOG("lhsc->lhsPart().empty()");
        handler(variables.back(), identifier);
    } else {
        DEBUG_LOG("!(lhsc->lhsPart().empty())");
        if (!variables.back()->count(identifier)) {
            DEBUG_LOG("!variables.back()->count(identifier)");
            Dictionary d;
            (*variables.back())[identifier] = d;
        }
        Any& result = (*variables.back())[identifier];
        for (size_t i = 0; i < lhsc->lhsPart().size(); i++) {
            AquilaParser::LhsPartContext* lhspc = lhsc->lhsPart()[i];
            std::string key;
            if (lhspc->Identifier()) {
                key = lhspc->Identifier()->getText();
            } else if (lhspc->expression()) {
                key = toString(visit(lhspc->expression()));
            } else {
                assert(0);
            }
            if (i < lhsc->lhsPart().size() - 1) {
                if (isDict(result)) {
                    Dictionary d = *toDict(result);
                    if (d.count(key)) {
                        result = d[key];
                    } else {
                        missingKey(d, key, lhspc);
                        assert(0);
                    }
                } else {
                    typeMismatch(TYPE_DICT, result, lhspc);
                    assert(0);
                }
            } else {
                if (isDict(result)) {
                    handler(toDict(result), key);
                } else {
                    typeMismatch(TYPE_DICT, result, lhspc);
                    assert(0);
                }
            }
        }
    }
    DEBUG_END();
}

Any Interpreter::callFunction(AquilaParser::FunctionCallContext* callsite, Function& function, const std::vector<Any>& arguments) {
    DEBUG_BGN();
    std::vector<std::string> parameters;
    std::vector<std::string> types;
    for (AquilaParser::LambdaExpressionParameterContext* lepc : function.lambdaExpressionParameter()) {
        parameters.push_back(lepc->Identifier()->getText());
        types.push_back(lepc->Type() ? lepc->Type()->getText() : TYPE_ANY);
    }
    if (!checkArgs(callsite, arguments, types)) {
        assert(0);
    }
    Dictionary d(*variables.back());
    variables.push_back(&d);
    for (size_t i = 0; i < parameters.size(); i++) {
        const std::string parameter = parameters[i];
        Any argument = arguments[i];
        // TODO does this have to be fully qualified?
        (*variables.back())[parameter] = argument;
    }
    Any result;
    if (function.expression()) {
        result = visit(function.expression());
    } else if (function.block()) {
        visit(function.block());
        result = 0;
    } else {
        assert(0);
    }
    variables.pop_back();
    DEBUG_END();
    return result;
}

bool Interpreter::checkArgs(AquilaParser::FunctionCallContext* callsite, const std::vector<Any>& arguments, std::vector<std::string> types) {
    return checkArgsHelper(callsite, arguments, true, types);
}

bool Interpreter::checkArgsNoFail(AquilaParser::FunctionCallContext* callsite, const std::vector<Any>& arguments, std::vector<std::string> types) {
    return checkArgsHelper(callsite, arguments, false, types);
}

bool Interpreter::checkArgsHelper(AquilaParser::FunctionCallContext* callsite, const std::vector<Any>& arguments, bool failOnTypeMismatch, std::vector<std::string> types) {
    if (arguments.size() != types.size()) {
        wrongNumberOfArguments(types.size(), arguments.size(), callsite);
        return false;
    }
    for (size_t i = 0; i < types.size(); i++) {
        const std::string type = types[i];
        Any argument = arguments[i];
        if (!strEquals(type, TYPE_ANY) && !strEquals(typeOf(argument), type)) {
            if (failOnTypeMismatch) {
                typeMismatch(type, argument, callsite->arguments[i]);
            }
            return false;
        }
    }
    return true;
}

Any Interpreter::visitProgram(AquilaParser::ProgramContext *ctx) {
    DEBUG_BGN();
    for (AquilaParser::StatementContext* sc : ctx->statement()) {
        DEBUG_HR();
        try {
            visit(sc);
        } catch (std::exception& e) {
            exception(e, sc);
            assert(0);
        }
    }
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitStatement(AquilaParser::StatementContext *ctx) {
    DEBUG_BGN();
    DEBUG_CTX();
    Any result = visitChildren(ctx);
    DEBUG_END();
    return result;
}

Any Interpreter::visitIfStatement(AquilaParser::IfStatementContext *ctx) {
    DEBUG_BGN();
    for (size_t i = 0; i < ctx->condition.size(); i++) {
        Any condition = visit(ctx->condition[i]);
        if (isBool(condition) && toBool(condition)) {
            visit(ctx->then[i]);
            goto end;
        }
    }
    if (ctx->elseBlock) {
        visit(ctx->elseBlock);
    }
end:
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitSwitchStatement(AquilaParser::SwitchStatementContext *ctx) {
    DEBUG_BGN();
    Any switchHeadExpression = visit(ctx->switchHeadExpression);
    for (size_t i = 0; i < ctx->labels.size(); i++) {
        AquilaParser::SwitchStatementLabelsContext* sslc = ctx->labels[i];
        for (size_t j = 0; j < sslc->expression().size(); j++) {
            Any label = visit(sslc->expression()[j]);
            if (anyEquals(switchHeadExpression, label)) {
                visit(ctx->then[i]);
                goto end;
            }
        }
    }
    if (ctx->defaultBlock) {
        visit(ctx->defaultBlock);
    }
end:
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitSwitchStatementLabels(AquilaParser::SwitchStatementLabelsContext *ctx) {
    assert(0);
}

Any Interpreter::visitLoopStatement(AquilaParser::LoopStatementContext *ctx) {
    DEBUG_BGN();
    while (true) {
        if (ctx->top) {
            visit(ctx->top);
        }
        Any condition = visit(ctx->expression());
        if (!isBool(condition) || !toBool(condition)) {
            break;
        }
        if (ctx->bottom) {
            visit(ctx->bottom);
        }
    }
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitForStatement(AquilaParser::ForStatementContext *ctx) {
    DEBUG_BGN();
    const std::string identifier = ctx->Identifier()->getText();
    Integer fromInt, toInt, stepInt, i;
    mpz_inits(fromInt.i, toInt.i, stepInt.i, i.i, NULL);
    /* from */
    Any from = visit(ctx->from);
    if (!isInt(from)) {
        typeMismatch(TYPE_INT, from, ctx->from);
        assert(0);
    }
    mpz_set(fromInt.i, toInt(from)->i);
    /* to */
    Any to = visit(ctx->to);
    if (!isInt(to)) {
        typeMismatch(TYPE_INT, to, ctx->to);
        assert(0);
    }
    mpz_set(toInt.i, toInt(to)->i);
    /* step */
    if (ctx->step) {
        Any step = visit(ctx->step);
        if (!isInt(step)) {
            typeMismatch(TYPE_INT, step, ctx->step);
            assert(0);
        }
        mpz_set(stepInt.i, toInt(step)->i);
    } else {
        mpz_set_ui(stepInt.i, 1);
    }
    Dictionary& d = *variables.back();
    bool overridesValue = d.count(identifier);
    Any overriddenValue;
    if (overridesValue) {
        overriddenValue = d[identifier];
    }
    for (mpz_set(i.i, fromInt.i); mpz_sgn(stepInt.i) >= 0 ? mpz_cmp(i.i, toInt.i) <= 0 : mpz_cmp(i.i, toInt.i) >= 0; mpz_add(i.i, i.i, stepInt.i)) {
        d[identifier] = i;
        visit(ctx->block());
    }
    mpz_clears(fromInt.i, toInt.i, stepInt.i, i.i, NULL);
    if (overridesValue) {
        d[identifier] = overriddenValue;
    } else {
        d.erase(identifier);
    }
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitReadStatement(AquilaParser::ReadStatementContext *ctx) {
    DEBUG_BGN();
    String line;
    std::getline(std::cin, line);
    DEBUG_LOG("line=" << line);
    handleLhs(ctx->lhs(), [this, line, ctx](Dictionary* map, const std::string& key) { assign(map, key, line, ctx); });
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitWriteStatement(AquilaParser::WriteStatementContext *ctx) {
    DEBUG_BGN();
    Any rhs = visit(ctx->rhs);
    DEBUG_LOG("rhs=" << toString(rhs));
    if (isStr(rhs)) {
        std::cout << *toStr(rhs) << std::endl;
    } else {
        typeMismatch(TYPE_STR, rhs, ctx->rhs);
        assert(0);
    }
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitAssignStatement(AquilaParser::AssignStatementContext *ctx) {
    DEBUG_BGN();
    Any rhs = visit(ctx->rhs);
    DEBUG_LOG("rhs=" << toString(rhs));
    handleLhs(ctx->lhs(), [this, rhs, ctx](Dictionary* map, const std::string& key) { assign(map, key, rhs, ctx->rhs); });
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitCallStatement(AquilaParser::CallStatementContext *ctx) {
    DEBUG_BGN();
    visit(ctx->functionCall());
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitRemoveStatement(AquilaParser::RemoveStatementContext *ctx) {
    DEBUG_BGN();
    handleLhs(ctx->lhs(), [this, ctx](Dictionary* map, const std::string& key) {
        if (map->count(key)) {
            map->erase(key);
        } else {
            missingKey(*map, key, ctx);
            assert(0);
        }
    });
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitRunStatement(AquilaParser::RunStatementContext *ctx) {
    DEBUG_BGN();
    Any rhs = visit(ctx->rhs);
    DEBUG_LOG("rhs=" << toString(rhs));
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
        typeMismatch(TYPE_STR, rhs, ctx->rhs);
        assert(0);
    }
    DEBUG_END();
    return POISON;
}

Any Interpreter::visitBlock(AquilaParser::BlockContext *ctx) {
    DEBUG_BGN();
    for (AquilaParser::StatementContext* sc : ctx->statement()) {
        visit(sc);
    }
    DEBUG_END();
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
    DEBUG_BGN();
    for (size_t i = 0; i < ctx->condition.size(); i++) {
        Any condition = visit(ctx->condition[i]);
        if (isBool(condition) && toBool(condition)) {
            return visit(ctx->then[i]);
        }
    }
    Any result = visit(ctx->elseExpression);
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitLetExpression(AquilaParser::LetExpressionContext *ctx) {
    DEBUG_BGN();
    Dictionary d(*variables.back());
    variables.push_back(&d);
    for (AquilaParser::LetBindExpressionContext* lbec : ctx->letBindExpression()) {
        Any rhs = visit(lbec->rhs);
        DEBUG_LOG("rhs=" << toString(rhs));
        handleLhs(lbec->lhs(), [this, rhs, lbec](Dictionary* map, const std::string& key) { assign(map, key, rhs, lbec); });
    }
    Any result = visit(ctx->body);
    variables.pop_back();
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitLetBindExpression(AquilaParser::LetBindExpressionContext *ctx) {
    assert(0);
}

Any Interpreter::visitSwitchExpression(AquilaParser::SwitchExpressionContext *ctx) {
    DEBUG_BGN();
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
    Any result = visit(ctx->defaultExpression);
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitSwitchExpressionLabels(AquilaParser::SwitchExpressionLabelsContext *ctx) {
    assert(0);
}

Any Interpreter::visitLogicalOperation(AquilaParser::LogicalOperationContext *ctx) {
    DEBUG_BGN();
    Any result = visit(ctx->unaryLogicalOperation(0));
    for (size_t i = 0; i < ctx->logicalOperator().size(); i++) {
        AquilaParser::LogicalOperatorContext* loc = ctx->logicalOperator(i);
        AquilaParser::UnaryLogicalOperationContext* uloc = ctx->unaryLogicalOperation(i + 1);
        std::string op = loc->getText();
        Any operand = visit(uloc);
        if (isBool(result)) {
            if (isBool(operand)) {
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
                typeMismatch(TYPE_BOOL, operand, uloc);
                assert(0);
            }
        } else {
            typeMismatch(TYPE_BOOL, result, ctx);
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitLogicalOperator(AquilaParser::LogicalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitUnaryLogicalOperation(AquilaParser::UnaryLogicalOperationContext *ctx) {
    DEBUG_BGN();
    AquilaParser::RelationContext* rc = ctx->relation();
    Any result = visit(rc);
    if (ctx->unaryLogicalOperator()) {
        if (isBool(result)) {
            std::string op = ctx->unaryLogicalOperator()->getText();
            if (strEquals(op, "not")) {
                result = !*toBool(result);
            } else {
                assert(0);
            }
        } else {
            typeMismatch(TYPE_BOOL, result, rc);
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitUnaryLogicalOperator(AquilaParser::UnaryLogicalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitRelation(AquilaParser::RelationContext *ctx) {
    DEBUG_BGN();
    Any result = visit(ctx->addition(0));
    if (ctx->relationalOperator() && ctx->addition().size() == 2) {
        AquilaParser::RelationalOperatorContext* roc = ctx->relationalOperator();
        AquilaParser::AdditionContext* ac = ctx->addition(1);
        std::string op = roc->getText();
        Any operand = visit(ac);
        if (strEquals(op, ":")) {
            if (isDict(operand)) {
                Dictionary d = *toDict(operand);
                Boolean r = false;
                for (const auto& [key, value] : d) {
                    if (anyEquals(value, result)) {
                        r = true;
                        break;
                    }
                }
                result = r;
            } else {
                typeMismatch(TYPE_DICT, operand, ac);
                assert(0);
            }
        } else if (strEquals(op, "<=")
                || strEquals(op, "<>")
                || strEquals(op, "<")
                || strEquals(op, "=")
                || strEquals(op, ">=")
                || strEquals(op, ">")) {
            if (isInt(result)) {
                if (isInt(operand)) {
                    if (strEquals(op, "<=")) {
                        result = mpz_cmp(toInt(result)->i, toInt(operand)->i) <= 0;
                    } else if (strEquals(op, "<>")) {
                        result = mpz_cmp(toInt(result)->i, toInt(operand)->i) != 0;
                    } else if (strEquals(op, "<")) {
                        result = mpz_cmp(toInt(result)->i, toInt(operand)->i) < 0;
                    } else if (strEquals(op, "=")) {
                        result = mpz_cmp(toInt(result)->i, toInt(operand)->i) == 0;
                    } else if (strEquals(op, ">=")) {
                        result = mpz_cmp(toInt(result)->i, toInt(operand)->i) >= 0;
                    } else if (strEquals(op, ">")) {
                        result = mpz_cmp(toInt(result)->i, toInt(operand)->i) > 0;
                    } else {
                        assert(0);
                    }
                } else {
                    typeMismatch(TYPE_INT, operand, ac);
                    assert(0);
                }
            } else {
                typeMismatch(TYPE_INT, result, ctx);
                assert(0);
            }
        } else if (strEquals(op, "eq")
                || strEquals(op, "ew")
                || strEquals(op, "in")
                || strEquals(op, "ne")
                || strEquals(op, "sw")) {
            if (isStr(result)) {
                if (isStr(operand)) {
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
                    typeMismatch(TYPE_STR, operand, ac);
                    assert(0);
                }
            } else {
                typeMismatch(TYPE_STR, result, ctx);
                assert(0);
            }
        } else {
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitRelationalOperator(AquilaParser::RelationalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitAddition(AquilaParser::AdditionContext *ctx) {
    DEBUG_BGN();
    Any result = visit(ctx->multiplication(0));
    for (size_t i = 0; i < ctx->additionalOperator().size(); i++) {
        AquilaParser::AdditionalOperatorContext* aoc = ctx->additionalOperator(i);
        AquilaParser::MultiplicationContext* mc = ctx->multiplication(i + 1);
        std::string op = aoc->getText();
        Any operand = visit(mc);
        if (strEquals(op, "&")) {
            if (isStr(result)) {
                if (isStr(operand)) {
                    std::stringstream ss;
                    ss << *toStr(result) << *toStr(operand);
                    result = ss.str();
                } else {
                    typeMismatch(TYPE_STR, operand, mc);
                    assert(0);
                }
            } else {
                typeMismatch(TYPE_STR, result, ctx);
                assert(0);
            }
        } else if (strEquals(op, "+")
                || strEquals(op, "-")) {
            if (isInt(result)) {
                if (isInt(operand)) {
                    Integer r;
                    mpz_init(r.i);
                    if (strEquals(op, "+")) {
                        mpz_add(r.i, toInt(result)->i, toInt(operand)->i);
                    } else if (strEquals(op, "-")) {
                        mpz_sub(r.i, toInt(result)->i, toInt(operand)->i);
                    } else {
                        assert(0);
                    }
                    result = r;
                } else {
                    typeMismatch(TYPE_INT, operand, mc);
                    assert(0);
                }
            } else {
                typeMismatch(TYPE_INT, result, ctx);
                assert(0);
            }
        } else {
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitAdditionalOperator(AquilaParser::AdditionalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitMultiplication(AquilaParser::MultiplicationContext *ctx) {
    DEBUG_BGN();
    Any result = visit(ctx->unaryAddition(0));
    for (size_t i = 0; i < ctx->multiplicationOperator().size(); i++) {
        AquilaParser::MultiplicationOperatorContext* moc = ctx->multiplicationOperator(i);
        AquilaParser::UnaryAdditionContext* uac = ctx->unaryAddition(i + 1);
        std::string op = moc->getText();
        Any operand = visit(uac);
        if (isInt(result)) {
            if (isInt(operand)) {
                Integer r;
                mpz_init(r.i);
                if (strEquals(op, "*")) {
                    mpz_mul(r.i, toInt(result)->i, toInt(operand)->i);
                } else if (strEquals(op, "/")) {
                    mpz_fdiv_q(r.i, toInt(result)->i, toInt(operand)->i);
                } else if (strEquals(op, "mod")) {
                    // FIXME double check fdiv_r() behavior
                    mpz_fdiv_r(r.i, toInt(result)->i, toInt(operand)->i);
                } else if (strEquals(op, "rem")) {
                    // FIXME double check mod() behavior
                    mpz_mod(r.i, toInt(result)->i, toInt(operand)->i);
                } else {
                    assert(0);
                }
                result = r;
            } else {
                typeMismatch(TYPE_INT, operand, uac);
                assert(0);
            }
        } else {
            typeMismatch(TYPE_INT, result, ctx);
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitMultiplicationOperator(AquilaParser::MultiplicationOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitUnaryAddition(AquilaParser::UnaryAdditionContext *ctx) {
    DEBUG_BGN();
    AquilaParser::FactorialContext* fc = ctx->factorial();
    Any result = visit(fc);
    if (ctx->unaryAdditionalOperator()) {
        if (isInt(result)) {
            std::string op = ctx->unaryAdditionalOperator()->getText();
            Integer r;
            mpz_init(r.i);
            if (strEquals(op, "+")) {
                /* do nothing */
            } else if (strEquals(op, "-")) {
                mpz_neg(r.i, toInt(result)->i);
            } else {
                assert(0);
            }
            result = r;
        } else {
            typeMismatch(TYPE_INT, result, fc);
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitUnaryAdditionalOperator(AquilaParser::UnaryAdditionalOperatorContext *ctx) {
    assert(0);
}

Any Interpreter::visitFactorial(AquilaParser::FactorialContext *ctx) {
    DEBUG_BGN();
    AquilaParser::FactorContext* fc = ctx->factor();
    Any result = visit(fc);
    if (ctx->factorialOperator()) {
        if (isInt(result)) {
            std::string op = ctx->factorialOperator()->getText();
            Integer r;
            mpz_init(r.i);
            if (strEquals(op, "!")) {
                mpz_fac_ui(r.i, mpz_get_ui(toInt(result)->i));
            } else {
                assert(0);
            }
            result = r;
        } else {
            typeMismatch(TYPE_INT, result, fc);
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
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
    DEBUG_BGN();
    AquilaParser::ExpressionContext* ec = ctx->expression();
    Any result = visit(ec);
    if (isInt(result)) {
        Integer r;
        mpz_init(r.i);
        mpz_abs(r.i, toInt(result)->i);
        result = r;
    } else {
        typeMismatch(TYPE_INT, result, ec);
        assert(0);
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitDictAggregate(AquilaParser::DictAggregateContext *ctx) {
    DEBUG_BGN();
    Dictionary result;
    for (size_t i = 0; i < ctx->dictAggregatePair().size(); i++) {
        AquilaParser::DictAggregatePairContext* dapc = ctx->dictAggregatePair(i);
        std::string key;
        if (dapc->key) {
            key = toString(visit(dapc->key));
        } else {
            std::stringstream ss;
            ss << i;
            key = ss.str();
        }
        Any value = visit(dapc->value);
        result[key] = value;
    }
    DEBUG_END_R(toString(result));
    return result;
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
        Integer result;
        mpz_init_set_str(result.i, s.c_str(), radix);
        return result;
    } else if (ctx->False()) {
        return false;
    } else if (ctx->True()) {
        return true;
    } else {
        assert(0);
    }
}

Any Interpreter::visitFunctionCall(AquilaParser::FunctionCallContext *ctx) {
    DEBUG_BGN();
    const std::string identifier = ctx->Identifier()->getText();
    std::vector<Any> arguments;
    for (AquilaParser::ExpressionContext* ec : ctx->arguments) {
        arguments.push_back(visit(ec));
    }
    Any result;
    if (strEquals(identifier, "bool2str")) {
        if (!checkArgs(ctx, arguments, { TYPE_BOOL })) {
            assert(0);
        }
        Boolean arg1 = *toBool(arguments[0]);
        result = toString(arg1);
    } else if (strEquals(identifier, "boolean")) {
        result = checkArgsNoFail(ctx, arguments, { TYPE_BOOL });
    } else if (strEquals(identifier, "char2ord")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer resultInt;
        mpz_set_ui(resultInt.i, arg1.at(0));
        result = resultInt;
    } else if (strEquals(identifier, "charat")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        result = arg1.substr(mpz_get_si(arg2.i), 1);
    } else if (strEquals(identifier, "dict2str")) {
        if (!checkArgs(ctx, arguments, { TYPE_DICT })) {
            assert(0);
        }
        Dictionary arg1 = *toDict(arguments[0]);
        result = toString(arg1);
    } else if (strEquals(identifier, "dictionary")) {
        result = checkArgsNoFail(ctx, arguments, { TYPE_DICT });
    } else if (strEquals(identifier, "error")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        error(arg1, ctx);
        result = arg1;
    } else if (strEquals(identifier, "exists")) {
        if (!checkArgs(ctx, arguments, { TYPE_DICT, TYPE_FUNC })) {
            assert(0);
        }
        Dictionary arg1 = *toDict(arguments[0]);
        //Function arg2 = *toFunc(arguments[1]);
        /* TODO
        std::vector<Any> localArguments;
        result = false;
        for (std::string key : arg1.keySet()) {
            const Any value = arg1[key];
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            if (Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                result = true;
                break;
            }
        }
        */
    } else if (strEquals(identifier, "exit")) {
        if (!checkArgs(ctx, arguments, { TYPE_INT })) {
            assert(0);
        }
        Integer arg1 = *toInt(arguments[0]);
        result = arg1;
        exit(mpz_get_si(arg1.i));
    } else if (strEquals(identifier, "filter")) {
        if (!checkArgs(ctx, arguments, { TYPE_DICT, TYPE_FUNC })) {
            assert(0);
        }
        Dictionary arg1 = *toDict(arguments[0]);
        //Function arg2 = *toFunc(arguments[1]);
        /* TODO
        std::vector<Any> localArguments;
        Dictionary resultMap = new TreeMap<>(DICT_COMPARATOR);
        for (std::string key : arg1.keySet()) {
            const Any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            if (Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                resultMap[key] = value;
            }
        }
        result = resultMap;
        */
    } else if (strEquals(identifier, "findleft")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_STR, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        String arg2 = *toStr(arguments[1]);
        Integer arg3 = *toInt(arguments[2]);
        Integer r;
        mpz_init_set_ui(r.i, arg1.find(arg2, mpz_get_si(arg3.i)));
        result = r;
    } else if (strEquals(identifier, "findright")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_STR, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        String arg2 = *toStr(arguments[1]);
        Integer arg3 = *toInt(arguments[2]);
        Integer r;
        mpz_init_set_ui(r.i, arg1.rfind(arg2, mpz_get_si(arg3.i)));
        result = r;
    } else if (strEquals(identifier, "fold")) {
        if (!checkArgs(ctx, arguments, { TYPE_DICT, TYPE_ANY, TYPE_FUNC })) {
            assert(0);
        }
        Dictionary arg1 = *toDict(arguments[0]);
        Any arg2 = arguments[1];
        //Function arg3 = *toFunc(arguments[2]);
        /* TODO
        std::vector<Any> localArguments;
        result = arg2;
        for (std::string key : arg1.keySet()) {
            const Any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(result);
            localArguments.push_back(value);
            result = callFunction(ctx, arg3, localArguments);
        }
        */
    } else if (strEquals(identifier, "forall")) {
        if (!checkArgs(ctx, arguments, { TYPE_DICT, TYPE_FUNC })) {
            assert(0);
        }
        Dictionary arg1 = *toDict(arguments[0]);
        //Function arg2 = *toFunc(arguments[1]);
        /* TODO
        std::vector<Any> localArguments;
        result = true;
        for (std::string key : arg1.keySet()) {
            const Any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            if (!Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                result = false;
                break;
            }
        }
        */
    } else if (strEquals(identifier, "foreach")) {
        if (!checkArgs(ctx, arguments, { TYPE_DICT, TYPE_FUNC })) {
            assert(0);
        }
        Dictionary arg1 = *toDict(arguments[0]);
        //Function arg2 = *toFunc(arguments[1]);
        /* TODO
        std::vector<Any> localArguments;
        for (std::string key : arg1.keySet()) {
            const Any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            callFunction(ctx, arg2, localArguments);
        }
        result = null;
        */
    } else if (strEquals(identifier, "function")) {
        result = checkArgsNoFail(ctx, arguments, { TYPE_FUNC });
    } else if (strEquals(identifier, "gcd")) {
        if (!checkArgs(ctx, arguments, { TYPE_INT, TYPE_INT })) {
            assert(0);
        }
        Integer arg1 = *toInt(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        Integer r;
        mpz_init(r.i);
        mpz_gcd(r.i, arg1.i, arg2.i);
        result = r;
    } else if (strEquals(identifier, "head")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        result = arg1.empty() ? "" : arg1.substr(0, 1);
    } else if (strEquals(identifier, "int2str")) {
        if (!checkArgs(ctx, arguments, { TYPE_INT })) {
            assert(0);
        }
        Integer arg1 = *toInt(arguments[0]);
        result = toString(arg1);
    } else if (strEquals(identifier, "integer")) {
        result = checkArgsNoFail(ctx, arguments, { TYPE_INT });
    } else if (strEquals(identifier, "join")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_DICT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Dictionary arg2 = *toDict(arguments[1]);
        // TODO result = std::string.join(arg1, arg2.values());
    } else if (strEquals(identifier, "left")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        result = arg1.substr(0, mpz_get_si(arg2.i));
    } else if (strEquals(identifier, "length")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer r;
        mpz_init_set_ui(r.i, arg1.size());
        result = r;
    } else if (strEquals(identifier, "map")) {
        if (!checkArgs(ctx, arguments, { TYPE_DICT, TYPE_FUNC })) {
            assert(0);
        }
        Dictionary arg1 = *toDict(arguments[0]);
        //Function arg2 = *toFunc(arguments[1]);
        /* TODO
        std::vector<Any> localArguments;
        Dictionary resultMap = new TreeMap<>(DICT_COMPARATOR);
        for (std::string key : arg1.keySet()) {
            const Any value = arg1.get(key);
            localArguments.clear();
            localArguments.push_back(key);
            localArguments.push_back(value);
            const Any newValue = callFunction(ctx, arg2, localArguments);
            resultMap[key] = newValue;
        }
        result = resultMap;
        */
    } else if (strEquals(identifier, "mid")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_INT, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        Integer arg3 = *toInt(arguments[2]);
        result = arg1.substr(mpz_get_si(arg2.i), mpz_get_si(arg3.i));
    } else if (strEquals(identifier, "ord2char")) {
        if (!checkArgs(ctx, arguments, { TYPE_INT })) {
            assert(0);
        }
        Integer arg1 = *toInt(arguments[0]);
        result = std::string(1, (char) mpz_get_si(arg1.i));
    } else if (strEquals(identifier, "pow")) {
        if (!checkArgs(ctx, arguments, { TYPE_INT, TYPE_INT })) {
            assert(0);
        }
        Integer arg1 = *toInt(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        Integer r;
        mpz_init(r.i);
        mpz_pow_ui(r.i, arg1.i, mpz_get_ui(arg2.i));
        result = r;
    } else if (strEquals(identifier, "repeat")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        // TODO result = arg1.repeat(mpz_get_si(arg2.i));
    } else if (strEquals(identifier, "replace")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_STR, TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        String arg2 = *toStr(arguments[1]);
        String arg3 = *toStr(arguments[2]);
        // TODO result = arg1.replace(arg2, arg3);
    } else if (strEquals(identifier, "right")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        result = arg1.substr(arg1.size() - mpz_get_si(arg2.i));
    } else if (strEquals(identifier, "sgn")) {
        if (!checkArgs(ctx, arguments, { TYPE_INT })) {
            assert(0);
        }
        Integer arg1 = *toInt(arguments[0]);
        Integer r;
        mpz_init_set_ui(r.i, mpz_sgn(arg1.i));
        result = r;
    } else if (strEquals(identifier, "size")) {
        if (!checkArgs(ctx, arguments, { TYPE_DICT })) {
            assert(0);
        }
        Dictionary arg1 = *toDict(arguments[0]);
        Integer r;
        mpz_init_set_ui(r.i, arg1.size());
        result = r;
    } else if (strEquals(identifier, "sleep")) {
        if (!checkArgs(ctx, arguments, { TYPE_INT })) {
            assert(0);
        }
        Integer arg1 = *toInt(arguments[0]);
        /* TODO
        try {
            Thread.sleep(arg1.longValueExact());
            result = true;
        } catch (InterruptedException e) {
            result = false;
        }
        */
    } else if (strEquals(identifier, "split")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        String arg2 = *toStr(arguments[1]);
        /* TODO
        String[] parts = arg2.split(arg1, -1);
        Dictionary resultMap = new TreeMap<>(DICT_COMPARATOR);
        for (size_t i = 0; i < parts.length; i++) {
            resultMap[BigInteger.valueOf(i)] = parts[i];
        }
        result = resultMap;
        */
    } else if (strEquals(identifier, "sqrt")) {
        if (!checkArgs(ctx, arguments, { TYPE_INT })) {
            assert(0);
        }
        Integer arg1 = *toInt(arguments[0]);
        Integer r;
        mpz_init(r.i);
        mpz_sqrt(r.i, arg1.i);
        result = r;
    } else if (strEquals(identifier, "str2bool")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        result = toBoolean(arg1, ctx);
    } else if (strEquals(identifier, "str2dict")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        result = toDictionary(arg1, ctx);
    } else if (strEquals(identifier, "str2int")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        result = toInteger(arg1, ctx);
    } else if (strEquals(identifier, "string")) {
        result = checkArgsNoFail(ctx, arguments, { TYPE_STR });
    } else if (strEquals(identifier, "substring1")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        result = arg1.substr(mpz_get_si(arg2.i));
    } else if (strEquals(identifier, "substring2")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR, TYPE_INT, TYPE_INT })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        Integer arg2 = *toInt(arguments[1]);
        Integer arg3 = *toInt(arguments[2]);
        result = arg1.substr(mpz_get_si(arg2.i), mpz_get_si(arg3.i) - mpz_get_si(arg2.i));
    } else if (strEquals(identifier, "tail")) {
        if (!checkArgs(ctx, arguments, { TYPE_STR })) {
            assert(0);
        }
        String arg1 = *toStr(arguments[0]);
        result = arg1.empty() ? "" : arg1.substr(1);
    } else {
        Dictionary d = *variables.back();
        if (d.count(identifier)) {
            Any object = d[identifier];
            if (isFunc(object)) {
                Function* function = toFunc(object);
                result = callFunction(ctx, *function, arguments);
            } else {
                typeMismatch(TYPE_FUNC, object, ctx);
                assert(0);
            }
        } else {
            missingKey(d, identifier, ctx);
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitAccessExpression(AquilaParser::AccessExpressionContext *ctx) {
    DEBUG_BGN();
    const std::string identifier = ctx->Identifier()->getText();
    if (!variables.back()->count(identifier)) {
        Dictionary d;
        (*variables.back())[identifier] = d;
    }
    Any result = (*variables.back())[identifier];
    for (AquilaParser::AccessExpressionPartContext* aepc : ctx->accessExpressionPart()) {
        std::string key;
        if (aepc->Identifier()) {
            key = aepc->Identifier()->getText();
        } else if (aepc->expression()) {
            key = toString(visit(aepc->expression()));
        } else {
            assert(0);
        }
        if (isDict(result)) {
            Dictionary d = *toDict(result);
            if (d.count(key)) {
                result = d[key];
            } else {
                missingKey(d, key, aepc);
                assert(0);
            }
        } else {
            typeMismatch(TYPE_DICT, result, aepc);
            assert(0);
        }
    }
    DEBUG_END_R(toString(result));
    return result;
}

Any Interpreter::visitAccessExpressionPart(AquilaParser::AccessExpressionPartContext *ctx) {
    assert(0);
}