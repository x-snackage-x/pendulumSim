#include <math.h>
#include "resource_dir.h"

#include "raylib.h"

#define RAYMATH_IMPLEMENTATIOn
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define VECTOR_DRAW_SCALE 10.0f
int pendulumLength = 200;

typedef struct PendulumMass {
    float mass;
    Vector2 tension;
    Vector2 acc;
    Vector2 anchor;
    Vector2 position;
    Vector2 velocity;
} PendulumMass;

void drawVector(Vector2 anchorPos,
                Vector2 vector,
                Color color,
                float scalingFactor) {
    vector = Vector2Scale(vector, scalingFactor);

    Vector2 sideOneV =
        Vector2Scale(Vector2Rotate(vector, 3.0f * DEG2RAD), 0.9f);
    Vector2 sideTwoV =
        Vector2Scale(Vector2Rotate(vector, -3.0f * DEG2RAD), 0.9f);

    Vector2 endPos = Vector2Add(anchorPos, vector);
    Vector2 endPosOne = Vector2Add(anchorPos, sideOneV);
    Vector2 endPosTwo = Vector2Add(anchorPos, sideTwoV);

    DrawLineEx(anchorPos, endPos, 1.0f, color);
    DrawLineEx(endPos, endPosOne, 1.0f, color);
    DrawLineEx(endPos, endPosTwo, 1.0f, color);
}

void accelerateMass(PendulumMass* mass,
                    float deltaTime,
                    float g,
                    bool numericalCorrection) {
    Vector2 gravityVector = {0.0f, g};
    float time = 0.0f;
    Vector2 tensionVecNorm =
        Vector2Normalize(Vector2Subtract(mass->position, mass->anchor));
    float amplitude = Vector2Angle(gravityVector, tensionVecNorm);
    mass->tension = Vector2Scale(tensionVecNorm, g * cosf(amplitude));

    mass->acc = Vector2Subtract(gravityVector, mass->tension);
    mass->velocity =
        Vector2Add(mass->velocity, Vector2Scale(mass->acc, deltaTime));
    mass->position =
        Vector2Add(mass->position, Vector2Scale(mass->velocity, deltaTime));

    // Numerical Correction
    Vector2 pendulumVec = Vector2Subtract(mass->position, mass->anchor);
    float calcPendulumLen = Vector2Length(pendulumVec);
    float correction = calcPendulumLen - pendulumLength;
    if(numericalCorrection && correction >= 1.0f) {
        Vector2 pendulumVecNorm = Vector2Normalize(pendulumVec);
        Vector2 correctionVec =
            Vector2Negate(Vector2Scale(pendulumVecNorm, correction));
        mass->position = Vector2Add(correctionVec, mass->position);

        pendulumVec = Vector2Subtract(mass->position, mass->anchor);
        calcPendulumLen = Vector2Length(pendulumVec);
        float curDistanceDelta = calcPendulumLen - pendulumLength;

        TraceLog(LOG_INFO, "Correction = %f - Corrected Dist. = %f", correction,
                 curDistanceDelta);
    }
}

int main() {
    bool drawForces = true;
    bool drawVelocities = true;
    bool withNumericalCor = false;
    const int padding = 2;
    const int panelWidth = 200;
    const int playGroundWidth = 1100;
    const int screenWidth = playGroundWidth + panelWidth + padding * 3;
    const int screenHeight = 900;
    Vector2 origin = {0.0f, 0.0f};

    Vector2 mousePosition = {0.0f};

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "PendulumSim");

    float g = 9.81f;
    float amplitude = 1.0f;
    float stringWidth = 2.0f;
    Vector2 pendulumAnchor = {(GetScreenWidth() - panelWidth) * 0.5f,
                              GetScreenHeight() * 0.2f};
    PendulumMass massA = {
        .mass = 25.0f,
        .anchor = pendulumAnchor,
        .position =
            Vector2Add(pendulumAnchor, (struct Vector2){0, pendulumLength})};

    SetTargetFPS(60);
    while(!WindowShouldClose()) {
        mousePosition = GetMousePosition();
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
           mousePosition.x < playGroundWidth) {
            Vector2 translationVec = Vector2Normalize(
                Vector2Subtract(mousePosition, pendulumAnchor));
            translationVec =
                Vector2Scale(translationVec, (float)pendulumLength);
            massA.position = Vector2Add(pendulumAnchor, translationVec);
            massA.velocity = Vector2Zero();
            massA.acc = Vector2Zero();
            massA.tension = Vector2Zero();
        }

        float dt = GetFrameTime();
        accelerateMass(&massA, dt, g, withNumericalCor);

        BeginDrawing();
        ClearBackground((struct Color){33, 33, 33, 0xFF});

        DrawRectangleLines(padding, padding, playGroundWidth,
                           screenHeight - padding * 2, RAYWHITE);
        DrawRectangleLines(padding * 2 + playGroundWidth, padding, panelWidth,
                           screenHeight - padding * 2, RAYWHITE);
        GuiCheckBox(
            (Rectangle){padding * 4 + playGroundWidth, padding * 4, 20, 20},
            "Force Vectors", &drawForces);
        GuiCheckBox((Rectangle){padding * 4 + playGroundWidth, padding * 8 + 20,
                                20, 20},
                    "Velocity Vectors", &drawForces);
        GuiCheckBox((Rectangle){padding * 4 + playGroundWidth,
                                padding * 12 + 40, 20, 20},
                    "Numerical Correction", &withNumericalCor);

        DrawLineEx(pendulumAnchor, massA.position, stringWidth, RAYWHITE);

        DrawCircleV(pendulumAnchor, 10.0f, BLACK);
        DrawCircleV(massA.position, massA.mass, MAROON);

        if(drawForces) {
            drawVector(massA.position, (struct Vector2){0.0f, g}, BLUE,
                       VECTOR_DRAW_SCALE);
            drawVector(massA.position, massA.tension, RED, VECTOR_DRAW_SCALE);
            drawVector(massA.position, massA.acc, GREEN, VECTOR_DRAW_SCALE);
        }

        if(drawVelocities) {
            drawVector(massA.position, Vector2Scale(massA.velocity, 20), WHITE,
                       VECTOR_DRAW_SCALE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}