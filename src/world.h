
typedef struct World World;

#define GetChunk(world, x, y) &world->chunks[(x)+(y)*world->xChunks]
#define GetChunkMesh(world, x, y) &world->chunkMeshes[(x)+(y)*world->xChunks]

struct World
{
    MemoryArena *arena;
    ui32 xChunks;
    ui32 yChunks;
    Chunk *chunks;
    ChunkMesh *chunkMeshes;
};

