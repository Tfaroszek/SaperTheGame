#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

/**
* @brief window do tworzenia okna
*/
SDL_Window* window = NULL;

/**
* @brief renderer do renderowania textur
*/
SDL_Renderer* renderer = NULL;

/**
* @brief texture do ladowania textur
*/
SDL_Texture* texture = NULL;

/**
* @brief windoww do tworzenia okna wynikow
*/
SDL_Window* windoww = NULL;

/**
* @brief startTime Zmienna od zegara
*/
/**
* @brief endTime Zmienna od zegara
*/
time_t startTime, endTime;

/**
* @brief textColor ladujaca kolor
*/
SDL_Color textColor = { 0, 255, 255, 255 };

#define SIZE 10
#define  WIDTHa  800
#define  HEIGHTa  800

/**
* @brief WIDTH wielkosci planszy
*/
int WIDTH = 10;

/**
* @brief HEIGHT  wielkosci planszy
*/
int HEIGHT = 10;

/**
* @brief BOMB_COUNT liczby biomb
*/
int BOMB_COUNT = 10;

/**
* @brief qpressed blokujaca po jednym uzyciu jedna z "pomocy"
*/
int qpressed = 1;

/**
* @brief wpressed blokujaca po jednym uzyciu jedna z "pomocy"
*/
int wpressed = 1;

/**
* @brief pomoce blokujaca "pomoce"
*/
bool pomoce = false;

/**
* @brief lista[] z nazwa plików do wynikow
*/
char lista[15] = "";

/**
* @brief Struktura odpowiedzialna za pole
*/
typedef struct {
    /**
    * @brief isBomb odpowiedzialna za czy pole ma bombe
    */
    int isBomb;

    /**
    * @brief isRevealed odpowiedzialna za czy pole jest odkryte
    */
    int isRevealed;

    /**
    * @brief bombsAround odpowiedzialna za czy pole sasiaduje z polami z bomba
    */
    int bombsAround;

    /**
    * @brief isFlagged odpowiedzialna za pole ma na sobie nalozona flage
    */
    int isFlagged;

    /**
    * @brief x odpowiedzialna za pozycja na dynamicznej macierzy
    */
    int x;

    /**
    * @brief y odpowiedzialna za pozycja na dynamicznej macierzy
    */
    int y;

    /**
    * @brief timeRevealed odpowiedzialna za tymczasowe odkrywanie planszy przy jeden z pomocy, odkrywajacej kwadrat 3x3
    */
    int timeRevealed;
} Field;


/**
* @brief Struktura odpowiedzialna za pozycje przyciskow w menu
*/
typedef struct {
    int pozycjax;
    int pozycjay;
    int pozycjax1;
    int pozycjay1;
    int pozycjax2;
    int pozycjay2;
    int pozycjax3;
    int pozycjay3;
    int pozycjax4;
    int pozycjay4;
} Pos;

Pos position;

/**
* @brief Struktura odpowiedzialna za nick i wynik gracza
*/
typedef struct {
    char nick[50];
    char time[50];
} Record;

/**
* @brief Funkcja odpowiedzialna za porownywanie wynikow
*/
int compare_records(const void* a, const void* b) {
    const Record* record_a = (const Record*)a;
    const Record* record_b = (const Record*)b;
    return strcmp(record_a->time, record_b->time);
}

// Tablica dwuwymiarowa przechowująca pola
Field** fields;

/**
* @brief gameOver odpowiedzialna za przegrana
*/
int gameOver = 0;

/**
* @brief gameWon odpowiedzialna za wygrana
*/
int gameWon = 0;

/**
* @brief isStart odpowiedzialna za przelaczanie sie miedzy odpowiednim menu w petli
*/
int isStart = 0;

/**
* @brief fontArial wskaznik odpowiedzialny za wyswietlanie czczcionki
*/
TTF_Font* fontArial;

/**
* @brief buffer[] odpowiedzialna za przechowywanie danych w odczycie plikow
*/
char buffer[50];

SDL_AudioDeviceID deviceId;

/**
* @brief Funkcja odpowiedzialna za odtwarzanie muzyki
*/
void playMusicLoop(const char* filename) {
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec wavSpec;
    Uint8* wavBuffer;
    Uint32 wavLength;

    if (SDL_LoadWAV(filename, &wavSpec, &wavBuffer, &wavLength) == NULL) {
        fprintf(stderr, "Nie można załadować pliku WAV: %s\n", SDL_GetError());
        return;
    }

    deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    if (deviceId == 0) {
        fprintf(stderr, "Nie można otworzyć urządzenia audio: %s\n", SDL_GetError());
        return;
    }

    int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    if (success < 0) {
        fprintf(stderr, "Błąd podczas kształtowania dźwięku: %s\n", SDL_GetError());
        return;
    }

    SDL_PauseAudioDevice(deviceId, 0);
}

/**
* @brief Funkcja odpowiedzialna za zatrzymywanie muzyki
*/
void stopMusic() {
    SDL_PauseAudioDevice(deviceId, 1);
    SDL_CloseAudioDevice(deviceId);

}

/**
* @brief Funkcja odpowiedzialna za wyswietlanie bledow
*/
int showErrorMessageBox(char* message) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
    return 1;
}

/**
* @brief Funkcja odpowiedzialna za czekanie wcisniecie klawisza ESC aby wyjsc z programu/gry
*/
void waitForQuit() {
    SDL_Event event;
    while (true) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyWindow(window);

                SDL_DestroyWindow(windoww);
                SDL_Quit();
                exit(0);

                return;

            } if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                isStart = 5;
                gameWon = 0;
                gameOver = 0;
                return;

            }

        }
    }
}

