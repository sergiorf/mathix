﻿#include "parser/Parser.hpp"
#include "expr/Expr.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

using namespace aleph3;

TEST_CASE("Basic expressions are parsed correctly", "[parser]") {
    auto expr = parse_expression("2 + 3");
    REQUIRE(to_string(expr) == "2 + 3");
}

TEST_CASE("Parser correctly parses negative numbers in basic expressions", "[parser]") {
    auto expr = parse_expression("-2 + 3");
    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "-2 + 3");

    expr = parse_expression("2 + -3");
    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "2 + -3");

    expr = parse_expression("-2 + -3");
    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "-2 + -3");
}

TEST_CASE("Variables are parsed correctly", "[parser]") {
    auto expr = parse_expression("x + 1");
    REQUIRE(to_string(expr) == "x + 1");
}

TEST_CASE("Function calls are parsed correctly", "[parser]") {
    auto expr = parse_expression("sin[x]");
    REQUIRE(to_string(expr) == "sin[x]");
}

TEST_CASE("Parser correctly parses negative numbers in function calls", "[parser]") {
    auto expr = parse_expression("sin[-x]");
    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "sin[-x]");

    expr = parse_expression("max[-2, min[-3, -4]]");
    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "max[-2, min[-3, -4]]");
}

TEST_CASE("Nested function calls are parsed correctly", "[parser]") {
    auto expr = parse_expression("max[2, min[3, 4]]");
    REQUIRE(to_string(expr) == "max[2, min[3, 4]]");
}

TEST_CASE("Parser correctly parses power expressions", "[parser]") {
    auto expr = parse_expression("2^3");

    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "2^3");

    expr = parse_expression("-2^3");
    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "-2^3");

    expr = parse_expression("2^-3");
    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "2^-3");
}

TEST_CASE("Parser correctly parses exponential function", "[parser][exp]") {
    auto expr = parse_expression("exp[1]");

    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "exp[1]");
}

TEST_CASE("Parser correctly parses floor function", "[parser][floor]") {
    auto expr = parse_expression("floor[3.7]");

    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "floor[3.7]");
}

TEST_CASE("Parser correctly parses ceil function", "[parser][ceil]") {
    auto expr = parse_expression("ceil[3.2]");

    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "ceil[3.2]");
}

TEST_CASE("Parser correctly parses round function", "[parser][round]") {
    auto expr = parse_expression("round[3.5]");

    REQUIRE(expr != nullptr);
    REQUIRE(to_string(expr) == "round[3.5]");
}

TEST_CASE("Parser handles multiplication with number and symbol (2x)") {
    std::string input = "2x";

    ExprPtr expr = parse_expression(input);

    // Check that the parsed expression is a multiplication: "Times(2, x)"
    auto* func_call = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(func_call != nullptr);
    REQUIRE(func_call->head == "Times");
    REQUIRE(func_call->args.size() == 2);

    auto* left = std::get_if<Number>(&(*func_call->args[0]));
    auto* right = std::get_if<Symbol>(&(*func_call->args[1]));

    REQUIRE(left != nullptr);
    REQUIRE(left->value == 2.0);
    REQUIRE(right != nullptr);
    REQUIRE(right->name == "x");
}

TEST_CASE("Parser handles negative numbers with multiplication (2x and -2x)") {
    std::string input = "-2x";

    ExprPtr expr = parse_expression(input);

    // Check that the parsed expression is Times[-2, x]
    auto* func_call = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(func_call != nullptr);
    REQUIRE(func_call->head == "Times");
    REQUIRE(func_call->args.size() == 2);

    auto* left = std::get_if<Number>(&(*func_call->args[0]));
    auto* right = std::get_if<Symbol>(&(*func_call->args[1]));

    REQUIRE(left != nullptr);
    REQUIRE(left->value == -2.0);
    REQUIRE(right != nullptr);
    REQUIRE(right->name == "x");
}

