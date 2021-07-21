
typedef struct
{
    Vec2 pos;
    int xEdge;
    int yEdge;
    r32 offset;     // Counterclockwise direction on edge.
} BoxEdgeLocation;

typedef struct
{
    union
    {
        struct
        {
            r32 x;
            r32 y;
            r32 width;
            r32 height;
        };
        struct
        {
            Vec2 pos;
            Vec2 dims;
        };
    };
} Rect2;

typedef struct
{
    Vec2 pos;
    Vec2 dims;
    r32 angle;
} OrientedBox;



