#include "codegen.hpp"
std::string CodeGenerator::primitiveTypeToCpp(PrimitiveType type) const{
    switch (type) {
        case PrimitiveType::UInt8:   return "uint8_t";
        case PrimitiveType::UInt16:  return "uint16_t";
        case PrimitiveType::UInt32:  return "uint32_t";
        case PrimitiveType::Int8:    return "int8_t";
        case PrimitiveType::Int16:   return "int16_t";
        case PrimitiveType::Int32:   return "int32_t";
        case PrimitiveType::Float32: return "float";
        case PrimitiveType::Crc16:   return "uint16_t";
    }
    throw std::runtime_error("Unknown primitive type in codegen");
}

std::string CodeGenerator::generateEnum(const EnumNode& node){
    std::string result = "";
    result += "enum class ";
    result += node.name + ": " + primitiveTypeToCpp(node.backingType.value()) + " {\n";
    for(const auto& val: node.values){
        result += val.name + " = " + value.val + ",\n";
    }
    result += "};\n";
}

std::string CodeGenerator::generatePackFunction(const PacketNode& node){
    std::string result;
    result += "struct " + node.name + "{\n";
    std::vector<FieldNode> fields = node.fields;

    for(const auto& field: node.fields){
        result += " ";
        if(field.primitiveType.has_value()){
            result += primitiveTypeToCpp(field.primitiveType.value()) + " ";
        } else if(field.typeRef.has_value()){
            result += field.typeRef.value() + " ";
        }
        result += field.name;
        if(field.arraySize.has_value()){
            result += " ";
            result += "[" + std::to_string(field.arraySize.value()) + "]";
        }
        result += ";\n";
       
    }
    result += "};\n";
    result += "static_assert(sizeof(" + node.name + ") == " + std::to_string(node.totalSize) + ");\n\n";
    return result;   
}

std::string CodeGenerator::generatePackFunction(const PacketNode& node){
    std::string result;
    result += "inline void pack(const " + node.name + "& value, uint8_t* buffer) {\n";
    for(const auto& field: node.fields){
        if(field.isAuto) continue;

        result += " std::memcpy( buffer + " + std::to_string(field.byteOffset) + ", &value."
        + field.name + ", " + std::to_string(field.byteSize) + ");\n";
    }
    result += "}\n";
    return result;
}

std::string CodeGenerator::generateUnPackFunction(const PacketNode& node){
    std::string result;
    result += "inline void unPack(const " + node.name + "& value, uint8_t* buffer) {\n";
    for(const auto& field: node.fields){
        if(field.isAuto) continue;
        result += " std::memcpy(&value." + field.name + ", " + "buffer" + std::to_string(field.byteOffset) + ", " +
        std::to_string(field.byteSize) + ");\n";
    }
    result += "}\n";
    return result;
}

std::string CodeGenerator::generate(const ProgramNode& program){
    std::string result = "#pragma once\n#include <cstdint>\n#include <cstring>\n\n";

    for(auto& declaration: program.declarations){
        if(auto* enumNode = std::get_if<EnumNode>(&decl)){
            result += generateEnum(*enumNode);
        }
    }

    for(auto& declaration: program.declarations) {
        if(auto* packetNode = std::get_if<PacketNode>(&decl)){
            result += generatePacket(*packetNode);
            result += generatePackFunction(*packetNode);
            result += generateUnPackFunction(*packetNode)
        }
    }
    return result;
}


