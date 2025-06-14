#include "parser/Parser.hpp"
#include "evaluator/Evaluator.hpp"
#include "expr/Expr.hpp"
#include "evaluator/EvaluationContext.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <stdexcept>

using namespace aleph3;

TEST_CASE("Basic operations are evaluated correctly", "[evaluator]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("2 + 3");
    auto result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 5.0);
}

TEST_CASE("Multiplication and parentheses are evaluated correctly", "[evaluator]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("2 * (3 + 4)");
    auto result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 14.0);
}

TEST_CASE("Trigonometric functions are evaluated correctly", "[evaluator]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("Sin[0]");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::abs(get_number_value(result) - 0.0) < 1e-6);

    expr = parse_expression("Cos[0]");
    result = evaluate(expr, ctx);
    REQUIRE(std::abs(get_number_value(result) - 1.0) < 1e-6);
}

TEST_CASE("Square root and exponential functions are evaluated correctly", "[evaluator]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("Sqrt[9]");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::abs(get_number_value(result) - 3.0) < 1e-6);

    expr = parse_expression("Exp[1]");
    result = evaluate(expr, ctx);
    REQUIRE(std::abs(get_number_value(result) - std::exp(1)) < 1e-6);
}

TEST_CASE("Variables are evaluated correctly using context", "[evaluator]") {
    EvaluationContext ctx;
    ctx.variables["x"] = make_expr<Number>(10.0);
    ctx.variables["y"] = make_expr<Number>(5.0);

    auto expr = parse_expression("x + y");
    auto result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 15.0);

    expr = parse_expression("x * y");
    result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 50.0);
}

TEST_CASE("Evaluator correctly evaluates power expressions", "[evaluator][pow]") {
    auto expr = parse_expression("2^3");
    EvaluationContext ctx; // Empty context if required by evaluate
    auto result = evaluate(expr, ctx);

    REQUIRE(result != nullptr);

    auto num = std::get<Number>(*result);
    REQUIRE(std::abs(num.value - 8.0) < 1e-9); // Floating-point comparison
}

TEST_CASE("Exponential function is evaluated correctly", "[evaluator][exp]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("Exp[1]");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::abs(get_number_value(result) - std::exp(1)) < 1e-6);

    expr = parse_expression("Exp[0]");
    result = evaluate(expr, ctx);
    REQUIRE(std::abs(get_number_value(result) - 1.0) < 1e-6);
}

TEST_CASE("Floor function is evaluated correctly", "[evaluator][floor]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("Floor[3.7]");
    auto result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 3.0);

    expr = parse_expression("Floor[-3.7]");
    result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == -4.0);
}

TEST_CASE("Ceil function is evaluated correctly", "[evaluator][ceiling]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("Ceiling[3.2]");
    auto result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 4.0);

    expr = parse_expression("Ceiling[-3.2]");
    result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == -3.0);
}

TEST_CASE("Round function is evaluated correctly", "[evaluator][round]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("Round[3.5]");
    auto result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 4.0);

    expr = parse_expression("Round[3.4]");
    result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 3.0);

    expr = parse_expression("Round[-3.5]");
    result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == -4.0);

    expr = parse_expression("Round[-3.4]");
    result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == -3.0);
}

TEST_CASE("Simplify addition with zero", "[evaluator][simplification]") {
    EvaluationContext ctx; // Empty context

    auto expr = parse_expression("0 + x");
    auto result = evaluate(expr, ctx);

    // Ensure the result is a symbolic expression: x
    REQUIRE(std::holds_alternative<Symbol>(*result));
    REQUIRE(std::get<Symbol>(*result).name == "x");

    expr = parse_expression("x + 0");
    result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Symbol>(*result));
    REQUIRE(std::get<Symbol>(*result).name == "x");
}

TEST_CASE("Simplify multiplication with one", "[evaluator][simplification]") {
    EvaluationContext ctx; // Empty context

    auto expr = parse_expression("1 * x");
    auto result = evaluate(expr, ctx);

    // Ensure the result is a symbolic expression: x
    REQUIRE(std::holds_alternative<Symbol>(*result));
    REQUIRE(std::get<Symbol>(*result).name == "x");

    expr = parse_expression("x * 1");
    result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Symbol>(*result));
    REQUIRE(std::get<Symbol>(*result).name == "x");
}

TEST_CASE("Simplify multiplication with zero", "[evaluator][simplification]") {
    EvaluationContext ctx; // Empty context

    auto expr = parse_expression("0 * x");
    auto result = evaluate(expr, ctx);

    // Ensure the result is a numeric value: 0
    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(get_number_value(result) == 0.0);

    expr = parse_expression("x * 0");
    result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(get_number_value(result) == 0.0);
}

