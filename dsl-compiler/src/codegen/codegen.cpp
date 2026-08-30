#include "codegen.hpp"
#include <stdexcept>
#include <variant>

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
    result += node.name + " : " + primitiveTypeToCpp(node.backingType.value()) + " {\n";
    for(const auto& val: node.values){
        result += "    " + val.name + " = " + std::to_string(val.value) + ",\n";
    }
    result += "};\n\n";
    return result;
}

std::string CodeGenerator::generatePacket(const PacketNode& node){
    std::string result;
    result += "struct " + node.name + " {\n";
    
    bool hasBitFields = false;
    for(const auto& field: node.fields){
        result += "    ";
        if(field.bitWidth.has_value()){hasBitFields = true;};

        if(field.primitiveType.has_value()){
            result += primitiveTypeToCpp(field.primitiveType.value()) + " ";
        } else if(field.typeRef.has_value()){
            result += field.typeRef.value() + " ";
        }
        result += field.name;
        if(field.arraySize.has_value()){
            result += "[" + std::to_string(field.arraySize.value()) + "]";
        }
        result += ";\n";
    }
    result += "};\n";
     if (!hasBitFields) {
        result += "static_assert(sizeof(" + node.name + ") == " + std::to_string(node.totalSize) + ");\n\n";
    } else {
        result += "// Note: " + node.name + " packs to " + std::to_string(node.totalSize)
                 + " bytes on the wire; in-memory struct size may differ.\n\n";
    }
    return result;
}

std::string CodeGenerator::generatePackFunction(const PacketNode& node){
    std::string result;
    result += "inline void pack(const " + node.name + "& value, uint8_t* buffer) {\n";
    const FieldNode* checksum = nullptr; 
    for(const auto& field: node.fields){
        if(field.isAuto){
            checksum = &field;
            continue;
        }

        if(field.bitWidth.has_value()){
            int mask = (1 << field.bitWidth.value()) - 1;
            if (field.bitShift + field.bitWidth.value() == 8) {
                result += "    buffer[" + std::to_string(field.byteOffset) + "] = 0;\n";
            }
            result += "    buffer[" + std::to_string(field.byteOffset) + "] = (buffer["
                + std::to_string(field.byteOffset) + "] & ~(" + std::to_string(mask)
                + " << " + std::to_string(field.bitShift) + ")) | ((value."
                + field.name + " & " + std::to_string(mask) + ") << "
                + std::to_string(field.bitShift) + ");\n";
            continue;
        }

        result += "    std::memcpy(buffer + " + std::to_string(field.byteOffset) + ", &value."
        + field.name + ", " + std::to_string(field.byteSize) + ");\n";
    }
    if(checksum){
        result += "    uint16_t computed_crc = crc16(buffer, " + std::to_string(checksum->byteOffset)
         + ");\n";
        result += "    std::memcpy(buffer + " + std::to_string(checksum->byteOffset) + ", &computed_crc, " + 
         std::to_string(checksum->byteSize) +");\n";
    }
    result += "}\n\n";
    return result;
}

std::string CodeGenerator::generateUnPackFunction(const PacketNode& node){
    std::string result;
    result += "inline bool unpack(const uint8_t* buffer, " + node.name + "& value) {\n";

    const FieldNode* checksumField = nullptr;
    for(const auto& field: node.fields){
        if (field.bitWidth.has_value()) {
            int mask = (1 << field.bitWidth.value()) - 1;
            result += "    value." + field.name + " = (buffer[" + std::to_string(field.byteOffset)
            + "] >> " + std::to_string(field.bitShift) + ") & " + std::to_string(mask) + ";\n";
            continue;
        }
        if(field.isAuto){
            checksumField = &field;
        }
        result += "    std::memcpy(&value." + field.name + ", buffer + " + std::to_string(field.byteOffset) + ", " +
        std::to_string(field.byteSize) + ");\n";
    }

    if(checksumField != nullptr){
        result += "    uint16_t recieved_crc;\n";
        result += "    std::memcpy(&recieved_crc, buffer + " + std::to_string(checksumField -> byteOffset) + ", "
        + std::to_string(checksumField -> byteSize) + ");\n";
        result += "    uint16_t calculated_crc = crc16(buffer, " + std::to_string(checksumField -> byteOffset) + ");\n";
        result += "    if (recieved_crc != calculated_crc) return false;\n";
    }
    result += "    return true;\n";
    result += "}\n\n";
    return result;
}

std::string CodeGenerator::generate(const ProgramNode& program){
    std::string result = "#pragma once\n#include <cstdint>\n#include <cstring>\n\n";
    result += generateChecksumHelpers();
    for(auto& decl: program.declarations){
        if(auto* enumNode = std::get_if<EnumNode>(&decl)){
            result += generateEnum(*enumNode);
        }
    }

    for(auto& decl: program.declarations) {
        if(auto* packetNode = std::get_if<PacketNode>(&decl)){
            result += generatePacket(*packetNode);
            result += generatePackFunction(*packetNode);
            result += generateUnPackFunction(*packetNode);
        }
    }
    return result;
}

std::string CodeGenerator::generateChecksumHelpers() {
    std::string result;
    result += "inline uint16_t crc16(const uint8_t* data, size_t length) {\n";
    result += " uint16_t crc = 0xFFFF;\n";
    result += " for (size_t i = 0; i < length; i++) {\n";
    result += "     crc ^= static_cast<uint16_t>(data[i]) << 8;\n";
    result += "     for(int bit = 0; bit < 8; bit++) {\n";
    result += "         if(crc & 0x8000) {\n";
    result += "             crc = (crc << 1) ^ 0x1021;\n";
    result += "         } else {\n";
    result += "             crc <<= 1;\n";
    result += "         }\n";
    result += "     }\n";
    result += " }\n";
    result += " return crc;\n";
    result += "}\n\n";
    return result;
}