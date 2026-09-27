#include <raylib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LARGURA_TELA 1280
#define ALTURA_TELA 680
#define RAIO_INIMIGO 20
#define CAPACIDADE_INICIAL 50

typedef enum 
{
    VIVO,
    MORTO
} Estado;

typedef enum
{
    VIDA,
    ALCANCE,
    DANO
} Melhoria;

typedef struct jogador
{
    char nome[15];
    int vida;
    int vida_max;
    float x;
    float y;
    float vel;
    float raio;
    int dano;
    float raio_ataque;
    int nivel;
    Estado estado;
} jogador;

typedef struct inimigo
{
    float x;
    float y;
    float vel;
    int vida;
    int vida_max;
    int dano;
    float raio;
    Estado estado;
} inimigo;

void manterNaTela (float *pos_x, float *pos_y){
  if (*pos_x >= LARGURA_TELA - 25){
    *pos_x = LARGURA_TELA - 25;
  }
  else if (*pos_x <=0 + 25){
    *pos_x = 0 + 25;
  }
  if (*pos_y >= ALTURA_TELA - 25){
    *pos_y = ALTURA_TELA - 25;
  }
  else if (*pos_y <=0 + 25){
    *pos_y = 0 + 25;
  }
}

void criarInimigo (inimigo *inimigos, int capacidade, float x, float y, float vel, int dano, float raio, int *pquantidade, float vida) {
    
    if (*pquantidade >= capacidade){
        //array maior que a quantidade máxima definida//
        return;
    }

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
    inimigos[*pquantidade].vida_max = vida;
    inimigos[*pquantidade].vida = vida;
    inimigos[*pquantidade].estado = VIVO;
    *pquantidade += 1;
}

void apagarInimigo (inimigo **pinimigos, int *pcapacidade, int *pquantidade) {
    for (int i = 0; i < *pquantidade; i++) {
        if ((*pinimigos)[i].estado == MORTO) {
            (*pinimigos)[i] = (*pinimigos)[*pquantidade - 1];
            (*pquantidade)--;

            if (*pquantidade > 0 && *pquantidade <= *pcapacidade / 2) {
                inimigo *temp = realloc(*pinimigos, *pquantidade * sizeof(inimigo));
                if (temp != NULL) {
                    *pinimigos = temp;
                    *pcapacidade = *pquantidade;
                }
            }
            i--;
        }
    }
}

void reiniciarJogo (jogador *jogador1, inimigo **pinimigos, int *pcapacidade, int *pquantidade, float *tempo, float *tempo_ultimo_dano) {
    jogador1->x = 400;
    jogador1->y = 300;
    jogador1->estado = VIVO;
    jogador1->vida = jogador1->vida_max;

    *pquantidade = 0;
    *tempo = 0;
    *tempo_ultimo_dano = 0;

    //Se a quatidade cresceu durante a partida, ela volta ao estado inicial quando reinicia.//    
    if (*pcapacidade > CAPACIDADE_INICIAL){
        inimigo *novo = realloc (*pinimigos, CAPACIDADE_INICIAL * sizeof (inimigo));
        
        if (novo != NULL){
            *pinimigos = novo;
            *pcapacidade = CAPACIDADE_INICIAL;
        }
    }

    criarInimigo(*pinimigos, *pcapacidade, GetScreenWidth(), GetScreenHeight(), 2, 5, 20, pquantidade, 15);
}

void subirNivel (jogador *jogador1, Melhoria caracteristica) {
    switch (caracteristica) {
        case VIDA:
            jogador1->vida_max += 10;
            jogador1->vida += 10;
            break;
        case ALCANCE:
            jogador1->raio_ataque += 10;
            break;
        case DANO:
            jogador1->dano += 3;
            break;
        default: return;
    }
    jogador1->nivel += 1;
}