TEST_CASE("Parser handles implicit multiplication with parentheses", "[parser]") {
    auto expr = parse_expression("2(3 + x)");
    REQUIRE(expr != nullptr);

    // Check that the parsed expression is a multiplication: "Times(2, Plus(3, x))"
    auto* func_call = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(func_call != nullptr);
    REQUIRE(func_call->head == "Times");
    REQUIRE(func_call->args.size() == 2);

    auto* left = std::get_if<Number>(&(*func_call->args[0]));
    REQUIRE(left != nullptr);
    REQUIRE(left->value == 2.0);

    auto* right = std::get_if<FunctionCall>(&(*func_call->args[1]));
    REQUIRE(right != nullptr);
    REQUIRE(right->head == "Plus");
    REQUIRE(right->args.size() == 2);

    auto* right_left = std::get_if<Number>(&(*right->args[0]));
    auto* right_right = std::get_if<Symbol>(&(*right->args[1]));

    REQUIRE(right_left != nullptr);
    REQUIRE(right_left->value == 3.0);
    REQUIRE(right_right != nullptr);
    REQUIRE(right_right->name == "x");
}

TEST_CASE("Parser handles variable assignments", "[parser]") {
    auto expr = parse_expression("x = 2");
    REQUIRE(expr != nullptr);

    // Check that the parsed expression is an assignment
    auto* assign = std::get_if<Assignment>(&(*expr));
    REQUIRE(assign != nullptr);
    REQUIRE(assign->name == "x");

    // Check the assigned value
    auto* value = std::get_if<Number>(&(*assign->value));
    REQUIRE(value != nullptr);
    REQUIRE(value->value == 2.0);
}

TEST_CASE("Parser handles simple If statement", "[parser]") {
    std::string input = "If[x == 0, 1, 2]";
    auto expr = parse_expression(input);

    REQUIRE(std::holds_alternative<FunctionCall>(*expr));
    auto func = std::get<FunctionCall>(*expr);

    REQUIRE(func.head == "If");
    REQUIRE(func.args.size() == 3);

    REQUIRE(std::holds_alternative<FunctionCall>(*func.args[0]));
    auto condition = std::get<FunctionCall>(*func.args[0]);
    REQUIRE(condition.head == "Equal");
    REQUIRE(condition.args.size() == 2);
    REQUIRE(std::holds_alternative<Symbol>(*condition.args[0]));
    REQUIRE(std::get<Symbol>(*condition.args[0]).name == "x");
    REQUIRE(std::holds_alternative<Number>(*condition.args[1]));
    REQUIRE(std::get<Number>(*condition.args[1]).value == 0.0);

    REQUIRE(std::holds_alternative<Number>(*func.args[1]));
    REQUIRE(std::get<Number>(*func.args[1]).value == 1.0);

    REQUIRE(std::holds_alternative<Number>(*func.args[2]));
    REQUIRE(std::get<Number>(*func.args[2]).value == 2.0);
}

TEST_CASE("Parser handles equality operator (==)", "[parser]") {
    std::string input = "x == 0";
    auto expr = parse_expression(input);

    REQUIRE(std::holds_alternative<FunctionCall>(*expr));
    auto func = std::get<FunctionCall>(*expr);

    REQUIRE(func.head == "Equal");
    REQUIRE(func.args.size() == 2);

    REQUIRE(std::holds_alternative<Symbol>(*func.args[0]));
    REQUIRE(std::get<Symbol>(*func.args[0]).name == "x");

    REQUIRE(std::holds_alternative<Number>(*func.args[1]));
    REQUIRE(std::get<Number>(*func.args[1]).value == 0.0);
}

TEST_CASE("Parser handles logical AND (&&)", "[parser][logic]") {
    auto expr = parse_expression("True && False");
    REQUIRE(std::holds_alternative<FunctionCall>(*expr));

    auto func = std::get<FunctionCall>(*expr);
    REQUIRE(func.head == "And");
    REQUIRE(func.args.size() == 2);

    REQUIRE(std::holds_alternative<Boolean>(*func.args[0]));
    REQUIRE(std::get<Boolean>(*func.args[0]).value == true);

    REQUIRE(std::holds_alternative<Boolean>(*func.args[1]));
    REQUIRE(std::get<Boolean>(*func.args[1]).value == false);
}

