#include <raylib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define MAX_INIMIGOS 50

typedef enum 
{
    VIVO,
    MORTO
} Estado;

typedef struct jogador
{
    char nome[15];
    int vida;
    int vida_max;
    float x;
    float y;
    float vel;
    float raio;
    Estado estado;
} jogador;

typedef struct inimigo
{
    float x;
    float y;
    float vel;
    int dano;
    float raio;
    Estado estado;
} inimigo;

void criarInimigo (inimigo *inimigos, float x, float y, float vel, int dano, float raio, int *pquantidade) {
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
            y1 = GetRandomValue(0, (int)y);
            break;
        default: // direita
            x1 = x;
            y1 = GetRandomValue(0, (int)y);
            break;
    }
    inimigos[*pquantidade].x = x1;
    inimigos[*pquantidade].y = y1;
    inimigos[*pquantidade].vel = vel;
    inimigos[*pquantidade].dano = dano;
    inimigos[*pquantidade].raio = raio;
    *pquantidade += 1;
}

void reiniciarJogo (jogador *jogador1, inimigo *inimigos, int *pquantidade, float *tempo, float *tempo_ultimo_dano) {
    jogador1->x = 400;
    jogador1->y = 300;
    jogador1->estado = VIVO;
    jogador1->vida = jogador1->vida_max;

    *pquantidade = 0;
    *tempo = 0;
    *tempo_ultimo_dano = 0;

    criarInimigo(inimigos, GetScreenWidth(), GetScreenHeight(), 2, 5, 20, pquantidade);
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
    jogador1.raio = 25;
    jogador1.estado = VIVO;

    inimigo inimigos[MAX_INIMIGOS];
    int quantidade = 0;
    int *pquantidade = &quantidade;
    float tempo = 0;
    float tempo_ultimo_dano = 0;

    do {
        printf("Informe seu nome para que o jogo possa comecar:");
        fgets(jogador1.nome, sizeof(jogador1.nome), stdin);
        jogador1.nome[strcspn(jogador1.nome, "\n")] = '\0';
    } while (strlen (jogador1.nome)== 0);

    InitWindow(1280, 680, "Jogo Bom");
    SetTargetFPS(60);

    criarInimigo(inimigos, 200, 150, 2, 10, 20, pquantidade);

    while (!WindowShouldClose())
    // Movimentação do Jogador. //
    {
        if (jogador1.estado == VIVO) {
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
                criarInimigo(inimigos, GetRandomValue(0, 1280), GetRandomValue(0, 680 ) , 2, 5, 20, pquantidade);
            }

            perseguir(inimigos, jogador1.x, jogador1.y, jogador1.vel, pquantidade);

            if (GetTime() - tempo_ultimo_dano >= 2.0f) {

                int dano_acumulado = 0;
                bool levou_dano = false;

                for (int i = 0; i < quantidade; i++) {    
                    float dx = jogador1.x - inimigos[i].x;
                    float dy = jogador1.y - inimigos[i].y;
                    float distancia = sqrtf(dx * dx + dy * dy);
                    if (distancia <= (jogador1.raio + inimigos[i].raio)) {
                        dano_acumulado += inimigos[i].dano;
                        levou_dano = true;
                    }
                }
                if (levou_dano) {
                    jogador1.vida -= dano_acumulado;
                    tempo_ultimo_dano = GetTime();

                    if (jogador1.vida <= 0) {
                        jogador1.vida = 0;
                        jogador1.estado = MORTO;
                    }
                }
            }

        } else {
            if (IsKeyPressed(KEY_ENTER)) {
                reiniciarJogo(&jogador1, inimigos, pquantidade, &tempo, &tempo_ultimo_dano);
            }
        }    
            BeginDrawing();
            ClearBackground(LIME);

            if (jogador1.estado == VIVO) {
                DrawText(jogador1.nome, 5, 10, 35, WHITE);
                DrawText(TextFormat ("%d|%d", jogador1.vida, jogador1.vida_max), 5, 50, 35, WHITE);
                DrawCircle(jogador1.x, jogador1.y, jogador1.raio, RED);
                for (int i = 0; i < quantidade; i++)
                {
                    DrawCircle(inimigos[i].x, inimigos[i].y, inimigos[i].raio, WHITE);
                }
            } else {
                const char *msg = "CORROMPIDO";
                int fontSize = 60;
                int largura = MeasureText(msg, fontSize);
                DrawText(msg, GetScreenWidth()/2 - largura/2, GetScreenHeight()/2 - fontSize/2, fontSize, MAROON);

                const char *msg2 = "Aperte ENTER para reiniciar";
                int fontSize2 = 25;
                int largura2 = MeasureText(msg2, fontSize2);
                DrawText(msg2, GetScreenWidth()/2 - largura2/2, GetScreenHeight()/2 + fontSize, fontSize2, WHITE);

                // ainda desenha o jogador parado, com cor diferente (ex: cinza)
                DrawCircle(jogador1.x, jogador1.y, jogador1.raio, GRAY);
                        }   
            EndDrawing();
    }                    
    CloseWindow();

    return 0;
}