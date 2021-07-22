
typedef struct
{
    Chunk *chunk;
    ChunkMesh *mesh;
    ui16 x; // In chunk.
    ui16 y;
    ui16 z;
} BlockPosition;
internal inline b32
GetBlockPositionSlow(World *world, ui32 x, ui32 y, ui32 z, BlockPosition *blockPosition)
{
    int chunkX = x/CHUNK_XDIMS;
    int chunkY = y/CHUNK_XDIMS;
    if(chunkX >= 0 && chunkY >= 0 &&
        chunkX < world->xChunks && chunkY < world->yChunks)
    {
        Chunk *chunk = GetChunk(world, chunkX, chunkY);
        ChunkMesh *mesh = GetChunkMesh(world, chunkX, chunkY);
        *blockPosition = (BlockPosition){chunk, mesh, x-CHUNK_XDIMS*chunkX, y-CHUNK_YDIMS*chunkY, z};
        return 1;
    }
    else
    {
        return 0;
    }
}



internal inline void
SetBlockNeighbourFlagSlow(World *world, int worldX, int worldY, int worldZ, NeighbourFlag flag)
{
    BlockPosition bPos;
    if(GetBlockPositionSlow(world, worldX, worldY, worldZ, &bPos))
    {
        bPos.mesh->neighbourInfo[bPos.x][bPos.y][bPos.z] |= flag;
    }
}

internal inline void
UpdateChunkNeighbourInfoSlow(World *world, ChunkMesh *mesh, Chunk *chunk)
{
    ui32 chunkX = chunk->xIndex*CHUNK_XDIMS;
    ui32 chunkY = chunk->yIndex*CHUNK_YDIMS;
    ChunkIterI16(x, y, z)
    {
        int xPos = chunkX+x;
        int yPos = chunkY+y;
        int zPos = z;
        ui16 block = chunk->blocks[x][y][z];
        if(block)
        {
            SetBlockNeighbourFlagSlow(world, xPos-1, yPos, zPos, OCCUPIED_XMAX);
            SetBlockNeighbourFlagSlow(world, xPos+1, yPos, zPos, OCCUPIED_XMIN);
            SetBlockNeighbourFlagSlow(world, xPos, yPos, zPos-1, OCCUPIED_ZMAX);
            SetBlockNeighbourFlagSlow(world, xPos, yPos, zPos+1, OCCUPIED_ZMIN);
            SetBlockNeighbourFlagSlow(world, xPos, yPos-1, zPos, OCCUPIED_YMAX);
            SetBlockNeighbourFlagSlow(world, xPos, yPos+1, zPos, OCCUPIED_YMIN);
        }
    }
}

