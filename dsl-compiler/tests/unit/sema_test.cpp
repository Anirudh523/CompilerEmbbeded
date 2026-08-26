#include <catch2/catch_test_macros.hpp>
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "sema/sema.hpp"

TEST_CASE("computes correct byte offsets"){
    Lexer lexer("packet Foo { uint16 seq; uint32 timestamp; }");
    Parser parser(lexer.tokenize());
    ProgramNode program = parser.parse();

    SemanticAnalyzer sema;
    sema.analyze(program);

    auto* pkt = std::get_if<PacketNode>(&program.declarations[0]);
    REQUIRE(pkt->fields[0].byteOffset == 0);
    REQUIRE(pkt->fields[1].byteOffset == 2);
    REQUIRE(pkt->totalSize == 6);
}

TEST_CASE("rejects unknown type reference") {
    Lexer lexer("packet Foo { Bar x; }");
    Parser parser(lexer.tokenize());
    ProgramNode program = parser.parse();

    SemanticAnalyzer sema;
    REQUIRE_THROWS_AS(sema.analyze(program), std::runtime_error);
}

TEST_CASE("rejects circular packet reference") {
    Lexer lexer("packet A { B b; } packet B { A a; }");
    Parser parser(lexer.tokenize());
    ProgramNode program = parser.parse();

    SemanticAnalyzer sema;
    REQUIRE_THROWS_AS(sema.analyze(program), std::runtime_error);
}