TEST_CASE("Parser handles logical OR (||)", "[parser][logic]") {
    auto expr = parse_expression("True || False");
    REQUIRE(std::holds_alternative<FunctionCall>(*expr));

    auto func = std::get<FunctionCall>(*expr);
    REQUIRE(func.head == "Or");
    REQUIRE(func.args.size() == 2);

    REQUIRE(std::holds_alternative<Boolean>(*func.args[0]));
    REQUIRE(std::get<Boolean>(*func.args[0]).value == true);

    REQUIRE(std::holds_alternative<Boolean>(*func.args[1]));
    REQUIRE(std::get<Boolean>(*func.args[1]).value == false);
}

TEST_CASE("Parser handles mixed logical expressions", "[parser][logic]") {
    auto expr = parse_expression("True && False || True");
    REQUIRE(std::holds_alternative<FunctionCall>(*expr));

    auto func = std::get<FunctionCall>(*expr);
    REQUIRE(func.head == "Or");
    REQUIRE(func.args.size() == 2);

    // Left side of the OR is an AND
    REQUIRE(std::holds_alternative<FunctionCall>(*func.args[0]));
    auto left_func = std::get<FunctionCall>(*func.args[0]);
    REQUIRE(left_func.head == "And");
    REQUIRE(left_func.args.size() == 2);

    REQUIRE(std::holds_alternative<Boolean>(*left_func.args[0]));
    REQUIRE(std::get<Boolean>(*left_func.args[0]).value == true);

    REQUIRE(std::holds_alternative<Boolean>(*left_func.args[1]));
    REQUIRE(std::get<Boolean>(*left_func.args[1]).value == false);

    // Right side of the OR is True
    REQUIRE(std::holds_alternative<Boolean>(*func.args[1]));
    REQUIRE(std::get<Boolean>(*func.args[1]).value == true);
}

TEST_CASE("Parser handles symbolic logical expressions", "[parser][logic]") {
    auto expr = parse_expression("x && y");
    REQUIRE(std::holds_alternative<FunctionCall>(*expr));

    auto func = std::get<FunctionCall>(*expr);
    REQUIRE(func.head == "And");
    REQUIRE(func.args.size() == 2);

    REQUIRE(std::holds_alternative<Symbol>(*func.args[0]));
    REQUIRE(std::get<Symbol>(*func.args[0]).name == "x");

    REQUIRE(std::holds_alternative<Symbol>(*func.args[1]));
    REQUIRE(std::get<Symbol>(*func.args[1]).name == "y");
}

TEST_CASE("Parser handles nested logical expressions", "[parser][logic]") {
    auto expr = parse_expression("(True || False) && x");
    REQUIRE(std::holds_alternative<FunctionCall>(*expr));

    auto func = std::get<FunctionCall>(*expr);
    REQUIRE(func.head == "And");
    REQUIRE(func.args.size() == 2);

    // Left side of the AND is an OR
    REQUIRE(std::holds_alternative<FunctionCall>(*func.args[0]));
    auto left_func = std::get<FunctionCall>(*func.args[0]);
    REQUIRE(left_func.head == "Or");
    REQUIRE(left_func.args.size() == 2);

    REQUIRE(std::holds_alternative<Boolean>(*left_func.args[0]));
    REQUIRE(std::get<Boolean>(*left_func.args[0]).value == true);

    REQUIRE(std::holds_alternative<Boolean>(*left_func.args[1]));
    REQUIRE(std::get<Boolean>(*left_func.args[1]).value == false);

    // Right side of the AND is x
    REQUIRE(std::holds_alternative<Symbol>(*func.args[1]));
    REQUIRE(std::get<Symbol>(*func.args[1]).name == "x");
}

