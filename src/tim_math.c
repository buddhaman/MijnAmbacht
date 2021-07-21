
// TODO: Better rand.
r32
RandomR32(r32 min, r32 max)
{
    return min + (max - min)*(rand()%RAND_MAX)/RAND_MAX;
}

int
RandomUI32(ui32 min, ui32 max)
{
    return min + (rand()%(max-min));
}

r32
Sign(r32 x)
{
    return x < 0 ? -1.0 : 1.0;
}

Vec2
RandomNormalPair()
{
    r32 x, y, s;
    do
    {
        x = RandomR32(-1, 1);
        y = RandomR32(-1, 1);
        s = x*x + y*y;
    } while(s > 1.0);
    r32 factor = sqrtf(-2*logf(s)/s);
    return vec2(factor*x, factor*y);
}

internal inline Vec2
v2_polar(r32 angle, r32 length)
{
    return vec2(cosf(angle)*length, sinf(angle)*length);
}

Vec3 
GetZIntersection(Vec3 rayPos, Vec3 rayDir, r32 z)
{
    r32 zDiff = z-rayPos.z;
    r32 s = zDiff/rayDir.z;
    return v3_add(rayPos, v3_muls(rayDir, s));
}

internal inline b32
BoxPoint2Intersect(Vec2 pos, Vec2 dims, Vec2 point)
{
    r32 xDiff = point.x-pos.x-dims.x/2;
    r32 yDiff = point.y-pos.y-dims.y/2;
    return (fabsf(xDiff) < dims.x/2 && fabsf(yDiff) < dims.y/2);
}

internal inline b32
OrientedBoxPoint2Intersect(Vec2 pos, Vec2 dims, r32 angle, Vec2 point)
{
    // Project onto axes see if dot < dims
    r32 c = cosf(angle);
    r32 s = sinf(angle);
    r32 xDiff = point.x-pos.x;
    r32 yDiff = point.y-pos.y;
    r32 xProj = c*xDiff + s*yDiff;
    r32 yProj = -s*xDiff + c*yDiff;
    return (fabsf(xProj) < dims.x/2 && fabsf(yProj) < dims.y/2);
}

internal inline Vec2
GetCoordinateInBox(Vec2 pos, Vec2 dims, r32 angle, Vec2 point)
{
    r32 c = cosf(angle);
    r32 s = sinf(angle);
    r32 xDiff = point.x-pos.x;
    r32 yDiff = point.y-pos.y;
    r32 xProj = c*xDiff + s*yDiff;
    r32 yProj = -s*xDiff + c*yDiff;
    return vec2(xProj/dims.x+0.5, yProj/dims.y+0.5);
}

internal inline b32
CirclePointIntersect(Vec2 pos, r32 radius, Vec2 point)
{
    Vec2 diff = v2_sub(point, pos);
    return v2_length2(diff) <= radius*radius;
}

// Ignores points inside the rectangle.
internal inline r32 
GetNearestBoxEdgeLocation(Vec2 pos, Vec2 dims, r32 angle, Vec2 point, BoxEdgeLocation *location)
{
    r32 c = cosf(angle);
    r32 s = sinf(angle);
    r32 xDiff = point.x-pos.x;
    r32 yDiff = point.y-pos.y;
    Vec2 p = vec2(c*xDiff + s*yDiff, -s*xDiff + c*yDiff);   // point coordinates seen from box transform.

    r32 result = -1.0;
    *location = (BoxEdgeLocation){};

    b32 inYRange = fabsf(p.x) < dims.x/2.0;
    b32 inXRange = fabsf(p.y) < dims.y/2.0;
    if(inYRange)
    {
        if(!inXRange)
        {
            int sign = Sign(p.y);
            r32 yPos = sign*dims.y/2.0;
            location->pos = vec2(p.x, yPos);
            location->yEdge = sign;
            location->offset = 1.0-sign*(p.x+sign*dims.x/2)/dims.x;
            result = fabsf(p.y-yPos);
        }
        else
        {
            // Inside rectangle
        }
    }
    else if(inXRange)
    {
        int sign = Sign(p.x);
        r32 xPos = sign*dims.x/2.0;
        location->pos = vec2(xPos, p.y);
        location->xEdge = sign;
        location->offset = sign*(p.y+sign*dims.y/2)/dims.y;
        result = fabsf(p.x-xPos);
    }
    else
    {
        int xSign = Sign(p.x);
        int ySign = Sign(p.y);
        Vec2 corner = vec2(xSign*dims.x/2.0, ySign*dims.y/2.0);
        location->pos = corner;
        location->xEdge = xSign==ySign ? xSign : 0;
        location->yEdge = xSign==ySign ? 0 : ySign;
        location->offset = 1.0;
        result = v2_dist(corner, p);
    }

    // Transform result back to rotated rectangle.
    location->pos = vec2(
            pos.x + c*location->pos.x - s*location->pos.y,
            pos.y + s*location->pos.x + c*location->pos.y
            );

    return result;
}

internal inline Vec2
GetBoxEdgePosition(Vec2 pos, Vec2 dims, r32 angle, int xEdge, int yEdge, r32 offset)
{
    Vec2 result;
    if(xEdge!=0)
    {
        int ySign = xEdge;
        result.x = xEdge*dims.x/2;
        result.y = ySign*offset*dims.y-ySign*dims.y/2;
    }
    else
    {
        int xSign = -yEdge;
        result.x = xSign*offset*dims.x-xSign*dims.x/2;
        result.y = yEdge*dims.y/2;
    }
    // Transform to rotated rectangle.
    result = v2_add(pos, v2_rotate(result, angle));
    return result;
}

internal inline Vec4
RGBAToVec4(ui32 hex)
{
    return vec4( (hex >> 24 & 255)/255.0f,
            (hex >> 16 & 255)/255.0f,
            (hex >> 8 & 255)/255.0f,
            (hex & 255)/255.0f);
}

internal inline ui32
Vec4ToRGBA(Vec4 color)
{
    int r = (color.x*255);
    int g = (color.y*255);
    int b = (color.z*255);
    int a = (color.w*255);
    return (a << 24) + (b << 16) + (g << 8) + r;
}

internal inline r32
RadToDeg(r32 rad)
{
    return (180 * rad / M_PI);
}

internal inline r32
DegToRad(r32 rad)
{
    return rad*(M_PI/180.0);
}

internal inline r32
NormalizeAngle(r32 rad)
{
    rad = fmodf(rad, M_PI*2);
    rad = fmodf(rad+M_PI*2, M_PI*2);
    if(rad > M_PI)
    {
        rad-=M_PI*2;
    }
    return rad;
}

internal inline r32
GetNormalizedAngDiff(r32 a, r32 b)
{
    r32 diff = a-b;
    return NormalizeAngle(diff);
}

internal inline r32
ClampF(r32 min, r32 max, r32 x)
{
    return x < min ? min : (x > max ? max : x);
}

// TODO: maybe move somewhere else

internal inline size_t
IndexOfElement(void *array, size_t elementSize, void *element)
{
    size_t index = ((size_t)element - (size_t)array)/elementSize;
    Assert(index >= 0);
    return index;
}

internal inline void
ArrayRemoveElement(void *array, size_t elementSize, size_t nElements, void *element)
{
    size_t index = IndexOfElement(array, elementSize, element);
    size_t nMoveElements = nElements-index;
    if(nMoveElements > 0)
    {
        memmove(element, (ui8*)element+elementSize, nMoveElements*elementSize);
    }
}

