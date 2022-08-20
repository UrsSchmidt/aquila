package aquila4j;

import aquila4j.antlr.AquilaLexer;
import aquila4j.antlr.AquilaParser;
import aquila4j.antlr.AquilaParser.ProgramContext;
import aquila4j.antlr.AquilaParser.StatementContext;
import aquila4j.antlr.AquilaParser.IfStatementContext;
import aquila4j.antlr.AquilaParser.SwitchStatementContext;
import aquila4j.antlr.AquilaParser.SwitchStatementLabelsContext;
import aquila4j.antlr.AquilaParser.LoopStatementContext;
import aquila4j.antlr.AquilaParser.ForStatementContext;
import aquila4j.antlr.AquilaParser.CallStatementContext;
import aquila4j.antlr.AquilaParser.ExitStatementContext;
import aquila4j.antlr.AquilaParser.NowStatementContext;
import aquila4j.antlr.AquilaParser.RandomStatementContext;
import aquila4j.antlr.AquilaParser.ReadStatementContext;
import aquila4j.antlr.AquilaParser.RemoveStatementContext;
import aquila4j.antlr.AquilaParser.RunStatementContext;
import aquila4j.antlr.AquilaParser.SleepStatementContext;
import aquila4j.antlr.AquilaParser.WriteStatementContext;
import aquila4j.antlr.AquilaParser.AssignStatementContext;
import aquila4j.antlr.AquilaParser.BlockContext;
import aquila4j.antlr.AquilaParser.LhsContext;
import aquila4j.antlr.AquilaParser.LhsPartContext;
import aquila4j.antlr.AquilaParser.ExpressionContext;
import aquila4j.antlr.AquilaParser.IfExpressionContext;
import aquila4j.antlr.AquilaParser.LetExpressionContext;
import aquila4j.antlr.AquilaParser.LetBindExpressionContext;
import aquila4j.antlr.AquilaParser.SwitchExpressionContext;
import aquila4j.antlr.AquilaParser.SwitchExpressionLabelsContext;
import aquila4j.antlr.AquilaParser.LogicalOperationContext;
import aquila4j.antlr.AquilaParser.LogicalOperatorContext;
import aquila4j.antlr.AquilaParser.UnaryLogicalOperationContext;
import aquila4j.antlr.AquilaParser.UnaryLogicalOperatorContext;
import aquila4j.antlr.AquilaParser.RelationContext;
import aquila4j.antlr.AquilaParser.RelationalOperatorContext;
import aquila4j.antlr.AquilaParser.AdditionContext;
import aquila4j.antlr.AquilaParser.AdditionalOperatorContext;
import aquila4j.antlr.AquilaParser.MultiplicationContext;
import aquila4j.antlr.AquilaParser.MultiplicationOperatorContext;
import aquila4j.antlr.AquilaParser.UnaryAdditionContext;
import aquila4j.antlr.AquilaParser.UnaryAdditionalOperatorContext;
import aquila4j.antlr.AquilaParser.FactorialContext;
import aquila4j.antlr.AquilaParser.FactorialOperatorContext;
import aquila4j.antlr.AquilaParser.FactorContext;
import aquila4j.antlr.AquilaParser.BracketExpressionContext;
import aquila4j.antlr.AquilaParser.AbsExpressionContext;
import aquila4j.antlr.AquilaParser.DictAggregateContext;
import aquila4j.antlr.AquilaParser.DictAggregatePairContext;
import aquila4j.antlr.AquilaParser.DictAggregateKeyContext;
import aquila4j.antlr.AquilaParser.LambdaExpressionContext;
import aquila4j.antlr.AquilaParser.LambdaExpressionParameterContext;
import aquila4j.antlr.AquilaParser.LiteralContext;
import aquila4j.antlr.AquilaParser.FunctionCallContext;
import aquila4j.antlr.AquilaParser.AccessExpressionContext;
import aquila4j.antlr.AquilaParser.AccessExpressionPartContext;
import aquila4j.antlr.AquilaVisitor;

import java.io.File;
import java.io.IOException;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.Stack;
import java.util.TreeMap;
import java.util.function.BiConsumer;

import org.antlr.v4.runtime.CharStreams;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.ParserRuleContext;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.tree.AbstractParseTreeVisitor;

public class Interpreter extends AbstractParseTreeVisitor<Object> implements AquilaVisitor<Object> {

    private static final boolean STRICT = true; // XXX make this an option

