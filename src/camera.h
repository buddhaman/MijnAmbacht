typedef struct Camera Camera;

struct Camera
{
    Vec3 pos;
    Vec3 direction; 
    r32 theta;
    r32 phi;
    Mat4 transform;
};