TEST_CASE("Parser handles simple string concatenation with <>", "[parser][string]") {
    auto expr = parse_expression("\"Aleph3\" <> \" Rocks\"");
    REQUIRE(expr != nullptr);

    // The result should be a FunctionCall to StringJoin with 2 arguments
    auto* func_call = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(func_call != nullptr);
    REQUIRE(func_call->head == "StringJoin");
    REQUIRE(func_call->args.size() == 2);

    auto* arg0 = std::get_if<String>(&(*func_call->args[0]));
    auto* arg1 = std::get_if<String>(&(*func_call->args[1]));

    REQUIRE(arg0 != nullptr);
    REQUIRE(arg0->value == "Aleph3");
    REQUIRE(arg1 != nullptr);
    REQUIRE(arg1->value == " Rocks");
}

TEST_CASE("Parser handles chained string concatenation with <>", "[parser][string]") {
    auto expr = parse_expression("\"Hello\" <> \" \" <> \"World\"");
    REQUIRE(expr != nullptr);

    // The result should be a FunctionCall to StringJoin with 3 arguments
    auto* func_call = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(func_call != nullptr);
    REQUIRE(func_call->head == "StringJoin");
    REQUIRE(func_call->args.size() == 3);

    auto* arg0 = std::get_if<String>(&(*func_call->args[0]));
    auto* arg1 = std::get_if<String>(&(*func_call->args[1]));
    auto* arg2 = std::get_if<String>(&(*func_call->args[2]));

    REQUIRE(arg0 != nullptr);
    REQUIRE(arg0->value == "Hello");
    REQUIRE(arg1 != nullptr);
    REQUIRE(arg1->value == " ");
    REQUIRE(arg2 != nullptr);
    REQUIRE(arg2->value == "World");
}

TEST_CASE("Parser handles simple rule operator (->)", "[parser][rule]") {
    auto expr = parse_expression("\"World\" -> \"Aleph3\"");
    REQUIRE(expr != nullptr);

    // The result should be a Rule node
    auto* rule = std::get_if<Rule>(&(*expr));
    REQUIRE(rule != nullptr);

    auto* lhs = std::get_if<String>(&(*rule->lhs));
    auto* rhs = std::get_if<String>(&(*rule->rhs));

    REQUIRE(lhs != nullptr);
    REQUIRE(lhs->value == "World");
    REQUIRE(rhs != nullptr);
    REQUIRE(rhs->value == "Aleph3");
}

TEST_CASE("Parser handles rule as argument in function call", "[parser][rule]") {
    auto expr = parse_expression("StringReplace[\"Hello World\", \"World\" -> \"Aleph3\"]");
    REQUIRE(expr != nullptr);

    auto* func_call = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(func_call != nullptr);
    REQUIRE(func_call->head == "StringReplace");
    REQUIRE(func_call->args.size() == 2);

    auto* arg0 = std::get_if<String>(&(*func_call->args[0]));
    REQUIRE(arg0 != nullptr);
    REQUIRE(arg0->value == "Hello World");

    auto* rule = std::get_if<Rule>(&(*func_call->args[1]));
    REQUIRE(rule != nullptr);

    auto* lhs = std::get_if<String>(&(*rule->lhs));
    auto* rhs = std::get_if<String>(&(*rule->rhs));
    REQUIRE(lhs != nullptr);
    REQUIRE(lhs->value == "World");
    REQUIRE(rhs != nullptr);
    REQUIRE(rhs->value == "Aleph3");
}