    private static final String TYPE_ANY  = "Any";
    private static final String TYPE_BOOL = "Boolean";
    private static final String TYPE_INT  = "Integer";
    private static final String TYPE_STR  = "String";
    private static final String TYPE_FUNC = "Function";
    private static final String TYPE_DICT = "Dictionary";

    private static final Comparator<String> DICT_COMPARATOR = new Comparator<>() {
        @Override
        public int compare(String s1, String s2) {
            return Helper.strIsNumeric(s1) && Helper.strIsNumeric(s2)
                ? new BigInteger(s1).compareTo(new BigInteger(s2))
                : s1.compareTo(s2);
        }
    };

    /* variables are shared between multiple instances
       to allow for function declarations to be used in different files */
    private static Stack<Map> variables = new Stack<>();

    private final File scriptRoot;

    private static String typeOf(Object o) {
        if (o == null) {
            return "<err:no-value>";
        } else if (o instanceof Boolean) {
            return TYPE_BOOL;
        } else if (o instanceof BigInteger) {
            return TYPE_INT;
        } else if (o instanceof String) {
            return TYPE_STR;
        } else if (o instanceof LambdaExpressionContext) {
            return TYPE_FUNC;
        } else if (o instanceof Map) {
            return TYPE_DICT;
        } else {
            return "<err:invalid-type>";
        }
    }

    private static Boolean toBoolean(String s, ParserRuleContext ctx) {
        return s != null && s.equals("true");
    }

    private static BigInteger toInteger(String s, ParserRuleContext ctx) {
        BigInteger result;
        if (Helper.strIsNumeric(s)) {
            result = new BigInteger(s);
        } else {
            result = BigInteger.ZERO;
        }
        return result;
    }

    private static String toString(Object o) {
        String result;
        if (o == null) {
            result = "<err:no-value>";
        } else if (o instanceof Boolean) {
            final Boolean b = (Boolean) o;
            result = b ? "true" : "false";
        } else if (o instanceof BigInteger) {
            final BigInteger i = (BigInteger) o;
            result = i.toString();
        } else if (o instanceof String) {
            final String s = (String) o;
            result = s.toString();
        } else if (o instanceof LambdaExpressionContext) {
            final LambdaExpressionContext f = (LambdaExpressionContext) o;
            result = f.getText();
        } else if (o instanceof Map) {
            final Map<String, Object> d = (Map) o;
            StringBuilder sb = new StringBuilder();
            sb.append("{\n");
            for (String key : d.keySet()) {
                final Object value = d.get(key);
                sb.append(key).append(": ");
                if (value instanceof String) {
                    sb.append("'").append(toString(value)).append("'");
                } else {
                    sb.append(toString(value));
                }
                sb.append(",\n");
            }
            sb.append("}");
            result = sb.toString();
        } else {
            result = "<err:invalid-type>";
        }
        return result;
    }

    private static Map toDictionary(String s, ParserRuleContext ctx) {
        if (s != null) {
            try {
                // FIXME rewrite this to not copy code from main
                AquilaLexer lexer = new AquilaLexer(CharStreams.fromString(s));
                AquilaParser parser = new AquilaParser(new CommonTokenStream(lexer));
                DictAggregateContext dictAggregate = parser.dictAggregate();
                if (parser.getNumberOfSyntaxErrors() == 0) {
                    return (Map) new Interpreter(new String[0]).visit(dictAggregate);
                }
            } catch (Exception e) {
                /* do nothing */
            }
        }
        return new TreeMap<>(DICT_COMPARATOR);
    }

    private static void error(String msg, ParserRuleContext ctx) {
        final Token token = ctx.getStart();
        System.err.println("ERROR: " + msg + " @L" + token.getLine() + ":C" + token.getCharPositionInLine() + ":`" + ctx.getText() + "`");
        System.exit(1);
    }

    private static void exception(Exception e, ParserRuleContext ctx) {
        error("An exception occurred: " + e.getClass().getSimpleName() + ": `" + e.getMessage() + "`", ctx);
    }

    private static void fileNotFound(File file, ParserRuleContext ctx) {
        error("File not found! Expected: " + file, ctx);
    }

    private static void missingKey(Map expected, String wasKey, ParserRuleContext ctx) {
        error("Missing key! Expected one of: " + expected.keySet() + ", but was key `" + wasKey + "`!", ctx);
    }

    private static void expectedNonVoidFunction(ParserRuleContext ctx) {
        error("Expected non-void " + TYPE_FUNC + "!", ctx);
    }

    private static void typeMismatch(String expectedType, Object wasValue, ParserRuleContext ctx) {
        error("Type mismatch! Expected type `" + expectedType + "`, but was `" + toString(wasValue) + "` which is of type `" + typeOf(wasValue) + "`!", ctx);
    }

