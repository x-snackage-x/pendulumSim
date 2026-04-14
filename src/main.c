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

    DrawLineEx(anchorPos, endPos, 2.f, color);
    DrawLineEx(endPos, endPosOne, 2.f, color);
    DrawLineEx(endPos, endPosTwo, 2.f, color);
}

void drawVectorLegend(Vector2 anchorPos, Color color) {
    Vector2 vector = {.x = 20};

    Vector2 sideOneV =
        Vector2Scale(Vector2Rotate(vector, 20.0f * DEG2RAD), 0.55f);
    Vector2 sideTwoV =
        Vector2Scale(Vector2Rotate(vector, -20.0f * DEG2RAD), 0.55f);

    Vector2 endPos = Vector2Add(anchorPos, vector);
    Vector2 endPosOne = Vector2Add(anchorPos, sideOneV);
    Vector2 endPosTwo = Vector2Add(anchorPos, sideTwoV);

    DrawLineEx(anchorPos, endPos, 2.0f, color);
    DrawLineEx(endPos, endPosOne, 2.0f, color);
    DrawLineEx(endPos, endPosTwo, 2.0f, color);
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
    bool isDragging = false;
    bool drawForces = true;
    bool drawVelocities = true;
    float gFactor = 1.0f;
    float simFactor = 1.0f;
    const int padding = 2;
    const int panelWidth = 260;
    const int playGroundWidth = 1100;
    const int screenWidth = playGroundWidth + panelWidth + padding * 3;
    const int screenHeight = 900;
    Vector2 origin = {0.0f, 0.0f};

    Vector2 mousePosition = {0.0f};

    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "PendulumSim");

    float g = 9.81f;
    float amplitude = -30.0f * DEG2RAD;
    float stringWidth = 4.0f;
    Vector2 pendulumAnchor = {
        (GetScreenWidth() - panelWidth - padding * 3) * 0.5f,
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
        int controlPanelStart = GetScreenWidth() - (panelWidth + 3 * padding);

        if(massA.pendulumLength != (int)pendulumLengthPxl ||
           massA.mass != firstMass) {
            massA.pendulumLength = (int)pendulumLengthPxl;
            massA.mass = firstMass;
            reset = true;
        }

        mousePosition = GetMousePosition();
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
           mousePosition.x < controlPanelStart) {
            resetMass(&massA, pendulumAnchor, mousePosition);
            isDragging = true;
        } else if(reset) {
            trnsltnVecPxlSpce.x = 0;
            trnsltnVecPxlSpce.y = pendulumLengthPxl;
            trnsltnVecPxlSpce = Vector2Rotate(trnsltnVecPxlSpce, amplitude);
            massA.position = Vector2Add(pendulumAnchor, trnsltnVecPxlSpce);
            massA.velocity = Vector2Zero();
            massA.accNet = Vector2Zero();
            massA.tensionSum = Vector2Zero();

            gFactor = 1.0f;
            reset = false;
        } else {
            isDragging = false;
        }

        if(IsKeyPressed(KEY_SPACE))
            pause = !pause;

        float dt = GetFrameTime();
        if(!pause)
            accelerateMass(&massA, dt * simFactor, g * gFactor);

        BeginDrawing();
        ClearBackground((struct Color){33, 33, 33, 0xFF});

        DrawRectangleLines(padding * 2 + controlPanelStart, padding, panelWidth,
                           screenHeight - padding * 2, RAYWHITE);

        int nGuiElement = 0;
        float guiElementHeight = 20;
        float guiElementsXStart = padding * 4 + controlPanelStart;
        float guiElementsYStart = padding * 4;
        float guiElementYStep = 20 + padding * 2;
        int guiSliderPaddingSt = 15 * padding;
        int guiSliderPaddingEnd = 40 * padding;
        char* buttonText = "";
        if(pause) {
            buttonText = "Start";
        } else {
            buttonText = "Pause";
        }
        if(GuiButton(
               (Rectangle){guiElementsXStart + 70,
                           guiElementsYStart + nGuiElement * guiElementYStep,
                           40, guiElementHeight},
               buttonText)) {
            pause = !pause;
        }
        reset = GuiButton(
            (Rectangle){guiElementsXStart + 70 + 40 + padding * 4,
                        guiElementsYStart + nGuiElement++ * guiElementYStep, 50,
                        guiElementHeight},
            "Reset");
        GuiCheckBox(
            (Rectangle){guiElementsXStart + 10,
                        guiElementsYStart + nGuiElement * guiElementYStep + 3,
                        15, 15},
            "Force Vectors", &drawForces);
        GuiCheckBox(
            (Rectangle){guiElementsXStart + panelWidth / 2,
                        guiElementsYStart + nGuiElement++ * guiElementYStep + 3,
                        15, 15},
            "Velocity Vectors", &drawVelocities);
        GuiLabel(
            (Rectangle){guiElementsXStart,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - guiSliderPaddingEnd, guiElementHeight},
            TextFormat("Simulation Speed Factor: %.3f", simFactor));
        GuiSlider(
            (Rectangle){guiElementsXStart + guiSliderPaddingSt,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - guiSliderPaddingEnd, guiElementHeight},
            "1", "10", &simFactor, 1, 10);
        GuiLabel(
            (Rectangle){guiElementsXStart,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - guiSliderPaddingEnd, guiElementHeight},
            TextFormat("g Factor: %.3f = %.3f m/s^2", gFactor, gFactor * g));
        GuiSlider(
            (Rectangle){guiElementsXStart + guiSliderPaddingSt,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - guiSliderPaddingEnd, guiElementHeight},
            "Moon", "Jupiter", &gFactor, 0.1656f, 2.527f);
        GuiLabel(
            (Rectangle){guiElementsXStart,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - guiSliderPaddingEnd, guiElementHeight},
            TextFormat("Pendulum Length: %.3f m", pendulumLengthPxl));
        GuiSlider(
            (Rectangle){guiElementsXStart + guiSliderPaddingSt,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - guiSliderPaddingEnd, guiElementHeight},
            "10", "1000", &pendulumLengthPxl, 10, 1000);
        GuiLabel(
            (Rectangle){guiElementsXStart,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - guiSliderPaddingEnd, guiElementHeight},
            TextFormat("Mass: %.3f kg", firstMass));
        GuiSlider(
            (Rectangle){guiElementsXStart + guiSliderPaddingSt,
                        guiElementsYStart + nGuiElement++ * guiElementYStep,
                        panelWidth - guiSliderPaddingEnd, guiElementHeight},
            "10", "200", &firstMass, 10, 200);

        nGuiElement++;
        int legendVectorOffset = 5;
        drawVectorLegend(
            (Vector2){guiElementsXStart, guiElementsYStart +
                                             nGuiElement * guiElementYStep +
                                             legendVectorOffset},
            RED);
        DrawText("F-Tension(G component)",
                 guiElementsXStart + guiSliderPaddingSt,
                 guiElementsYStart + nGuiElement++ * guiElementYStep, 1, RED);
        drawVectorLegend(
            (Vector2){guiElementsXStart, guiElementsYStart +
                                             nGuiElement * guiElementYStep +
                                             legendVectorOffset},
            PINK);
        DrawText("F-Tension(Centripetal component)",
                 guiElementsXStart + guiSliderPaddingSt,
                 guiElementsYStart + nGuiElement++ * guiElementYStep, 1, PINK);
        drawVectorLegend(
            (Vector2){guiElementsXStart, guiElementsYStart +
                                             nGuiElement * guiElementYStep +
                                             legendVectorOffset},
            GREEN);
        DrawText("F-Tangential(Gravity component)",
                 guiElementsXStart + guiSliderPaddingSt,
                 guiElementsYStart + nGuiElement++ * guiElementYStep, 1, GREEN);
        drawVectorLegend(
            (Vector2){guiElementsXStart, guiElementsYStart +
                                             nGuiElement * guiElementYStep +
                                             legendVectorOffset},
            BLUE);
        DrawText("F-Gravity", guiElementsXStart + guiSliderPaddingSt,
                 guiElementsYStart + nGuiElement * guiElementYStep, 1, BLUE);
        drawVectorLegend(
            (Vector2){guiElementsXStart + 95,
                      guiElementsYStart + nGuiElement * guiElementYStep +
                          legendVectorOffset},
            YELLOW);
        DrawText("F-Net", guiElementsXStart + guiSliderPaddingSt + 95,
                 guiElementsYStart + nGuiElement * guiElementYStep, 1, YELLOW);
        drawVectorLegend(
            (Vector2){guiElementsXStart + 170,
                      guiElementsYStart + nGuiElement * guiElementYStep +
                          legendVectorOffset},
            WHITE);
        DrawText("Velocity", guiElementsXStart + guiSliderPaddingSt + 170,
                 guiElementsYStart + nGuiElement++ * guiElementYStep, 1, WHITE);

        DrawLineEx(pendulumAnchor, massA.position, stringWidth, RAYWHITE);

        DrawCircleV(pendulumAnchor, 10.0f, BLACK);
        DrawCircleV(massA.position, massA.mass, MAROON);

        if(drawForces && !isDragging) {
            drawVector(massA.position,
                       (struct Vector2){0.0f, massA.mass * g * gFactor}, BLUE,
                       VECTOR_DRAW_SCALE);
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