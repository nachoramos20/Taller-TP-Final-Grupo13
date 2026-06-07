#ifndef WORLD_ID_ALLOCATOR_H
#define WORLD_ID_ALLOCATOR_H

#include <cstdint>

// Generador simple de IDs de entidad (NPCs / items en suelo).
class IdAllocator {
private:
    uint16_t next;
public:
    explicit IdAllocator(uint16_t start) : next(start) {}
    uint16_t allocate() { return next++; }
};

#endif
