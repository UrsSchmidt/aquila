package aquila;

import aquila.antlr.AquilaLexer;
import aquila.antlr.AquilaParser;
import aquila.antlr.AquilaParser.ProgramContext;
import aquila.antlr.AquilaParser.StatementContext;
import aquila.antlr.AquilaParser.IfStatementContext;
import aquila.antlr.AquilaParser.LoopStatementContext;
import aquila.antlr.AquilaParser.ForStatementContext;
import aquila.antlr.AquilaParser.ForeachStatementContext;
import aquila.antlr.AquilaParser.ReadStatementContext;
import aquila.antlr.AquilaParser.WriteStatementContext;
import aquila.antlr.AquilaParser.AssignStatementContext;
import aquila.antlr.AquilaParser.CallStatementContext;
import aquila.antlr.AquilaParser.RemoveStatementContext;
import aquila.antlr.AquilaParser.BlockContext;
import aquila.antlr.AquilaParser.LhsContext;
import aquila.antlr.AquilaParser.LhsPartContext;
import aquila.antlr.AquilaParser.ExpressionContext;
import aquila.antlr.AquilaParser.LogicalOperationContext;
import aquila.antlr.AquilaParser.LogicalOperatorContext;
import aquila.antlr.AquilaParser.UnaryLogicalOperationContext;
import aquila.antlr.AquilaParser.UnaryLogicalOperatorContext;
import aquila.antlr.AquilaParser.RelationContext;
import aquila.antlr.AquilaParser.RelationalOperatorContext;
import aquila.antlr.AquilaParser.AdditionContext;
import aquila.antlr.AquilaParser.AdditionalOperatorContext;
import aquila.antlr.AquilaParser.MultiplicationContext;
import aquila.antlr.AquilaParser.MultiplicationOperatorContext;
import aquila.antlr.AquilaParser.UnaryAdditionContext;
import aquila.antlr.AquilaParser.UnaryAdditionalOperatorContext;
import aquila.antlr.AquilaParser.FactorContext;
import aquila.antlr.AquilaParser.BracketExpressionContext;
import aquila.antlr.AquilaParser.AbsExpressionContext;
import aquila.antlr.AquilaParser.DictAggregateContext;
import aquila.antlr.AquilaParser.DictAggregatePairContext;
import aquila.antlr.AquilaParser.DictAggregateKeyContext;
import aquila.antlr.AquilaParser.LiteralContext;
import aquila.antlr.AquilaParser.FunctionCallContext;
import aquila.antlr.AquilaParser.AccessExpressionContext;
import aquila.antlr.AquilaParser.AccessExpressionPartContext;
import aquila.antlr.AquilaVisitor;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.TreeMap;
import java.util.function.BiConsumer;

import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.ParserRuleContext;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.tree.AbstractParseTreeVisitor;

public class Interpreter extends AbstractParseTreeVisitor<Object> implements AquilaVisitor<Object> {

    private static final boolean STRICT = true; // XXX make this an option

    private static final String TYPE_DICT = "Dictionary";
    private static final String TYPE_STR = "String";
    private static final String TYPE_INT = "Integer";
    private static final String TYPE_BOOL = "Boolean";

    private static final Comparator DICT_COMPARATOR = new Comparator<>() {
        @Override
        public int compare(Object o1, Object o2) {
            final String s1 = Interpreter.toString(o1);
            final String s2 = Interpreter.toString(o2);
            if (s1.matches("[0-9]+") && s2.matches("[0-9]+")) {
                return Integer.compare(Integer.parseInt(s1), Integer.parseInt(s2));
            } else {
                return s1.compareTo(s2);
            }
        }
    };

    private Map variables = new TreeMap<>(DICT_COMPARATOR);

