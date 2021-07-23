
#define CHUNK_XDIMS 16
#define CHUNK_YDIMS 16
#define CHUNK_ZDIMS 255
#define CHUNK_UNIT 256
#define CHUNK_TEX_SQUARE_SIZE 16

typedef struct IVec3 IVec3;
typedef struct ChunkMesh ChunkMesh;
typedef struct Chunk Chunk;

typedef enum
{
    OCCUPIED_XMIN = 1,
    OCCUPIED_XMAX = 2,
    OCCUPIED_YMIN = 4,
    OCCUPIED_YMAX = 8,
    OCCUPIED_ZMIN = 16,
    OCCUPIED_ZMAX = 32,
} NeighbourFlag;

struct IVec3 
{
    ui16 x;
    ui16 y;
    ui16 z;
};

struct ChunkMesh
{
    ui32 vao;
    ui32 vbo;
    ui32 ebo;

    ui32 stride;
    ui16 *vertexBuffer;
    ui32 nVertices;
    ui32 maxVertices;

    ui32 *indexBuffer;
    ui32 nIndices;
    ui32 maxIndices;
};

#define ChunkIterI16(xIter, yIter, zIter) \
    for(i16 xIter = 0; xIter < CHUNK_XDIMS; xIter++)\
    for(i16 yIter = 0; yIter < CHUNK_YDIMS; yIter++)\
    for(i16 zIter = 0; zIter < CHUNK_ZDIMS; zIter++)

struct Chunk 
{
    int xIndex;
    int yIndex;
    int z;
    b32 isLoaded;
    b32 isActive;
    ui16 blocks[CHUNK_XDIMS][CHUNK_YDIMS][CHUNK_ZDIMS];

    // transient
    ui8 neighbourInfo[CHUNK_XDIMS][CHUNK_YDIMS][CHUNK_ZDIMS];
    ChunkMesh *mesh;
};


