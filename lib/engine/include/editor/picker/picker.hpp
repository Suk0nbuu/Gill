#pragma once
#ifndef CLEAR_VALUE
#define CLEAR_VALUE 4294967295
#endif
#include <functional>
#include <cstdint>
#include <iostream>
#include "encode.hpp"
namespace gill_editor {





class Picker {
public:
    void Init(unsigned int width, unsigned int height);
    void Resize(unsigned int width, unsigned int height);
    void Run(const std::vector<Entry>& entry);
    unsigned int  ReadAt(int x, int y);


private:
    void Reset();
    void CalculateBits();
    unsigned int m_pickWidth, m_pickHeight;
    unsigned int m_pickFBO;
    unsigned int m_pickDepthRenderbuffer;
    unsigned int m_pickTexture;
    unsigned int m_clearValue =  4294967295; // 0xFFFFFFFF , the largest maximum value;
};
}