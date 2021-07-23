
typedef struct
{
    int x;
    int y;
} Int16Vec;
internal inline Int16Vec
GetChunkIndicesAtPosition(Vec2 pos)
{
    return (Int16Vec){(int)floor(pos.x/CHUNK_XDIMS), (int)floor(pos.y/CHUNK_XDIMS)};
}

internal inline void
GenerateChunk(World *world, Chunk *chunk)
{
    memset(chunk->blocks, 0, sizeof(chunk->blocks));
    r32 roughness2 = world->generator.roughness2;
    r32 roughness3 = world->generator.roughness3;
    r32 hillRoughness = world->generator.hillRoughness;

    for(ui16 bx = 0; bx < CHUNK_XDIMS; bx++)
    for(ui16 by = 0; by < CHUNK_YDIMS; by++)
    {
        int x = chunk->xIndex*CHUNK_XDIMS+bx + 1000;
        int y = chunk->yIndex*CHUNK_YDIMS+by + 1000;
        r32 noise2Value = fnlGetNoise2D(&world->generator.noise2d, x*roughness2, y*roughness2);
        r32 hillNoiseValue = fnlGetNoise2D(&world->generator.hillNoise, x*hillRoughness, y*hillRoughness);
        r32 hillFactor = 0.0f;
        if(noise2Value > 0.0f)
        {
            hillFactor = noise2Value;
        }
        ui32 height = world->generator.groundLevel+
            (ui32)(noise2Value*world->generator.firstSmoothness + 
                    world->generator.hillHeight*hillFactor*hillNoiseValue);
        Assert(height < 255);
        for(ui16 bz = 0; bz < height; bz++)
        {
            if(bz > world->generator.waterLevel-2 &&
                    height < world->generator.waterLevel+2)
            {
                chunk->blocks[bx][by][bz] = BLOCK_SAND;
            }
            else if(bz > world->generator.waterLevel-15)
            {
                chunk->blocks[bx][by][bz] = BLOCK_DIRT;
            }
            else
            {
                chunk->blocks[bx][by][bz] = BLOCK_STONE;
            }
        }
        for(ui16 bz = height; bz < world->generator.waterLevel; bz++)
        {
            chunk->blocks[bx][by][bz] = BLOCK_WATER;
        }
        for(ui16 bz = 0; bz < height; bz++)
        {
            r32 heightFactor = ((r32)bz)/height;
            r32 noise3Value = fnlGetNoise3D(&world->generator.noise3d, x*roughness3/4, y*roughness3/4, bz*roughness3);
            if((1.0-heightFactor*heightFactor*heightFactor)*noise3Value < -0.4 && chunk->blocks[bx][by][bz]!=BLOCK_WATER)
            {
                chunk->blocks[bx][by][bz] = 0;
            }
        }
    }

}

// Generates chunk if it doesnt exist.
internal inline Chunk *
GetChunkSlow(World *world, int x, int y)
{
    int xIdx = x-world->activeChunkX;
    int yIdx = y-world->activeChunkY;
    if(xIdx >= 0 && yIdx >= 0
            && xIdx < world->xChunks && yIdx < world->yChunks)
    {
        Chunk *chunk = world->chunks[xIdx+yIdx*world->xChunks];
        if(!chunk)
        {
            // load inactive chunk
            for(ui32 chunkIdx = 0; 
                    chunkIdx < world->nChunks; 
                    chunkIdx++)
            {
                Chunk *newChunk = world->chunkArray + chunkIdx;
                if(!newChunk->isActive)
                {
                    chunk = world->chunks[xIdx+yIdx*world->xChunks] = newChunk;
                    chunk->xIndex = x;
                    chunk->yIndex = y;
                    chunk->z = 0;
                    GenerateChunk(world, chunk);
                    chunk->isActive = 1;
                    DebugOut("Generated chunk %d %d", chunk->xIndex, chunk->yIndex);
                    break;
                }
            }
        }
        return chunk;
    }
    else
    {
        return NULL;
    }
}

