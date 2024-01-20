#include <print>

#include "Lexer.hpp"
#include "Parser.hpp"

using namespace kaleidoscope;

static void print_ast(ast::Node *ast, int indent = 0)
{
    if (ast == nullptr)
        return;
    auto print_indent = [indent]() {
        for (int i = 0; i < indent; ++i)
            std::print("  ");
    };

    print_indent();

    if (auto num = dynamic_cast<ast::Number *>(ast)) {
        std::print("Number {}\n", num->value.value);
    } else if (auto id = dynamic_cast<ast::Identifier *>(ast)) {
        std::print("Identifier {}\n", id->name.value);
    } else if (auto binop = dynamic_cast<ast::BinaryOperation *>(ast)) {
        std::print("BinaryOperation {}\n", binop->operation.value);
        print_ast(binop->left_hand_side.get(), indent + 1);
        print_ast(binop->right_hand_side.get(), indent + 1);
    } else if (auto call = dynamic_cast<ast::Function::Call *>(ast)) {
        std::print("Call {}\n", call->callee.value);
        for (auto &&arg : call->arguments)
            print_ast(arg.get(), indent + 1);
    } else if (auto proto = dynamic_cast<ast::Function::Prototype *>(ast)) {
        std::print("Prototype {}:\n", proto->name.value);
        for (auto &&arg : proto->arguments)
            std::print("    - {}\n", arg.value);
    } else if (auto func = dynamic_cast<ast::Function *>(ast)) {
        std::print("Function\n");
        print_ast(func->prototype.get(), indent + 1);
        print_ast(func->body.get(), indent + 1);
    } else {
        std::print("Unknown\n");
    }
}

int main(int argc, const char *argv[])
{

    auto input = argc > 1 ? argv[1] : R"(
        extern sin(x)
        sin(add(1, 2))
        def add(x, y)
            x + (y * 2.0);
    )";
    auto parser = Parser(Lexer(input));
    auto ast = parser.parse();

    return 0;
}
