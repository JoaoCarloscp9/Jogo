#include <raylib.h>

int main(void)
{

    InitWindow(800, 600, "personagem");
    SetTargetFPS(60);

    int x = 400;
    int y = 300;
    int velocidade = 5;

    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_RIGHT))
            x += velocidade;
        if (IsKeyDown(KEY_LEFT))
            x -= velocidade;
        if (IsKeyDown(KEY_DOWN))
            y += velocidade;
        if (IsKeyDown(KEY_UP))
            y -= velocidade;

        BeginDrawing();
        ClearBackground(GREEN);
        DrawCircle(x, y, 30, RED);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}