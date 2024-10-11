#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_messagebox.h>

#define MAX(x,y) (((x) > (y)) ? (x) : (y))

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int recordeSessao = 0;
int recordeGeral = 0;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
TTF_Font* fontmenor = NULL;

typedef struct {
    int x, y;
    int raio;
    float velocidade_Y;
    float velocidade_X; 
    Uint8 r, g, b, a;
    int encostaChao; 
} Bola;

Bola bola = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 70, 0, 0, 255, 0, 0, 255, 1}; 

const float GRAVIDADE = 500.0f; 
const float VEL_INICIAL = 600.0f; 
const float VEL_HORIZONTAL = 200.0f;

int cont = 0;

typedef enum {
    MENU_PRINCIPAL,
    MENU_JOGAR,
    MENU_SOBRE,
    JOGANDO,
    PAUSADO,
    SAIR
} EstadoMenu;

EstadoMenu estadoAtual = MENU_PRINCIPAL;

void desenhoBola(const Bola* bola) {
    filledCircleRGBA(renderer, bola->x, bola->y, bola->raio, bola->r, bola->g, bola->b, bola->a);
}

void movimentoBola(Bola* bola, Uint32 deltaTime) {
    bola->velocidade_Y += GRAVIDADE * deltaTime / 1000.0f;
    bola->y += bola->velocidade_Y * deltaTime / 1000.0f;
    bola->x += bola->velocidade_X * deltaTime / 1000.0f;

    if (bola->y + bola->raio > SCREEN_HEIGHT) {
        bola->y = SCREEN_HEIGHT - bola->raio;
        bola->velocidade_Y = 0;
        bola->encostaChao = 1;
        cont = 0;
    } else if (bola->y - bola->raio < 0) {
        bola->y = bola->raio;
        bola->velocidade_Y = 0;
    } else {
        bola->encostaChao = 0;
    }

    if (bola->x + bola->raio > SCREEN_WIDTH) {
        bola->x = SCREEN_WIDTH - bola->raio;
        bola->velocidade_X = -bola->velocidade_X;
    } else if (bola->x - bola->raio < 0) {
        bola->x = bola->raio;
        bola->velocidade_X = -bola->velocidade_X;
    }
}

void acaoClique(int mouseX, int mouseY) {
    int dx = mouseX - bola.x;
    int dy = mouseY - bola.y;
    float distanciaFormula = dx * dx + dy * dy;

    if (distanciaFormula <= bola.raio * bola.raio) {
        if (mouseY < bola.y - bola.raio / 3) {
            bola.velocidade_Y = -VEL_INICIAL * 0.5;
        } else if (mouseY > bola.y + bola.raio / 3) {
            bola.velocidade_Y = -VEL_INICIAL * 0.8;
        } else { 
            bola.velocidade_Y = -VEL_INICIAL;
        }

        if (mouseX < bola.x) { 
            bola.velocidade_X = VEL_HORIZONTAL;
        } else if (mouseX > bola.x) {
            bola.velocidade_X = -VEL_HORIZONTAL;
        }

        cont++;
        
        if (cont > recordeSessao) {
            recordeSessao = cont;
        }
        
        if (cont > recordeGeral) {
            recordeGeral = cont;
            FILE* arquivo = fopen("recorde_geral.txt", "w");
            if (arquivo) {
                fprintf(arquivo, "%d", recordeGeral);
                fclose(arquivo);    
            } else {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erro!", "Nao foi possivel salvar!", window);
            }
        }    
    }
}

void renderizaTexto(const char* text, int x, int y, SDL_Color corTexto) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, corTexto);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    int texW = 0; 
    int texH = 0;
    
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
}

void textoAuxiliar(const char* text, int x, int y, SDL_Color corTexto) {
    SDL_Surface* surface = TTF_RenderText_Solid(fontmenor, text, corTexto);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    int texW = 0; 
    int texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstRect = { x, y, texW, texH };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
}

int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms) {
    Uint32 antes = SDL_GetTicks();
    SDL_FlushEvent(SDL_MOUSEMOTION);
    if (SDL_WaitEventTimeout(evt, *ms)){
        *ms = MAX(0, *ms - (int)(SDL_GetTicks() - antes));
        return 1;
    }
    return 0;
}


void zerarRecordes() {
    recordeGeral = 0;
    recordeSessao = 0;
    
    FILE* arquivo = fopen("recorde_geral.txt", "w");
    if (arquivo) {
        fprintf(arquivo, "%d", recordeGeral);
        fclose(arquivo);
    } else {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erro!", "Nao foi possivel resetar!", window);
    }
}

