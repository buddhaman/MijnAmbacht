
internal inline void
UpdateChunkNeighbourInfo(World *world, ChunkMesh *mesh, Chunk *chunk)
{
    if(chunk->x > 0)
    {
        ChunkMesh *neighbourMesh = GetChunkMesh(world, chunk->x-1, chunk->y);
        for(ui32 y = 0; y < CHUNK_YDIMS; y++)
        for(ui32 z = 0; z < CHUNK_ZDIMS; z++)
        {
            ui16 block = chunk->blocks[0][y][z];
            if(block)
            {
                neighbourMesh->neighbourInfo[CHUNK_XDIMS-1][y][z] |= OCCUPIED_XMAX;
            }
        }
    }
    if(chunk->x < world->xChunks-1)
    {
        ChunkMesh *neighbourMesh = GetChunkMesh(world, chunk->x+1, chunk->y);
        for(ui32 y = 0; y < CHUNK_YDIMS; y++)
        for(ui32 z = 0; z < CHUNK_ZDIMS; z++)
        {
            ui16 block = chunk->blocks[CHUNK_XDIMS-1][y][z];
            if(block)
            {
                neighbourMesh->neighbourInfo[0][y][z] |= OCCUPIED_XMIN;
            }
        }
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

            // X
            if(!(neighbourInfo & OCCUPIED_XMIN))
            {
                PushQuadToChunkMesh(mesh,
                        p001,
                        p011,
                        p010,
                        p000,
                        -CHUNK_UNIT, 0, 0);
            }
            if(!(neighbourInfo & OCCUPIED_XMAX))
            {
                PushQuadToChunkMesh(mesh,
                        p100,
                        p110,
                        p111,
                        p101,
                        CHUNK_UNIT, 0, 0);
            }

            // Y
            if(!(neighbourInfo & OCCUPIED_YMIN))
            {
                PushQuadToChunkMesh(mesh,
                        p000,
                        p100,
                        p101,
                        p001,
                        0, -CHUNK_UNIT, 0);
            }
            if(!(neighbourInfo & OCCUPIED_YMAX))
            {
                PushQuadToChunkMesh(mesh,
                        p011,
                        p111,
                        p110,
                        p010,
                        0, CHUNK_UNIT, 0);
            }

            // Z
            if(!(neighbourInfo & OCCUPIED_ZMIN))
            {
                PushQuadToChunkMesh(mesh,
                        p010,
                        p110,
                        p100,
                        p000,
                        0, 0, -CHUNK_UNIT);
            }
            if(!(neighbourInfo & OCCUPIED_ZMAX))
            {
                PushQuadToChunkMesh(mesh,
                        p001,
                        p101,
                        p111,
                        p011,
                        0, 0, CHUNK_UNIT);
            }
        }
    }
}

void
InitWorld(MemoryArena *arena, World *world)
{
    world->arena = arena;
    world->xChunks = 4;
    world->yChunks = 4;
    world->chunks = PushAndZeroArray(arena, Chunk, world->xChunks*world->yChunks);
    world->chunkMeshes = PushAndZeroArray(arena, ChunkMesh, world->xChunks*world->yChunks);

    // ORDER IS IMPORTANT!!!
    //
    // Generate world
    for(ui32 y = 0; y < world->yChunks; y++)
    for(ui32 x = 0; x < world->xChunks; x++)
    {
        Chunk *chunk = GetChunk(world, x, y);
        chunk->x = x;
        chunk->y = y;
        chunk->z = 0;
        for(ui16 bx = 0; bx < CHUNK_XDIMS; bx++)
        for(ui16 by = 0; by < CHUNK_YDIMS; by++)
        {
            ui32 height = RandomUI32(30, 40);
            for(ui16 bz = 0; bz < height; bz++)
                chunk->blocks[bx][by][bz] = 1;
        }
    }
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