/**
* @brief Funkcja odpowiedzialna za zapisywanie wynikow do konkretnych pilkow, w zaleznosci od granej gry
*/
void zapis(const char* nick, const char* time) {
    // otwarcie pliku do odczytu
    FILE* fp;
    if (BOMB_COUNT == 10 && pomoce == false) {
        const char* newLista = "lista.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 10 && pomoce == true) {
        const char* newLista = "listaP.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 30 && pomoce == false) {
        const char* newLista = "listas.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 30 && pomoce == true) {
        const char* newLista = "listasP.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 60 && pomoce == false) {
        const char* newLista = "listad.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 60 && pomoce == true) {
        const char* newLista = "listadP.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 60 && pomoce == false && BOMB_COUNT != 30 && BOMB_COUNT != 10) {
        const char* newLista = "listal.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT != 60 && pomoce == true && BOMB_COUNT != 30 && BOMB_COUNT != 10) {
        const char* newLista = "listalP.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (fopen_s(&fp, lista, "r") != 0) {
        printf("Nie udalo sie otworzyc pliku do odczytu.");
        return;
    }
    // wczytanie istniejących rekordów z pliku
    Record records[SIZE];
    int num_records = 0;
    while (fscanf_s(fp, "%s Sekundy: %s", records[num_records].nick, sizeof(records[num_records].nick), records[num_records].time, sizeof(records[num_records].time)) == 2) {
        num_records++;
        if (num_records >= SIZE) {
            break;
        }
    }
    fclose(fp);
    // dodanie nowego rekordu
    if (num_records < SIZE) {
        strcpy_s(records[num_records].nick, nick);
        strcpy_s(records[num_records].time, time);
        num_records++;
    }
    // sortowanie rekordów
    qsort(records, num_records, sizeof(Record), compare_records);
    // otwarcie pliku do zapisu
    if (fopen_s(&fp, lista, "w") != 0) {
        printf("Nie udalo sie otworzyc pliku do zapisu.");
        return;
    }
    // zapis posortowanych rekordów do pliku
    for (int i = 0; i < num_records; i++) {
        fprintf(fp, "%s Sekundy: %s\n", records[i].nick, records[i].time);
    }
    // zamknięcie pliku
    fclose(fp);
}

/**
* @brief Funkcja odpowiedzialna za odczyt wynikow do konkretnych pilkow, w zaleznosci od granej gry
*/
int wyniki() {

    SDL_Surface* surface = NULL;
    SDL_Surface* text_surface = NULL;
    TTF_Font* font = NULL;
    FILE* fp;
    SDL_Event event;
    if (BOMB_COUNT == 10 && pomoce == false) {
        const char* newLista = "lista.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 10 && pomoce == true) {
        const char* newLista = "listaP.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 30 && pomoce == false) {
        const char* newLista = "listas.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 30 && pomoce == true) {
        const char* newLista = "listasP.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 60 && pomoce == false) {
        const char* newLista = "listad.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 60 && pomoce == true) {
        const char* newLista = "listadP.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT == 60 && pomoce == false && BOMB_COUNT != 30 && BOMB_COUNT != 10) {
        const char* newLista = "listal.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (BOMB_COUNT != 60 && pomoce == true && BOMB_COUNT != 30 && BOMB_COUNT != 10) {
        const char* newLista = "listalP.txt";
        strcpy_s(lista, sizeof(lista), newLista);
    }
    if (fopen_s(&fp, lista, "r") != 0) {
        printf("Nie udalo sie otworzyc pliku .");
        return 1;
    }
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END); // ustaw wskaźnik na koniec pliku
        long size = ftell(fp); // odczytaj aktualną pozycję wskaźnika, czyli długość pliku
        if (size == 0) {
            printf("brak wynikow.\n");
        }
        else {
            fseek(fp, 0, SEEK_SET);
            // Tworzenie bufora i odczytywanie danych z pliku
            char* buffer = (char*)malloc(size * sizeof(char));
            fread(buffer, sizeof(char), size, fp);

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    SDL_DestroyWindow(windoww);
                    SDL_Quit();
                    exit(0);
                }
            }
            fclose(fp);
            // Inicjalizacja SDL i SDL_ttf
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                printf("Nie udalo sie zainicjowac SDL!");
                return 1;
            }
            if (TTF_Init() < 0) {
                printf("Nie udalo sie zainicjowac SDL_ttf!");
                return 1;
            }
            // Importowanie pliku czcionki TrueType
            font = TTF_OpenFont("arial.ttf", 24);
            if (font == NULL) {
                printf("Nie udalo sie zaimportowac czcionki!");
                return 1;
            }
            // Tworzenie okna
            windoww = SDL_CreateWindow("lista wynikow", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
            if (windoww == NULL) {
                printf("Nie udalo sie stworzyc okna!");
                return 1;
            }
            // Tworzenie powierzchni
            surface = SDL_GetWindowSurface(windoww);
            if (surface == NULL) {
                printf("Nie udalo sie stworzyc powierzchni!");
                return 1;
            }
            char* line = strtok_s(NULL, "\n\n", &buffer); // Podział bufora na linie
            int y = 50; // Pozycja tekstu na osi Y
            while (line != NULL) {
                text_surface = TTF_RenderText_Solid(font, line, textColor); // Tworzenie powierzchni z tekstem
                SDL_Rect text_rect = { 50, y, 0, 0 }; // Pozycja tekstu na ekranie
                SDL_BlitSurface(text_surface, NULL, surface, &text_rect); // Wklejenie powierzchni tekstu na powierzchnię ekranu
                y += 30; // Zwiększenie pozycji tekstu na osi Y dla kolejnej linii
                line = strtok_s(NULL, "\n", &buffer); // Odczytanie kolejnej linii z bufora
            }
            // Wyświetlenie tekstu na powierzchni
            SDL_Rect text_rect = { 50, 50, 0, 0 }; // Pozycja tekstu na ekranie
            //  SDL_BlitSurface(text_surface, NULL, surface, &text_rect);
            SDL_UpdateWindowSurface(windoww);
        }
    }
    while (isStart != 5)
        waitForQuit();
}

/**
* @brief Funkcja odpowiedzialna za inicjowanie dynamiczne rozmiarow planszy oraz przydzielanie odpowiednich stanow dla pol
*/
void initFields() {
    fields = (Field**)malloc(WIDTHa * sizeof(Field*));
    int i, j;
    for (i = 0; i < WIDTH; i++) {
        fields[i] = (Field*)malloc(HEIGHTa * sizeof(Field));
        for (j = 0; j < HEIGHT; j++) {
            fields[i][j].isBomb = 0;
            fields[i][j].isRevealed = 0;
            fields[i][j].bombsAround = 0;
            fields[i][j].isFlagged = 0;
            fields[i][j].x = i;
            fields[i][j].y = j;
        }
    }
}

