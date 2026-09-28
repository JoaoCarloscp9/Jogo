#include <raylib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LARGURA_TELA 1280
#define ALTURA_TELA 680
#define RAIO_INIMIGO 20
#define CAPACIDADE_INICIAL 50
#define TAMANHO_CELULA 40
#define LARGURA_FRAME 200
#define ALTURA_FRAME 200
#define ESCALA_JOGADOR 5.0f
#define DURACAO_MAGIA 0.5f

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

typedef struct posicao 
{
    float x;
    float y;
} posicao;

typedef struct jogador
{
    char nome[15];
    int vida;
    int vida_max;
    posicao pos;
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
    int id;
} inimigo;

int **criarMatriz(int linhas, int colunas) {
    int **matriz = (int **)malloc(linhas * sizeof(int *));
    if (matriz == NULL) return NULL;

    for (int i = 0; i < linhas; i++) {
        matriz[i] = (int *)malloc(colunas * sizeof(int));
        for (int j = 0; j < colunas; j++) {
            matriz[i][j] = 0;
        }
    }
    return matriz;
}

void liberarMatriz(int **matriz, int linhas) {
    for (int i = 0; i < linhas; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

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
    static int proximo_id = 0;

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
    inimigos[*pquantidade].id = proximo_id;
    proximo_id++;
    *pquantidade += 1;
}

void apagarInimigo (inimigo **pinimigos, int *pcapacidade, int *pquantidade) {
    for (int i = 0; i < *pquantidade; i++) {
        if ((*pinimigos)[i].estado == MORTO) {
            (*pinimigos)[i] = (*pinimigos)[*pquantidade - 1];
            (*pquantidade)--;
            i--;
        }
    }
}

void reiniciarJogo (jogador *jogador1, inimigo **pinimigos, int *pcapacidade, int *pquantidade, float *tempo, float *tempo_ultimo_dano) {
    jogador1->pos.x = 400;
    jogador1->pos.y = 300;
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
    jogador1.pos.x = 400;
    jogador1.pos.y = 300;
    jogador1.vel = 2.75;
    jogador1.vida_max = 50;
    jogador1.vida = jogador1.vida_max;
    jogador1.raio = 25;
    jogador1.dano = 5;
    jogador1.raio_ataque = 180;
    jogador1.estado = VIVO;
    jogador1.nivel = 0;

    Rectangle origemJogador = {
        1 * LARGURA_FRAME,
        0,
        LARGURA_FRAME,
        ALTURA_FRAME
    };

    int experiencia = 0;
    int experiencia_max = 5;
    
    int capacidade = CAPACIDADE_INICIAL;

    int efeito_magia_alvo_id = -1;

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

    float tempo_magia = -DURACAO_MAGIA;
    float efeito_magia_x = 0;
    float efeito_magia_y = 0;

    do {
        printf("Informe seu nome para que o jogo possa comecar:");
        fgets(jogador1.nome, sizeof(jogador1.nome), stdin);
        jogador1.nome[strcspn(jogador1.nome, "\n")] = '\0';
    } while (strlen (jogador1.nome)== 0);

    InitWindow(LARGURA_TELA, ALTURA_TELA, "Jogo Bom");
    SetTargetFPS(60);

    Texture2D grama = LoadTexture("sprites/grama.png");
    Texture2D cogumelo = LoadTexture("sprites/Cogumelo.png");
    Texture2D mago = LoadTexture("sprites/Mago.png");
    Texture2D magia = LoadTexture("sprites/Magia.png");

    int colunas = LARGURA_TELA / TAMANHO_CELULA;
    int linhas = ALTURA_TELA / TAMANHO_CELULA;
    int **mapa = criarMatriz (linhas, colunas);
    if (mapa == NULL) {
        printf("Erro de alocação");
        return 1;
    }

    criarInimigo(inimigos, capacidade, 200, 150, 2, 10, 20, pquantidade, 15);

    while (!WindowShouldClose())
    {
        // Movimentação do Jogador. //
        if (jogador1.estado == VIVO && !pausaMelhoria) {
            if (IsKeyDown(KEY_RIGHT))
                jogador1.pos.x += jogador1.vel;
            if (IsKeyDown(KEY_LEFT))
                jogador1.pos.x -= jogador1.vel;
            if (IsKeyDown(KEY_DOWN))
                jogador1.pos.y += jogador1.vel;
            if (IsKeyDown(KEY_UP))
                jogador1.pos.y -= jogador1.vel;

        // Atacar inimigos //       
        int alvo = inimigoMaisProximo(pquantidade, jogador1.pos.x, jogador1.pos.y, inimigos);
        if (GetTime() - tempo_ataque >= 2.0f) {
            if (IsKeyPressed(KEY_SPACE)) {
                if (alvo >= 0) {
                    float dx = jogador1.pos.x - inimigos[alvo].x;
                    float dy = jogador1.pos.y - inimigos[alvo].y;
                    float distancia = sqrtf(dx * dx + dy * dy);

                    if (distancia < jogador1.raio_ataque) {
                        inimigos[alvo].vida -= jogador1.dano;
                        tempo_ataque = GetTime();

                        efeito_magia_alvo_id = inimigos[alvo].id;
                        tempo_magia = GetTime();

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
        manterNaTela(&jogador1.pos.x, &jogador1.pos.y);

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

            perseguir(inimigos, jogador1.pos.x, jogador1.pos.y, pquantidade);
            separarInimigos(inimigos, quantidade);

            if (GetTime() - tempo_ultimo_dano >= 2.0f) {

                int dano_acumulado = 0;
                bool levou_dano = false;

                for (int i = 0; i < quantidade; i++) {    
                    float dx = jogador1.pos.x - inimigos[i].x;
                    float dy = jogador1.pos.y - inimigos[i].y;
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

            for (int i = 0; i < linhas; i++) {
                for (int j = 0; j < colunas; j++) {
                    DrawTexture(grama, j * TAMANHO_CELULA, i * TAMANHO_CELULA, WHITE);
                    // mapa[i][j] pode, no futuro, indicar qual variação de tile desenhar aqui
                }
            }

            if (jogador1.estado == VIVO && !pausaMelhoria) {
                DrawText(jogador1.nome, 5, 10, 35, WHITE);
                DrawText(TextFormat ("%d|%d", jogador1.vida, jogador1.vida_max), 5, 50, 35, WHITE);
                DrawText(TextFormat ("Nivel:%d Exp:%d|%d",jogador1.nivel , experiencia, experiencia_max), 5, 80, 35, WHITE);
                DrawCircle(jogador1.pos.x, jogador1.pos.y, jogador1.raio_ataque, Fade(SKYBLUE, 0.05f));
                DrawCircleLines(jogador1.pos.x, jogador1.pos.y, jogador1.raio_ataque, PURPLE);
                Rectangle destinoJogador = {
                    jogador1.pos.x,
                    jogador1.pos.y,
                    jogador1.raio * ESCALA_JOGADOR,   // ajuste a escala visual como preferir
                    jogador1.raio * ESCALA_JOGADOR
                };
                Vector2 origemRotacaoJogador = { destinoJogador.width / 2, destinoJogador.height / 2 };

                DrawTexturePro(mago, origemJogador, destinoJogador, origemRotacaoJogador, 0.0f, WHITE);
                
                for (int i = 0; i < quantidade; i++)
                {
                    Rectangle origem = { 0, 0, (float)cogumelo.width, (float)cogumelo.height };
                    Rectangle destino = {
                        inimigos[i].x,
                        inimigos[i].y,
                        inimigos[i].raio * 2.5,
                        inimigos[i].raio * 2.5
                    };
                    Vector2 origemRotacao = { inimigos[i].raio, inimigos[i].raio }; // centraliza a textura na posição

                    DrawTexturePro(cogumelo, origem, destino, origemRotacao, 0.0f, WHITE);

                    //Mostra a vida dos inimigos//
                    DrawText(TextFormat("%d", inimigos[i].vida), inimigos[i].x + 10, inimigos[i].y + 15, 20, WHITE);
                }
                if (GetTime() - tempo_magia <= DURACAO_MAGIA) {
                    for (int i = 0; i < quantidade; i++) {
                        if (inimigos[i].id == efeito_magia_alvo_id) {
                            Rectangle origemMagia = { 0, 0, (float)magia.width, (float)magia.height };
                            Rectangle destinoMagia = {
                                inimigos[i].x + 7,
                                inimigos[i].y + 7,
                                inimigos[i].raio * 2.5f,
                                inimigos[i].raio * 2.5f
                            };
                            Vector2 origemRotacaoMagia = { destinoMagia.width / 2, destinoMagia.height / 2 };

                            DrawTexturePro(magia, origemMagia, destinoMagia, origemRotacaoMagia, 0.0f, WHITE);
                            break; // já achou, não precisa continuar procurando
                        }
                    }
                }
            } else if (jogador1.estado == VIVO && pausaMelhoria) {
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));

                    const char *titulo = "ESCOLHA UMA MELHORIA";
                    int fontSizeTitulo = 40;
                    int larguraTitulo = MeasureText(titulo, fontSizeTitulo);
                    DrawText(titulo, GetScreenWidth()/2 - larguraTitulo/2, 200, fontSizeTitulo, WHITE);

                    DrawText("[1] Vida       +10 vida maxima", GetScreenWidth()/2 - 150, 280, 25, WHITE);
                    DrawText("[2] Dano       +3 dano",         GetScreenWidth()/2 - 150, 320, 25, WHITE);
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
                DrawCircle(jogador1.pos.x, jogador1.pos.y, jogador1.raio, GRAY);
            }   
            EndDrawing();
    }      
    
    free(inimigos);
    liberarMatriz(mapa, linhas);
    UnloadTexture(grama);
    UnloadTexture(cogumelo);
    UnloadTexture(mago);
    UnloadTexture(magia);

    CloseWindow();

    return 0;
}