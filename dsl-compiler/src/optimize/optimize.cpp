#include "optimize.hpp"
#include <variant>

bool Optimizer::canReorder(const PacketNode& packet) const{
    for(const auto& field: packet.fields){
        if(field.bitWidth.has_value() || field.isAuto){
            return false;
        }
    }
    return true;
}

void Optimizer::packFields(PacketNode& packet) {
    if(!canReorder(packet)) return;

    std::stable_sort(packet.fields.begin(), packet.fields.end(), [](const FieldNode& a, const FieldNode& b){
        return a.byteSize > b.byteSize;
    });
}

void Optimizer::optimize(ProgramNode& program){
    for (auto& decl : program.declarations) {
        if (auto* packet = std::get_if<PacketNode>(&decl)) {
            packFields(*packet);
        }
    }
}