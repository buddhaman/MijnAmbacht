typedef struct AppState AppState;

typedef enum 
{
    ACTION_UNKNOWN,
    ACTION_MOUSE_BUTTON_LEFT,
    ACTION_MOUSE_BUTTON_RIGHT,
    ACTION_RIGHT,
    ACTION_UP,
    ACTION_LEFT,
    ACTION_DOWN,
    ACTION_Z,
    ACTION_X,
    ACTION_Q,
    ACTION_E,
    ACTION_R,
    ACTION_P, 
    ACTION_ESCAPE,
    ACTION_DELETE,
    NUM_KEY_ACTIONS
} KeyAction;

struct AppState
{
    MemoryArena *gameArena;
    i32 screenWidth;
    i32 screenHeight;
    r32 ratio;
    i32 mx;
    i32 my;
    i32 dx;
    i32 dy;
    i32 mouseScrollY;
    r32 normalizedMX;
    r32 normalizedMY;
    Vec4 clearColor;

    b32 isActionDown[NUM_KEY_ACTIONS];
    b32 wasActionDown[NUM_KEY_ACTIONS];
};