global_variable fnl_state noiseX;
void
UpdateChunkMesh(World *world, ChunkMesh *mesh, Chunk *chunk)
{
    mesh->nVertices = 0;
    mesh->nIndices = 0;

    local_persist b32 init = 0;
    if(!init)
    {
        noiseX = fnlCreateState();
        noiseX.noise_type = FNL_NOISE_PERLIN;
        noiseX.octaves = 4;
        noiseX.seed = RandomUI32(0, 123123U);
        init = 1;
    }

    ChunkIterI16(x, y, z)
    {
        ui16 block = chunk->blocks[x][y][z];
        IVec3 pos = ivec3(x*CHUNK_UNIT, y*CHUNK_UNIT, z*CHUNK_UNIT);
        if(block)
        {
            ui8 neighbourInfo = mesh->neighbourInfo[x][y][z];
            IVec3 p000 = pos;
            IVec3 p001 = iv3_add(pos, ivec3(0, 0, CHUNK_UNIT));
            IVec3 p010 = iv3_add(pos, ivec3(0, CHUNK_UNIT, 0));
            IVec3 p011 = iv3_add(pos, ivec3(0, CHUNK_UNIT, CHUNK_UNIT));
            IVec3 p100 = iv3_add(pos, ivec3(CHUNK_UNIT, 0, 0));
            IVec3 p101 = iv3_add(pos, ivec3(CHUNK_UNIT, 0, CHUNK_UNIT));
            IVec3 p110 = iv3_add(pos, ivec3(CHUNK_UNIT, CHUNK_UNIT, 0));
            IVec3 p111 = iv3_add(pos, ivec3(CHUNK_UNIT, CHUNK_UNIT, CHUNK_UNIT));

            ui16 texU = world->textureTableU[block];
            ui16 texV = world->textureTableV[block];
            ui16 texUTop = world->textureTableUTop[block];
            ui16 texVTop = world->textureTableVTop[block];

            r32 roughness = 1;
            int worldX = chunk->xIndex*CHUNK_XDIMS + x;
            int worldY = chunk->yIndex*CHUNK_YDIMS + y;
            r32 noise2Value = fnlGetNoise2D(&noiseX, worldX*roughness, worldY*roughness);
            ui32 r = 255U*(noise2Value*0.8+0.5);
            ui32 g = 255-z;
            ui32 b = 50;
            ui32 greenColor = (r << 24) + (g << 16) + (b << 8) + 0xff;
            //greenColor = RandomUI32(0, 0xffffffff);
            ui32 topColor = block==BLOCK_DIRT ? greenColor : 0xffffffff;

            // X
            if(!(neighbourInfo & OCCUPIED_XMIN))
            {
                PushQuadToChunkMesh(mesh,
                        p001,
                        p011,
                        p010,
                        p000,
                        -CHUNK_UNIT, 0, 0, 
                        texU, texV);
            }
            if(!(neighbourInfo & OCCUPIED_XMAX))
            {
                PushQuadToChunkMesh(mesh,
                        p100,
                        p110,
                        p111,
                        p101,
                        CHUNK_UNIT, 0, 0,
                        texU, texV);
            }

            // Y
            if(!(neighbourInfo & OCCUPIED_YMIN))
            {
                PushQuadToChunkMesh(mesh,
                        p000,
                        p100,
                        p101,
                        p001,
                        0, -CHUNK_UNIT, 0,
                        texU, texV);
            }
            if(!(neighbourInfo & OCCUPIED_YMAX))
            {
                PushQuadToChunkMesh(mesh,
                        p011,
                        p111,
                        p110,
                        p010,
                        0, CHUNK_UNIT, 0,
                        texU, texV);
            }

            // Z
            if(!(neighbourInfo & OCCUPIED_ZMIN))
            {
                PushQuadToChunkMesh(mesh,
                        p010,
                        p110,
                        p100,
                        p000,
                        0, 0, -CHUNK_UNIT,
                        texU, texV);
            }
            if(!(neighbourInfo & OCCUPIED_ZMAX))
            {
#if 1
                ui32 fromIdx = mesh->nVertices;

                ui16 texSize = CHUNK_TEX_SQUARE_SIZE;
                ui16 xn = 0; ui16 yn = 0; ui16 zn = CHUNK_UNIT;
                ui16 u = texUTop; ui16 v = texVTop;
                PushColoredVertexToChunkMesh(mesh, p001, xn, yn, zn, u, v, topColor);
                PushColoredVertexToChunkMesh(mesh, p101, xn, yn, zn, u+texSize, v, topColor);
                PushColoredVertexToChunkMesh(mesh, p111, xn, yn, zn, u+texSize, v+texSize, topColor);
                PushColoredVertexToChunkMesh(mesh, p011, xn, yn, zn, u, v+texSize, topColor);

                PushIndexToChunkMesh(mesh, fromIdx);
                PushIndexToChunkMesh(mesh, fromIdx+1);
                PushIndexToChunkMesh(mesh, fromIdx+2);
                PushIndexToChunkMesh(mesh, fromIdx+2);
                PushIndexToChunkMesh(mesh, fromIdx+3);
                PushIndexToChunkMesh(mesh, fromIdx+0);

#else
                PushQuadToChunkMesh(mesh,
                        p010,
                        p110,
                        p100,
                        p000,
                        0, 0, CHUNK_UNIT,
                        texU, texV);
#endif
            }
        }
    }
}

