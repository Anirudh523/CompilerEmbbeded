#include "sema.hpp"
#include <iostream>
#include <string>
#include <variant>
#include <vector>
void SemanticAnalyzer::buildSymbolTable(ProgramNode& program){
    for(auto& decl: program.declarations){
        if(auto* packet = std::get_if<PacketNode>(&decl)) {
            if(packetTable_.count(packet->name) || enumTable_.count(packet->name)){
                throw std::runtime_error("Duplicate declaration name. " + packet->name);
            }
            packetTable_[packet->name] = packet;
        } else if(auto* enum = std::get_if<EnumNode>(&decl)){
            if(packetTable_.count(enum->name) || enumTable_.count(packet->name)){
                throw std::runtime_error("Duplicate declaration name." + )
            }
            enumTable_[enum->name] = packet;
        }
    }
}

void SemanticAnalyzer::resolveField(FieldNode& field, const std::string& packetName){
    if(!field.typeRef.has_value()) return;

    const std::string& ref = field.typeRef.value();

    if(ref){
        if(packetTable_.count(ref)){
            //size will be calculated later
        } else if(enumTable_.count(ref)){
            //size will be calculate later
        } else {
            throw std::runtime_error(
                "Unknown type " + ref + " in the field " + field.name + "of packet " + packetName;
            )
        }
    }
}

void SemanticAnalyzer::resolvePacket(PacketNode& packet) {
    static std::vector<std::string> resolutionStack;

    if(std::find(resolutionStack.begin(), resolutionStack.end(), packet.name) != resolutionStack.end()){
        throw std::runtime_error("Circular packet detected involving" + packet.name);
    }

    resolutionStack.push_back(packet);


    for(auto& field: packet.fields){
        resolveField(field, packet);
        if(field.typeRef && packetTable_.count(field.typeRef.value())){
            resolvePacket(field.typeRef.value());
        }

    }

    resolutionStack.pop_back();
}

