#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "shadergraph/codegen.hpp"

class GeneratedShaderCache {
public:
    int Register(const CodegenResult& r) {                 // identical source -> same id
        if (auto it = m_bySource.find(r.fragSource); it != m_bySource.end()) return it->second;
        const int id = static_cast<int>(m_programs.size());
        m_programs.push_back(r);
        m_bySource.emplace(r.fragSource, id);
        return id;
    }
    const CodegenResult& Program(int id) const { return m_programs[id]; }
private:
    std::vector<CodegenResult> m_programs;
    std::unordered_map<std::string, int> m_bySource;
};