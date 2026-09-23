#include <raylib.h>
#include <math.h>

#define MAX_INIMIGOS 50

typedef struct inimigo
{
    float x;
    float y;
    float vel;
} inimigo;

int main(void)
{

    InitWindow(1280, 680, "personagem");
    SetTargetFPS(60);

    float x = 400;
    float y = 300;
    float velocidade = 5;

    inimigo inimigos[MAX_INIMIGOS];
    int quantidade = 0;
    float tempo = 0;

    inimigos[0].x = 200;
    inimigos[0].y = 150;
    inimigos[0].vel = 2;
    quantidade = 1;

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

        tempo += GetFrameTime();
        if (tempo > 10.0f && quantidade < MAX_INIMIGOS)
        {
            tempo = 0;
            inimigos[quantidade].x = GetRandomValue(0, 800);
            inimigos[quantidade].y = 0;
            inimigos[quantidade].vel = 2;
            quantidade++;
        }

        for (int i = 0; i < quantidade; i++)
        {
            float dx = x - inimigos[i].x;
            float dy = y - inimigos[i].y;
            float distancia = sqrtf(dx * dx + dy * dy);

            if (distancia > 0)
            {
                inimigos[i].x += (dx / distancia) * inimigos[i].vel;
                inimigos[i].y += (dy / distancia) * inimigos[i].vel;
            }
        }


        BeginDrawing();
        ClearBackground(GREEN);
        DrawCircle(x, y, 30, RED);
        for (int i = 0; i < quantidade; i++)
        {
            DrawCircle(inimigos[i].x, inimigos[i].y, 20, WHITE);
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}