TEST_CASE("Parser handles StringJoin and Rule precedence", "[parser][rule][string]") {
    auto expr = parse_expression("\"a\" <> \"b\" -> \"c\"");
    REQUIRE(expr != nullptr);

    // Should parse as Rule[StringJoin["a", "b"], "c"]
    auto* rule = std::get_if<Rule>(&(*expr));
    REQUIRE(rule != nullptr);

    auto* join = std::get_if<FunctionCall>(&(*rule->lhs));
    REQUIRE(join != nullptr);
    REQUIRE(join->head == "StringJoin");
    REQUIRE(join->args.size() == 2);

    auto* arg0 = std::get_if<String>(&(*join->args[0]));
    auto* arg1 = std::get_if<String>(&(*join->args[1]));
    REQUIRE(arg0 != nullptr);
    REQUIRE(arg0->value == "a");
    REQUIRE(arg1 != nullptr);
    REQUIRE(arg1->value == "b");

    auto* rhs = std::get_if<String>(&(*rule->rhs));
    REQUIRE(rhs != nullptr);
    REQUIRE(rhs->value == "c");
}

TEST_CASE("Parser handles simple lists", "[parser][list]") {
    auto expr = parse_expression("{1, 2, 3}");
    REQUIRE(expr != nullptr);

    auto* list = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(list != nullptr);
    REQUIRE(list->head == "List");
    REQUIRE(list->args.size() == 3);

    auto* n1 = std::get_if<Number>(&(*list->args[0]));
    auto* n2 = std::get_if<Number>(&(*list->args[1]));
    auto* n3 = std::get_if<Number>(&(*list->args[2]));
    REQUIRE(n1);
    REQUIRE(n2);
    REQUIRE(n3);
    REQUIRE(n1->value == 1.0);
    REQUIRE(n2->value == 2.0);
    REQUIRE(n3->value == 3.0);
}

TEST_CASE("Parser handles nested lists", "[parser][list]") {
    auto expr = parse_expression("{1, {2, 3}, 4}");
    REQUIRE(expr != nullptr);

    auto* list = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(list != nullptr);
    REQUIRE(list->head == "List");
    REQUIRE(list->args.size() == 3);

    auto* n1 = std::get_if<Number>(&(*list->args[0]));
    REQUIRE(n1);
    REQUIRE(n1->value == 1.0);

    auto* inner_list = std::get_if<FunctionCall>(&(*list->args[1]));
    REQUIRE(inner_list != nullptr);
    REQUIRE(inner_list->head == "List");
    REQUIRE(inner_list->args.size() == 2);

    auto* n2 = std::get_if<Number>(&(*inner_list->args[0]));
    auto* n3 = std::get_if<Number>(&(*inner_list->args[1]));
    REQUIRE(n2);
    REQUIRE(n2->value == 2.0);
    REQUIRE(n3);
    REQUIRE(n3->value == 3.0);

    auto* n4 = std::get_if<Number>(&(*list->args[2]));
    REQUIRE(n4);
    REQUIRE(n4->value == 4.0);
}

TEST_CASE("Parser handles lists as function arguments", "[parser][list]") {
    auto expr = parse_expression("f[{1, 2}, 3]");
    REQUIRE(expr != nullptr);

    auto* func = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(func != nullptr);
    REQUIRE(func->head == "f");
    REQUIRE(func->args.size() == 2);

    auto* list = std::get_if<FunctionCall>(&(*func->args[0]));
    REQUIRE(list != nullptr);
    REQUIRE(list->head == "List");
    REQUIRE(list->args.size() == 2);

    auto* n1 = std::get_if<Number>(&(*list->args[0]));
    auto* n2 = std::get_if<Number>(&(*list->args[1]));
    REQUIRE(n1);
    REQUIRE(n1->value == 1.0);
    REQUIRE(n2);
    REQUIRE(n2->value == 2.0);

    auto* n3 = std::get_if<Number>(&(*func->args[1]));
    REQUIRE(n3);
    REQUIRE(n3->value == 3.0);
}

TEST_CASE("Parser handles empty lists", "[parser][list]") {
    auto expr = parse_expression("{}");
    REQUIRE(expr != nullptr);

    auto* list = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(list != nullptr);
    REQUIRE(list->head == "List");
    REQUIRE(list->args.empty());
}