    private static void wrongKey(String expectedKey, String wasKey, ParserRuleContext ctx) {
        error("Wrong key! Expected key `" + expectedKey + "`, but was key `" + wasKey + "`!", ctx);
    }

    private static void wrongNumberOfArguments(int expected, int was, ParserRuleContext ctx) {
        error("Wrong number of arguments! Expected " + expected + ", but was " + was + "!", ctx);
    }

    private static void assign(Map d, String key, Object value, ParserRuleContext ctx) {
        if (STRICT && d.containsKey(key)) {
            final String expected = typeOf(d.get(key));
            final String was = typeOf(value);
            if (!expected.equals(was)) {
                typeMismatch(expected, value, ctx);
                return;
            }
        }
        d.put(key, value);
    }

    public Interpreter(String[] args) {
        super();
        scriptRoot = args.length > 0 ? new File(args[0]).getParentFile() : null;
        Map root = new TreeMap<>(DICT_COMPARATOR);

        /* passing command line arguments */
        Map dArgs = new TreeMap<>(DICT_COMPARATOR);
        for (int i = 0; i < args.length; i++) {
            dArgs.put(Integer.toString(i), args[i]);
        }
        root.put("args", dArgs);

        /* passing environment variables */
        Map dEnv = new TreeMap<>(DICT_COMPARATOR);
        final Map<String, String> env = System.getenv();
        for (String envName : env.keySet()) {
            dEnv.put(envName, env.get(envName));
        }
        root.put("env", dEnv);

        variables.push(root);
    }

    private void handleLhs(LhsContext lhsc, BiConsumer<Map, String> handler) {
        final String identifier = lhsc.Identifier().getText();
        if (lhsc.lhsPart().isEmpty()) {
            handler.accept(variables.peek(), identifier);
        } else {
            if (!variables.peek().containsKey(identifier)) {
                variables.peek().put(identifier, new TreeMap<>(DICT_COMPARATOR));
            }
            Object a1 = variables.peek().get(identifier);
            if (!(a1 instanceof Map)) {
                typeMismatch(TYPE_DICT, a1, lhsc);
                return;
            }
            Map d = (Map) a1;
            for (int i = 0; i < lhsc.lhsPart().size(); i++) {
                final LhsPartContext lhspc = lhsc.lhsPart(i);
                String key;
                if (lhspc.Identifier() != null) {
                    key = lhspc.Identifier().getText();
                } else if (lhspc.expression() != null) {
                    key = toString(visit(lhspc.expression()));
                } else {
                    throw new AssertionError();
                }
                if (i < lhsc.lhsPart().size() - 1) {
                    if (!d.containsKey(key)) {
                        d.put(key, new TreeMap<>(DICT_COMPARATOR));
                    }
                    Object a2 = d.get(key);
                    if (!(a2 instanceof Map)) {
                        typeMismatch(TYPE_DICT, a2, lhspc);
                        return;
                    }
                    d = (Map) a2;
                } else {
                    handler.accept(d, key);
                }
            }
        }
    }

    private Object callFunction(FunctionCallContext callsite, LambdaExpressionContext function, List<Object> arguments) {
        List<String> parameters = new ArrayList<>();
        List<String> types = new ArrayList<>();
        for (LambdaExpressionParameterContext lepc : function.lambdaExpressionParameter()) {
            parameters.add(lepc.Identifier().getText());
            types.add(lepc.Type() != null ? lepc.Type().getText() : TYPE_ANY);
        }
        if (!checkArgs(callsite, arguments, types.toArray(new String[0]))) {
            return null;
        }
        variables.push(new TreeMap<>(variables.peek()));
        for (int i = 0; i < parameters.size(); i++) {
            final String parameter = parameters.get(i);
            final Object argument = arguments.get(i);
            // TODO does this have to be fully qualified?
            variables.peek().put(parameter, argument);
        }
        Object result;
        if (function.expression() != null) {
            result = visit(function.expression());
        } else if (function.block() != null) {
            visit(function.block());
            result = null;
        } else {
            throw new AssertionError();
        }
        variables.pop();
        return result;
    }

    private static boolean checkArgs(FunctionCallContext callsite, List<Object> arguments, String... types) {
        return checkArgsHelper(callsite, arguments, true, types);
    }

    private static boolean checkArgsNoFail(FunctionCallContext callsite, List<Object> arguments, String... types) {
        return checkArgsHelper(callsite, arguments, false, types);
    }

