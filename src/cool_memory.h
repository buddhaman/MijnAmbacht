#include <stdlib.h>

typedef struct 
{
    ui8 *base;
    size_t size;
} GameMemory;

typedef struct
{
    ui8 *base;
    size_t used;
    size_t size;
    GameMemory *memory;
} MemoryArena;

typedef struct
{
    ui8 *base;
    size_t sizeInBytes;

    size_t elementSize;
    size_t maxBlocks;
    ui32 *blocks;           // Blocks of size 32.
} MemoryPool;

