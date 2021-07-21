
internal inline IVec3
ivec3(ui16 x, ui16 y, ui16 z)
{
    return (IVec3){x, y, z};
}

internal inline IVec3
iv3_add(IVec3 a, IVec3 b)
{
    return ivec3(a.x+b.x, a.y+b.y, a.z+b.z);
}

internal inline void
PushIndexToChunkMesh(ChunkMesh *mesh, ui32 index)
{
    Assert(mesh->nIndices < mesh->maxIndices);
    mesh->indexBuffer[mesh->nIndices++] = index;
}

internal inline void
PushVertexToChunkMesh(ChunkMesh *mesh, IVec3 p, i16 xn, i16 yn, i16 zn, 
        ui16 u, ui16 v)
{
    Assert(mesh->nVertices < mesh->maxVertices);
    ui32 idx = mesh->nVertices*mesh->stride;
    mesh->vertexBuffer[idx] = p.x;
    mesh->vertexBuffer[idx+1] = p.y;
    mesh->vertexBuffer[idx+2] = p.z;
    mesh->vertexBuffer[idx+3] = xn+CHUNK_UNIT;
    mesh->vertexBuffer[idx+4] = yn+CHUNK_UNIT;
    mesh->vertexBuffer[idx+5] = zn+CHUNK_UNIT;
    mesh->vertexBuffer[idx+6] = (v<<8)+u;
    mesh->nVertices++;
}

#if 0
internal inline void
PushTriangleToChunkMesh(ChunkMesh *mesh, IVec3 p0, IVec3 p1, IVec3 p2)
{
    ui16 fromIdx = mesh->nVertices;
    PushVertexToChunkMesh(mesh, p0);
    PushVertexToChunkMesh(mesh, p1);
    PushVertexToChunkMesh(mesh, p2);
    PushIndexToChunkMesh(mesh, fromIdx);
    PushIndexToChunkMesh(mesh, fromIdx+1);
    PushIndexToChunkMesh(mesh, fromIdx+2);
}
#endif

internal inline void
PushQuadToChunkMesh(ChunkMesh *mesh, IVec3 p0, IVec3 p1, IVec3 p2, IVec3 p3, 
        i16 xn, i16 yn, i16 zn,
        ui16 u, ui16 v)
{
    ui32 fromIdx = mesh->nVertices;

    ui16 texSize = CHUNK_TEX_SQUARE_SIZE-1;
    PushVertexToChunkMesh(mesh, p0, xn, yn, zn, u, v);
    PushVertexToChunkMesh(mesh, p1, xn, yn, zn, u+texSize, v);
    PushVertexToChunkMesh(mesh, p2, xn, yn, zn, u+texSize, v+texSize);
    PushVertexToChunkMesh(mesh, p3, xn, yn, zn, u, v+texSize);

    PushIndexToChunkMesh(mesh, fromIdx);
    PushIndexToChunkMesh(mesh, fromIdx+1);
    PushIndexToChunkMesh(mesh, fromIdx+2);
    PushIndexToChunkMesh(mesh, fromIdx+2);
    PushIndexToChunkMesh(mesh, fromIdx+3);
    PushIndexToChunkMesh(mesh, fromIdx+0);
}

#if 0
internal inline void
PushCubeToChunkMesh(ChunkMesh *mesh, IVec3 pos, IVec3 dims)
{
    ui16 x = dims.x;
    ui16 y = dims.y;
    ui16 z = dims.z;
    IVec3 p000 = pos;
    IVec3 p001 = iv3_add(pos, ivec3(0, 0, z));
    IVec3 p010 = iv3_add(pos, ivec3(0, y, 0));
    IVec3 p011 = iv3_add(pos, ivec3(0, y, z));
    IVec3 p100 = iv3_add(pos, ivec3(x, 0, 0));
    IVec3 p101 = iv3_add(pos, ivec3(x, 0, z));
    IVec3 p110 = iv3_add(pos, ivec3(x, y, 0));
    IVec3 p111 = iv3_add(pos, ivec3(x, y, z));
    PushQuadToChunkMesh(mesh,
            p010,
            p110,
            p100,
            p000,
            0, 0, -CHUNK_UNIT);
    PushQuadToChunkMesh(mesh,
            p001,
            p011,
            p010,
            p000,
            -CHUNK_UNIT, 0, 0);
    PushQuadToChunkMesh(mesh,
            p001,
            p101,
            p111,
            p011,
            0, 0, CHUNK_UNIT);
    PushQuadToChunkMesh(mesh,
            p100,
            p110,
            p111,
            p101,
            CHUNK_UNIT, 0, 0);
    PushQuadToChunkMesh(mesh,
            p000,
            p100,
            p101,
            p001,
            0, -CHUNK_UNIT, 0);
    PushQuadToChunkMesh(mesh,
            p011,
            p111,
            p110,
            p010,
            0, CHUNK_UNIT, 0);
}
#endif

void
BufferChunkMesh(ChunkMesh *mesh)
{
    GLenum usage = GL_STATIC_DRAW;

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, 
            mesh->nVertices*mesh->stride*sizeof(ui16),
            mesh->vertexBuffer, 
            usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
            mesh->nIndices*sizeof(ui32),
            mesh->indexBuffer,
            usage);
    // Print some stats
    DebugOut("%p VertexBuffer: %u / %u", mesh, mesh->nIndices, mesh->maxIndices);
}

void
RenderChunkMesh(ChunkMesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->nIndices, GL_UNSIGNED_INT, 0);
}

void
AllocateChunkMesh(MemoryArena *arena, ChunkMesh *mesh)
{
    *mesh = (ChunkMesh){};

    mesh->stride = 7;
    mesh->maxVertices = 32U*256U*24U;
    mesh->maxIndices = 32U*256U*36U;
    mesh->vertexBuffer = PushArray(arena, ui16, ((size_t)mesh->maxVertices)*((size_t)mesh->stride));
    mesh->indexBuffer = PushArray(arena, ui32, mesh->maxIndices);

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    size_t memOffset = 0;

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_UNSIGNED_SHORT, GL_TRUE, 
            mesh->stride*sizeof(ui16), (void*)memOffset);
    memOffset+=3*sizeof(ui16);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_SHORT, GL_TRUE, 
            mesh->stride*sizeof(ui16), (void*)memOffset);
    memOffset+=3*sizeof(ui16);

    // Texture index
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 2, GL_UNSIGNED_BYTE,
            mesh->stride*sizeof(ui16), (void*)memOffset);
    memOffset+=1*sizeof(ui16);
}

