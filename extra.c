#include <raylib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define MAX_INIMIGOS 50

typedef struct jogador
{
    char nome[15];
    int vida;
    int vida_max;
    float x;
    float y;
    float vel;
} jogador;

typedef struct inimigo
{
    float x;
    float y;
    float vel;
} inimigo;

void criarInimigo (inimigo *inimigos, float x, float y, float vel, int *pquantidade) {
    float x1, y1;
    int lado = GetRandomValue(0, 3); // 0=cima, 1=baixo, 2=esquerda, 3=direita

    switch (lado)
    {
        case 0: // topo
            x1 = GetRandomValue(0, (int)x);
            y1 = 0;
            break;
        case 1: // baixo
            x1 = GetRandomValue(0, (int)x);
            y1 = y;
            break;
        case 2: // esquerda
            x1 = 0;
            y1 = GetRandomValue(0, (int)x);
            break;
        default: // direita
            x1 = x;
            y1 = GetRandomValue(0, (int)y);
            break;
    }
    inimigos[*pquantidade].x = x1;
    inimigos[*pquantidade].y = y1;
    inimigos[*pquantidade].vel = vel;
    *pquantidade += 1;
}

void perseguir (inimigo *inimigos, float x, float y, float vel, int *pquantidade) {
    for (int i = 0; i < *pquantidade; i++) {
            float dx = x - inimigos[i].x;
            float dy = y - inimigos[i].y;
            float distancia = sqrtf(dx * dx + dy * dy);

            if (distancia > 0)
            {
                inimigos[i].x += (dx / distancia) * inimigos[i].vel;
                inimigos[i].y += (dy / distancia) * inimigos[i].vel;
            }
    }
}

int main(void)
{

    jogador jogador1;
    jogador1.x = 400;
    jogador1.y = 300;
    jogador1.vel = 2.75;
    jogador1.vida_max = 100;
    jogador1.vida = jogador1.vida_max;

    inimigo inimigos[MAX_INIMIGOS];
    int quantidade = 0;
    int *pquantidade = &quantidade;
    float tempo = 0;

    do {
        printf("Informe seu nome para que o jogo possa comecar:");
        fgets(jogador1.nome, sizeof(jogador1.nome), stdin);
        jogador1.nome[strcspn(jogador1.nome, "\n")] = '\0';
    } while (strlen (jogador1.nome)== 0);

    InitWindow(1280, 680, "Jogo Bom");
    SetTargetFPS(60);

    criarInimigo(inimigos, 200, 150, 2, pquantidade);

    while (!WindowShouldClose())
    // Movimentação do Jogador. //
    {
        if (IsKeyDown(KEY_RIGHT))
            jogador1.x += jogador1.vel;
        if (IsKeyDown(KEY_LEFT))
            jogador1.x -= jogador1.vel;
        if (IsKeyDown(KEY_DOWN))
            jogador1.y += jogador1.vel;
        if (IsKeyDown(KEY_UP))
            jogador1.y -= jogador1.vel;

    // Nascimento de inimigos com o tempo. //
        tempo += GetFrameTime();
        if (tempo > 10.0f && quantidade < MAX_INIMIGOS)
        {
            tempo = 0;
            criarInimigo(inimigos, GetRandomValue(0, 1280), GetRandomValue(0, 680 ) , 2, pquantidade);
        }

    // Inimigos que seguem. //
        perseguir(inimigos, jogador1.x, jogador1.y, jogador1.vel, pquantidade);

        BeginDrawing();
        ClearBackground(LIME);
        DrawText(jogador1.nome, 5, 10, 35, WHITE);
        DrawText(TextFormat ("%d|%d", jogador1.vida, jogador1.vida_max), 5, 50, 35, WHITE);
        DrawCircle(jogador1.x, jogador1.y, 25, RED);
        for (int i = 0; i < quantidade; i++)
        {
            DrawCircle(inimigos[i].x, inimigos[i].y, 20, WHITE);
        }
        EndDrawing();
    }
    CloseWindow();

    return 0;
}