TEST_CASE("Simplify exponentiation", "[evaluator][simplification]") {
    EvaluationContext ctx; // Empty context

    auto expr = parse_expression("x^0");
    auto result = evaluate(expr, ctx);

    // Ensure the result is a numeric value: 1
    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(get_number_value(result) == 1.0);

    expr = parse_expression("x^1");
    result = evaluate(expr, ctx);

    // Ensure the result is a symbolic expression: x
    REQUIRE(std::holds_alternative<Symbol>(*result));
    REQUIRE(std::get<Symbol>(*result).name == "x");
}

TEST_CASE("Simplify nested expressions", "[evaluator][simplification]") {
    EvaluationContext ctx; // Empty context

    auto expr = parse_expression("0 + (1 * x)");
    auto result = evaluate(expr, ctx);

    // Ensure the result is a symbolic expression: x
    REQUIRE(std::holds_alternative<Symbol>(*result));
    REQUIRE(std::get<Symbol>(*result).name == "x");

    expr = parse_expression("(x * 0) + 1");
    result = evaluate(expr, ctx);

    // Ensure the result is a numeric value: 1
    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(get_number_value(result) == 1.0);
}

TEST_CASE("Simplify Plus[2, 3, 4] to 9", "[evaluator][simplification]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("2 + 3 + 4");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(get_number_value(result) == 9.0);
}

TEST_CASE("Simplify Times[2, 3, 0] to 0", "[evaluator][simplification]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("2 * 3 * 0");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(get_number_value(result) == 0.0);
}

TEST_CASE("Evaluator handles variable assignments", "[evaluator]") {
    EvaluationContext ctx;

    // Assign a value to x
    auto assign = make_expr<Assignment>("x", make_expr<Number>(2));
    auto result = evaluate(assign, ctx);

    // Check that the feedback is the variable name
    REQUIRE(std::get<Symbol>(*result).name == "x");

    // Check that x is stored in the context
    REQUIRE(ctx.variables.find("x") != ctx.variables.end());
    REQUIRE(std::get<Number>(*ctx.variables["x"]).value == 2);

    // Evaluate x
    auto x = make_expr<Symbol>("x");
    result = evaluate(x, ctx);
    REQUIRE(std::get<Number>(*result).value == 2);
}

//TODO: needs support for complex numbers, disable temporarily
#if 0
TEST_CASE("Evaluator handles division by zero as DirectedInfinity", "[evaluator][infinity]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("1 / 0");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<FunctionCall>(*result));
    auto func = std::get<FunctionCall>(*result);
    REQUIRE(func.head == "DirectedInfinity");
    REQUIRE(func.args.size() == 1);
    REQUIRE(std::get<Number>(*func.args[0]).value == 1.0); // Positive infinity
}

TEST_CASE("Evaluator handles negative infinity", "[evaluator][infinity]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("-1 / 0");
    auto result = evaluate(expr, ctx);

    // Ensure the result is a symbolic representation of negative infinity
    REQUIRE(std::holds_alternative<FunctionCall>(*result));
    auto func = std::get<FunctionCall>(*result);
    REQUIRE(func.head == "DirectedInfinity");
    REQUIRE(func.args.size() == 1);
    REQUIRE(std::get<Number>(*func.args[0]).value == -1.0); // Negative infinity
}
#endif

TEST_CASE("Evaluator handles 0/0 as Indeterminate", "[evaluator][indeterminate]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("0 / 0");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Indeterminate>(*result));
}

TEST_CASE("Unknown variables are treated as symbolic", "[evaluator]") {
    EvaluationContext ctx; // Empty context

    auto expr = parse_expression("z + 1");
    auto result = evaluate(expr, ctx);

    // Ensure the result is a symbolic expression: Plus[1, z]
    REQUIRE(std::holds_alternative<FunctionCall>(*result));
    auto func = std::get<FunctionCall>(*result);
    REQUIRE(func.head == "Plus");
    REQUIRE(func.args.size() == 2);
    REQUIRE(std::holds_alternative<Number>(*func.args[0])); // Numeric argument comes first
    REQUIRE(std::get<Number>(*func.args[0]).value == 1.0);
    REQUIRE(std::holds_alternative<Symbol>(*func.args[1])); // Symbolic argument comes second
    REQUIRE(std::get<Symbol>(*func.args[1]).name == "z");
}

TEST_CASE("Evaluator handles nested parentheses correctly", "[evaluator]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("2 * (3 + (4 * (5 - 1)))");
    auto result = evaluate(expr, ctx);
    REQUIRE(get_number_value(result) == 38.0);
}