    private static boolean checkArgsHelper(FunctionCallContext callsite, List<Object> arguments, boolean failOnTypeMismatch, String... types) {
        if (arguments.size() != types.length) {
            wrongNumberOfArguments(types.length, arguments.size(), callsite);
            return false;
        }
        for (int i = 0; i < types.length; i++) {
            final String type = types[i];
            final Object argument = arguments.get(i);
            if (!type.equals(TYPE_ANY) && !typeOf(argument).equals(type)) {
                if (failOnTypeMismatch) {
                    typeMismatch(type, argument, callsite.arguments.get(i));
                }
                return false;
            }
        }
        return true;
    }

    @Override
    public Object visitProgram(ProgramContext ctx) {
        for (StatementContext sc : ctx.statement()) {
            try {
                visit(sc);
            } catch (Exception e) {
                exception(e, sc);
                return null;
            }
        }
        return null;
    }

    @Override
    public Object visitStatement(StatementContext ctx) {
        return visitChildren(ctx);
    }

    @Override
    public Object visitIfStatement(IfStatementContext ctx) {
        for (int i = 0; i < ctx.condition.size(); i++) {
            final Object condition = visit(ctx.condition.get(i));
            if (Boolean.TRUE.equals(condition)) {
                visit(ctx.then.get(i));
                return null;
            }
        }
        if (ctx.elseBlock != null) {
            visit(ctx.elseBlock);
        }
        return null;
    }

    @Override
    public Object visitSwitchStatement(SwitchStatementContext ctx) {
        final Object switchHeadExpression = visit(ctx.switchHeadExpression);
        for (int i = 0; i < ctx.labels.size(); i++) {
            final SwitchStatementLabelsContext sslc = ctx.labels.get(i);
            for (int j = 0; j < sslc.expression().size(); j++) {
                final Object label = visit(sslc.expression(j));
                if (switchHeadExpression.equals(label)) {
                    visit(ctx.then.get(i));
                    return null;
                }
            }
        }
        if (ctx.defaultBlock != null) {
            visit(ctx.defaultBlock);
        }
        return null;
    }

    @Override
    public Object visitSwitchStatementLabels(SwitchStatementLabelsContext ctx) {
        throw new AssertionError();
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
        /* from */
        final Object from = visit(ctx.from);
        if (!(from instanceof BigInteger)) {
            typeMismatch(TYPE_INT, from, ctx.from);
            return null;
        }
        final BigInteger fromInt = (BigInteger) from;
        /* to */
        final Object to = visit(ctx.to);
        if (!(to instanceof BigInteger)) {
            typeMismatch(TYPE_INT, to, ctx.to);
            return null;
        }
        final BigInteger toInt = (BigInteger) to;
        /* step */
        final BigInteger stepInt;
        if (ctx.step != null) {
            final Object step = visit(ctx.step);
            if (!(step instanceof BigInteger)) {
                typeMismatch(TYPE_INT, step, ctx.step);
                return null;
            }
            stepInt = (BigInteger) step;
        } else {
            stepInt = BigInteger.ONE;
        }
        Map d = variables.peek();
        final boolean overridesValue = d.containsKey(identifier);
        Object overriddenValue = null;
        if (overridesValue) {
            overriddenValue = d.get(identifier);
        }
        for (BigInteger i = fromInt; stepInt.signum() >= 0 ? i.compareTo(toInt) <= 0 : i.compareTo(toInt) >= 0; i = i.add(stepInt)) {
            d.put(identifier, i);
            visit(ctx.block());
        }
        if (overridesValue) {
            d.put(identifier, overriddenValue);
        } else {
            d.remove(identifier);
        }
        return null;
    }

    @Override
    public Object visitCallStatement(CallStatementContext ctx) {
        visit(ctx.functionCall());
        return null;
    }

    @Override
    public Object visitExitStatement(ExitStatementContext ctx) {
        final Object rhs = visit(ctx.rhs);
        if (rhs instanceof BigInteger) {
            System.exit(((BigInteger) rhs).intValueExact());
        } else {
            typeMismatch(TYPE_INT, rhs, ctx.rhs);
            return null;
        }
        return null;
    }

    @Override
    public Object visitNowStatement(NowStatementContext ctx) {
        final BigInteger now = BigInteger.valueOf(System.currentTimeMillis());
        handleLhs(ctx.lhs(), (d, key) -> assign(d, key, now, ctx));
        return null;
    }

