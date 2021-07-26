
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
                chunk->data->blocks[bx][by][bz] = BLOCK_SAND;
            }
            else if(bz > world->generator.waterLevel-15)
            {
                chunk->data->blocks[bx][by][bz] = BLOCK_DIRT;
            }
            else
            {
                chunk->data->blocks[bx][by][bz] = BLOCK_STONE;
            }
        }
        for(ui16 bz = height; bz < world->generator.waterLevel; bz++)
        {
            chunk->data->blocks[bx][by][bz] = BLOCK_WATER;
        }
        for(ui16 bz = 0; bz < height; bz++)
        {
            r32 heightFactor = ((r32)bz)/height;
            r32 noise3Value = fnlGetNoise3D(&world->generator.noise3d, x*roughness3/4, y*roughness3/4, bz*roughness3);
            if((1.0-heightFactor*heightFactor*heightFactor)*noise3Value < -0.4 && chunk->data->blocks[bx][by][bz]!=BLOCK_WATER)
            {
                chunk->data->blocks[bx][by][bz] = 0;
            }
        }
    }

}

internal inline ui32
GetPositionHash(int x, int y)
{
    ui32 xHash = x < 0 ? -5*x : 7*x;
    ui32 yHash = y < 0 ? -11*y : 13*y;
    return xHash+yHash;
}

internal inline Chunk *
GetChunkHashSlot(World *world, int x, int y)
{
    ui32 hash = GetPositionHash(x, y);
    ui32 mask = world->chunkMapSize-1;
    ui32 hashMapStartIdx = hash & world->chunkMapSize;
    Chunk *chunk = NULL;
    for(ui32 counter = 0;
            counter < world->chunkMapSize; 
            counter++)
    {
        ui32 chunkIdx = (counter+hashMapStartIdx) & mask;
        Chunk *tryChunk = world->chunkMap + chunkIdx;
        if(tryChunk->isActive)
        {
            if(tryChunk->xIndex == x && tryChunk->yIndex==y)
            {
                chunk = tryChunk;
                break;
            }
        }
        else
        {
            chunk = tryChunk;
            break;
        }
    }
    Assert(chunk);
    return chunk;
}

internal inline ChunkData *
AllocateChunkData(World *world)
{
    // TODO: Do this with free list
    for(int i = 0; i < world->maxChunkData; i++)
    {
        ChunkData *data = world->chunkData+i;
        if(!data->isUsed)
        {
            data->isUsed = 1;
            return data;
        }
    }
    return NULL;
}

// Generates chunk if it doesnt exist.
internal inline Chunk *
GetChunkSlow(World *world, int x, int y)
{
    Chunk *chunk = GetChunkHashSlot(world, x, y);
    if(!chunk->isActive)
    {
        chunk->xIndex = x;
        chunk->yIndex = y;
        chunk->z = 0;
        chunk->isActive = 1;
    }
    if(!chunk->inWorld)
    {
        chunk->inWorld = 1;
        chunk->data = AllocateChunkData(world);
        Assert(chunk->data);
        GenerateChunk(world, chunk);
        DebugOut("Generated chunk %d %d", chunk->xIndex, chunk->yIndex);
    }
    return chunk;
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
    // TODO: This is still really terrible
    Int16Vec chunkIdx = GetChunkIndicesAtPosition(vec2(x, y));
    Chunk *chunk = GetChunkHashSlot(world, chunkIdx.x, chunkIdx.y);
    if(chunk->inWorld)
    {
        Assert(chunk->xIndex==chunkIdx.x && chunk->yIndex==chunkIdx.y);
        i16 bx = x-CHUNK_XDIMS*chunk->xIndex;
        i16 by = y-CHUNK_YDIMS*chunk->yIndex;
        *blockPosition = (BlockPosition){chunk, bx, by, z};
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
        bPos.chunk->data->neighbourInfo[bPos.x][bPos.y][bPos.z] |= flag;
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
        ui16 block = chunk->data->blocks[x][y][z];
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
    ChunkMesh *mesh = chunk->data->mesh;
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
        ui16 block = chunk->data->blocks[x][y][z];
        Assert(block < 5);
        IVec3 pos = ivec3(x*CHUNK_UNIT, y*CHUNK_UNIT, z*CHUNK_UNIT);
        if(block)
        {
            ui8 neighbourInfo = chunk->data->neighbourInfo[x][y][z];
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

    world->chunkMapSize = 1024;
    world->chunkMap = PushAndZeroArray(arena, Chunk, world->chunkMapSize);
    world->maxChunkData = 32;
    world->chunkData = PushAndZeroArray(arena, ChunkData, world->maxChunkData);

    world->nChunksInWorld = 0;
    world->maxChunksInWorld = 20;
    world->chunksInWorld = PushAndZeroArray(arena, Chunk*, world->maxChunksInWorld);

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
    for(int chunkDataIdx = 0;
            chunkDataIdx < world->maxChunkData;
            chunkDataIdx++)
    {
        ChunkData *chunkData = world->chunkData+chunkDataIdx;
        chunkData->mesh = PushStruct(arena, ChunkMesh);
        AllocateChunkMesh(arena, chunkData->mesh);
    }
}