TEST_CASE("Evaluator handles equality operator (==) with True result", "[evaluator]") {
    EvaluationContext ctx;
    ctx.variables["x"] = make_expr<Number>(5);

    auto expr = parse_expression("x == 5");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Boolean>(*result));
    REQUIRE(std::get<Boolean>(*result).value == true);
}

TEST_CASE("Evaluator handles equality operator (==) with False result", "[evaluator]") {
    EvaluationContext ctx;
    ctx.variables["x"] = make_expr<Number>(3);

    auto expr = parse_expression("x == 5");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Boolean>(*result));
    REQUIRE(std::get<Boolean>(*result).value == false);
}

TEST_CASE("Evaluator handles symbolic equality operator (==)", "[evaluator]") {
    EvaluationContext ctx; // Empty context, no variables defined

    auto expr = parse_expression("x == y");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<FunctionCall>(*result));
    auto func = std::get<FunctionCall>(*result);

    REQUIRE(func.head == "Equal");
    REQUIRE(func.args.size() == 2);

    REQUIRE(std::holds_alternative<Symbol>(*func.args[0]));
    REQUIRE(std::get<Symbol>(*func.args[0]).name == "x");

    REQUIRE(std::holds_alternative<Symbol>(*func.args[1]));
    REQUIRE(std::get<Symbol>(*func.args[1]).name == "y");
}

TEST_CASE("Evaluator handles logical AND (&&)", "[evaluator][logic]") {
    EvaluationContext ctx;

    // True && False -> False
    auto expr = parse_expression("True && False");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<Boolean>(*result));
    REQUIRE(std::get<Boolean>(*result).value == false);

    // True && True -> True
    expr = parse_expression("True && True");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<Boolean>(*result));
    REQUIRE(std::get<Boolean>(*result).value == true);

    // True && x -> Unevaluated
    expr = parse_expression("True && x");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<FunctionCall>(*result));
    auto func = std::get<FunctionCall>(*result);
    REQUIRE(func.head == "And");
    REQUIRE(func.args.size() == 2);
    REQUIRE(std::holds_alternative<Boolean>(*func.args[0]));
    REQUIRE(std::get<Boolean>(*func.args[0]).value == true);
    REQUIRE(std::holds_alternative<Symbol>(*func.args[1]));
    REQUIRE(std::get<Symbol>(*func.args[1]).name == "x");
}

TEST_CASE("Evaluator handles logical OR (||)", "[evaluator][logic]") {
    EvaluationContext ctx;

    // True || False -> True
    auto expr = parse_expression("True || False");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<Boolean>(*result));
    REQUIRE(std::get<Boolean>(*result).value == true);

    // False || False -> False
    expr = parse_expression("False || False");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<Boolean>(*result));
    REQUIRE(std::get<Boolean>(*result).value == false);

    // False || x -> Unevaluated
    expr = parse_expression("False || x");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<FunctionCall>(*result));
    auto func = std::get<FunctionCall>(*result);
    REQUIRE(func.head == "Or");
    REQUIRE(func.args.size() == 2);
    REQUIRE(std::holds_alternative<Boolean>(*func.args[0]));
    REQUIRE(std::get<Boolean>(*func.args[0]).value == false);
    REQUIRE(std::holds_alternative<Symbol>(*func.args[1]));
    REQUIRE(std::get<Symbol>(*func.args[1]).name == "x");
}

TEST_CASE("Evaluator handles StringJoin", "[evaluator][string]") {
    EvaluationContext ctx;

    // Test "Aleph3" <> " Rocks"
    auto expr = parse_expression("\"Aleph3\" <> \" Rocks\"");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "Aleph3 Rocks");

    // Test "Hello" <> " " <> "World"
    expr = parse_expression("\"Hello\" <> \" \" <> \"World\"");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "Hello World");

    // Test empty string concatenation
    expr = parse_expression("\"\" <> \"Hello\"");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "Hello");
}

TEST_CASE("Evaluator handles StringLength", "[evaluator][string]") {
    EvaluationContext ctx;

    // Test StringLength["Hello"]
    auto expr = parse_expression("StringLength[\"Hello\"]");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(std::get<Number>(*result).value == 5);

    // Test StringLength[""]
    expr = parse_expression("StringLength[\"\"]");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(std::get<Number>(*result).value == 0);

    // Test StringLength["Aleph3 Rocks"]
    expr = parse_expression("StringLength[\"Aleph3 Rocks\"]");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(std::get<Number>(*result).value == 12);
}