void perseguir (inimigo *inimigos, float x, float y, int *pquantidade) {
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

void separarInimigos(inimigo *inimigos, int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        for (int j = i + 1; j < quantidade; j++) {
            float dx = inimigos[j].x - inimigos[i].x;
            float dy = inimigos[j].y - inimigos[i].y;
            float distancia = sqrtf(dx * dx + dy * dy);
            float distancia_minima = RAIO_INIMIGO * 2; // soma dos dois raios

            if (distancia > 0 && distancia < distancia_minima)
            {
                float sobreposicao = distancia_minima - distancia;
                float empurra_x = (dx / distancia) * (sobreposicao / 2);
                float empurra_y = (dy / distancia) * (sobreposicao / 2);

                inimigos[i].x -= empurra_x;
                inimigos[i].y -= empurra_y;
                inimigos[j].x += empurra_x;
                inimigos[j].y += empurra_y;
            }
        }
    }
}

int inimigoMaisProximo(int *pquantidade, float x, float y, inimigo *inimigos) {
    int maisProximo = -1;
    float menorDistancia = 0.0f;

    for (int i = 0; i < *pquantidade; i++) {

        float dx =  inimigos[i].x - x;
        float dy = inimigos[i].y - y;
        float distancia = sqrtf(dx * dx + dy * dy);

        if (maisProximo == -1 || distancia < menorDistancia) {
            maisProximo = i;
            menorDistancia = distancia;
        }
    }
    return maisProximo;
}