    public Interpreter(String[] args) {
        super();
        /* passing command line arguments */
        Map argsMap = new TreeMap<>(DICT_COMPARATOR);
        for (int i = 0; i < args.length; i++) {
            argsMap.put(BigInteger.valueOf(i), args[i]);
        }
        variables.put("args", argsMap);
        /* passing environment variables */
        Map envMap = new TreeMap<>(DICT_COMPARATOR);
        final Map<String, String> env = System.getenv();
        for (String envName : env.keySet()) {
            envMap.put(envName, env.get(envName));
        }
        variables.put("env", envMap);
    }

    @Override
    public Object visitProgram(ProgramContext ctx) {
        for (StatementContext sc : ctx.statement()) {
            visit(sc);
        }
        return null;
    }

    @Override
    public Object visitStatement(StatementContext ctx) {
        return visitChildren(ctx);
    }

    @Override
    public Object visitIfStatement(IfStatementContext ctx) {
        for (int i = 0; i < ctx.expression().size(); i++) {
            final Object expression = visit(ctx.expression(i));
            if (Boolean.TRUE.equals(expression)) {
                visit(ctx.block(i));
                return null;
            }
        }
        if (ctx.elseBlock != null) {
            visit(ctx.elseBlock);
        }
        return null;
    }

    @Override
    public Object visitLoopStatement(LoopStatementContext ctx) {
        while (true) {
            if (ctx.top != null) {
                visit(ctx.top);
            }
            if (!Boolean.TRUE.equals(visit(ctx.expression()))) {
                break;
            }
            if (ctx.bottom != null) {
                visit(ctx.bottom);
            }
        }
        return null;
    }

    @Override
    public Object visitForStatement(ForStatementContext ctx) {
        final String identifier = ctx.Identifier().getText();
        final Object from = visit(ctx.from);
        if (!(from instanceof BigInteger)) {
            typeMismatch(TYPE_INT, typeOf(from), ctx.from);
            return null;
        }
        final Object to = visit(ctx.to);
        if (!(to instanceof BigInteger)) {
            typeMismatch(TYPE_INT, typeOf(to), ctx.to);
            return null;
        }
        final Object step = ctx.step != null ? visit(ctx.step) : BigInteger.ONE;
        if (!(step instanceof BigInteger)) {
            typeMismatch(TYPE_INT, typeOf(step), ctx.step);
            return null;
        }
        final BigInteger fromInt = (BigInteger) from;
        final BigInteger toInt = (BigInteger) to;
        final BigInteger stepInt = (BigInteger) step;
        final Object overriddenValue = variables.get(identifier);
        for (BigInteger i = fromInt; stepInt.signum() >= 0 ? i.compareTo(toInt) <= 0 : i.compareTo(toInt) >= 0; i = i.add(stepInt)) {
            variables.put(identifier, i);
            visit(ctx.block());
        }
        variables.put(identifier, overriddenValue);
        return null;
    }

    @Override
    public Object visitForeachStatement(ForeachStatementContext ctx) {
        final Object expression = visit(ctx.expression());
        if (!(expression instanceof Map)) {
            typeMismatch(TYPE_DICT, typeOf(expression), ctx.expression());
            return null;
        }
        final Map map = (Map) expression;
        final Object keyIdentifier = ctx.key != null ? ctx.key.getText() : null;
        final Object valueIdentifier = ctx.value != null ? ctx.value.getText() : null;
        for (Object key : map.keySet()) {
            final Object value = map.get(key);
            if (keyIdentifier != null) {
                variables.put(keyIdentifier, key);
            }
            if (valueIdentifier != null) {
                variables.put(valueIdentifier, value);
            }
            visit(ctx.block());
        }
        if (keyIdentifier != null) {
            variables.remove(keyIdentifier);
        }
        if (valueIdentifier != null) {
            variables.remove(valueIdentifier);
        }
        return null;
    }

    @Override
    public Object visitReadStatement(ReadStatementContext ctx) {
        final String line = new Scanner(System.in).nextLine();
        handleLhs(ctx.lhs(), (map, key) -> assign(map, key, line, ctx));
        return null;
    }

