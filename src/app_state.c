KeyAction
MapKeyCodeToAction(SDL_Keycode code)
{
    switch(code)
    {
        case SDLK_w: { return ACTION_UP; } break;
        case SDLK_a: { return ACTION_LEFT; } break;
        case SDLK_s: { return ACTION_DOWN; } break;
        case SDLK_d: { return ACTION_RIGHT; } break;
        case SDLK_z: { return ACTION_Z; } break;
        case SDLK_x: { return ACTION_X; } break;
        case SDLK_e: { return ACTION_E; } break;
        case SDLK_q: { return ACTION_Q; } break;
        case SDLK_r: { return ACTION_R; } break;
        case SDLK_p: { return ACTION_P; } break;
        case SDLK_UP: { return ACTION_UP; } break;
        case SDLK_DOWN: { return ACTION_DOWN; } break;
        case SDLK_LEFT: { return ACTION_LEFT; } break;
        case SDLK_RIGHT: { return ACTION_RIGHT; } break;
        case SDLK_ESCAPE: { return ACTION_ESCAPE; } break;
        case SDLK_BACKSPACE: { return ACTION_DELETE; } break;
        case SDLK_DELETE: { return ACTION_DELETE; } break;
        default: { return ACTION_UNKNOWN; } break;
    }
}

void
RegisterKeyAction(AppState *appState, KeyAction action, b32 down)
{
    appState->isActionDown[action] = down;
}

void
ResetKeyActions(AppState *appState)
{
    appState->dx = appState->dy = 0;
    appState->mouseScrollY = 0;
    memcpy(appState->wasActionDown, appState->isActionDown, sizeof(appState->isActionDown));
}

b32
IsKeyActionDown(AppState *appState, KeyAction action)
{
    return appState->isActionDown[action];
}

b32
IsKeyActionJustDown(AppState *appState, KeyAction action)
{
    return appState->isActionDown[action] && !appState->wasActionDown[action];
}

b32
IsKeyActionJustReleased(AppState *appState, KeyAction action)
{
    return !appState->isActionDown[action] && appState->wasActionDown[action];
}