int main(void)
{

    jogador jogador1;
    jogador1.x = 400;
    jogador1.y = 300;
    jogador1.vel = 2.75;
    jogador1.vida_max = 50;
    jogador1.vida = jogador1.vida_max;
    jogador1.raio = 25;
    jogador1.dano = 5;
    jogador1.raio_ataque = 180;
    jogador1.estado = VIVO;
    jogador1.nivel = 0;

    int experiencia = 0;
    int experiencia_max = 5;
    
    int capacidade = CAPACIDADE_INICIAL;

    //Vetor dinâmico dos inimigos.//
    inimigo *inimigos = malloc (capacidade * sizeof (inimigo));
    if (inimigos == NULL) {
        printf("Erro ao alocar memoria para os inimigos.\n");
        return 1;
    }


    int quantidade = 0;
    int *pquantidade = &quantidade;
    float tempo = 0;
    float tempo_ultimo_dano = 0;
    float tempo_ataque = 0;
    bool pausaMelhoria = false;

    do {
        printf("Informe seu nome para que o jogo possa comecar:");
        fgets(jogador1.nome, sizeof(jogador1.nome), stdin);
        jogador1.nome[strcspn(jogador1.nome, "\n")] = '\0';
    } while (strlen (jogador1.nome)== 0);

    InitWindow(1280, 680, "Jogo Bom");
    SetTargetFPS(60);

    criarInimigo(inimigos, capacidade, 200, 150, 2, 10, 20, pquantidade, 15);

    while (!WindowShouldClose())
    {
        // Movimentação do Jogador. //
        if (jogador1.estado == VIVO && !pausaMelhoria) {
            if (IsKeyDown(KEY_RIGHT))
                jogador1.x += jogador1.vel;
            if (IsKeyDown(KEY_LEFT))
                jogador1.x -= jogador1.vel;
            if (IsKeyDown(KEY_DOWN))
                jogador1.y += jogador1.vel;
            if (IsKeyDown(KEY_UP))
                jogador1.y -= jogador1.vel;

        // Atacar inimigos //       
        int alvo = inimigoMaisProximo(pquantidade, jogador1.x, jogador1.y, inimigos);
        if (GetTime() - tempo_ataque >= 1.0f) {
            if (IsKeyPressed(KEY_SPACE)) {
                if (alvo >= 0) {
                    float dx = jogador1.x - inimigos[alvo].x;
                    float dy = jogador1.y - inimigos[alvo].y;
                    float distancia = sqrtf(dx * dx + dy * dy);

                    if (distancia < jogador1.raio_ataque) {
                        inimigos[alvo].vida -= jogador1.dano;
                        tempo_ataque = GetTime();
                        if (inimigos[alvo].vida <= 0) {
                            experiencia += 1;
                            if (experiencia >= experiencia_max) {
                                pausaMelhoria = true;
                            }
                        }
                    }
                }
            }
        }

        // Impede que o jogador passe da tela. //
        manterNaTela(&jogador1.x, &jogador1.y);

        // Atribuir estado MORTO a um inimigo //
        for (int i = 0; i < quantidade; i++) {
            if (inimigos[i].vida <= 0) {
                inimigos[i].estado = MORTO;
            }
        }

        // Apaga um inimigo se ele estiver morto //
        apagarInimigo (&inimigos, &capacidade, pquantidade);

        // Nascimento de inimigos com o tempo. //
            tempo += GetFrameTime();
            if (tempo > 2.0f && quantidade < capacidade)
            {
                tempo = 0;
                criarInimigo(inimigos, capacidade, GetRandomValue(0, 1280), GetRandomValue(0, 680 ) , 2, 5, 20, pquantidade, 15);
            }

            perseguir(inimigos, jogador1.x, jogador1.y, pquantidade);
            separarInimigos(inimigos, quantidade);

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

        } else if (jogador1.estado == VIVO && pausaMelhoria) {
            if (IsKeyPressed(KEY_ONE)) {
                subirNivel(&jogador1, VIDA);
                experiencia = 0;
                experiencia_max = experiencia_max * 2;
                pausaMelhoria = false;
        } else if (IsKeyPressed(KEY_TWO)) {
                subirNivel(&jogador1, DANO);
                experiencia = 0;
                experiencia_max = experiencia_max * 2;
                pausaMelhoria = false;
        } else if (IsKeyPressed(KEY_THREE)) {
                subirNivel(&jogador1, ALCANCE);
                experiencia = 0;
                experiencia_max = experiencia_max * 2;
                pausaMelhoria = false;
        } else {
            if (IsKeyPressed(KEY_ENTER)) {
                reiniciarJogo(&jogador1, &inimigos, &capacidade, pquantidade, &tempo, &tempo_ultimo_dano);
            }
        }
        }    
            BeginDrawing();
            ClearBackground(LIME);

            if (jogador1.estado == VIVO && !pausaMelhoria) {
                DrawText(jogador1.nome, 5, 10, 35, WHITE);
                DrawText(TextFormat ("%d|%d", jogador1.vida, jogador1.vida_max), 5, 50, 35, WHITE);
                DrawText(TextFormat ("Nivel:%d Exp:%d|%d",jogador1.nivel , experiencia, experiencia_max), 5, 80, 35, WHITE);
                DrawCircle(jogador1.x, jogador1.y, jogador1.raio_ataque, Fade(SKYBLUE, 0.05f));
                DrawCircleLines(jogador1.x, jogador1.y, jogador1.raio_ataque, PURPLE);
                DrawCircle(jogador1.x, jogador1.y, jogador1.raio, RED);
                
                for (int i = 0; i < quantidade; i++)
                {
                    DrawCircle(inimigos[i].x, inimigos[i].y, inimigos[i].raio, WHITE);
                    //Mostra a vida dos inimigos//
                    DrawText( TextFormat ("%d", inimigos[i].vida), inimigos[i].x, inimigos[i].y, 20, BLACK);
                }
            } else if (jogador1.estado == VIVO && pausaMelhoria) {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));

                    const char *titulo = "ESCOLHA UMA MELHORIA";
                    int fontSizeTitulo = 40;
                    int larguraTitulo = MeasureText(titulo, fontSizeTitulo);
                    DrawText(titulo, GetScreenWidth()/2 - larguraTitulo/2, 200, fontSizeTitulo, WHITE);

                    DrawText("[1] Vida       +10 vida maxima", GetScreenWidth()/2 - 150, 280, 25, WHITE);
                    DrawText("[2] Dano       +5 dano",         GetScreenWidth()/2 - 150, 320, 25, WHITE);
                    DrawText("[3] Alcance    +10 raio de ataque", GetScreenWidth()/2 - 150, 360, 25, WHITE);
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

    free(inimigos);

    return 0;
}