    @Override
    public Object visitWriteStatement(WriteStatementContext ctx) {
        final Object rhs = visit(ctx.rhs);
        if (rhs instanceof String) {
            System.out.println(rhs);
        } else {
            typeMismatch(TYPE_STR, typeOf(rhs), ctx.rhs);
        }
        return null;
    }

    @Override
    public Object visitAssignStatement(AssignStatementContext ctx) {
        final Object rhs = visit(ctx.rhs);
        handleLhs(ctx.lhs(), (map, key) -> assign(map, key, rhs, ctx.rhs));
        return null;
    }

    private void assign(Map map, Object key, Object value, ParserRuleContext ctx) {
        if (STRICT && map.containsKey(key)) {
            final String expected = typeOf(map.get(key));
            final String was = typeOf(value);
            if (!expected.equals(was)) {
                typeMismatch(expected, was, ctx);
                return;
            }
        }
        map.put(key, value);
    }

    @Override
    public Object visitCallStatement(CallStatementContext ctx) {
        visit(ctx.functionCall());
        return null;
    }

    @Override
    public Object visitRemoveStatement(RemoveStatementContext ctx) {
        handleLhs(ctx.lhs(), (map, key) -> map.remove(key));
        return null;
    }

    private void handleLhs(LhsContext lhsc, BiConsumer<Map, Object> handler) {
        final String identifier = lhsc.Identifier().getText();
        if (lhsc.lhsPart().isEmpty()) {
            handler.accept(variables, identifier);
            return;
        }
        if (!variables.containsKey(identifier)) {
            variables.put(identifier, new TreeMap<>(DICT_COMPARATOR));
        }
        Object result = variables.get(identifier);
        for (int i = 0; i < lhsc.lhsPart().size(); i++) {
            final LhsPartContext lhspc = lhsc.lhsPart(i);
            Object key;
            if (lhspc.Identifier() != null) {
                key = lhspc.Identifier().getText();
            } else if (lhspc.expression() != null) {
                key = visit(lhspc.expression());
            } else {
                throw new AssertionError();
            }
            if (i < lhsc.lhsPart().size() - 1) {
                if (result instanceof Map) {
                    final Map map = (Map) result;
                    if (map.containsKey(key)) {
                        result = map.get(key);
                    } else {
                        missingKey(map, key, lhspc);
                        return;
                    }
                } else {
                    typeMismatch(TYPE_DICT, typeOf(result), lhspc);
                    return;
                }
            } else {
                if (result instanceof Map) {
                    final Map map = (Map) result;
                    handler.accept(map, key);
                } else {
                    typeMismatch(TYPE_DICT, typeOf(result), lhspc);
                    return;
                }
            }
        }
    }

    @Override
    public Object visitBlock(BlockContext ctx) {
        for (StatementContext sc : ctx.statement()) {
            visit(sc);
        }
        return null;
    }

