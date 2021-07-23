
typedef struct World World;
typedef struct WorldGenerator WorldGenerator;

typedef enum
{
    BLOCK_EMPTY = 0,
    BLOCK_DIRT = 1,
    BLOCK_SAND = 2,
    BLOCK_WATER = 3,
    BLOCK_STONE = 4,
} BlockType;

struct WorldGenerator 
{
    r32 firstSmoothness;
    r32 roughness2;
    r32 roughness3;
    r32 hillRoughness;
    r32 hillHeight;
    int waterLevel;
    int groundLevel;
    fnl_state noise2d;
    fnl_state hillNoise;
    fnl_state noise3d;
};

struct World
{
    MemoryArena *arena;
    int xChunks;
    int yChunks;
    int nChunks;
    int activeChunkX;
    int activeChunkY;
    Chunk *chunkArray;
    Chunk **chunks;
    WorldGenerator generator;

    ui16 textureTableUTop[8];
    ui16 textureTableVTop[8];
    ui16 textureTableU[8];
    ui16 textureTableV[8];
};