typedef struct
{
    Chunk *chunk;
    i16 x; // In chunk.
    i16 y;
    i16 z;
} BlockPosition;
internal inline b32
GetBlockPositionSlow(World *world, int x, int y, int z, BlockPosition *blockPosition)
{
    if(z < 0 || z > CHUNK_ZDIMS)
        return 0;
    Int16Vec chunkIdx = GetChunkIndicesAtPosition(vec2(x, y));
    Chunk *chunk = GetChunkSlow(world, chunkIdx.x, chunkIdx.y);
    if(chunk)
    {
        *blockPosition = (BlockPosition){chunk, x-CHUNK_XDIMS*chunk->xIndex, y-CHUNK_YDIMS*chunk->yIndex, z};
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
        Assert(bPos.x >= 0 && bPos.y >= 0 && bPos.z >= 0
                && bPos.x < CHUNK_XDIMS && bPos.y < CHUNK_YDIMS && bPos.z < CHUNK_ZDIMS);
        bPos.chunk->neighbourInfo[bPos.x][bPos.y][bPos.z] |= flag;
    }
}

internal inline void
UpdateChunkNeighbourInfoSlow(World *world, Chunk *chunk)
{
    int chunkX = chunk->xIndex*CHUNK_XDIMS;
    int chunkY = chunk->yIndex*CHUNK_YDIMS;
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
UpdateChunkMesh(World *world, Chunk *chunk)
{
    ChunkMesh *mesh = chunk->mesh;
    mesh->nVertices = 0;
    mesh->nIndices = 0;

    local_persist b32 init = 0;
    if(!init)
    {
        noiseX = fnlCreateState();
        noiseX.noise_type = FNL_NOISE_OPENSIMPLEX2S,
        noiseX.octaves = 4;
        noiseX.seed = RandomUI32(0, 123123U);
        init = 1;
    }

    ChunkIterI16(x, y, z)
    {
        ui16 block = chunk->blocks[x][y][z];
        Assert(block < 5);
        IVec3 pos = ivec3(x*CHUNK_UNIT, y*CHUNK_UNIT, z*CHUNK_UNIT);
        if(block)
        {
            ui8 neighbourInfo = chunk->neighbourInfo[x][y][z];
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
InitWorld(MemoryArena *arena, World *world)
{
    world->arena = arena;
    world->xChunks = 6;
    world->yChunks = 6;
    world->chunkArray = PushAndZeroArray(arena, Chunk, world->xChunks*world->yChunks);
    world->chunks = PushAndZeroArray(arena, Chunk*, world->xChunks*world->yChunks);
    world->nChunks = world->xChunks*world->yChunks;
    world->activeChunkX = 0;
    world->activeChunkY = 0;

    // Init generator
    world->generator.firstSmoothness = 2;
    world->generator.roughness2 = 3;
    world->generator.roughness3 = 9;
    world->generator.hillRoughness = 1;
    world->generator.hillHeight = 30;
    world->generator.waterLevel = 75;
    world->generator.groundLevel = 80;
    world->generator.noise2d = fnlCreateState();
    world->generator.hillNoise = fnlCreateState();

    world->generator.hillNoise.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    world->generator.hillNoise.seed = RandomUI32(0, 123123U);
    
    world->generator.noise2d.noise_type = FNL_NOISE_OPENSIMPLEX2S;
    world->generator.noise2d.octaves = 4;
    world->generator.noise2d.gain = 4;
    world->generator.noise2d.seed = RandomUI32(0, 123123U);
    world->generator.noise2d.lacunarity = 4.0;

    world->generator.noise3d = fnlCreateState();
    world->generator.noise3d.seed = RandomUI32(0, 123123U);
    world->generator.noise3d.noise_type=FNL_NOISE_OPENSIMPLEX2S;

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

    // Allocate meshes
    for(ui32 y = 0; y < world->yChunks; y++)
    for(ui32 x = 0; x < world->xChunks; x++)
    {
        Chunk *chunk = world->chunkArray+x+y*world->xChunks;
        chunk->mesh = PushStruct(arena, ChunkMesh);
        AllocateChunkMesh(arena, chunk->mesh);
    }
}

