#pragma once
#include "ast/ast.hpp"

class Optimizer {
public:
    void optimize(ProgramNode& program);

private:
    bool canReorder(const PacketNode& packet) const;
    void packFields(PacketNode& packet);
};