/**
* @brief Funkcja odpowiedzialna za wyswietalnie odpowiedniego napisu w zaleznosci czy gra zostala wygrana lub przegrana
*/
void wonlose(SDL_Renderer* renderer, int x) {
    SDL_Texture* texturee;

    time(&endTime);
    if (x == 0) {
        SDL_Rect rect = { (WIDTHa / 2) - (369 / 2), (HEIGHTa / 2) - 59 , 369, 59 };
        texturee = IMG_LoadTexture(renderer, "grafiki/winwin.png");
        SDL_RenderCopy(renderer, texturee, NULL, &rect);
        int i, j;
        for (i = 0; i < WIDTH; i++) {
            for (j = 0; j < HEIGHT; j++) {
                if (fields[i][j].isFlagged && !fields[i][j].isBomb) {
                    fields[i][j].isRevealed = 1;
                }
            }
        }

    }
    else {
        SDL_Rect rect = { (WIDTHa / 2) - (390 / 2), (HEIGHTa / 2) - 59 , 390, 59 };
        texturee = IMG_LoadTexture(renderer, "grafiki/przegrales.png");
        SDL_RenderCopy(renderer, texturee, NULL, &rect);
        int i, j;
        for (i = 0; i < WIDTH; i++) {
            for (j = 0; j < HEIGHT; j++) {
                if (fields[i][j].isFlagged && !fields[i][j].isBomb) {
                    fields[i][j].isRevealed = 1;
                }
            }
        }
    }
    TTF_SetFontStyle(fontArial, TTF_STYLE_NORMAL);
    SDL_Color textColor = { 0, 255, 255, 255 };
    SDL_Surface* timeSurface = TTF_RenderText_Solid(fontArial, "Time: ", textColor);
    SDL_Surface* secondsSurface = TTF_RenderText_Solid(fontArial, SDL_itoa((int)difftime(endTime, startTime), buffer, 10), textColor);
    SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, timeSurface);
    SDL_Texture* secondsTexture = SDL_CreateTextureFromSurface(renderer, secondsSurface);
    SDL_Rect timeRect = { (WIDTHa / 2) - 100, static_cast<int>(HEIGHTa / 1.9), timeSurface->w, timeSurface->h };//tutaj rzutowanie aby niwelowanc warningi
    SDL_Rect secondsRect = { (WIDTHa / 2) - 100 + timeSurface->w, static_cast<int>(HEIGHTa / 1.9), secondsSurface->w, secondsSurface->h };//tutaj rzutowanie aby niwelowanc warningi

    SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);
    SDL_RenderCopy(renderer, secondsTexture, NULL, &secondsRect);

    SDL_FreeSurface(timeSurface);
    SDL_FreeSurface(secondsSurface);
    SDL_DestroyTexture(timeTexture);
    SDL_DestroyTexture(texturee);
    SDL_DestroyTexture(secondsTexture);
    SDL_RenderPresent(renderer);
}

/**
* @brief Funkcja odpowiedzialna za igenerowanie losowo bomb na planszy
*/
void generateBombs() {
    int bombsPlaced = 0;//zmienia do petli
    while (bombsPlaced < BOMB_COUNT) {//petla losujaca pola, w ktorych bedzie bomba, robi to tyle razy ile jest ustawionych bomb
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;
        if (fields[x][y].isBomb == 0) {//petla losujaca pola, w ktorych bedzie bomba, robi to tyle razy ile jest ustawionych bomb
            fields[x][y].isBomb = 1;
            bombsPlaced++;//zwieksza ilosc pol ktore juz zostaly oznaczone bomba
        }
    }
}

/**
* @brief Funkcja odpowiedzialna za obliczanie ilosci bomb wokol konkretnego pola (zeby pozniej wyswietlic, po odkryciu pola, odpowiednia liczbe na polu)
*/
void calculateBombsAround() {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            int bombsAround = 0;
            for (int x = i - 1; x <= i + 1; x++) {
                for (int y = j - 1; y <= j + 1; y++) {
                    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                        if (fields[x][y].isBomb == 1) {
                            bombsAround++;
                        }
                    }
                }
            }
            fields[i][j].bombsAround = bombsAround;
        }
    }
}

/**
* @brief Funkcja odpowiedzialna za odkrycie konkretnego pola i odkrywanie innych jesli nie ma wokol zadnych bomb
*/
void revealField(int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        if (fields[x][y].isRevealed == 0 && fields[x][y].isFlagged == 0) {

            fields[x][y].isRevealed = 1;

            if (fields[x][y].bombsAround == 0 && fields[x][y].isFlagged == 0) {
                revealField(x - 1, y - 1);
                revealField(x - 1, y);
                revealField(x - 1, y + 1);
                revealField(x, y - 1);
                revealField(x, y + 1);
                revealField(x + 1, y - 1);
                revealField(x + 1, y);
                revealField(x + 1, y + 1);
            }
        }
    }
}

/**
* @brief Funkcja odpowiedzialna za odkrywanie calej planszy
*/
void viewFields() {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            revealField(i, j);
            if (fields[i][j].isRevealed == 0 && fields[i][j].isFlagged == 1 && fields[i][j].isBomb == 0) {
                fields[i][j].isRevealed = 1;
            }
        }
    }
}

/**
* @brief Funkcja odpowiedzialna za sprawdzanie czy gra zostala wygrana
*/
void checkWin(SDL_Renderer* renderer) {
    int revealedFields = 0;//zmienia do petli zliczajaca odkryte pola
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            if (fields[i][j].isRevealed == 1) {//warunek sprawdzajacy czy pole jest odkryte, czy nie
                revealedFields++;//zwieksza ilosc zliczonych odkrytych pol
            }
        }
    }
    if (revealedFields == WIDTH * HEIGHT - BOMB_COUNT) {
        viewFields();//Wyswietlanie calej planszy po zakonczeniu gry
        gameWon = 1;//zmienna gameWon na 1
    }
}

