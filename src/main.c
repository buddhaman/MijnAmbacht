#include "external_headers.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Special file.
#include "tim_types.h"
#include "math_2d.h"

// My own math typedefs.
typedef vec2_t Vec2;
typedef vec3_t Vec3;
typedef vec4_t Vec4;
typedef mat4_t Mat4;
typedef mat3_t Mat3;

#define MAX_VERTEX_MEMORY 512*1024
#define MAX_ELEMENT_MEMORY 128*1024

#define CheckOpenglError() { GLenum err = glGetError(); \
    if(err) { DebugOut("err = %04x", err);Assert(0); }}

char *
ReadEntireFile(const char *path)
{
    char *buffer = NULL;
    size_t stringSize, readSize;
    (void)readSize;
    FILE *handler = fopen(path, "r");
    if(handler)
    {
        fseek(handler, 0, SEEK_END);
        stringSize = ftell(handler);
        rewind(handler);
        buffer = (char *)malloc(sizeof(char) * (stringSize + 1));
        readSize = fread(buffer, sizeof(char), stringSize, handler);
        buffer[stringSize] = 0;
        fclose(handler);
        return buffer;
    }
    else
    {
        DebugOut("Can't read file %s", path);
        return NULL;
    }
}

ui32
TimLoadImage(char *path)
{
    int x, y, n;
    ui32 tex;
    ui8 *data = stbi_load(path, &x, &y, &n, 4);
    if(!data)
    {
        DebugOut("Cannot load image at %s.", path);
        return 0;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}

// Own files
#include "cool_memory.h"
#include "tim_math.h"
#include "shader.h"
#include "app_state.h"
#include "chunk.h"
#include "camera.h"

#include "cool_memory.c"
#include "tim_math.c"
#include "shader.c"
#include "app_state.c"
#include "chunk.c"
#include "camera.c"

#define FRAMES_PER_SECOND 60

int 
main(int argc, char**argv)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)!=0)
    {
        DebugOut("SDL does not work\n");
    }

    srand(time(0));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_WindowFlags window_flags = 
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    int screen_width = 1280;
    int screen_height = 720;

    SDL_Window *window = SDL_CreateWindow("Cool", SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, screen_width, screen_height, window_flags);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);

    SDL_GL_SetSwapInterval(1);

    b32 err = gl3wInit()!=0;
    if(err)
    {
        DebugOut("Failed to initialize OpenGl loader!\n");
    }
    
    // Setup nuklear
    struct nk_context *ctx = NULL;
    DebugOut("Before init");
    ctx = nk_sdl_init(window);
    DebugOut("After init");

    // Nuklear font
    struct nk_font_atlas *nkFontAtlas;
    nk_sdl_font_stash_begin(&nkFontAtlas);
    struct nk_font *font = nk_font_atlas_add_from_file(nkFontAtlas, "assets/DejaVuSansMono.ttf", 16.0, 0);
    nk_sdl_font_stash_end();
    nk_style_set_font(ctx, &font->handle);

    // MemoryArena
    MemoryArena *gameArena = CreateMemoryArena(128*1000*1000);
    // Creating appstate
    AppState *appState = (AppState *)malloc(sizeof(AppState));
    *appState = (AppState){};
    appState->gameArena = gameArena;

    appState->clearColor = RGBAToVec4(0x36c7f2ff);

    r32 time = 0.0;
    r32 deltaTime = 0.0;
    r32 timeSinceLastFrame = 0.0;
    r32 updateTime = 1.0/FRAMES_PER_SECOND;
    ui32 frameStart = SDL_GetTicks(); 
    // Timing
    b32 done = 0;
    ui32 frameCounter = 0;

    // Setup shader
    Shader *shader = PushStruct(gameArena, Shader);
    InitShader(gameArena, shader, "assets/shaders/mijnshader.vert", "assets/shaders/mijnshader.frag");
    LoadShader(shader);
    ShaderInstance *shaderInstance = PushStruct(gameArena, ShaderInstance);
    InitShaderInstance(shaderInstance, shader);

    Camera *camera = PushStruct(gameArena, Camera);
    InitCamera(camera);
    camera->pos = vec3(1, 1, 3);

    ChunkMesh *chunkMesh = PushStruct(gameArena, ChunkMesh);
    AllocateChunkMesh(gameArena, chunkMesh);

    Chunk *chunk = PushStruct(gameArena, Chunk);
    *chunk = (Chunk){};
    for(ui16 y = 0; y < CHUNK_SIZE; y++)
    for(ui16 x = 0; x < CHUNK_SIZE; x++)
    {
        ui32 depth = RandomUI32(0U, 16U);
        for(ui16 z = 0; z < depth; z++)
        {
            chunk->blocks[x][y][z] = 1;
            PushCubeToChunkMesh(chunkMesh, ivec3(CHUNK_UNIT*x,CHUNK_UNIT*y,CHUNK_UNIT*z), 
                    ivec3(CHUNK_UNIT, CHUNK_UNIT, CHUNK_UNIT));
        }
    }

    BufferChunkMesh(chunkMesh);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    while(!done)
    {
        SDL_Event event;
        nk_input_begin(ctx);
        ResetKeyActions(appState);
        while(SDL_PollEvent(&event))
        {
            nk_sdl_handle_event(&event);
            switch(event.type)
            {

            case SDL_MOUSEBUTTONUP:
            {
                if(event.button.button==SDL_BUTTON_LEFT)
                {
                    RegisterKeyAction(appState, ACTION_MOUSE_BUTTON_LEFT, 0);
                }
                else if(event.button.button==SDL_BUTTON_RIGHT)
                {
                    RegisterKeyAction(appState, ACTION_MOUSE_BUTTON_RIGHT, 0);
                }
            } break;

            case SDL_MOUSEBUTTONDOWN:
            {
                if(event.button.button==SDL_BUTTON_LEFT)
                {
                    RegisterKeyAction(appState, ACTION_MOUSE_BUTTON_LEFT, 1);
                }
                else if(event.button.button==SDL_BUTTON_RIGHT)
                {
                    RegisterKeyAction(appState, ACTION_MOUSE_BUTTON_RIGHT, 1);
                }
            } break;

            case SDL_MOUSEWHEEL:
            {
                appState->mouseScrollY += event.wheel.y;
            } break;

            case SDL_KEYUP:
            {
                RegisterKeyAction(appState, MapKeyCodeToAction(event.key.keysym.sym), 0);
            } break;

            case SDL_KEYDOWN:
            {
                RegisterKeyAction(appState, MapKeyCodeToAction(event.key.keysym.sym), 1);
            } break;

            case SDL_QUIT:
            {
                done = 1;
            } break;

            default:
            {
                if(event.type == SDL_WINDOWEVENT
                        && event.window.event == SDL_WINDOWEVENT_CLOSE
                        && event.window.windowID == SDL_GetWindowID(window))
                {
                    done = 1;
                }
            } break;

            }
        }
        nk_input_end(ctx);
        // Set Appstate

        SDL_GetWindowSize(window, &appState->screenWidth, &appState->screenHeight);
        appState->ratio = (r32)appState->screenWidth / ((r32)appState->screenHeight);
        i32 mx, my;
        SDL_GetMouseState(&mx, &my);
        appState->dx = mx-appState->mx;
        appState->dy = my-appState->my;
        appState->mx = mx;
        appState->my = my;
        appState->normalizedMX = 2*((r32)mx)/appState->screenWidth - 1.0;
        appState->normalizedMY = -2*((r32)my)/appState->screenHeight + 1.0;

        UpdateCameraInput(camera, appState);

        // Clear screen
        Vec4 clearColor = appState->clearColor;
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, appState->screenWidth, appState->screenHeight);

        // Anti aliasing in opengl.
        glEnable(GL_MULTISAMPLE);

        // Render own stuff
        Mat4 modelMatrix = m4_identity();
        UpdateCamera(camera, appState->ratio);
        shaderInstance->transformMatrix = &camera->transform;
        shaderInstance->modelMatrix = &modelMatrix;

        BeginShaderInstance(shaderInstance);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        RenderChunkMesh(chunkMesh);

        // Render UI
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

        // frame timing
        ui32 frameEnd = SDL_GetTicks();
#if 0
        if(frameEnd > (lastSecond+1)*1000)
        {
            // new second
            lastSecond = frameEnd/1000;
            DebugOut("Frames per second = %d\n", frameCounter);
            frameCounter = 0;
        }
#endif
        ui32 frameTicks = frameEnd-frameStart;
        frameStart=frameEnd;
        deltaTime=((r32)frameTicks)/1000.0;
        time+=deltaTime;
        timeSinceLastFrame+=deltaTime;
        if(timeSinceLastFrame < updateTime)
        {
            i32 waitForTicks = (i32)((updateTime-timeSinceLastFrame)*1000);
            if(waitForTicks > 0)
            {
                //DebugOut("this happened, waitForTicks = %d\n", waitForTicks);
                if(waitForTicks < 2*updateTime*1000)
                {
                    SDL_Delay(waitForTicks);
                    timeSinceLastFrame-=waitForTicks/1000.0;
                }
                else
                {
                    timeSinceLastFrame = 0;
                }
            }
        }
        //DebugOut("time = %f, deltaTime = %f, ticks = %d\n", time, deltaTime, frameTicks);
        SDL_GL_SwapWindow(window);
        frameCounter++;
    }
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