void desenhaMenuPrincipal() {
    SDL_Color vermelho_pastel = { 255,180,180, 255 };
    SDL_Color laranja_pastel = { 255,225,180, 255 };
    SDL_Color azul_pastel = { 185,225,255, 255 };
            
    renderizaTexto("JOGAR", 350, 200, vermelho_pastel);
    renderizaTexto("SOBRE", 350, 300, laranja_pastel);
    renderizaTexto("SAIR", 350, 400, azul_pastel);
}

void desenhaTelaSobre() {
    SDL_Color azul_pastel = { 185,225,255, 255 };    
    SDL_Color branco = { 225,225,225, 255 };    
    SDL_Color corTexto = { 35,40,43, 255 };
        
    textoAuxiliar("Trabalho para a disciplina de Estruturas de linguagens 2024.1", 120, 120, branco);
    textoAuxiliar("Como jogar:",350,200,azul_pastel);    

    textoAuxiliar("Clique para manter a bola no ar (Nao deixe cair!!)",130,230,azul_pastel);    
    textoAuxiliar("Aperte ENTER para visualizar o recorde atual e global",130,260,azul_pastel);    
    textoAuxiliar("Aperte R para mudar a tela de fundo",130,290,azul_pastel); 
            
    renderizaTexto("VOLTAR", 350, 500, corTexto);   
}

void desenhaTelaPausado() {
    SDL_Color corTexto = { 255,180,180, 255 };
    SDL_Color opcoes = { 180,175,175, 255 };

    renderizaTexto("CONTINUAR", 320, 320, opcoes); //350 300 - 350 400
    renderizaTexto("MENU PRINCIPAL", 320, 350, opcoes);
}

void processarEventos(SDL_Event* evt, SDL_Texture** imgFundoAtual, int* fundoAux) {
    int mouseX = evt->button.x;
    int mouseY = evt->button.y;

    switch (estadoAtual) {
        case MENU_PRINCIPAL:
            if (evt->type == SDL_MOUSEBUTTONDOWN) {
                if (mouseY >= 200 && mouseY <= 240 && mouseX >= 350 && mouseX <= 450) {
                    estadoAtual = MENU_JOGAR;
                } else if (mouseY >= 300 && mouseY <= 340 && mouseX >= 350 && mouseX <= 450) {
                    estadoAtual = MENU_SOBRE;
                } else if (mouseY >= 400 && mouseY <= 440 && mouseX >= 350 && mouseX <= 450) {
                    estadoAtual = SAIR;
                }
            }
            break;

        case MENU_SOBRE:
            if (evt->type == SDL_MOUSEBUTTONDOWN) {
                if (mouseY >= 500 && mouseY <= 540 && mouseX >= 350 && mouseX <= 450) {
                    estadoAtual = MENU_PRINCIPAL;
                }
            }
            break;

        case MENU_JOGAR:
            estadoAtual = JOGANDO;   
            break;

        case JOGANDO:
            if (evt->type == SDL_MOUSEBUTTONDOWN) {
                acaoClique(mouseX, mouseY);
            }
            break;

        case PAUSADO:
            if (evt->type == SDL_MOUSEBUTTONDOWN) {
                if (mouseY >= 300 && mouseY <= 340 && mouseX >= 320 && mouseX <= 420) {
                    estadoAtual = JOGANDO;
                } else if (mouseY >= 350 && mouseY <= 390 && mouseX >= 320 && mouseX <= 400) {
                    estadoAtual = MENU_PRINCIPAL;
                }
            }
            break;

        default:
            break;
    }

    if (evt->type == SDL_KEYUP) { //antes era keydown
    	if(evt->key.keysym.sym == SDLK_r){
    		printf("\nFundoAux: %d",*fundoAux);
			(*fundoAux)++;
			switch(*fundoAux){
				case 1:
				    *imgFundoAtual = IMG_LoadTexture(renderer, "fundo1.png");
					printf("\nFundo1");
				break;
				case 2:
    				*imgFundoAtual= IMG_LoadTexture(renderer, "fundo2.png");
					printf("\nFundo2");
				break;
				case 3:
    				*imgFundoAtual = IMG_LoadTexture(renderer, "fundo3.png");
					printf("\nFundo3");
				break;
				default: 
					*fundoAux = 0;
			}
		}
        if (evt->key.keysym.sym == SDLK_ESCAPE) {
            if (estadoAtual == JOGANDO) {
                estadoAtual = PAUSADO;
            } else if (estadoAtual == PAUSADO) {
                estadoAtual = JOGANDO;
            } else if (estadoAtual == MENU_PRINCIPAL) {
                estadoAtual = MENU_JOGAR;
            }
        } else if (evt->key.keysym.sym == SDLK_SPACE && estadoAtual == JOGANDO) {
            Uint8 r = rand() % 256;
            Uint8 g = rand() % 256;
            Uint8 b = rand() % 256;
            
            bola.r = r;
            bola.g = g;
            bola.b = b;
        }else if (evt->key.keysym.sym == SDLK_RETURN){
            		// apertando tecla ENTER pressionada 
                    char mensagem[200];
                    snprintf(mensagem, sizeof(mensagem), "Recorde da sessão atual: %d\nRecorde Geral: %d\n\nResetar todas as pontuacoes?", recordeSessao, recordeGeral);
                    
                    SDL_MessageBoxButtonData buttons[2];
                    buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
                    buttons[0].buttonid = 0;
                    buttons[0].text = "Sim";

                    buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
                    buttons[1].buttonid = 1;
                    buttons[1].text = "Nao";

                    SDL_MessageBoxData messageboxdata;
                    SDL_zero(messageboxdata); //limpa a estrutura garantindo q esteja 0 
                    
                    messageboxdata.flags = SDL_MESSAGEBOX_INFORMATION;
                    messageboxdata.window = window;
                    messageboxdata.title = "Recordes";
                    messageboxdata.message = mensagem;
                    messageboxdata.numbuttons = 2;
                    messageboxdata.buttons = buttons;
                    messageboxdata.colorScheme = NULL;

                    int clicou_Botao;
                    if (SDL_ShowMessageBox(&messageboxdata, &clicou_Botao) == 0) {
                        if (clicou_Botao == 0) { //click sim
                            zerarRecordes();
                        }
                    } else {
                        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Erro", "Não foi possível exibir a mensagem", window);
                    }
              
            	};
    }
}

