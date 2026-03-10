#pragma once

#include <unordered_map>

// LRU cache
template<class Key, class Value>
class Cache
{
    struct Node
    {
        Key key;
        Value value;
        size_t prev;
        size_t next;
    };

public:
    static constexpr size_t InvalidNode = -1;

    explicit Cache(size_t capacity)
    : m_capacity(capacity)
    {
        m_nodes = new Node[capacity];

        m_nodesHead = 0;
        m_nodesTail = m_capacity - 1;

        for (size_t i = 0; i < m_capacity; i++)
        {
            m_nodes[i].prev = i > 0 ? i - 1 : InvalidNode;
            m_nodes[i].next = i < m_capacity - 1 ? i + 1 : InvalidNode;
        }
    }

    ~Cache()
    {
        delete m_nodes;
    }

    size_t touchNode(const Key& key)
    {
        if (m_nodeMap.find(key) == m_nodeMap.end()) return InvalidNode;

        size_t n = m_nodeMap[key];

        remove_node(n);
        push_back(n);

        return n;
    }

    size_t addNode(const Key& key)
    {
        size_t n = pop_front();

        m_nodes[n].key = key;
        push_back(n);

        m_nodeMap[key] = n;

        return n;
    }

    const Value& getValue(const Key& key) const
    {
        size_t n = m_nodeMap.at(key);
        return m_nodes[n].value;
    }

    Value& operator[](size_t n) { return m_nodes[n].value; }
    const Value& operator[](size_t n) const { return m_nodes[n].value; }

private:
    
    void push_back(size_t n)
    {
        if (m_nodesHead == InvalidNode)
        {
            m_nodesHead = n;
            m_nodes[n].prev = InvalidNode;
        }
        else
        {
            m_nodes[m_nodesTail].next = n;
            m_nodes[n].prev = m_nodesTail;
        }

        m_nodes[n].next = InvalidNode;
        m_nodesTail = n;
    }

    size_t pop_front()
    {
        size_t n = m_nodesHead;
        remove_node(n);

        m_nodeMap.erase(m_nodes[n].key);

        return n;
    }

    void remove_node(size_t n)
    {
        size_t onext = m_nodes[n].next;
        size_t oprev = m_nodes[n].prev;

        if (oprev != InvalidNode) m_nodes[oprev].next = onext;
        else m_nodesHead = onext;

        if (onext != InvalidNode) m_nodes[onext].prev = oprev;
        else m_nodesTail = oprev;
    }

private:

    const size_t m_capacity;

    //Nodes list
    Node* m_nodes;
    size_t m_nodesHead;
    size_t m_nodesTail;

    std::unordered_map<Key, size_t> m_nodeMap;
};