/**
* @brief Funkcja odpowiedzialna za wyswietalnie wyswietalnie odpowiedniej tektusty w zaleznosci od stanu pola
*/
void drawField(SDL_Renderer* renderer, Field field) {
    int x = field.x * (WIDTHa / WIDTH);
    int y = field.y * (HEIGHTa / HEIGHT);
    int size = WIDTHa / WIDTH;
    int i = field.x;
    int j = field.y;
    SDL_Rect rect = { x, y, size, size };
    SDL_Texture* texture;
    //warunki sprawdzajace w jakim stanie znajduje sie pole i wyswietlajaca dana grafike w zaleznosci od stanu
    if (field.isRevealed == 0) {
        texture = IMG_LoadTexture(renderer, "grafiki/podstawa.png");
        if (field.isFlagged == 1) {
            texture = IMG_LoadTexture(renderer, "grafiki/flag.png");
        }
    }
    else {
        if (field.isBomb == 1) {
            texture = IMG_LoadTexture(renderer, "grafiki/bomba.png");
        }
        else {
            texture = IMG_LoadTexture(renderer, "grafiki/podstawa2.png");

            if (field.isFlagged == 1) {
                texture = IMG_LoadTexture(renderer, "grafiki/flag.png");
            }
            if (fields[i][j].bombsAround == 0) {
                texture = IMG_LoadTexture(renderer, "grafiki/podstawa2.png");
            }
            if (fields[i][j].bombsAround == 1) {
                texture = IMG_LoadTexture(renderer, "grafiki/1.png");
            }
            if (fields[i][j].bombsAround == 2) {
                texture = IMG_LoadTexture(renderer, "grafiki/2.png");

            }
            if (fields[i][j].bombsAround == 3) {
                texture = IMG_LoadTexture(renderer, "grafiki/3.png");
            }
            if (fields[i][j].bombsAround == 4) {
                texture = IMG_LoadTexture(renderer, "grafiki/4.png");
            }
            if (fields[i][j].bombsAround == 5) {
                texture = IMG_LoadTexture(renderer, "grafiki/5.png");
            }
            if (fields[i][j].bombsAround == 6) {
                texture = IMG_LoadTexture(renderer, "grafiki/6.png");
            }
            if (fields[i][j].bombsAround == 7) {
                texture = IMG_LoadTexture(renderer, "grafiki/7.png");
            }
            if (fields[i][j].bombsAround == 8) {
                texture = IMG_LoadTexture(renderer, "grafiki/8.png");
            }
        }
    }
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    if (gameOver) {
        //viewFields();
        SDL_RenderPresent(renderer);
    }
    if (gameWon) {
        // viewFields();
        SDL_RenderPresent(renderer);
    }
}


/**
* @brief Funkcja odpowiedzialna za wyswietlanie calej planszy
*/
void drawFields(SDL_Renderer* renderer) {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            drawField(renderer, fields[i][j]);
        }
    }
}

/**
* @brief Funkcja odpowiedzialna za obsluge klikniec myszy na pola, czyli okrywanie ich
*/
void handleClick(int x, int y, SDL_Renderer* renderer) {
    int X = x / (WIDTHa / WIDTH);
    int Y = y / (HEIGHTa / HEIGHT);
    if (fields[X][Y].isBomb == 1) {
        if (fields[X][Y].isFlagged == 0) {
            viewFields();
            gameOver = 1;
        }
    }
    else {
        revealField(X, Y);
        checkWin(renderer);
    }
}

/**
* @brief Funkcja odpowiedzialna za sprawdzanie czy dane pola jest bomba, jest ona wykorzystywana do obslugi flag
*/
void czybomba(int x, int y, SDL_Renderer* renderer) {
    if (fields[x][y].isBomb == 1) {
        if (fields[x][y].isFlagged == 0) {
            viewFields();
            gameOver = 1;
        }

    }
    else {
        revealField(x, y);
        checkWin(renderer);
    }
}

/**
* @brief Funkcja odpowiedzialna za jedna z funkcji "Pomocy", generuje od poczatku nieodkryte pola na planszy
*/
void generateAgain() {
    int bombs = 0;
    int bombsPlaced = 0;
    int i, j;
    for (i = 0; i < WIDTH; i++) {
        for (j = 0; j < HEIGHT; j++) {
            if (fields[i][j].isRevealed == 0) {
                if (fields[i][j].isBomb == 1) {
                    bombs++;
                    fields[i][j].isBomb = 0;
                }
            }
        }
    }
    while (bombsPlaced < bombs) {
        int i = rand() % WIDTH;
        int j = rand() % HEIGHT;
        if (fields[i][j].isBomb == 0 && fields[i][j].isRevealed == 0) {
            fields[i][j].isBomb = 1;
            bombsPlaced++;
        }
    }
}
int fieldXX = 0;
int fieldYY = 0;

/**
* @brief Struktura odpowiedzialna za timer w "Pomocach" w funkcjach timerCallback i reavelTime
*/
typedef struct {
    SDL_TimerID timerID;
    Uint32 startTime;
} TimerData;

/**
* @brief Funkcja odpowiedzialna za zakrywanie pola 3x3, po czasie 0.2sekundy, przy wlaczonej opcji "Pomoce"
*/
Uint32 timerCallback(Uint32 interval, void* param)
{
    TimerData* timerData = (TimerData*)param;

    printf(" 5 ");
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - timerData->startTime;

    if (elapsedTime >= 200)
    {
        SDL_RemoveTimer(timerData->timerID);
    }
    for (int i = fieldXX - 1; i <= fieldXX + 1; i++)
    {
        for (int j = fieldYY - 1; j <= fieldYY + 1; j++)
        {
            if (i >= 0 && i < WIDTH && j >= 0 && j < HEIGHT)
            {
                if (fields[i][j].timeRevealed == 1)
                {
                    fields[i][j].isRevealed = 0;
                    fields[i][j].timeRevealed = 0;
                }
                else
                {
                    fields[i][j].isRevealed = 1;
                    fields[i][j].timeRevealed = 0;
                }
            }
        }
    }
    return 0;
}