TEST_CASE("Parser handles lists with mixed types", "[parser][list]") {
    auto expr = parse_expression("{1, \"hello\", True, x}");
    REQUIRE(expr != nullptr);

    auto* list = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(list != nullptr);
    REQUIRE(list->head == "List");
    REQUIRE(list->args.size() == 4);

    REQUIRE(std::holds_alternative<Number>(*list->args[0]));
    REQUIRE(std::get<Number>(*list->args[0]).value == 1.0);

    REQUIRE(std::holds_alternative<String>(*list->args[1]));
    REQUIRE(std::get<String>(*list->args[1]).value == "hello");

    REQUIRE(std::holds_alternative<Boolean>(*list->args[2]));
    REQUIRE(std::get<Boolean>(*list->args[2]).value == true);

    REQUIRE(std::holds_alternative<Symbol>(*list->args[3]));
    REQUIRE(std::get<Symbol>(*list->args[3]).name == "x");
}

TEST_CASE("Parser handles lists with expressions", "[parser][list]") {
    auto expr = parse_expression("{1+2, x^2, f[3]}");
    REQUIRE(expr != nullptr);

    auto* list = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(list != nullptr);
    REQUIRE(list->head == "List");
    REQUIRE(list->args.size() == 3);

    // 1+2
    REQUIRE(std::holds_alternative<FunctionCall>(*list->args[0]));
    auto plus = std::get<FunctionCall>(*list->args[0]);
    REQUIRE(plus.head == "Plus");

    // x^2
    REQUIRE(std::holds_alternative<FunctionCall>(*list->args[1]));
    auto pow = std::get<FunctionCall>(*list->args[1]);
    REQUIRE(pow.head == "Power");

    // f[3]
    REQUIRE(std::holds_alternative<FunctionCall>(*list->args[2]));
    auto fcall = std::get<FunctionCall>(*list->args[2]);
    REQUIRE(fcall.head == "f");
    REQUIRE(fcall.args.size() == 1);
    REQUIRE(std::holds_alternative<Number>(*fcall.args[0]));
    REQUIRE(std::get<Number>(*fcall.args[0]).value == 3.0);
}

TEST_CASE("Parser handles lists as arguments to built-in functions", "[parser][list]") {
    auto expr = parse_expression("Length[{1, 2, 3}]");
    REQUIRE(expr != nullptr);

    auto* func = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(func != nullptr);
    REQUIRE(func->head == "Length");
    REQUIRE(func->args.size() == 1);

    auto* list = std::get_if<FunctionCall>(&(*func->args[0]));
    REQUIRE(list != nullptr);
    REQUIRE(list->head == "List");
    REQUIRE(list->args.size() == 3);
}

TEST_CASE("Parser handles nested empty lists", "[parser][list]") {
    auto expr = parse_expression("{{}, {}}");
    REQUIRE(expr != nullptr);

    auto* list = std::get_if<FunctionCall>(&(*expr));
    REQUIRE(list != nullptr);
    REQUIRE(list->head == "List");
    REQUIRE(list->args.size() == 2);

    auto* l1 = std::get_if<FunctionCall>(&(*list->args[0]));
    auto* l2 = std::get_if<FunctionCall>(&(*list->args[1]));
    REQUIRE(l1 != nullptr);
    REQUIRE(l2 != nullptr);
    REQUIRE(l1->head == "List");
    REQUIRE(l2->head == "List");
    REQUIRE(l1->args.empty());
    REQUIRE(l2->args.empty());
}

TEST_CASE("Parser parses Aleph3 constants as symbols", "[parser][constants]") {
    std::vector<std::string> constants = {
        "Pi", "E", "Degree", "GoldenRatio", "Catalan", "EulerGamma", "Infinity"
    };

    for (const auto& name : constants) {
        CAPTURE(name);
        auto expr = parse_expression(name);
        REQUIRE(expr != nullptr);
        REQUIRE(std::holds_alternative<Symbol>(*expr));
        REQUIRE(std::get<Symbol>(*expr).name == name);
    }
}

struct DivideCase {
    std::string input;
    double denom_num; // The numeric value in the denominator (e.g. -3 for -3x)
    std::string numer_var;
    std::string denom_var;
};