TEST_CASE("Evaluator handles StringReplace", "[evaluator][string]") {
    EvaluationContext ctx;

    // Test StringReplace["Hello World", "World" -> "Aleph3"]
    auto expr = parse_expression("StringReplace[\"Hello World\", \"World\" -> \"Aleph3\"]");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "Hello Aleph3");

    // Test StringReplace["abcabc", "abc" -> "x"]
    expr = parse_expression("StringReplace[\"abcabc\", \"abc\" -> \"x\"]");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "xx");

    // Test StringReplace["Hello", "x" -> "y"] (no match)
    expr = parse_expression("StringReplace[\"Hello\", \"x\" -> \"y\"]");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "Hello");
}

TEST_CASE("Evaluator handles StringTake", "[evaluator][string]") {
    EvaluationContext ctx;

    // Test StringTake["Hello", 3]
    auto expr = parse_expression("StringTake[\"Hello\", 3]");
    auto result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "Hel");

    // Test StringTake["Hello", -2]
    expr = parse_expression("StringTake[\"Hello\", -2]");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "lo");

    // Test StringTake["Hello", {2, 4}]
    expr = parse_expression("StringTake[\"Hello\", {2, 4}]");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<String>(*result));
    REQUIRE(std::get<String>(*result).value == "ell");

    // Test StringTake["Hello", 0] (invalid index)
    expr = parse_expression("StringTake[\"Hello\", 0]");
    REQUIRE_THROWS_WITH(evaluate(expr, ctx), "StringTake expects a valid index or range");
}

TEST_CASE("Evaluator throws error for invalid StringJoin arguments", "[evaluator][string]") {
    EvaluationContext ctx;

    // Test invalid argument: "Hello" <> 123
    auto expr = parse_expression("\"Hello\" <> 123");
    REQUIRE_THROWS_WITH(evaluate(expr, ctx), "StringJoin expects string arguments");
}

TEST_CASE("Evaluator handles list operations", "[evaluator][lists]") {
    EvaluationContext ctx; // Empty context
    auto expr = parse_expression("{1, 2, 3} + {4, 5, 6}");
    auto result = evaluate(expr, ctx);

    // Ensure the result is a list: {5, 7, 9}
    REQUIRE(std::holds_alternative<List>(*result));
    auto list = std::get<List>(*result);
    REQUIRE(list.elements.size() == 3);
    REQUIRE(get_number_value(list.elements[0]) == 5.0);
    REQUIRE(get_number_value(list.elements[1]) == 7.0);
    REQUIRE(get_number_value(list.elements[2]) == 9.0);
}

TEST_CASE("Evaluator handles scalar and list addition (broadcast)", "[evaluator][lists]") {
    EvaluationContext ctx;
    auto expr = parse_expression("10 + {1, 2, 3}");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<List>(*result));
    auto list = std::get<List>(*result);
    REQUIRE(list.elements.size() == 3);
    REQUIRE(get_number_value(list.elements[0]) == 11.0);
    REQUIRE(get_number_value(list.elements[1]) == 12.0);
    REQUIRE(get_number_value(list.elements[2]) == 13.0);

    expr = parse_expression("{1, 2, 3} + 10");
    result = evaluate(expr, ctx);
    list = std::get<List>(*result);
    REQUIRE(list.elements.size() == 3);
    REQUIRE(get_number_value(list.elements[0]) == 11.0);
    REQUIRE(get_number_value(list.elements[1]) == 12.0);
    REQUIRE(get_number_value(list.elements[2]) == 13.0);
}

TEST_CASE("Evaluator handles elementwise multiplication of lists", "[evaluator][lists]") {
    EvaluationContext ctx;
    auto expr = parse_expression("{1, 2, 3} * {4, 5, 6}");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<List>(*result));
    auto list = std::get<List>(*result);
    REQUIRE(list.elements.size() == 3);
    REQUIRE(get_number_value(list.elements[0]) == 4.0);
    REQUIRE(get_number_value(list.elements[1]) == 10.0);
    REQUIRE(get_number_value(list.elements[2]) == 18.0);
}

TEST_CASE("Evaluator handles scalar and list multiplication (broadcast)", "[evaluator][lists]") {
    EvaluationContext ctx;
    auto expr = parse_expression("2 * {4, 5, 6}");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<List>(*result));
    auto list = std::get<List>(*result);
    REQUIRE(list.elements.size() == 3);
    REQUIRE(get_number_value(list.elements[0]) == 8.0);
    REQUIRE(get_number_value(list.elements[1]) == 10.0);
    REQUIRE(get_number_value(list.elements[2]) == 12.0);

    expr = parse_expression("{4, 5, 6} * 2");
    result = evaluate(expr, ctx);
    list = std::get<List>(*result);
    REQUIRE(list.elements.size() == 3);
    REQUIRE(get_number_value(list.elements[0]) == 8.0);
    REQUIRE(get_number_value(list.elements[1]) == 10.0);
    REQUIRE(get_number_value(list.elements[2]) == 12.0);
}

