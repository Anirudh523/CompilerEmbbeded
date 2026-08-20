#include <fstream>
#include <iostream>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "ast/ast.hpp"
#include "sema/sema.hpp"
#include <string>
#include <vector>
#include <sstream>

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }
    
    std::ifstream inputFile(argv[1]);
    if(!inputFile) {
        std::cerr << "Could not open file: " << argv[1] << std::endl;
        return 1;
    }

    try{
        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        std::string input = buffer.str();

        Lexer lexer(input);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(tokens);
        ProgramNode program = parser.parse();
        std::cout << "Parsed " << program.declarations.size() << " declaration(s):\n";
        SemanticAnalyzer sema;
        sema.analyze(program);
        std::cout << "Program validated" << program.declarations.size() << "declaration(s)\n";

        for (const auto& decl : program.declarations) {
            std::visit([](const auto& node) {
                std::cout << "  - " << node.name;
                if constexpr (std::is_same_v<std::decay_t<decltype(node)>, PacketNode>) {
                    std::cout << " (" << node.totalSize << " bytes)";
                }
            std::cout << "\n";
            }, decl);
        }
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    

    return 0;
}