TEST_CASE("Parser handles division by products and negatives") {
    std::vector<DivideCase> cases = {
        {"x/-3x",   -3.0, "x", "x"},
        {"y/2y",     2.0, "y", "y"},
        {"a/-b",    -1.0, "a", "b"},
        {"z/4w",     4.0, "z", "w"},
        {"t/-7t",   -7.0, "t", "t"},
        {"m/(-2m)", -2.0, "m", "m"},
        {"p/(-q)",  -1.0, "p", "q"},
    };

    for (const auto& c : cases) {
        CAPTURE(c.input, c.denom_num, c.numer_var, c.denom_var);

        ExprPtr expr = parse_expression(c.input);

        // Should be Divide[numer_var, Times[denom_num, denom_var]]
        auto* divide = std::get_if<FunctionCall>(&(*expr));
        REQUIRE(divide != nullptr);
        REQUIRE(divide->head == "Divide");
        REQUIRE(divide->args.size() == 2);

        // Numerator: variable
        auto* numerator = std::get_if<Symbol>(&(*divide->args[0]));
        REQUIRE(numerator != nullptr);
        REQUIRE(numerator->name == c.numer_var);

        // Denominator: Times[denom_num, denom_var]
        auto* times = std::get_if<FunctionCall>(&(*divide->args[1]));
        REQUIRE(times != nullptr);
        REQUIRE(times->head == "Times");
        REQUIRE(times->args.size() == 2);

        auto* num = std::get_if<Number>(&(*times->args[0]));
        REQUIRE(num != nullptr);
        REQUIRE(num->value == c.denom_num);

        auto* var = std::get_if<Symbol>(&(*times->args[1]));
        REQUIRE(var != nullptr);
        REQUIRE(var->name == c.denom_var);
    }
}

