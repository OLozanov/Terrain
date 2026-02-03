#pragma once

#include "Render/Vulkan/Buffer.h"

namespace Render
{

template<class T>
class ConstantBuffer : public Buffer
{
public:
    explicit ConstantBuffer(size_t size = 1)
    : Buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(T) * size)
    {
        m_mappedData = static_cast<T*>(map(sizeof(T) * size));
    }

    ~ConstantBuffer()
    {
        unmap();
    }

    T& operator*() const { return *m_mappedData; }
    T* operator->() const { return m_mappedData; }
    T& operator[](size_t n) const { return m_mappedData[n]; }

private:

    T* m_mappedData;
};

} // namespace Render