void
GenerateWorld(World *world)
{
    r32 hills = 30;
    r32 roughness = 6;
    r32 roughness3 = 8;
    ui32 waterLevel = 70;
    ui32 groundLevel = 80;
    fnl_state noise2d = fnlCreateState();
    
    noise2d.noise_type = FNL_NOISE_PERLIN;
    noise2d.octaves = 4;
    noise2d.seed = RandomUI32(0, 123123U);

    fnl_state noise3d = fnlCreateState();

    for(ui32 yChunk = 0; yChunk < world->yChunks; yChunk++)
    for(ui32 xChunk = 0; xChunk < world->xChunks; xChunk++)
    {
        Chunk *chunk = GetChunk(world, xChunk, yChunk);
        chunk->xIndex = xChunk;
        chunk->yIndex = yChunk;
        chunk->z = 0;
        for(ui16 bx = 0; bx < CHUNK_XDIMS; bx++)
        for(ui16 by = 0; by < CHUNK_YDIMS; by++)
        {
            int x = xChunk*CHUNK_XDIMS+bx;
            int y = yChunk*CHUNK_YDIMS+by;
            r32 noise2Value = fnlGetNoise2D(&noise2d, x*roughness, y*roughness);
            ui32 height = groundLevel+(ui32)(noise2Value*hills);
            for(ui16 bz = 0; bz < height; bz++)
            {
                if(bz > waterLevel-2 &&
                        height < waterLevel+2)
                {
                    chunk->blocks[bx][by][bz] = BLOCK_SAND;
                }
                else if(bz > waterLevel-15)
                {
                    chunk->blocks[bx][by][bz] = BLOCK_DIRT;
                }
                else
                {
                    chunk->blocks[bx][by][bz] = BLOCK_STONE;
                }
            }
            for(ui16 bz = height; bz < waterLevel; bz++)
            {
                chunk->blocks[bx][by][bz] = BLOCK_WATER;
            }
            for(ui16 bz = 0; bz < height; bz++)
            {
                r32 noise3Value = fnlGetNoise3D(&noise3d, x*roughness3/4, y*roughness3/4, bz*roughness3);
                if(noise3Value < -0.4 && chunk->blocks[bx][by][bz]!=BLOCK_WATER)
                {
                    chunk->blocks[bx][by][bz] = 0;
                }
            }
        }
    }
}

void
InitWorld(MemoryArena *arena, World *world)
{
    world->arena = arena;
    world->xChunks = 10;
    world->yChunks = 10;
    world->chunks = PushAndZeroArray(arena, Chunk, world->xChunks*world->yChunks);
    world->chunkMeshes = PushAndZeroArray(arena, ChunkMesh, world->xChunks*world->yChunks);

    // Texture table
    ui16 texSize = CHUNK_TEX_SQUARE_SIZE;

    world->textureTableU[BLOCK_DIRT] = texSize*2;
    world->textureTableV[BLOCK_DIRT] = 0;
    world->textureTableUTop[BLOCK_DIRT] = 0;
    world->textureTableVTop[BLOCK_DIRT] = 0;

    world->textureTableU[BLOCK_SAND] = texSize*2;
    world->textureTableV[BLOCK_SAND] = texSize;
    world->textureTableUTop[BLOCK_SAND] = texSize*2;
    world->textureTableVTop[BLOCK_SAND] = texSize;

    world->textureTableU[BLOCK_WATER] = texSize*14;
    world->textureTableV[BLOCK_WATER] = 0;
    world->textureTableUTop[BLOCK_WATER] = texSize*14;
    world->textureTableVTop[BLOCK_WATER] = 0;

    world->textureTableU[BLOCK_STONE] = texSize;
    world->textureTableV[BLOCK_STONE] = 0;
    world->textureTableUTop[BLOCK_STONE] = texSize;
    world->textureTableVTop[BLOCK_STONE] = 0;

    // ORDER IS IMPORTANT!!!

    GenerateWorld(world);
    // Allocate meshes
    for(ui32 y = 0; y < world->yChunks; y++)
    for(ui32 x = 0; x < world->xChunks; x++)
    {
        ChunkMesh *mesh = GetChunkMesh(world, x, y);
        AllocateChunkMesh(arena, mesh);
    }

    // Calculate neighbourhood info 
    for(ui32 y = 0; y < world->yChunks; y++)
    for(ui32 x = 0; x < world->xChunks; x++)
    {
        Chunk *chunk = GetChunk(world, x, y);
        ChunkMesh *mesh = GetChunkMesh(world, x, y);
        UpdateChunkNeighbourInfoSlow(world, mesh, chunk);
    }
    
    // Update mesh and buffer data
    for(ui32 y = 0; y < world->yChunks; y++)
    for(ui32 x = 0; x < world->xChunks; x++)
    {
        Chunk *chunk = GetChunk(world, x, y);
        ChunkMesh *mesh = GetChunkMesh(world, x, y);
        UpdateChunkMesh(world, mesh, chunk);
        BufferChunkMesh(mesh);
    }
}