/**
* @brief Funkcja odpowiedzialna za okrywaniepola 3x3, przy wlaczonej opcji "Pomoce"
*/
void revealTime()
{

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    int size = WIDTHa / WIDTH;
    fieldXX = mouseX / size;
    fieldYY = mouseY / size;

    SDL_Init(SDL_INIT_TIMER);
    TimerData timerData;
    SDL_TimerID timerID = SDL_AddTimer(200, timerCallback, &timerData); // Ustawia timer na 200ms (0.2 sekund)

    Uint32 startTime = SDL_GetTicks();
    // Pobierz czas rozpoczęcia programu

    // Odkryj pola w zasięgu 3x3 wokół pozycji kursora myszy
    for (int i = fieldXX - 1; i <= fieldXX + 1; i++)
    {
        for (int j = fieldYY - 1; j <= fieldYY + 1; j++)
        {
            if (i >= 0 && i < WIDTH && j >= 0 && j < HEIGHT)
            {
                if (fields[i][j].isRevealed == 0)
                {
                    fields[i][j].timeRevealed = 1;
                    fields[i][j].isRevealed = 1;
                }
            }
        }
    }
}

/**
* @brief Funkcja odpowiedzialna za obsluge "eventow" na planszy, prawe klikniecie- obsluga flag, lewe klikniecie- odkrycie bomby i odkrycie bomb wokol pola, klawisz w- generateAgain(), klawisz q- okrycie pola 3x3 na 0.2sekundy
*/
void handleEvents(SDL_Event* event, SDL_Renderer* renderer) {

    while (SDL_PollEvent(event) != 0) {

        switch (event->type) {

        case SDL_QUIT:
            gameOver = 1;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT) {
                handleClick(event->button.x, event->button.y, renderer);
            }
            if (event->button.button == SDL_BUTTON_RIGHT) {
                int x = event->button.x / (WIDTHa / WIDTH);
                int y = event->button.y / (HEIGHTa / HEIGHT);
                if (fields[x][y].isRevealed == 0) {
                    if (fields[x][y].isFlagged == 0) {
                        fields[x][y].isFlagged = 1;
                    }
                    else if (fields[x][y].isFlagged == 1) {
                        fields[x][y].isFlagged = 0;
                    }
                }
                if (fields[x][y].isRevealed == 1) {

                    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                        if (x != 0) {
                            czybomba(x - 1, y, renderer);
                        }
                        if (y != 0)
                        {
                            czybomba(x, y - 1, renderer);
                        }
                        if (x != WIDTH - 1 && y != 0) {
                            czybomba(x + 1, y - 1, renderer);
                        }
                        if (x != 0 && y != HEIGHT - 1) {
                            czybomba(x - 1, y + 1, renderer);
                        }
                        if (y != 0 && x != 0) {
                            czybomba(x - 1, y - 1, renderer);
                        }

                        if (x != WIDTH - 1)
                        {
                            czybomba(x + 1, y, renderer);
                        }

                        if (y != HEIGHT - 1 && x != WIDTH - 1) {
                            czybomba(x + 1, y + 1, renderer);
                        }

                        if (y != HEIGHT - 1)
                        {
                            czybomba(x, y + 1, renderer);
                        }
                    }
                }
            }
            break;
        case SDL_KEYDOWN:
            if (event->key.keysym.sym == SDLK_ESCAPE) {
                isStart = 5;
                gameWon = 0;
                gameOver = 0;
                return;

            }

            if (pomoce) {
                if (event->key.keysym.sym == SDLK_w) {
                    if (wpressed == 1)
                    {
                        printf("Key 'W' pressed\n");
                        generateAgain();
                        calculateBombsAround();
                        wpressed = 0;
                    }
                }
                if (event->key.keysym.sym == SDLK_q) {

                    if (qpressed == 1)
                    {
                        printf("Key 'Q' pressed\n");
                        revealTime();
                        qpressed = 0;
                    }
                }
            }
            break;
        }
    }
}

int mouseX, mouseY;
int imgX, imgY;
int imgWidth, imgHeight;
int quit = 0;

/**
* @brief Funkcja odpowiedzialna za ladowanie grafik menu
*/
int LoadImage(const char grafika[], SDL_Renderer* renderer, SDL_Texture** texture, int* imgWidth, int* imgHeight)
{
    SDL_Surface* surface = IMG_Load(grafika);
    if (!surface)
    {
        printf("Nie można wczytać obrazka: %s\n", IMG_GetError());
        return 0;
    }

    // Tworzenie tekstury z powierzchni
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!(*texture))
    {
        printf("Nie można stworzyć tekstury: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return 0;
    }

    // Pobieranie wymiarów obrazka
    *imgWidth = surface->w;
    *imgHeight = surface->h;

    SDL_FreeSurface(surface);
    return 1;
}

/**
* @brief Funkcja odpowiedzialna za renderowanie grafik menu na ekranie
*/
void RenderImage(SDL_Renderer* renderer, SDL_Texture* texture, int pozycjax, int pozycjay, int imgWidth, int imgHeight)
{
    // Ustalanie pozycji obrazka
    SDL_Rect destRect = { pozycjax, pozycjay, imgWidth, imgHeight };

    // Rysowanie obrazka na ekranie
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
}


