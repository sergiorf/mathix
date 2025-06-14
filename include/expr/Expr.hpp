/*
 * Aleph3 Expression Types
 * -----------------------
 * This header defines the core expression data structures for Aleph3, a modern C++20-based computer algebra system.
 * The Expr type is a tagged union (std::variant) representing all supported symbolic and numeric objects,
 * including numbers, rationals, booleans, symbols, strings, lists, function calls, assignments, rules, and more.
 *
 * Features:
 * - Unified variant type (Expr) for all mathematical and symbolic objects
 * - Smart pointer management for expression trees (ExprPtr)
 * - Factory functions for constructing expressions
 * - Type-safe representation of mathematical constructs for parsing, evaluation, and transformation
 *
 * See README.md for project overview.
 */
#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <iostream>
#include <cstdint>

namespace aleph3 {

// Forward declarations
struct Symbol;
struct Number;
struct Complex;
struct Rational;
struct Boolean;
struct String;
struct FunctionCall;
struct FunctionDefinition;
struct Assignment;
struct Rule;
struct List;
struct Infinity;
struct Indeterminate;

// Core Expression type: variant of all expression types
using Expr = std::variant < Symbol, Number, Complex, Rational, Boolean, String, FunctionCall, FunctionDefinition, Assignment, Rule, List, Infinity, Indeterminate > ;

// Smart pointer to expressions
using ExprPtr = std::shared_ptr<Expr>;

// Factory function to make an ExprPtr
template <typename T, typename... Args>
ExprPtr make_expr(Args&&... args) {
    return std::make_shared<Expr>(T{std::forward<Args>(args)...});
}

// Expression types

struct Symbol {
    std::string name;

    Symbol(const std::string& n) : name(n) {}
};

struct String {
    std::string value;

    String(const std::string& v) : value(v) {}
};

struct Number {
    double value;

    Number(double v) : value(v) {}
    Number(int v) : value(static_cast<double>(v)) {}
};

struct Complex {
    double real;
    double imag;
};

struct Rational {
    int64_t numerator;
    int64_t denominator;
    Rational(int64_t n, int64_t d) : numerator(n), denominator(d) {}
};

struct Boolean {
    bool value;

    Boolean(bool v) : value(v) {}
};

struct List {
    std::vector<ExprPtr> elements;
};

struct FunctionCall {
    std::string head;            // Like "Plus", "Times", "Sin"
    std::vector<ExprPtr> args;    // Arguments

    FunctionCall(std::string h, const std::vector<ExprPtr>& a)
        : head(h), args(a) {}
};

struct Parameter {
    std::string name;
    ExprPtr default_value; // nullptr if no default

    Parameter(const std::string& n, ExprPtr def = nullptr)
        : name(n), default_value(def) {
    }
};

struct FunctionDefinition {
    std::string name;                       // Function name
    std::vector<Parameter> params;          // Parameters (with optional defaults)
    ExprPtr body;                           // Function body
    bool delayed;                           // True for `:=`, false for `=`

    FunctionDefinition() : name(""), params(), body(nullptr), delayed(true) {}

    FunctionDefinition(const std::string& name, const std::vector<Parameter>& params, const ExprPtr& body, bool delayed)
        : name(name), params(params), body(body), delayed(delayed) {
    }
};

struct Assignment {
    std::string name; // Variable name
    ExprPtr value;    // Assigned value

    Assignment(const std::string& name, const ExprPtr& value)
        : name(name), value(value) {
    }
};

struct Rule {
    ExprPtr lhs;
    ExprPtr rhs;
    Rule(const ExprPtr& lhs, const ExprPtr& rhs) : lhs(lhs), rhs(rhs) {}
};

struct Infinity {};

struct Indeterminate {};

// Utility functions

inline std::string to_string(int64_t v) {
    return std::to_string(v);
}
inline std::string to_string_raw(int64_t v) {
    return std::to_string(v);
}

std::string to_string(const Expr& expr);

std::string to_string_raw(const Expr& expr);

inline std::string to_string(const ExprPtr& expr_ptr) {
    return to_string(*expr_ptr);
}

inline std::string to_string_raw(const ExprPtr& expr_ptr) {
    return to_string_raw(*expr_ptr);
}

}