TEST_CASE("Evaluator handles nested lists with elementwise addition", "[evaluator][lists]") {
    EvaluationContext ctx;
    auto expr = parse_expression("{{1, 2}, {3, 4}} + {{10, 20}, {30, 40}}");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<List>(*result));
    auto outer = std::get<List>(*result);
    REQUIRE(outer.elements.size() == 2);

    auto* inner1 = std::get_if<List>(outer.elements[0].get());
    auto* inner2 = std::get_if<List>(outer.elements[1].get());
    REQUIRE(inner1);
    REQUIRE(inner2);

    REQUIRE(get_number_value(inner1->elements[0]) == 11.0);
    REQUIRE(get_number_value(inner1->elements[1]) == 22.0);
    REQUIRE(get_number_value(inner2->elements[0]) == 33.0);
    REQUIRE(get_number_value(inner2->elements[1]) == 44.0);
}

TEST_CASE("Evaluator throws on mismatched list sizes", "[evaluator][lists]") {
    EvaluationContext ctx;
    auto expr = parse_expression("{1, 2} + {3, 4, 5}");
    REQUIRE_THROWS_WITH(evaluate(expr, ctx), "List sizes must match for elementwise operation");
}

TEST_CASE("Evaluator handles lists with symbolic elements", "[evaluator][lists]") {
    EvaluationContext ctx;
    auto expr = parse_expression("{x, y, 3} + {1, 2, z}");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<List>(*result));
    auto list = std::get<List>(*result);
    REQUIRE(list.elements.size() == 3);

    // x + 1
    REQUIRE(std::holds_alternative<FunctionCall>(*list.elements[0]));
    auto f0 = std::get<FunctionCall>(*list.elements[0]);
    REQUIRE(f0.head == "Plus");

    // y + 2
    REQUIRE(std::holds_alternative<FunctionCall>(*list.elements[1]));
    auto f1 = std::get<FunctionCall>(*list.elements[1]);
    REQUIRE(f1.head == "Plus");

    // 3 + z
    REQUIRE(std::holds_alternative<FunctionCall>(*list.elements[2]));
    auto f2 = std::get<FunctionCall>(*list.elements[2]);
    REQUIRE(f2.head == "Plus");
}

TEST_CASE("Evaluator handles Length for lists", "[evaluator][lists]") {
    EvaluationContext ctx;
    auto expr = parse_expression("Length[{1, 2, 3, 4}]");
    auto result = evaluate(expr, ctx);

    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(std::get<Number>(*result).value == 4.0);

    expr = parse_expression("Length[{}]");
    result = evaluate(expr, ctx);
    REQUIRE(std::holds_alternative<Number>(*result));
    REQUIRE(std::get<Number>(*result).value == 0.0);
}

struct EvalCase {
    std::string input;
    std::map<std::string, double> env;
    double expected;
};

TEST_CASE("Evaluator handles division by products and negatives") {
    std::vector<EvalCase> cases = {
        {"x/-3x",   {{"x", 2.0}}, -1.0 / 3.0},
        {"y/2y",    {{"y", 5.0}}, 0.5},
        {"a/-b",    {{"a", 6.0}, {"b", 2.0}}, -3.0},
        {"z/4w",    {{"z", 8.0}, {"w", 2.0}}, 1.0},
        {"t/-7t",   {{"t", 7.0}}, -1.0 / 7.0},
        {"m/(-2m)", {{"m", 10.0}}, -0.5},
        {"p/(-q)",  {{"p", 9.0}, {"q", 3.0}}, -3.0},
    };

    for (const auto& c : cases) {
        CAPTURE(c.input, c.env, c.expected);

        EvaluationContext ctx;
        for (const auto& [var, val] : c.env) {
            ctx.variables[var] = make_expr<Number>(val);
        }

        ExprPtr expr = parse_expression(c.input);
        ExprPtr result_expr = evaluate(expr, ctx);

        double value = 0.0;
        if (auto* num = std::get_if<Number>(&(*result_expr))) {
            value = num->value;
        }
        else if (auto* rat = std::get_if<Rational>(&(*result_expr))) {
            value = static_cast<double>(rat->numerator) / rat->denominator;
        }
        else {
            FAIL("Evaluator did not return a numeric result");
        }
        REQUIRE(value == Catch::Approx(c.expected));
    }
}