#pragma once

#include <cstdint>

// Metadata de items que solo necesita el cliente para mostrar texto
// (nombre largo, abreviatura de 3 letras, categoría) y que Items::get()
// del server no expone. Extraída de InventoryRenderer: es lookup de
// datos, no dibujado — antes vivía mezclada con los draw_* en la misma
// clase (y antes de eso, en InventoryPanel).
namespace ItemLabels {
const char* name(uint8_t id);
const char* abbr(uint8_t id);
const char* kind(uint8_t id);
}  // namespace ItemLabels
