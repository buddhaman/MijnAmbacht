
typedef struct World World;

#define GetChunk(world, x, y) &world->chunks[(x)+(y)*world->xChunks]
#define GetChunkMesh(world, x, y) &world->chunkMeshes[(x)+(y)*world->xChunks]

typedef enum
{
    BLOCK_EMPTY = 0,
    BLOCK_DIRT = 1,
    BLOCK_SAND = 2,
    BLOCK_WATER = 3,
    BLOCK_STONE = 4,
} BlockType;

struct World
{
    MemoryArena *arena;
    ui32 xChunks;
    ui32 yChunks;
    Chunk *chunks;
    ChunkMesh *chunkMeshes;

    ui16 textureTableUTop[8];
    ui16 textureTableVTop[8];
    ui16 textureTableU[8];
    ui16 textureTableV[8];
};