    @Override
    public Object visitRandomStatement(RandomStatementContext ctx) {
        /* from */
        final Object from = visit(ctx.from);
        if (!(from instanceof BigInteger)) {
            typeMismatch(TYPE_INT, from, ctx.from);
            return null;
        }
        final BigInteger fromInt = (BigInteger) from;
        /* to */
        final Object to = visit(ctx.to);
        if (!(to instanceof BigInteger)) {
            typeMismatch(TYPE_INT, to, ctx.to);
            return null;
        }
        final BigInteger toInt = (BigInteger) to;
        /* random */
        final BigInteger random = BigInteger.valueOf((long) ((toInt.subtract(fromInt).longValueExact()) * Math.random())).add(fromInt);
        handleLhs(ctx.lhs(), (d, key) -> assign(d, key, random, ctx));
        return null;
    }

    @Override
    public Object visitReadStatement(ReadStatementContext ctx) {
        final String line = new Scanner(System.in).nextLine();
        handleLhs(ctx.lhs(), (d, key) -> assign(d, key, line, ctx));
        return null;
    }

    @Override
    public Object visitRemoveStatement(RemoveStatementContext ctx) {
        handleLhs(ctx.lhs(), (d, key) -> {
            if (d.containsKey(key)) {
                d.remove(key);
            } else {
                missingKey(d, key, ctx);
                return;
            }
        });
        return null;
    }

    @Override
    public Object visitRunStatement(RunStatementContext ctx) {
        final Object rhs = visit(ctx.rhs);
        if (rhs instanceof String) {
            final File file = new File(scriptRoot, (String) rhs);
            if (file.exists()) {
                try {
                    // FIXME pass further arguments
                    Main.run(new String[] { file.toString() });
                } catch (IOException e) {
                    exception(e, ctx.rhs);
                    return null;
                }
            } else {
                fileNotFound(file, ctx.rhs);
                return null;
            }
        } else {
            typeMismatch(TYPE_STR, rhs, ctx.rhs);
            return null;
        }
        return null;
    }

    @Override
    public Object visitSleepStatement(SleepStatementContext ctx) {
        final Object rhs = visit(ctx.rhs);
        if (rhs instanceof BigInteger) {
            try {
                Thread.sleep(((BigInteger) rhs).longValueExact());
            } catch (InterruptedException e) {
                /* do nothing */
            }
        } else {
            typeMismatch(TYPE_INT, rhs, ctx.rhs);
            return null;
        }
        return null;
    }

    @Override
    public Object visitWriteStatement(WriteStatementContext ctx) {
        final Object rhs = visit(ctx.rhs);
        if (rhs instanceof String) {
            System.out.println((String) rhs);
        } else {
            typeMismatch(TYPE_STR, rhs, ctx.rhs);
            return null;
        }
        return null;
    }