    @Override
    public Object visitLhs(LhsContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitLhsPart(LhsPartContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitExpression(ExpressionContext ctx) {
        return visitChildren(ctx);
    }

    @Override
    public Object visitLogicalOperation(LogicalOperationContext ctx) {
        Object result = visit(ctx.unaryLogicalOperation(0));
        for (int i = 0; i < ctx.logicalOperator().size(); i++) {
            final LogicalOperatorContext loc = ctx.logicalOperator(i);
            final UnaryLogicalOperationContext uloc = ctx.unaryLogicalOperation(i + 1);
            final String operator = loc.getText();
            final Object operand = visit(uloc);
            if (result instanceof Boolean && operand instanceof Boolean) {
                switch (operator) {
                case "and":
                    result = ((Boolean) result) && ((Boolean) operand);
                    break;
                case "or":
                    result = ((Boolean) result) || ((Boolean) operand);
                    break;
                case "xor":
                    result = ((Boolean) result) ^ ((Boolean) operand);
                    break;
                default:
                    throw new AssertionError();
                }
            } else {
                typeMismatch(TYPE_BOOL + " and " + TYPE_BOOL, typeOf(result) + " and " + typeOf(operand), uloc);
                return null;
            }
        }
        return result;
    }

    @Override
    public Object visitLogicalOperator(LogicalOperatorContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitUnaryLogicalOperation(UnaryLogicalOperationContext ctx) {
        Object result = visit(ctx.relation());
        if (ctx.unaryLogicalOperator() != null) {
            if (result instanceof Boolean) {
                switch (ctx.unaryLogicalOperator().getText()) {
                case "not":
                    result = !((Boolean) result);
                    break;
                default:
                    throw new AssertionError();
                }
            } else {
                typeMismatch(TYPE_BOOL, typeOf(result), ctx.relation());
                return null;
            }
        }
        return result;
    }

    @Override
    public Object visitUnaryLogicalOperator(UnaryLogicalOperatorContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitRelation(RelationContext ctx) {
        Object result = visit(ctx.addition(0));
        if (ctx.relationalOperator() != null && ctx.addition().size() == 2) {
            final RelationalOperatorContext roc = ctx.relationalOperator();
            final AdditionContext ac = ctx.addition(1);
            final String operator = roc.getText();
            final Object operand = visit(ac);
            switch (operator) {
            case "<=":
            case "<>":
            case "<":
            case "=":
            case ">=":
            case ">":
                if (result instanceof BigInteger && operand instanceof BigInteger) {
                    switch (operator) {
                    case "<=":
                        result = ((BigInteger) result).compareTo((BigInteger) operand) <= 0;
                        break;
                    case "<>":
                        result = ((BigInteger) result).compareTo((BigInteger) operand) != 0;
                        break;
                    case "<":
                        result = ((BigInteger) result).compareTo((BigInteger) operand) < 0;
                        break;
                    case "=":
                        result = ((BigInteger) result).compareTo((BigInteger) operand) == 0;
                        break;
                    case ">=":
                        result = ((BigInteger) result).compareTo((BigInteger) operand) >= 0;
                        break;
                    case ">":
                        result = ((BigInteger) result).compareTo((BigInteger) operand) > 0;
                        break;
                    default:
                        throw new AssertionError();
                    }
                } else {
                    typeMismatch(TYPE_INT + " and " + TYPE_INT, typeOf(result) + " and " + typeOf(operand), ac);
                    return null;
                }
                break;
            case "eq":
            case "ne":
                if (result instanceof String && operand instanceof String) {
                    switch (operator) {
                    case "eq":
                        result = ((String) result).equals((String) operand);
                        break;
                    case "ne":
                        result = !((String) result).equals((String) operand);
                        break;
                    default:
                        throw new AssertionError();
                    }
                } else {
                    typeMismatch(TYPE_STR + " and " + TYPE_STR, typeOf(result) + " and " + typeOf(operand), ac);
                    return null;
                }
                break;
            default:
                throw new AssertionError();
            }
        }
        return result;
    }

    @Override
    public Object visitRelationalOperator(RelationalOperatorContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitAddition(AdditionContext ctx) {
        Object result = visit(ctx.multiplication(0));
        for (int i = 0; i < ctx.additionalOperator().size(); i++) {
            final AdditionalOperatorContext aoc = ctx.additionalOperator(i);
            final MultiplicationContext mc = ctx.multiplication(i + 1);
            final String operator = aoc.getText();
            final Object operand = visit(mc);
            switch (operator) {
            case "+":
            case "-":
                if (result instanceof BigInteger && operand instanceof BigInteger) {
                    switch (operator) {
                    case "+":
                        result = ((BigInteger) result).add((BigInteger) operand);
                        break;
                    case "-":
                        result = ((BigInteger) result).subtract((BigInteger) operand);
                        break;
                    default:
                        throw new AssertionError();
                    }
                } else {
                    typeMismatch(TYPE_INT + " and " + TYPE_INT, typeOf(result) + " and " + typeOf(operand), mc);
                    return null;
                }
                break;
            case "&":
                if (result instanceof String && operand instanceof String) {
                    result = ((String) result) + (String) operand;
                } else {
                    typeMismatch(TYPE_STR + " and " + TYPE_STR, typeOf(result) + " and " + typeOf(operand), mc);
                    return null;
                }
                break;
            default:
                throw new AssertionError();
            }
        }
        return result;
    }

    @Override
    public Object visitAdditionalOperator(AdditionalOperatorContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitMultiplication(MultiplicationContext ctx) {
        Object result = visit(ctx.unaryAddition(0));
        for (int i = 0; i < ctx.multiplicationOperator().size(); i++) {
            final MultiplicationOperatorContext moc = ctx.multiplicationOperator(i);
            final UnaryAdditionContext uac = ctx.unaryAddition(i + 1);
            final String operator = moc.getText();
            final Object operand = visit(uac);
            if (result instanceof BigInteger && operand instanceof BigInteger) {
                switch (operator) {
                case "*":
                    result = ((BigInteger) result).multiply((BigInteger) operand);
                    break;
                case "/":
                    result = ((BigInteger) result).divide((BigInteger) operand);
                    break;
                case "mod":
                    result = ((BigInteger) result).mod((BigInteger) operand);
                    break;
                case "rem":
                    result = ((BigInteger) result).remainder((BigInteger) operand);
                    break;
                default:
                    throw new AssertionError();
                }
            } else {
                typeMismatch(TYPE_INT + " and " + TYPE_INT, typeOf(result) + " and " + typeOf(operand), uac);
                return null;
            }
        }
        return result;
    }

    @Override
    public Object visitMultiplicationOperator(MultiplicationOperatorContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitUnaryAddition(UnaryAdditionContext ctx) {
        Object result = visit(ctx.factor());
        if (ctx.unaryAdditionalOperator() != null) {
            if (result instanceof BigInteger) {
                switch (ctx.unaryAdditionalOperator().getText()) {
                case "+":
                    /* do nothing */
                    break;
                case "-":
                    result = ((BigInteger) result).negate();
                    break;
                default:
                    throw new AssertionError();
                }
            } else {
                typeMismatch(TYPE_INT, typeOf(result), ctx.factor());
                return null;
            }
        }
        return result;
    }

    @Override
    public Object visitUnaryAdditionalOperator(UnaryAdditionalOperatorContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitFactor(FactorContext ctx) {
        if (ctx.bracketExpression() != null) {
            return visit(ctx.bracketExpression());
        } else if (ctx.absExpression() != null) {
            return visit(ctx.absExpression());
        } else if (ctx.dictAggregate() != null) {
            return visit(ctx.dictAggregate());
        } else if (ctx.literal() != null) {
            return visit(ctx.literal());
        } else if (ctx.functionCall() != null) {
            return visit(ctx.functionCall());
        } else if (ctx.accessExpression() != null) {
            return visit(ctx.accessExpression());
        } else {
            throw new AssertionError();
        }
    }

    @Override
    public Object visitBracketExpression(BracketExpressionContext ctx) {
        return visit(ctx.expression());
    }

    @Override
    public Object visitAbsExpression(AbsExpressionContext ctx) {
        Object result = visit(ctx.expression());
        if (result instanceof BigInteger) {
            result = ((BigInteger) result).abs();
        } else {
            typeMismatch(TYPE_INT, typeOf(result), ctx.expression());
            return null;
        }
        return result;
    }

    @Override
    public Object visitDictAggregate(DictAggregateContext ctx) {
        Map map = new TreeMap<>(DICT_COMPARATOR);
        for (int i = 0; i < ctx.dictAggregatePair().size(); i++) {
            final DictAggregatePairContext mapc = ctx.dictAggregatePair(i);
            final Object key = mapc.key != null ? visit(mapc.key) : BigInteger.valueOf(i);
            final Object value = visit(mapc.value);
            map.put(key, value);
        }
        return map;
    }

    @Override
    public Object visitDictAggregatePair(DictAggregatePairContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitDictAggregateKey(DictAggregateKeyContext ctx) {
        if (ctx.Identifier() != null) {
            return ctx.Identifier().getText();
        } else if (ctx.expression() != null) {
            return visit(ctx.expression());
        } else {
            throw new AssertionError();
        }
    }

    @Override
    public Object visitLiteral(LiteralContext ctx) {
        if (ctx.String() != null) {
            return dropQuotes(ctx.String().getText());
        } else if (ctx.Integer() != null) {
            String s = ctx.Integer().getText();
            int radix = 10;
            if (!s.matches("[0-9]+")) {
                if (s.startsWith("0b")) {
                    radix = 2;
                } else if (s.startsWith("0o")) {
                    radix = 8;
                } else if (s.startsWith("0x")) {
                    radix = 16;
                } else {
                    throw new AssertionError();
                }
                s = s.substring(2);
            }
            return new BigInteger(s, radix);
        } else if (ctx.False() != null) {
            return false;
        } else if (ctx.True() != null) {
            return true;
        } else {
            throw new AssertionError();
        }
    }

    @Override
    public Object visitFunctionCall(FunctionCallContext ctx) {
        final String identifier = ctx.Identifier().getText();
        List<Object> arguments = new ArrayList<>();
        for (ExpressionContext ec : ctx.arguments) {
            arguments.add(visit(ec));
        }
        Object result;
        switch (identifier) {
        case "bool2str": {
            if (!checkArgs(ctx, arguments, TYPE_BOOL)) {
                return null;
            }
            final Boolean arg1 = (Boolean) arguments.get(0);
            result = toString(arg1);
        }   break;
        case "boolean":
            result = checkArgsNoFail(ctx, arguments, TYPE_BOOL);
            break;
        case "char2ord": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            result = BigInteger.valueOf((long) arg1.charAt(0));
        }   break;
        case "charat": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            result = arg1.charAt(arg2.intValueExact());
        }   break;
        case "contains": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final String arg2 = (String) arguments.get(1);
            result = arg1.contains(arg2);
        }   break;
        case "dict2str": {
            if (!checkArgs(ctx, arguments, TYPE_DICT)) {
                return null;
            }
            final Map arg1 = (Map) arguments.get(0);
            result = toString(arg1);
        }   break;
        case "dictionary":
            result = checkArgsNoFail(ctx, arguments, TYPE_DICT);
            break;
        case "endswith": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final String arg2 = (String) arguments.get(1);
            result = arg1.endsWith(arg2);
        }   break;
        case "exit": {
            if (!checkArgs(ctx, arguments, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            result = arg1;
            System.exit(arg1.intValueExact());
        }   break;
        case "gcd": {
            if (!checkArgs(ctx, arguments, TYPE_INT, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            result = arg1.gcd(arg2);
        }   break;
        case "int2str": {
            if (!checkArgs(ctx, arguments, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            result = toString(arg1);
        }   break;
        case "integer":
            result = checkArgsNoFail(ctx, arguments, TYPE_INT);
            break;
        case "join": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_DICT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final Map arg2 = (Map) arguments.get(1);
            result = String.join(arg1, arg2.values());
        }   break;
        case "length": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            result = BigInteger.valueOf(arg1.length());
        }   break;
        case "ord2char": {
            if (!checkArgs(ctx, arguments, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            result = Character.toString((char) arg1.intValueExact());
        }   break;
        case "pow": {
            if (!checkArgs(ctx, arguments, TYPE_INT, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            result = arg1.pow(arg2.intValueExact());
        }   break;
        case "repeat": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            result = arg1.repeat(arg2.intValueExact());
        }   break;
        case "replace": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final String arg2 = (String) arguments.get(1);
            final String arg3 = (String) arguments.get(2);
            result = arg1.replace(arg2, arg3);
        }   break;
        case "sgn": {
            if (!checkArgs(ctx, arguments, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            result = BigInteger.valueOf(arg1.signum());
        }   break;
        case "size": {
            if (!checkArgs(ctx, arguments, TYPE_DICT)) {
                return null;
            }
            final Map arg1 = (Map) arguments.get(0);
            result = BigInteger.valueOf(arg1.size());
        }   break;
        case "sleep": {
            if (!checkArgs(ctx, arguments, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            try {
                Thread.sleep(arg1.longValueExact());
                result = true;
            } catch (InterruptedException e) {
                result = false;
            }
        }   break;
        case "split": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final String arg2 = (String) arguments.get(1);
            final String[] parts = arg2.split(arg1, -1);
            final Map resultMap = new TreeMap<>(DICT_COMPARATOR);
            for (int i = 0; i < parts.length; i++) {
                resultMap.put(BigInteger.valueOf(i), parts[i]);
            }
            result = resultMap;
        }   break;
        case "sqrt": {
            if (!checkArgs(ctx, arguments, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            result = arg1.sqrt();
        }   break;
        case "startswith": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final String arg2 = (String) arguments.get(1);
            result = arg1.endsWith(arg2);
        }   break;
        case "str2bool": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            result = toBoolean(arg1, ctx);
        }   break;
        case "str2dict": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            result = toDictionary(arg1, ctx);
        }   break;
        case "str2int": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            result = toInteger(arg1, ctx);
        }   break;
        case "string":
            result = checkArgsNoFail(ctx, arguments, TYPE_STR);
            break;
        case "substring": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            final BigInteger arg3 = (BigInteger) arguments.get(2);
            result = arg1.substring(arg2.intValueExact(), arg3.intValueExact());
        }   break;
        default:
            result = null; // TODO implement functions!
            break;
        }
        return result;
    }

    private static boolean checkArgs(FunctionCallContext ctx, List<Object> arguments, String... types) {
        return checkArgsHelper(ctx, arguments, true, types);
    }

    private static boolean checkArgsNoFail(FunctionCallContext ctx, List<Object> arguments, String... types) {
        return checkArgsHelper(ctx, arguments, false, types);
    }

    private static boolean checkArgsHelper(FunctionCallContext ctx, List<Object> arguments, boolean failOnTypeMismatch, String... types) {
        if (arguments.size() != types.length) {
            wrongNumberOfArguments(types.length, arguments.size(), ctx);
            return false;
        }
        for (int i = 0; i < types.length; i++) {
            final String type = types[i];
            final Object argument = arguments.get(i);
            if (!typeOf(argument).equals(type)) {
                if (failOnTypeMismatch) {
                    typeMismatch(type, typeOf(argument), ctx);
                }
                return false;
            }
        }
        return true;
    }

    @Override
    public Object visitAccessExpression(AccessExpressionContext ctx) {
        final String identifier = ctx.Identifier().getText();
        if (!variables.containsKey(identifier)) {
            variables.put(identifier, new TreeMap<>(DICT_COMPARATOR));
        }
        Object result = variables.get(identifier);
        for (AccessExpressionPartContext aepc : ctx.accessExpressionPart()) {
            Object key;
            if (aepc.Identifier() != null) {
                key = aepc.Identifier().getText();
            } else if (aepc.expression() != null) {
                key = visit(aepc.expression());
            } else {
                throw new AssertionError();
            }
            if (result instanceof Map) {
                final Map map = (Map) result;
                if (map.containsKey(key)) {
                    result = map.get(key);
                } else {
                    missingKey(map, key, aepc);
                    return null;
                }
            } else {
                typeMismatch(TYPE_DICT, typeOf(result), aepc);
                return null;
            }
        }
        return result;
    }

    @Override
    public Object visitAccessExpressionPart(AccessExpressionPartContext ctx) {
        throw new AssertionError();
    }

    private static String dropQuotes(String s) {
        if (s == null || s.length() < 2 || !s.startsWith("'") || !s.endsWith("'")) {
            throw new IllegalArgumentException();
        }
        return s.substring(1, s.length() - 1);
    }

    private static String typeOf(Object o) {
        if (o == null) {
            throw new NullPointerException();
        }
        if (o instanceof Map) {
            return TYPE_DICT;
        } else if (o instanceof String) {
            return TYPE_STR;
        } else if (o instanceof BigInteger) {
            return TYPE_INT;
        } else if (o instanceof Boolean) {
            return TYPE_BOOL;
        } else {
            throw new AssertionError();
        }
    }

    private static Boolean toBoolean(String s, ParserRuleContext ctx) {
        if (s == null) {
            throw new NullPointerException();
        }
        if (s.equals("false")) {
            return false;
        } else if (s.equals("true")) {
            return true;
        } else {
            parsingError(TYPE_BOOL, s, ctx);
            return null;
        }
    }

    private static Map toDictionary(String s, ParserRuleContext ctx) {
        if (s == null) {
            throw new NullPointerException();
        }
        try {
            AquilaLexer lexer = new AquilaLexer(CharStreams.fromString(s));
            AquilaParser parser = new AquilaParser(new CommonTokenStream(lexer));
            DictAggregateContext dictAggregate = parser.dictAggregate();
            if (parser.getNumberOfSyntaxErrors() == 0) {
                return (Map) new Interpreter(new String[0]).visit(dictAggregate);
            }
        } catch (Exception e) {
            /* do nothing */
        }
        parsingError(TYPE_DICT, s, ctx);
        return null;
    }

    private static BigInteger toInteger(String s, ParserRuleContext ctx) {
        if (s == null) {
            throw new NullPointerException();
        }
        if (s.matches("[0-9]+")) {
            return new BigInteger(s);
        } else {
            parsingError(TYPE_INT, s, ctx);
            return null;
        }
    }

    private static String toString(Object o) {
        if (o == null) {
            throw new NullPointerException();
        }
        if (o instanceof Map) {
            final Map map = (Map) o;
            StringBuilder result = new StringBuilder();
            result.append("{\n");
            for (Object key : map.keySet()) {
                final Object value = map.get(key);
                result.append(toString(key) + ": " + toString(value) + ";\n");
            }
            result.append("}");
            return result.toString();
        } else if (o instanceof String) {
            return o.toString();
        } else if (o instanceof BigInteger) {
            return o.toString();
        } else if (o instanceof Boolean) {
            return ((Boolean) o) ? "true" : "false";
        } else {
            throw new AssertionError();
        }
    }

    private static void missingKey(Map expectedMap, Object wasKey, ParserRuleContext ctx) {
        error("Missing key! Expected one of: " + expectedMap.keySet() + ", but was key `" + toString(wasKey) + "`!", ctx);
    }

    private static void parsingError(String expectedType, String wasValue, ParserRuleContext ctx) {
        error("Parsing error! Expected type `" + expectedType + "`, but was `" + wasValue + "`!", ctx);
    }

    private static void typeMismatch(String expectedType, String wasType, ParserRuleContext ctx) {
        error("Type mismatch! Expected type `" + expectedType + "`, but was type `" + wasType + "`!", ctx);
    }

    private static void wrongKey(String expectedKey, Object wasKey, ParserRuleContext ctx) {
        error("Wrong key! Expected key `" + expectedKey + "`, but was key `" + toString(wasKey) + "`!", ctx);
    }

    private static void wrongNumberOfArguments(int expected, int was, ParserRuleContext ctx) {
        error("Wrong number of arguments! Expected " + expected + ", but was " + was + "!", ctx);
    }

    private static void error(String msg, ParserRuleContext ctx) {
        final Token token = ctx.getStart();
        System.err.println("ERROR: " + msg + " @L" + token.getLine() + ":C" + token.getCharPositionInLine() + ":`" + ctx.getText() + "`");
        System.exit(1);
    }

}
