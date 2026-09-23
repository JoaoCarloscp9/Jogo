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
    float velocidade = 2.75;

    inimigo inimigos[MAX_INIMIGOS];
    int quantidade = 0;
    float tempo = 0;

    inimigos[0].x = 200;
    inimigos[0].y = 150;
    inimigos[0].vel = 2;
    quantidade = 1;

    while (!WindowShouldClose())
    // Movimentação do Jogador. //
    {
        if (IsKeyDown(KEY_RIGHT))
            x += velocidade;
        if (IsKeyDown(KEY_LEFT))
            x -= velocidade;
        if (IsKeyDown(KEY_DOWN))
            y += velocidade;
        if (IsKeyDown(KEY_UP))
            y -= velocidade;

    // Nascimento de inimigos com o tempo. //
        tempo += GetFrameTime();
        if (tempo > 10.0f && quantidade < MAX_INIMIGOS)
        {
            tempo = 0;
            inimigos[quantidade].x = GetRandomValue(0, 800);
            inimigos[quantidade].y = 0;
            inimigos[quantidade].vel = 2;
            quantidade++;
        }

    // Inimigos que seguem. //
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
        ClearBackground(LIME);
        DrawCircle(x, y, 25, RED);
        for (int i = 0; i < quantidade; i++)
        {
            DrawCircle(inimigos[i].x, inimigos[i].y, 20, WHITE);
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}