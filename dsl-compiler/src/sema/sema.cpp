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

void SemanticAnalyzer::validateBitfields(PacketNode& packet) {
    int bitAccumulator = 0;
    for(auto& field: packet.fields){
        const std::string& ref = field.typeRef.value();
        if(field.bit_width.has_value()){
            bitAccumulator += field.bit_width;
        } else {
            bitAccumulator = 0;
        }
        if(bitAccumulator > 8){throw std::runtime_error("Bit width exceeded 8 in the packet. " + packet.name );};
    }
}

int SemanticAnalyzer::primitiveSizeInBytes(PrimitiveType type) const {
    switch(type){
        case PrimitiveType::UInt8:
        case PrimitiveType::Int8: return 1;
        case PrimitiveType::UInt16:
        case PrimitiveType::Crc16:
        case PrimitiveType::Int16: return 2;
        case PrimitiveType::UInt32:
        case PrimitiveType::Int32:
        case PrimitiveType::Float32: return 4;   
    }
    throw std::runtime_error("Unknown primitive type used in the size calculation");
}

void SemanticAnalyzer::computeLayout(PacketNode& packet){
    int offset = 0;
    for(auto& field: packet.fields){
        int size = 0;
        if(field.p_type.has_value()){
            size += primitiveSizeInBytes(field.p_type.value);
        } else {
            std::string external_pack = field.typeRef.value();
            if(packetTable_.count(external_pack)){
                size += packetTable_[external_pack].totalSize;
            } else if(enumTable_.count(external_pack)){
                size += enumTable_[external_pack].totalSize;
            }
        }
        if(field.arraySize.has_value()){size *= field.arraySize.value();};
        offset += size;
        packet.byteOffset = offset;
        packet.byteSize = size;
    }
}

void SemanticAnalyzer::resolveEnumBackingType(EnumNode& node){
    if(node.backingType.has_value()){return;};

    int min_value = 0;
    int max_value = 0;
    for(const auto& e: node.values){
        min_value = std::min(min_value, e.value);
        max_value = std::max(max_value, e.value);
    }

    bool needs_sign = (min_value < 0);
    if(!needs_sign){
        if(max_value <= 255){
            node.backingType = PrimitiveType::UInt8;
        } else if(max_value <= 65535){
            node.backingType = PrimitiveType::UInt16;
        } else{
            node.backingType = PrimitiveType::UInt32;
        }
    } else {
        if (min_value >= -128 && max_value <= 127)          node.backingType = PrimitiveType::Int8;
        else if (min_value >= -32768 && max_value <= 32767) node.backingType = PrimitiveType::Int16;
        else                                                node.backingType = PrimitiveType::Int32;

    }

}

void SemanticAnalyzer::analyze(ProgramNode& program){
    buildSymbolTable(program);
    for (auto& [name, enumNode] : enumTable_) {
        resolveEnumBackingType(*enumNode);
    }

    for (auto& [name, packetNode] : packetTable_) {
        resolvePacket(*packetNode);       
        computeLayout(*packetNode);        
    }
}