// Funkcja główna
int main(int argc, char* argv[]) {
    while (isStart != 8) {
        playMusicLoop("BoomBoom.wav");
        while (isStart != 2) {
            SDL_RenderClear(renderer);
            if (SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                printf("Nie można zainicjować SDL: %s\n", SDL_GetError());
                return 1;
            }
            // Tworzenie okna
            window = SDL_CreateWindow("SAPER THE GAME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTHa, HEIGHTa, SDL_WINDOW_SHOWN);
            if (!window)
            {
                printf("Nie można stworzyć okna: %s\n", SDL_GetError());
                SDL_Quit();
                return 1;
            }
            // Tworzenie renderera
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer)
            {
                printf("Nie można stworzyć renderera: %s\n", SDL_GetError());
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 1;
            }
            // Inicjalizacja SDL_image
            int imgFlags = IMG_INIT_PNG;
            if (!(IMG_Init(imgFlags) & imgFlags))
            {
                printf("Nie można zainicjować SDL_image: %s\n", IMG_GetError());
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 1;
            }
            SDL_Event event;
            while (isStart != 1 && isStart != 4) {
                while (SDL_PollEvent(&event) != 0)
                {
                    if (event.type == SDL_QUIT)
                    {
                        quit = 1;
                    }
                    else if (event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        // Sprawdzanie czy kliknięcie nastąpiło na obrazku
                        if (event.button.x >= 300 && event.button.x <= 300 + position.pozycjax &&
                            event.button.y >= 500 && event.button.y <= 500 + position.pozycjay)
                        {

                            isStart = 1;

                        }
                        if (event.button.x >= 230 && event.button.x <= 230 + position.pozycjax1 &&
                            event.button.y >= 570 && event.button.y <= 570 + position.pozycjay1)
                        {

                            isStart = 4;

                        }
                        if (event.button.x >= 300 && event.button.x <= 300 + position.pozycjax2 &&
                            event.button.y >= 630 && event.button.y <= 630 + position.pozycjay2)
                        {
                            if (pomoce) {
                                pomoce = false;
                            }
                            else {
                                pomoce = true;
                            }
                        }
                        if (event.button.x >= 570 && event.button.x <= 570 + position.pozycjax3 &&
                            event.button.y >= 630 && event.button.y <= 630 + position.pozycjay3)
                        {
                            if (pomoce) {
                                pomoce = false;
                            }
                            else {
                                pomoce = true;
                            }
                        }
                        if (event.button.x >= 230 && event.button.x <= 230 + position.pozycjax4 &&
                            event.button.y >= 700 && event.button.y <= 700 + position.pozycjay4)
                        {
                            SDL_DestroyWindow(window);
                            SDL_Quit();
                        }
                    }
                    SDL_GetMouseState(&mouseX, &mouseY);
                    SDL_RenderClear(renderer);

                    /**
                    * @brief success odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success = LoadImage("grafiki/SaperMenuTlo.png", renderer, &texture, &imgWidth, &imgHeight);
                    if (success)
                    {
                        RenderImage(renderer, texture, 0, 0, imgWidth, imgHeight);
                    }

                    /**
                    * @brief success2 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success2 = LoadImage("grafiki/Graj.png", renderer, &texture, &position.pozycjax, &position.pozycjay);
                    if (success2)
                    {
                        RenderImage(renderer, texture, 300, 500, position.pozycjax, position.pozycjay);
                    }

                    /**
                    * @brief success3 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success3 = LoadImage("grafiki/Wyjscie.png", renderer, &texture, &position.pozycjax4, &position.pozycjay4);
                    if (success3)
                    {
                        RenderImage(renderer, texture, 230, 700, position.pozycjax4, position.pozycjay4);
                    }

                    /**
                    * @brief success4 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success4 = LoadImage("grafiki/Instrukcja.png", renderer, &texture, &position.pozycjax1, &position.pozycjay1);
                    if (success4)
                    {
                        RenderImage(renderer, texture, 230, 570, position.pozycjax1, position.pozycjay1);
                    }


                    /**
                    * @brief success5 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success5 = LoadImage("grafiki/Pomoce.png", renderer, &texture, &position.pozycjax2, &position.pozycjay2);
                    if (success5)
                    {
                        RenderImage(renderer, texture, 300, 630, position.pozycjax2, position.pozycjay2);
                    }


                    if (mouseX >= 300 && mouseX <= 300 + position.pozycjax &&
                        mouseY >= 500 && mouseY <= 500 + position.pozycjay)
                    {
                        /**
                        * @brief success21  odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success21 = LoadImage("grafiki/GrajNeon.png", renderer, &texture, &position.pozycjax, &position.pozycjay);
                        if (success21)
                        {
                            RenderImage(renderer, texture, 300, 500, position.pozycjax, position.pozycjay);
                        }
                    }
                    if (mouseX >= 230 && mouseX <= 230 + position.pozycjax4 &&
                        mouseY >= 700 && mouseY <= 700 + position.pozycjay4)
                    {
                        /**
                        * @brief success22 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success22 = LoadImage("grafiki/WyjscieNeon.png", renderer, &texture, &position.pozycjax4, &position.pozycjay4);
                        if (success22)
                        {
                            RenderImage(renderer, texture, 230, 700, position.pozycjax4, position.pozycjay4);
                        }
                    }

                    if (mouseX >= 230 && mouseX <= 230 + position.pozycjax1 &&
                        mouseY >= 570 && mouseY <= 570 + position.pozycjay1)
                    {
                        /**
                        * @brief success23 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success23 = LoadImage("grafiki/InstrukcjaNeon.png", renderer, &texture, &position.pozycjax1, &position.pozycjay1);
                        if (success23)
                        {
                            RenderImage(renderer, texture, 230, 570, position.pozycjax1, position.pozycjay1);
                        }
                    }

                    if (mouseX >= 300 && mouseX <= 300 + position.pozycjax2 &&
                        mouseY >= 630 && mouseY <= 630 + position.pozycjay2)
                    {
                        /**
                        * @brief success24 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success24 = LoadImage("grafiki/PomoceNeon.png", renderer, &texture, &position.pozycjax2, &position.pozycjay2);
                        if (success24)
                        {
                            RenderImage(renderer, texture, 300, 630, position.pozycjax2, position.pozycjay2);
                        }
                    }

                    if (pomoce) {
                        /**
                        * @brief success6 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success6 = LoadImage("grafiki/CheckMarkFajfka.png", renderer, &texture, &position.pozycjax3, &position.pozycjay3);
                        if (success6)
                        {

                            RenderImage(renderer, texture, 570, 630, position.pozycjax3, position.pozycjay3);

                        }
                    }

                    if (!pomoce) {
                        /**
                        * @brief success7 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success7 = LoadImage("grafiki/CheckMarkX.png", renderer, &texture, &position.pozycjax3, &position.pozycjay3);
                        if (success7)
                        {
                            RenderImage(renderer, texture, 570, 630, position.pozycjax3, position.pozycjay3);
                        }
                    }
                    SDL_RenderPresent(renderer);
                }
                SDL_DestroyTexture(texture);
            }
            if (isStart == 4) {
                while (isStart != 3) {
                    SDL_Event event;
                    while (SDL_PollEvent(&event) != 0)
                    {
                        if (event.type == SDL_QUIT)
                        {
                            SDL_DestroyWindow(window);
                            SDL_Quit();
                        }
                        else if (event.type == SDL_MOUSEBUTTONDOWN)
                        {
                            // Sprawdzanie czy kliknięcie nastąpiło na obrazku

                            if (event.button.x >= 230 && event.button.x <= 230 + imgWidth &&
                                event.button.y >= 600 && event.button.y <= 600 + imgHeight)
                            {
                                isStart = 3;
                                SDL_DestroyWindow(window);
                            }
                        }
                        SDL_GetMouseState(&mouseX, &mouseY);

                        // Czyszczenie ekranu
                        SDL_RenderClear(renderer);

                        /**
                        * @brief success odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success = LoadImage("grafiki/WyporPoziomow.png", renderer, &texture, &imgWidth, &imgHeight);
                        if (success)
                        {
                            RenderImage(renderer, texture, 0, 0, imgWidth, imgHeight);
                        }

                        /**
                        * @brief success2 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success2 = LoadImage("grafiki/Tutorial.png", renderer, &texture, &imgWidth, &imgHeight);
                        if (success2)
                        {
                            RenderImage(renderer, texture, 0, 0, imgWidth, imgHeight);
                        }

                        /**
                        * @brief success7 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success7 = LoadImage("grafiki/Back.png", renderer, &texture, &imgWidth, &imgHeight);
                        if (success7)
                        {
                            RenderImage(renderer, texture, 230, 600, imgWidth, imgHeight);
                        }
                        if (mouseX >= 230 && mouseX <= 230 + imgWidth &&
                            mouseY >= 600 && mouseY <= 600 + imgHeight)
                        {
                            /**
                            * @brief success21 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                            */
                            int success21 = LoadImage("grafiki/BackNeon.png", renderer, &texture, &imgWidth, &imgHeight);
                            if (success21)
                            {
                                RenderImage(renderer, texture, 230, 600, imgWidth, imgHeight);
                            }
                        }
                        SDL_RenderPresent(renderer);
                    }

                }
            }
            while (isStart != 2 && isStart != 3) {
                SDL_Event event;
                while (SDL_PollEvent(&event) != 0)
                {
                    if (event.type == SDL_QUIT)
                    {
                        SDL_DestroyWindow(window);
                        SDL_Quit();
                    }
                    else if (event.type == SDL_MOUSEBUTTONDOWN)
                    {
                        // Sprawdzanie czy kliknięcie nastąpiło na obrazku
                        if (event.button.x >= 300 && event.button.x <= 300 + position.pozycjax &&
                            event.button.y >= 200 && event.button.y <= 200 + position.pozycjay)
                        {

                            WIDTH = 10;
                            HEIGHT = 10;

                            isStart = 2;
                            printf("%d", isStart);
                            // quit = 1;

                        }
                        if (event.button.x >= 230 && event.button.x <= 230 + position.pozycjax1 &&
                            event.button.y >= 300 && event.button.y <= 300 + position.pozycjay1)
                        {
                            isStart = 2;
                            printf("%d", isStart);

                            WIDTH = 15;
                            HEIGHT = 15;
                            BOMB_COUNT = 30;
                        }
                        if (event.button.x >= 300 && event.button.x <= 300 + position.pozycjax2 &&
                            event.button.y >= 400 && event.button.y <= 400 + position.pozycjay2)
                        {
                            isStart = 2;
                            printf("%d", isStart);

                            WIDTH = 20;
                            HEIGHT = 20;
                            BOMB_COUNT = 60;
                        }
                        if (event.button.x >= 230 && event.button.x <= 230 + imgWidth &&
                            event.button.y >= 600 && event.button.y <= 600 + imgHeight)
                        {
                            isStart = 3;
                            SDL_DestroyWindow(window);
                        }
                        if (event.button.x >= 285 && event.button.x <= 285 + position.pozycjax3 &&
                            event.button.y >= 500 && event.button.y <= 500 + position.pozycjay3)
                        {
                            srand(time(NULL));
                            WIDTH = HEIGHT = rand() % 31 + 10;
                            BOMB_COUNT = HEIGHT * WIDTH / 5;
                            isStart = 2;
                        }
                    }
                    SDL_RenderClear(renderer);
                    SDL_GetMouseState(&mouseX, &mouseY);
                    /**
                    * @brief success odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success = LoadImage("grafiki/WyporPoziomow.png", renderer, &texture, &imgWidth, &imgHeight);
                    if (success)
                    {
                        RenderImage(renderer, texture, 0, 0, imgWidth, imgHeight);
                    }

                    /**
                    * @brief success2 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success2 = LoadImage("grafiki/Wybor.png", renderer, &texture, &imgWidth, &imgHeight);
                    if (success2)
                    {
                        RenderImage(renderer, texture, 150, 50, imgWidth, imgHeight);
                    }

                    /**
                    * @brief success3 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success3 = LoadImage("grafiki/Mala.png", renderer, &texture, &position.pozycjax, &position.pozycjay);
                    if (success3)
                    {
                        RenderImage(renderer, texture, 300, 200, position.pozycjax, position.pozycjay);
                    }

                    /**
                    * @brief success4 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success4 = LoadImage("grafiki/Srednia.png", renderer, &texture, &position.pozycjax1, &position.pozycjay1);
                    if (success4)
                    {
                        RenderImage(renderer, texture, 230, 300, position.pozycjax1, position.pozycjay1);
                    }
                    /**
                    * @brief success5 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success5 = LoadImage("grafiki/Duza.png", renderer, &texture, &position.pozycjax2, &position.pozycjay2);
                    if (success5)
                    {
                        RenderImage(renderer, texture, 300, 400, position.pozycjax2, position.pozycjay2);
                    }
                    /**
                    * @brief success6 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success6 = LoadImage("grafiki/Dostosuj.png", renderer, &texture, &position.pozycjax3, &position.pozycjay3);
                    if (success6)
                    {
                        RenderImage(renderer, texture, 285, 500, position.pozycjax3, position.pozycjay3);
                    }
                    /**
                    * @brief success7 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                    */
                    int success7 = LoadImage("grafiki/Back.png", renderer, &texture, &imgWidth, &imgHeight);
                    if (success7)
                    {
                        RenderImage(renderer, texture, 230, 600, imgWidth, imgHeight);
                    }

                    if (mouseX >= 300 && mouseX <= 300 + position.pozycjax &&
                        mouseY >= 200 && mouseY <= 200 + position.pozycjay)
                    {
                        /**
                        * @brief success21 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success21 = LoadImage("grafiki/MalaNeon.png", renderer, &texture, &position.pozycjax, &position.pozycjay);
                        if (success21)
                        {
                            RenderImage(renderer, texture, 300, 200, position.pozycjax, position.pozycjay);
                        }
                    }

                    if (mouseX >= 230 && mouseX <= 230 + position.pozycjax1 &&
                        mouseY >= 300 && mouseY <= 300 + position.pozycjay1)
                    {
                        /**
                        * @brief success22 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success22 = LoadImage("grafiki/SredniaNeon.png", renderer, &texture, &position.pozycjax1, &position.pozycjay1);
                        if (success22)
                        {
                            RenderImage(renderer, texture, 230, 300, position.pozycjax1, position.pozycjay1);
                        }

                    }

                    if (mouseX >= 300 && mouseX <= 300 + position.pozycjax2 &&
                        mouseY >= 400 && mouseY <= 400 + position.pozycjay2)
                    {
                        /**
                        * @brief success23 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success23 = LoadImage("grafiki/DuzaNeon.png", renderer, &texture, &position.pozycjax2, &position.pozycjay2);
                        if (success23)
                        {
                            RenderImage(renderer, texture, 300, 400, position.pozycjax2, position.pozycjay2);
                        }
                    }

                    if (mouseX >= 285 && mouseX <= 285 + position.pozycjax3 &&
                        mouseY >= 500 && mouseY <= 500 + position.pozycjay3)
                    {
                        /**
                        * @brief success24 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success24 = LoadImage("grafiki/DostosujNeon.png", renderer, &texture, &position.pozycjax3, &position.pozycjay3);
                        if (success24)
                        {
                            RenderImage(renderer, texture, 285, 500, position.pozycjax3, position.pozycjay3);
                        }
                    }
                    if (mouseX >= 230 && mouseX <= 230 + imgWidth &&
                        mouseY >= 600 && mouseY <= 600 + imgHeight)
                    {
                        /**
                        * @brief success21 odpowiedzialna za sprawdzanie czy grafika sie zaladowala
                        */
                        int success21 = LoadImage("grafiki/BackNeon.png", renderer, &texture, &imgWidth, &imgHeight);
                        if (success21)
                        {
                            RenderImage(renderer, texture, 230, 600, imgWidth, imgHeight);
                        }
                    }
                    SDL_RenderPresent(renderer);
                }
                // Czyszczenie zasobów
                SDL_DestroyTexture(texture);
            }
        }
        if (isStart == 2) {

            while (isStart != 5) {
                SDL_RenderClear(renderer);
                SDL_Init(SDL_INIT_VIDEO);
                time(&startTime);
                TTF_Init();

                fontArial = TTF_OpenFont("arial.ttf", 25);
                if (fontArial == NULL) {
                    printf("Nie mozna zaladowac czcionki: %s\n", TTF_GetError());
                    SDL_Quit();
                    exit(1);
                }
                srand(static_cast<unsigned int>(time(NULL)));//zniwelowanie warningu       
                SDL_Event event;
                initFields();
                generateBombs();
                calculateBombsAround();

                if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
                    return 1;
                }

                if (window == NULL) {
                    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
                    return 1;
                }
                if (renderer == NULL) {
                    printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                    return 1;
                }

                while (!gameOver && !gameWon && isStart != 5) {

                    handleEvents(&event, renderer);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);
                    drawFields(renderer);
                    SDL_RenderPresent(renderer);
                    if (gameOver) {

                        wonlose(renderer, 1);
                        SDL_MessageBoxButtonData buttons[] = {
              { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Nie" },
              { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Tak" },
                        };
                        SDL_MessageBoxData messageboxdata = {
                            SDL_MESSAGEBOX_INFORMATION, /* typ okna dialogowego */
                            window, /* wskaźnik na okno SDL (NULL, aby użyć domyślnego okna) */
                            "Przegrana!", /* tytuł okna */
                            "Czy chcesz zobaczyć tabele wyników?", /* treść wiadomości */
                            SDL_arraysize(buttons), /* liczba przycisków */
                            buttons, /* wskaźnik na tablicę przycisków */
                            NULL /* wskaźnik na ikonę (NULL, aby użyć domyślnej ikony) */
                        };
                        int choice = 0;
                        if (SDL_ShowMessageBox(&messageboxdata, &choice) < 0) {
                            SDL_Log("Nie można wyświetlić okna dialogowego: %s\n", SDL_GetError());
                        }

                        if (choice == 1) {

                            wyniki();

                        }

                    }
                    if (gameWon) {
                        wonlose(renderer, 0);

                        SDL_MessageBoxButtonData buttons[] = {
                   { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Nie" },
                   { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Tak" },
                        };

                        SDL_MessageBoxData messageboxdata = {
                            SDL_MESSAGEBOX_INFORMATION, /* typ okna dialogowego */
                            window, /* wskaźnik na okno SDL (NULL, aby użyć domyślnego okna) */
                            "Wygrana!", /* tytuł okna */
                            "Gratulacje, wygrałeś grę! Chcesz zapisać swój wynik?", /* treść wiadomości */
                            SDL_arraysize(buttons), /* liczba przycisków */
                            buttons, /* wskaźnik na tablicę przycisków */
                            NULL /* wskaźnik na ikonę (NULL, aby użyć domyślnej ikony) */
                        };

                        /**
                        * @brief Zmienna odpowiedzialna za wybor przy wyrazaniu checi wyswietlenia okna z wynikami gry
                        */
                        int choice = 0;
                        if (SDL_ShowMessageBox(&messageboxdata, &choice) < 0) {
                            SDL_Log("Nie można wyświetlić okna dialogowego: %s\n", SDL_GetError());
                        }

                        if (choice == 1) { // Wybór "Tak"

                            char nick[100] = "";
                            SDL_StartTextInput();

                            while (1) {
                                SDL_Event event;
                                SDL_PollEvent(&event);

                                if (event.type == SDL_TEXTINPUT) {
                                    strcat_s(nick, event.text.text);
                                }
                                else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                                    break;
                                }
                            }
                            SDL_RenderPresent(renderer);
                            sprintf_s(buffer, "%ld\n", endTime - startTime); // Konwersja czasu na napis
                            zapis(nick, buffer);
                            SDL_StopTextInput();
                        }

                        wyniki();
                    }
                }
                waitForQuit();
                TTF_CloseFont(fontArial);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
            }
        }
    }
    return 0;
}