void mainLoop() {
    Uint32 startTime = SDL_GetTicks();
    Uint32 deltaTime = 0;
    Uint32 frameStart = startTime;
	int fundoAux = 0;
    int in_game = 1;
    SDL_Texture* imgFundoQuadra = IMG_LoadTexture(renderer, "fundo3.png");
    SDL_Texture* imgFundoCampo = IMG_LoadTexture(renderer, "fundo1.png");
    SDL_Texture* imgFundoAtual = imgFundoQuadra;

    SDL_Texture* imgFundoMenu = IMG_LoadTexture(renderer, "fundomenu.png");    
    SDL_Texture* imgFundoSobre = IMG_LoadTexture(renderer, "fundosobre.png");
    SDL_Texture* imgFundoPausa = IMG_LoadTexture(renderer, "fundopausa.png");
        
    while (in_game) {
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        frameStart = SDL_GetTicks();

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_Event event;
        Uint32 waitTime = 16;

        if (AUX_WaitEventTimeoutCount(&event, &waitTime)) {
            if (event.type == SDL_QUIT) {
                in_game = 0;
            } else {
                processarEventos(&event, &imgFundoAtual, &fundoAux);
            }
        }

	switch(estadoAtual){
		case MENU_PRINCIPAL:
            SDL_RenderCopy(renderer, imgFundoMenu, NULL, NULL);		
			desenhaMenuPrincipal();
			break;
		
		case MENU_SOBRE:
            SDL_RenderCopy(renderer, imgFundoSobre, NULL, NULL);		
			desenhaTelaSobre();
			break;
		
		case JOGANDO:
            SDL_RenderCopy(renderer, imgFundoAtual, NULL, NULL);
            movimentoBola(&bola, frameTime);
            desenhoBola(&bola);
            
            SDL_Color corTexto = { 0, 0, 0, 255 };
            char textoContador[50];
            snprintf(textoContador, sizeof(textoContador), "Contador: %d", cont);
            
            renderizaTexto(textoContador, 10, 10, corTexto);		
        
			break;
		
		case PAUSADO:
            SDL_RenderCopy(renderer, imgFundoPausa, NULL, NULL);		
            desenhaTelaPausado();
			break;
			
		case SAIR:
			in_game = 0;
			break;
		}	
		
        SDL_RenderPresent(renderer);
        deltaTime = SDL_GetTicks() - startTime;
        if (frameTime < 16) {
            SDL_Delay(16 - frameTime);
        }
    }

    SDL_DestroyTexture(imgFundoQuadra);
    SDL_DestroyTexture(imgFundoCampo);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("Erro ao inicializar SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("Confetti's SIDETURN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erro ao criar renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    font = TTF_OpenFont("Mont-HeavyDEMO.otf", 24);
    
    fontmenor = TTF_OpenFont("Mont-HeavyDEMO.otf", 18);
    if (!font) {
        printf("Erro ao carregar fonte: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!fontmenor) {
        printf("Erro ao carregar fonte: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    FILE* arquivo = fopen("recorde_geral.txt", "r");
    if (arquivo) {
        fscanf(arquivo, "%d", &recordeGeral);
        fclose(arquivo);
    } else {
        recordeGeral = 0;
    }

    mainLoop();

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