TEST_CASE("Parser handles division with negative numerators and products in denominator") {
    struct ParseCase {
        std::string input;
        enum class DenomType { Times, Symbol } denom_type;
        double numer;
        double denom_num;
        std::string denom_var;
        std::string denom_symbol;
    };

    std::vector<ParseCase> cases = {
        {"-52/(3X)",   ParseCase::DenomType::Times,   -52.0, 3.0, "X", ""},
        {"-7/(2y)",    ParseCase::DenomType::Times,   -7.0,  2.0, "y", ""},
        {"-a/(bC)",    ParseCase::DenomType::Symbol,   0.0,  0.0, "", "bC"}, // single symbol
        {"-a/(b C)",   ParseCase::DenomType::Times,    0.0,  0.0, "", ""},   // product
        {"-1/(4z)",    ParseCase::DenomType::Times,   -1.0,  4.0, "z", ""},
        {"-2/(x^2)",   ParseCase::DenomType::Times,   -2.0,  0.0, "", ""},   // power in denominator
        {"-x/(y+z)",   ParseCase::DenomType::Times,    0.0,  0.0, "", ""},   // sum in denominator
        {"-3/(Sin[x])",ParseCase::DenomType::Times,   -3.0,  0.0, "", ""},   // function call in denominator
        {"-a/(b_c)",   ParseCase::DenomType::Symbol,   0.0,  0.0, "", "b_c"}, // symbol with underscore
    };

    for (const auto& c : cases) {
        CAPTURE(c.input);

        ExprPtr expr = parse_expression(c.input);

        auto* divide = std::get_if<FunctionCall>(&(*expr));
        REQUIRE(divide != nullptr);
        REQUIRE(divide->head == "Divide");
        REQUIRE(divide->args.size() == 2);

        // Numerator: handle -a and -x as Times[-1, a] or Times[-1, x]
        if (c.input.find("-a/") == 0 || c.input.find("-x/") == 0) {
            auto* times = std::get_if<FunctionCall>(&(*divide->args[0]));
            REQUIRE(times != nullptr);
            REQUIRE(times->head == "Times");
            REQUIRE(times->args.size() == 2);
            auto* minus_one = std::get_if<Number>(&(*times->args[0]));
            REQUIRE(minus_one != nullptr);
            REQUIRE(minus_one->value == -1.0);
            auto* sym = std::get_if<Symbol>(&(*times->args[1]));
            REQUIRE(sym != nullptr);
            if (c.input.find("-a/") == 0)
                REQUIRE(sym->name == "a");
            else
                REQUIRE(sym->name == "x");
        }
        else {
            // Numerator: Number
            auto* numerator = std::get_if<Number>(&(*divide->args[0]));
            REQUIRE(numerator != nullptr);
            REQUIRE(numerator->value == c.numer);
        }

        // Denominator
        if (c.denom_type == ParseCase::DenomType::Symbol) {
            auto* denom_sym = std::get_if<Symbol>(&(*divide->args[1]));
            REQUIRE(denom_sym != nullptr);
            REQUIRE(denom_sym->name == c.denom_symbol);
        }
        else if (c.input == "-a/(b C)") {
            // Product of two symbols
            auto* denom_times = std::get_if<FunctionCall>(&(*divide->args[1]));
            REQUIRE(denom_times != nullptr);
            REQUIRE(denom_times->head == "Times");
            REQUIRE(denom_times->args.size() == 2);
            auto* b = std::get_if<Symbol>(&(*denom_times->args[0]));
            REQUIRE(b != nullptr);
            REQUIRE(b->name == "b");
            auto* C = std::get_if<Symbol>(&(*denom_times->args[1]));
            REQUIRE(C != nullptr);
            REQUIRE(C->name == "C");
        }
        else if (c.input == "-2/(x^2)") {
            // Power in denominator
            auto* power = std::get_if<FunctionCall>(&(*divide->args[1]));
            REQUIRE(power != nullptr);
            REQUIRE(power->head == "Power");
            REQUIRE(power->args.size() == 2);
            auto* x = std::get_if<Symbol>(&(*power->args[0]));
            REQUIRE(x != nullptr);
            REQUIRE(x->name == "x");
            auto* two = std::get_if<Number>(&(*power->args[1]));
            REQUIRE(two != nullptr);
            REQUIRE(two->value == 2.0);
        }
        else if (c.input == "-x/(y+z)") {
            // Sum in denominator
            auto* plus = std::get_if<FunctionCall>(&(*divide->args[1]));
            REQUIRE(plus != nullptr);
            REQUIRE(plus->head == "Plus");
            REQUIRE(plus->args.size() == 2);
            auto* y = std::get_if<Symbol>(&(*plus->args[0]));
            REQUIRE(y != nullptr);
            REQUIRE(y->name == "y");
            auto* z = std::get_if<Symbol>(&(*plus->args[1]));
            REQUIRE(z != nullptr);
            REQUIRE(z->name == "z");
        }
        else if (c.input == "-3/(Sin[x])") {
            // Function call in denominator
            auto* sin = std::get_if<FunctionCall>(&(*divide->args[1]));
            REQUIRE(sin != nullptr);
            REQUIRE(sin->head == "Sin");
            REQUIRE(sin->args.size() == 1);
            auto* x = std::get_if<Symbol>(&(*sin->args[0]));
            REQUIRE(x != nullptr);
            REQUIRE(x->name == "x");
        }
        else if (c.denom_type == ParseCase::DenomType::Times) {
            // Times[denom_num, denom_var]
            auto* times = std::get_if<FunctionCall>(&(*divide->args[1]));
            REQUIRE(times != nullptr);
            REQUIRE(times->head == "Times");
            REQUIRE(times->args.size() == 2);

            auto* denom_num = std::get_if<Number>(&(*times->args[0]));
            REQUIRE(denom_num != nullptr);
            REQUIRE(denom_num->value == c.denom_num);

            auto* denom_var = std::get_if<Symbol>(&(*times->args[1]));
            REQUIRE(denom_var != nullptr);
            REQUIRE(denom_var->name == c.denom_var);
        }
    }
}
