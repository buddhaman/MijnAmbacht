

internal inline void
UpdateCameraInput(Camera *camera, AppState *appState)
{
    r32 factor = 0.01;
    r32 speed = 0.24;
    Vec3 dir = camera->direction;
    Vec3 perp = v3_norm(v3_cross(dir, vec3(0,0,1)));
    camera->theta-=appState->dx*factor;
    camera->phi-=appState->dy*factor;
    if(camera->phi > M_PI/2.0-0.01) { camera->phi = M_PI/2.0-0.01; }
    if(camera->phi < -M_PI/2.0+0.01) { camera->phi = -M_PI/2.0+0.01; }
    if(IsKeyActionDown(appState, ACTION_UP))
    {
        camera->pos = v3_add(camera->pos, v3_muls(dir, speed));
    }
    if(IsKeyActionDown(appState, ACTION_DOWN))
    {
        camera->pos = v3_sub(camera->pos, v3_muls(dir, speed));
    }
    if(IsKeyActionDown(appState, ACTION_LEFT))
    {
        camera->pos = v3_sub(camera->pos, v3_muls(perp, speed));
    }
    if(IsKeyActionDown(appState, ACTION_RIGHT))
    {
        camera->pos = v3_add(camera->pos, v3_muls(perp, speed));
    }
    if(IsKeyActionDown(appState, ACTION_Q))
    {
        camera->pos = v3_add(camera->pos, vec3(0,0,-speed));
    }
    if(IsKeyActionDown(appState, ACTION_E))
    {
        camera->pos = v3_add(camera->pos, vec3(0,0,speed));
    }
}

internal inline void
UpdateCamera(Camera *camera, r32 ratio)
{
    camera->direction = vec3(
            cosf(camera->theta)*cosf(camera->phi),
            sinf(camera->theta)*cosf(camera->phi),
            sinf(camera->phi)
            );
    Mat4 lookAtMatrix = m4_look_at(camera->pos, 
            v3_add(camera->pos, camera->direction), 
            vec3(0,0,1));
    Mat4 perspectiveMatrix = m4_perspective(60.0, ratio, 0.1, 300.0);
    camera->transform = m4_mul(perspectiveMatrix, lookAtMatrix);
}

internal inline void
InitCamera(Camera *camera)
{
    *camera = (Camera){};
}
