
typedef struct
{
    ui32 program;

    char *fragmentSourcePath;
    char *fragmentSource;

    char *vertexSourcePath;
    char *vertexSource;
} Shader;

typedef struct
{
    Shader *shader;

    ui32 transformMatrixLocation;
    Mat4 *transformMatrix;

    ui32 modelMatrixLocation;
    Mat4 *modelMatrix;

} ShaderInstance;