    @Override
    public Object visitAssignStatement(AssignStatementContext ctx) {
        final Object rhs = visit(ctx.rhs);
        handleLhs(ctx.lhs(), (d, key) -> assign(d, key, rhs, ctx.rhs));
        return null;
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
    public Object visitIfExpression(IfExpressionContext ctx) {
        for (int i = 0; i < ctx.condition.size(); i++) {
            final Object condition = visit(ctx.condition.get(i));
            if (Boolean.TRUE.equals(condition)) {
                return visit(ctx.then.get(i));
            }
        }
        return visit(ctx.elseExpression);
    }

    @Override
    public Object visitLetExpression(LetExpressionContext ctx) {
        variables.push(new TreeMap<>(variables.peek()));
        for (LetBindExpressionContext lbec : ctx.letBindExpression()) {
            final Object rhs = visit(lbec.rhs);
            handleLhs(lbec.lhs(), (d, key) -> assign(d, key, rhs, lbec));
        }
        final Object result = visit(ctx.body);
        variables.pop();
        return result;
    }

    @Override
    public Object visitLetBindExpression(LetBindExpressionContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitSwitchExpression(SwitchExpressionContext ctx) {
        final Object switchHeadExpression = visit(ctx.switchHeadExpression);
        for (int i = 0; i < ctx.labels.size(); i++) {
            final SwitchExpressionLabelsContext selc = ctx.labels.get(i);
            for (int j = 0; j < selc.expression().size(); j++) {
                final Object label = visit(selc.expression(j));
                if (switchHeadExpression.equals(label)) {
                    return visit(ctx.then.get(i));
                }
            }
        }
        return visit(ctx.defaultExpression);
    }

    @Override
    public Object visitSwitchExpressionLabels(SwitchExpressionLabelsContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitLogicalOperation(LogicalOperationContext ctx) {
        Object result = visit(ctx.unaryLogicalOperation(0));
        for (int i = 0; i < ctx.logicalOperator().size(); i++) {
            final LogicalOperatorContext loc = ctx.logicalOperator(i);
            final UnaryLogicalOperationContext uloc = ctx.unaryLogicalOperation(i + 1);
            final String op = loc.getText();
            final Object operand = visit(uloc);
            if (result instanceof Boolean) {
                if (operand instanceof Boolean) {
                    switch (op) {
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
                    typeMismatch(TYPE_BOOL, operand, uloc);
                    return null;
                }
            } else {
                typeMismatch(TYPE_BOOL, result, ctx);
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
        final RelationContext rc = ctx.relation();
        Object result = visit(rc);
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
                typeMismatch(TYPE_BOOL, result, rc);
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
            final String op = roc.getText();
            final Object operand = visit(ac);
            switch (op) {
            case ":":
                if (operand instanceof Map) {
                    result = ((Map) operand).containsValue(result);
                } else {
                    typeMismatch(TYPE_DICT, operand, ac);
                    return null;
                }
                break;
            case "<=":
            case "<>":
            case "<":
            case "=":
            case ">=":
            case ">":
                if (result instanceof BigInteger) {
                    if (operand instanceof BigInteger) {
                        switch (op) {
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
                        typeMismatch(TYPE_INT, operand, ac);
                        return null;
                    }
                } else {
                    typeMismatch(TYPE_INT, result, ctx);
                    return null;
                }
                break;
            case "eq":
            case "ew":
            case "in":
            case "ne":
            case "sw":
                if (result instanceof String) {
                    if (operand instanceof String) {
                        switch (op) {
                        case "eq":
                            result = ((String) result).equals((String) operand);
                            break;
                        case "ew":
                            result = ((String) result).endsWith((String) operand);
                            break;
                        case "in":
                            result = ((String) operand).contains((String) result);
                            break;
                        case "ne":
                            result = !((String) result).equals((String) operand);
                            break;
                        case "sw":
                            result = ((String) result).startsWith((String) operand);
                            break;
                        default:
                            throw new AssertionError();
                        }
                    } else {
                        typeMismatch(TYPE_STR, operand, ac);
                        return null;
                    }
                } else {
                    typeMismatch(TYPE_STR, result, ctx);
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
            final String op = aoc.getText();
            final Object operand = visit(mc);
            switch (op) {
            case "&":
                if (result instanceof String) {
                    if (operand instanceof String) {
                        result = ((String) result) + (String) operand;
                    } else {
                        typeMismatch(TYPE_STR, operand, mc);
                        return null;
                    }
                } else {
                    typeMismatch(TYPE_STR, result, ctx);
                    return null;
                }
                break;
            case "+":
            case "-":
                if (result instanceof BigInteger) {
                    if (operand instanceof BigInteger) {
                        switch (op) {
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
                        typeMismatch(TYPE_INT, operand, mc);
                        return null;
                    }
                } else {
                    typeMismatch(TYPE_INT, result, ctx);
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
            final String op = moc.getText();
            final Object operand = visit(uac);
            if (result instanceof BigInteger) {
                if (operand instanceof BigInteger) {
                    switch (op) {
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
                    typeMismatch(TYPE_INT, operand, uac);
                    return null;
                }
            } else {
                typeMismatch(TYPE_INT, result, ctx);
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
        final FactorialContext fc = ctx.factorial();
        Object result = visit(fc);
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
                typeMismatch(TYPE_INT, result, fc);
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
    public Object visitFactorial(FactorialContext ctx) {
        final FactorContext fc = ctx.factor();
        Object result = visit(fc);
        if (ctx.factorialOperator() != null) {
            if (result instanceof BigInteger) {
                switch (ctx.factorialOperator().getText()) {
                case "!":
                    result = Helper.intFactorial((BigInteger) result);
                    break;
                default:
                    throw new AssertionError();
                }
            } else {
                typeMismatch(TYPE_INT, result, fc);
                return null;
            }
        }
        return result;
    }

    @Override
    public Object visitFactorialOperator(FactorialOperatorContext ctx) {
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
        } else if (ctx.lambdaExpression() != null) {
            return visit(ctx.lambdaExpression());
        } else if (ctx.literal() != null) {
            return visit(ctx.literal());
        } else if (ctx.functionCall() != null) {
            final Object result = visit(ctx.functionCall());
            if (result == null) {
                expectedNonVoidFunction(ctx.functionCall());
                return null;
            }
            return result;
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
        final ExpressionContext ec = ctx.expression();
        Object result = visit(ec);
        if (result instanceof BigInteger) {
            result = ((BigInteger) result).abs();
        } else {
            typeMismatch(TYPE_INT, result, ec);
            return null;
        }
        return result;
    }

    @Override
    public Object visitDictAggregate(DictAggregateContext ctx) {
        Map result = new TreeMap<>(DICT_COMPARATOR);
        for (int i = 0; i < ctx.dictAggregatePair().size(); i++) {
            final DictAggregatePairContext dapc = ctx.dictAggregatePair(i);
            final String key = dapc.key != null
                ? toString(visit(dapc.key))
                : Integer.toString(i);
            final Object value = visit(dapc.value);
            result.put(key, value);
        }
        return result;
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
    public Object visitLambdaExpression(LambdaExpressionContext ctx) {
        return ctx;
    }

    @Override
    public Object visitLambdaExpressionParameter(LambdaExpressionParameterContext ctx) {
        throw new AssertionError();
    }

    @Override
    public Object visitLiteral(LiteralContext ctx) {
        if (ctx.String() != null) {
            return Helper.strDropQuotes(ctx.String().getText());
        } else if (ctx.Integer() != null) {
            String s = ctx.Integer().getText();
            int radix = 10;
            if (!Helper.strIsNumeric(s)) {
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
            result = Character.toString(arg1.charAt(arg2.intValueExact()));
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
        case "error": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            error(arg1, ctx);
            result = arg1;
        }   break;
        case "exists": {
            if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
                return null;
            }
            final Map<String, Object> arg1 = (Map) arguments.get(0);
            final LambdaExpressionContext arg2 = (LambdaExpressionContext) arguments.get(1);
            final List<Object> localArguments = new ArrayList<>();
            result = false;
            for (String key : arg1.keySet()) {
                final Object value = arg1.get(key);
                localArguments.clear();
                localArguments.add(key);
                localArguments.add(value);
                if (Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                    result = true;
                    break;
                }
            }
        }   break;
        case "filter": {
            if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
                return null;
            }
            final Map<String, Object> arg1 = (Map) arguments.get(0);
            final LambdaExpressionContext arg2 = (LambdaExpressionContext) arguments.get(1);
            final List<Object> localArguments = new ArrayList<>();
            final Map d = new TreeMap<>(DICT_COMPARATOR);
            for (String key : arg1.keySet()) {
                final Object value = arg1.get(key);
                localArguments.clear();
                localArguments.add(key);
                localArguments.add(value);
                if (Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                    d.put(key, value);
                }
            }
            result = d;
        }   break;
        case "findleft": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final String arg2 = (String) arguments.get(1);
            final BigInteger arg3 = (BigInteger) arguments.get(2);
            result = BigInteger.valueOf(arg1.indexOf(arg2, arg3.intValueExact()));
        }   break;
        case "findright": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final String arg2 = (String) arguments.get(1);
            final BigInteger arg3 = (BigInteger) arguments.get(2);
            result = BigInteger.valueOf(arg1.lastIndexOf(arg2, arg3.intValueExact()));
        }   break;
        case "fold": {
            if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_ANY, TYPE_FUNC)) {
                return null;
            }
            final Map<String, Object> arg1 = (Map) arguments.get(0);
            final Object arg2 = arguments.get(1);
            final LambdaExpressionContext arg3 = (LambdaExpressionContext) arguments.get(2);
            final List<Object> localArguments = new ArrayList<>();
            result = arg2;
            for (String key : arg1.keySet()) {
                final Object value = arg1.get(key);
                localArguments.clear();
                localArguments.add(result);
                localArguments.add(value);
                result = callFunction(ctx, arg3, localArguments);
            }
        }   break;
        case "forall": {
            if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
                return null;
            }
            final Map<String, Object> arg1 = (Map) arguments.get(0);
            final LambdaExpressionContext arg2 = (LambdaExpressionContext) arguments.get(1);
            final List<Object> localArguments = new ArrayList<>();
            result = true;
            for (String key : arg1.keySet()) {
                final Object value = arg1.get(key);
                localArguments.clear();
                localArguments.add(key);
                localArguments.add(value);
                if (!Boolean.TRUE.equals(callFunction(ctx, arg2, localArguments))) {
                    result = false;
                    break;
                }
            }
        }   break;
        case "foreach": {
            if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
                return null;
            }
            final Map<String, Object> arg1 = (Map) arguments.get(0);
            final LambdaExpressionContext arg2 = (LambdaExpressionContext) arguments.get(1);
            final List<Object> localArguments = new ArrayList<>();
            for (String key : arg1.keySet()) {
                final Object value = arg1.get(key);
                localArguments.clear();
                localArguments.add(key);
                localArguments.add(value);
                callFunction(ctx, arg2, localArguments);
            }
            result = null;
        }   break;
        case "function":
            result = checkArgsNoFail(ctx, arguments, TYPE_FUNC);
            break;
        case "gcd": {
            if (!checkArgs(ctx, arguments, TYPE_INT, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            result = arg1.gcd(arg2);
        }   break;
        case "head": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            result = arg1.isEmpty() ? "" : arg1.substring(0, 1);
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
            // FIXME this won't call my custom toString() method on the individual values
            result = String.join(arg1, arg2.values());
        }   break;
        case "left": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            result = arg1.substring(0, arg2.intValueExact());
        }   break;
        case "length": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            result = BigInteger.valueOf(arg1.length());
        }   break;
        case "map": {
            if (!checkArgs(ctx, arguments, TYPE_DICT, TYPE_FUNC)) {
                return null;
            }
            final Map<String, Object> arg1 = (Map) arguments.get(0);
            final LambdaExpressionContext arg2 = (LambdaExpressionContext) arguments.get(1);
            final List<Object> localArguments = new ArrayList<>();
            final Map d = new TreeMap<>(DICT_COMPARATOR);
            for (String key : arg1.keySet()) {
                final Object value = arg1.get(key);
                localArguments.clear();
                localArguments.add(key);
                localArguments.add(value);
                final Object newValue = callFunction(ctx, arg2, localArguments);
                d.put(key, newValue);
            }
            result = d;
        }   break;
        case "mid": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            final BigInteger arg3 = (BigInteger) arguments.get(2);
            result = arg1.substring(arg2.intValueExact(), arg2.intValueExact() + arg3.intValueExact());
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
        case "right": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            result = arg1.substring(arg1.length() - arg2.intValueExact());
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
        case "split": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final String arg2 = (String) arguments.get(1);
            final String[] parts = arg2.split(arg1, -1);
            final Map d = new TreeMap<>(DICT_COMPARATOR);
            for (int i = 0; i < parts.length; i++) {
                d.put(Integer.toString(i), parts[i]);
            }
            result = d;
        }   break;
        case "sqrt": {
            if (!checkArgs(ctx, arguments, TYPE_INT)) {
                return null;
            }
            final BigInteger arg1 = (BigInteger) arguments.get(0);
            result = arg1.sqrt();
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
        case "substring1": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            result = arg1.substring(arg2.intValueExact());
        }   break;
        case "substring2": {
            if (!checkArgs(ctx, arguments, TYPE_STR, TYPE_INT, TYPE_INT)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            final BigInteger arg2 = (BigInteger) arguments.get(1);
            final BigInteger arg3 = (BigInteger) arguments.get(2);
            result = arg1.substring(arg2.intValueExact(), arg3.intValueExact());
        }   break;
        case "tail": {
            if (!checkArgs(ctx, arguments, TYPE_STR)) {
                return null;
            }
            final String arg1 = (String) arguments.get(0);
            result = arg1.isEmpty() ? "" : arg1.substring(1);
        }   break;
        default:
            final Map d = variables.peek();
            if (d.containsKey(identifier)) {
                final Object object = d.get(identifier);
                if (object instanceof LambdaExpressionContext) {
                    final LambdaExpressionContext function = (LambdaExpressionContext) object;
                    result = callFunction(ctx, function, arguments);
                } else {
                    typeMismatch(TYPE_FUNC, object, ctx);
                    return null;
                }
            } else {
                missingKey(d, identifier, ctx);
                return null;
            }
        }
        return result;
    }

    @Override
    public Object visitAccessExpression(AccessExpressionContext ctx) {
        final String identifier = ctx.Identifier().getText();
        Object result = variables.peek().get(identifier);
        for (AccessExpressionPartContext aepc : ctx.accessExpressionPart()) {
            String key;
            if (aepc.Identifier() != null) {
                key = aepc.Identifier().getText();
            } else if (aepc.expression() != null) {
                key = toString(visit(aepc.expression()));
            } else {
                throw new AssertionError();
            }
            if (result instanceof Map) {
                final Map d = (Map) result;
                if (d.containsKey(key)) {
                    result = d.get(key);
                } else {
                    missingKey(d, key, aepc);
                    return null;
                }
            } else {
                typeMismatch(TYPE_DICT, result, aepc);
                return null;
            }
        }
        return result;
    }

    @Override
    public Object visitAccessExpressionPart(AccessExpressionPartContext ctx) {
        throw new AssertionError();
    }

}
