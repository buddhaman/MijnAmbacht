
#define CHUNK_SIZE 16
#define CHUNK_DEPTH 256
#define CHUNK_UNIT 256

typedef struct IVec3 IVec3;
typedef struct ChunkMesh ChunkMesh;
typedef struct Chunk Chunk;

struct IVec3 
{
    ui32 x;
    ui32 y;
    ui32 z;
};

struct ChunkMesh
{
    ui32 vao;
    ui32 vbo;
    ui32 ebo;

    ui32 stride;
    ui16 *vertexBuffer;
    ui32 nVertices;
    ui16 *indexBuffer;
    ui32 nIndices;
};

struct Chunk 
{
    i16 x;
    i16 y;
    i16 z;
    ui16 blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_DEPTH];
};

