#include <print>

#include "Lexer.hpp"
#include "Parser.hpp"

using namespace kaleidoscope;

static void print_ast(const std::unique_ptr<Parser::Node> &ast)
{
    if (not ast)
        return;
    if (auto number = dynamic_cast<Parser::Number *>(ast.get())) {
        std::print("Number: {}\n", number->value.value);
    } else if (auto identifier = dynamic_cast<Parser::Identifier *>(ast.get())) {
        std::print("Identifier: {}\n", identifier->name.value);
    } else if (auto binary_operation = dynamic_cast<Parser::BinaryOperation *>(ast.get())) {
        std::print("BinaryOperation: {}\n", binary_operation->operation.value);
        print_ast(binary_operation->lhs);
        print_ast(binary_operation->rhs);
    } else if (auto function = dynamic_cast<Parser::Function *>(ast.get())) {
        std::print("Function: {}\n", function->prototype->name.value);
        for (auto &arg : function->prototype->argument_names)
            std::print("  Argument: {}\n", arg.value);
        print_ast(function->body);
    } else if (auto function_call = dynamic_cast<Parser::Function::Call *>(ast.get())) {
        std::print("FunctionCall: {}\n", function_call->callee->name.value);
        for (auto &arg : function_call->arguments)
            print_ast(arg);
    }
}

int main()
{
    auto parser = Parser(Lexer(R"(
        function add(x y)
            x + y
    )"));

    auto ast = parser.parse();
    print_ast(ast);

    return 0;
}
