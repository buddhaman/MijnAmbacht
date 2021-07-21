
internal inline void
UpdateChunkFaceNeighbourInfo(World *world, Chunk *chunk, int xDir, int yDir)
{
    ChunkMesh *mesh = GetChunkMesh(world, chunk->x, chunk->y);
    ChunkMesh *neighbourMesh = GetChunkMesh(world, chunk->x+xDir, chunk->y+yDir);
    ui32 xIter = xDir==0;
    ui32 yIter = yDir==0;
    ui32 dims = xIter ? CHUNK_XDIMS : CHUNK_YDIMS;
    ui32 meshFace = xDir!=0 ? (xDir==1 ? CHUNK_XDIMS-1 : 0) : (yDir==1 ? CHUNK_YDIMS-1 : 0);
    ui32 neighbourFace = xDir!=0 ? (xDir==1 ? 0 : CHUNK_XDIMS-1) : (yDir==1 ? 0 : CHUNK_YDIMS-1);

    NeighbourFlag setFlag = xDir!=0 ? (xDir==1 ? OCCUPIED_XMIN : OCCUPIED_XMAX) : 
        (yDir==1 ? OCCUPIED_YMIN : OCCUPIED_YMAX);
    NeighbourFlag setFlagOpposite = xDir!=0 ? (xDir==1 ? OCCUPIED_XMAX : OCCUPIED_XMIN) : 
        (yDir==1 ? OCCUPIED_YMAX : OCCUPIED_YMIN);
    NeighbourFlag setFlagLateral = xIter ? OCCUPIED_XMIN : OCCUPIED_YMIN;
    NeighbourFlag setFlagLateralOpposite = xIter ? OCCUPIED_XMAX : OCCUPIED_YMAX;

    for(ui32 i = 0; i < dims; i++)
    {
        ui32 chunkX = xIter*i + (1-xIter)*meshFace;
        ui32 chunkY = yIter*i + (1-yIter)*meshFace;
        ui32 neighbourX = xIter*i + (1-xIter)*neighbourFace;
        ui32 neighbourY = yIter*i + (1-yIter)*neighbourFace;
        for(ui32 z = 0; z < CHUNK_ZDIMS; z++)
        {
            ui32 block = chunk->blocks[chunkX][chunkY][z];
            if(block)
            {
                neighbourMesh->neighbourInfo[neighbourX][neighbourY][z] |= setFlag;
                mesh->neighbourInfo[chunkX-xDir][chunkY-yDir][z] |= setFlagOpposite;
                if(z < CHUNK_ZDIMS-1) mesh->neighbourInfo[chunkX][chunkY][z+1] |= OCCUPIED_ZMIN;
                if(z > 0) mesh->neighbourInfo[chunkX][chunkY][z-1] |= OCCUPIED_ZMAX;
                if(i < dims-1) mesh->neighbourInfo[chunkX+xIter][chunkY+yIter][z] |= setFlagLateral;
                if(i > 0) mesh->neighbourInfo[chunkX-xIter][chunkY-yIter][z] |= setFlagLateralOpposite;
            }
        }
    }
}

internal inline void
UpdateChunkNeighbourInfo(World *world, ChunkMesh *mesh, Chunk *chunk)
{
    // Some are set twice. This is no problem. 
    if(chunk->x > 0)
    {
        UpdateChunkFaceNeighbourInfo(world, chunk, -1, 0);
    }
    if(chunk->x < world->xChunks-1)
    {
        UpdateChunkFaceNeighbourInfo(world, chunk, 1, 0);
    }
    if(chunk->y > 0)
    {
        UpdateChunkFaceNeighbourInfo(world, chunk, 0, -1);
    }
    if(chunk->y < world->yChunks-1)
    {
        UpdateChunkFaceNeighbourInfo(world, chunk, 0, 1);
    }

    for(ui32 x = 1; x < CHUNK_XDIMS-1; x++)
    for(ui32 y = 1; y < CHUNK_YDIMS-1; y++)
    for(ui32 z = 1; z < CHUNK_ZDIMS-1; z++)
    {
        ui16 block = chunk->blocks[x][y][z];
        if(block)
        {
            mesh->neighbourInfo[x][y][z-1] |= OCCUPIED_ZMAX;
            mesh->neighbourInfo[x][y][z+1] |= OCCUPIED_ZMIN;

            mesh->neighbourInfo[x-1][y][z] |= OCCUPIED_XMAX;
            mesh->neighbourInfo[x+1][y][z] |= OCCUPIED_XMIN;

            mesh->neighbourInfo[x][y-1][z] |= OCCUPIED_YMAX;
            mesh->neighbourInfo[x][y+1][z] |= OCCUPIED_YMIN;
        }
    }
}

void
UpdateChunkMesh(World *world, ChunkMesh *mesh, Chunk *chunk)
{
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
                PushQuadToChunkMesh(mesh,
                        p001,
                        p101,
                        p111,
                        p011,
                        0, 0, CHUNK_UNIT,
                        texUTop, texVTop);
            }
        }
    }
}

void
GenerateWorld(World *world)
{
    r32 hills = 32.0;
    r32 roughness = 0.05;
    ui32 waterLevel = 216;
    ui32 groundLevel = 200;
    for(ui32 yChunk = 0; yChunk < world->yChunks; yChunk++)
    for(ui32 xChunk = 0; xChunk < world->xChunks; xChunk++)
    {
        Chunk *chunk = GetChunk(world, xChunk, yChunk);
        chunk->x = xChunk;
        chunk->y = yChunk;
        chunk->z = 0;
        for(ui16 bx = 0; bx < CHUNK_XDIMS; bx++)
        for(ui16 by = 0; by < CHUNK_YDIMS; by++)
        {
            int x = xChunk*CHUNK_XDIMS+bx;
            int y = yChunk*CHUNK_YDIMS+by;
            ui32 height = groundLevel+(ui32)(perlin2d(x, y, roughness, 4)*hills);
            for(ui16 bz = 0; bz < height; bz++)
            {
                if(bz > waterLevel-2 &&
                        height < waterLevel+2)
                {
                    chunk->blocks[bx][by][bz] = BLOCK_SAND;
                }
                else
                {
                    chunk->blocks[bx][by][bz] = BLOCK_DIRT;
                }
            }
            for(ui16 bz = height; bz < waterLevel; bz++)
            {
                chunk->blocks[bx][by][bz] = BLOCK_WATER;
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

    world->textureTableU[BLOCK_DIRT] = texSize;
    world->textureTableV[BLOCK_DIRT] = 0;
    world->textureTableUTop[BLOCK_DIRT] = 0;
    world->textureTableVTop[BLOCK_DIRT] = 0;

    world->textureTableU[BLOCK_SAND] = texSize*2;
    world->textureTableV[BLOCK_SAND] = 0;
    world->textureTableUTop[BLOCK_SAND] = texSize*2;
    world->textureTableVTop[BLOCK_SAND] = 0;

    world->textureTableU[BLOCK_WATER] = texSize*3;
    world->textureTableV[BLOCK_WATER] = 0;
    world->textureTableUTop[BLOCK_WATER] = texSize*3;
    world->textureTableVTop[BLOCK_WATER] = 0;

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
        UpdateChunkNeighbourInfo(world, mesh, chunk);
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

