#include <math.h>
#include "resource_dir.h"

#include "raylib.h"

#define RAYMATH_IMPLEMENTATIOn
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define VECTOR_DRAW_SCALE 1.0f
float pendulumLengthPxl = 200.0f;
float firstMass = 25.0f;

typedef struct PendulumMass {
    int pendulumLength;
    float mass;
    Vector2 tensionGravityComp;
    Vector2 tensionCentripetalComp;
    Vector2 accGravityComp;

    Vector2 tensionSum;
    Vector2 accNet;
    Vector2 anchor;
    Vector2 position;
    Vector2 velocity;
} PendulumMass;

void resetMass(PendulumMass* mass,
               Vector2 pendulumAnchor,
               Vector2 mousePosition) {
    Vector2 trnsltnVecPxlSpce =
        Vector2Normalize(Vector2Subtract(mousePosition, pendulumAnchor));
    trnsltnVecPxlSpce = Vector2Scale(trnsltnVecPxlSpce, pendulumLengthPxl);
    mass->position = Vector2Add(pendulumAnchor, trnsltnVecPxlSpce);
    mass->velocity = Vector2Zero();
    mass->accNet = Vector2Zero();
    mass->tensionSum = Vector2Zero();
}

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

void accelerateMass(PendulumMass* mass, float deltaTime, float g) {
    Vector2 gravityVector = {0.0f, mass->mass * g};

    Vector2 tensionVecNorm =
        Vector2Normalize(Vector2Subtract(mass->position, mass->anchor));

    float amplitude = Vector2Angle(gravityVector, tensionVecNorm);

    // Calculate Kinetics: Gravity Components
    mass->tensionGravityComp =
        Vector2Scale(tensionVecNorm, mass->mass * g * cosf(amplitude));
    mass->accGravityComp =
        Vector2Subtract(gravityVector, mass->tensionGravityComp);

    // Calculate Kinetics: Centripetal Components
    float cntrptlForceFactor = mass->mass / mass->pendulumLength;
    float vSquared = Vector2LengthSqr(mass->velocity);

    mass->tensionCentripetalComp =
        Vector2Scale(tensionVecNorm, -cntrptlForceFactor * vSquared);

    // Sum Forces
    if(vSquared == 0.0f) {
        mass->tensionSum = Vector2Zero();
    } else {
        mass->tensionSum = Vector2Add(mass->tensionCentripetalComp,
                                      Vector2Negate(mass->tensionGravityComp));
    }

    mass->accNet = Vector2Add(mass->tensionSum, gravityVector);

    // Calculate Velocity and Position step delta
    mass->velocity = Vector2Add(
        mass->velocity, Vector2Scale(mass->accNet, deltaTime / mass->mass));

    mass->position =
        Vector2Add(mass->position, Vector2Scale(mass->velocity, deltaTime));
}

int main() {
    bool pause = true;
    bool reset = false;
    bool drawForces = true;
    bool drawVelocities = true;
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
    float amplitude = -30.0f * DEG2RAD;
    float stringWidth = 2.0f;
    Vector2 pendulumAnchor = {(GetScreenWidth() - panelWidth) * 0.5f,
                              GetScreenHeight() * 0.2f};

    Vector2 trnsltnVecPxlSpce = {.y = pendulumLengthPxl};
    trnsltnVecPxlSpce = Vector2Rotate(trnsltnVecPxlSpce, amplitude);

    PendulumMass massA = {
        .mass = 25.0f,
        .pendulumLength = pendulumLengthPxl,
        .anchor = pendulumAnchor,
        .position = Vector2Add(pendulumAnchor, trnsltnVecPxlSpce)};

    SetTargetFPS(60);
    while(!WindowShouldClose()) {
        if(massA.pendulumLength != (int)pendulumLengthPxl ||
           massA.mass != firstMass) {
            massA.pendulumLength = (int)pendulumLengthPxl;
            massA.mass = firstMass;
            reset = true;
        }

        mousePosition = GetMousePosition();
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
           mousePosition.x < playGroundWidth) {
            resetMass(&massA, pendulumAnchor, mousePosition);
        } else if(reset) {
            trnsltnVecPxlSpce.x = 0;
            trnsltnVecPxlSpce.y = pendulumLengthPxl;
            trnsltnVecPxlSpce = Vector2Rotate(trnsltnVecPxlSpce, amplitude);
            massA.position = Vector2Add(pendulumAnchor, trnsltnVecPxlSpce);
            massA.velocity = Vector2Zero();
            massA.accNet = Vector2Zero();
            massA.tensionSum = Vector2Zero();

            reset = false;
        }

        if(IsKeyPressed(KEY_SPACE))
            pause = !pause;

        float dt = GetFrameTime();
        if(!pause)
            accelerateMass(&massA, dt * 3, g);

        BeginDrawing();
        ClearBackground((struct Color){33, 33, 33, 0xFF});

        DrawRectangleLines(padding * 2 + playGroundWidth, padding, panelWidth,
                           screenHeight - padding * 2, RAYWHITE);

        int nGuiElement = 0;
        float guiElementHeight = 20;
        float guiElementsXStart = padding * 4 + playGroundWidth;
        float guiElementsYStart = padding * 4;
        float guiElementYStep = 20 + padding * 2;
        GuiCheckBox(
            (Rectangle){guiElementsXStart,
                        guiElementsYStart + nGuiElement++ * guiElementYStep, 20,
                        guiElementHeight},
            "Force Vectors", &drawForces);
        GuiCheckBox(
            (Rectangle){guiElementsXStart,
                        guiElementsYStart + nGuiElement++ * guiElementYStep, 20,
                        guiElementHeight},
            "Velocity Vectors", &drawVelocities);

        GuiLabel(
            (Rectangle){guiElementsXStart + 15,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - 30 * padding, guiElementHeight},
            "Pendulum Length 1 [m]");
        GuiSlider(
            (Rectangle){guiElementsXStart + 15,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - 30 * padding, guiElementHeight},
            "10", "1000", &pendulumLengthPxl, 10, 1000);
        GuiLabel(
            (Rectangle){guiElementsXStart + 15,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - 30 * padding, guiElementHeight},
            "Mass [kg]");
        GuiSlider(
            (Rectangle){guiElementsXStart + 15,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - 30 * padding, guiElementHeight},
            "10", "200", &firstMass, 10, 200);

        DrawLineEx(pendulumAnchor, massA.position, stringWidth, RAYWHITE);

        DrawCircleV(pendulumAnchor, 10.0f, BLACK);
        DrawCircleV(massA.position, massA.mass, MAROON);

        if(drawForces) {
            drawVector(massA.position, (struct Vector2){0.0f, massA.mass * g},
                       BLUE, VECTOR_DRAW_SCALE);
            drawVector(massA.position, Vector2Negate(massA.tensionGravityComp),
                       RED, VECTOR_DRAW_SCALE);
            drawVector(
                Vector2Subtract(massA.position, massA.tensionGravityComp),
                massA.tensionCentripetalComp, PINK, VECTOR_DRAW_SCALE);
            drawVector(massA.position, massA.accGravityComp, GREEN,
                       VECTOR_DRAW_SCALE);
            drawVector(massA.position, massA.accNet, YELLOW, VECTOR_DRAW_SCALE);
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