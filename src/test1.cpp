#include <chrono>

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <Windows.h>
#include <math.h>

bool RunningGame = true;

const int iScreenWidth = 120;
const int iScreenHeight = 40;

const int mapWidth = 28;
const int mapHeight = 31;

int playerX = 15;
int playerY = 23;

const int maxLives = 3;
int Lives = 3;

int TotalDots = 0;
int Dots = 0;





using namespace std;

enum PlayerState{
    PAC,
    MAN
};

enum Direction : char{
    UP = 'W',
    LEFT = 'A',
    DOWN = 'S',
    RIGHT = 'D',
    STOP = '-'
};

enum GhostPhase{
    CHASE_PHASE,
    SCATTER_PHASE,
    FRIGHTENED_PHASE
};

Direction lastMoveBuffer = STOP;
Direction moveBuffer = STOP;
PlayerState pState = PAC;



GhostPhase ghostPhase = SCATTER_PHASE;
int stage = 1;


wchar_t mapPiece[18];

void PlayerInput();

bool movePlayer(wstring map, std::chrono::steady_clock::time_point* cooldownRate, Direction* moveBuffer);

void ResetPlayer();

struct GhostNode{
    int posX;
    int posY;
    bool isSpecial = false;
};

struct Ghost{
    int DefaultPosX;
    int DefaultPosY;
    int posX;
    int posY;
    wchar_t character;
    int minDotsToLeave;
    bool leftSpawn = false;

    void ResetGhost(){
        posX = DefaultPosX;
        posY = DefaultPosY;
    }
};

Ghost* ghosts[4];

int FindDistance(int x, int y, int DistX, int DistY){
    int dist = sqrt(pow((float)DistX - (float)x, 2) + pow((float)DistY - (float)y, 2));
    return dist;
}

int main(){
    // Create Screen Buffer
	wchar_t *screen = new wchar_t[iScreenWidth*iScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
    
    Ghost Blinky = {};
    Blinky.posX = 12;
    Blinky.DefaultPosX = Blinky.posX;
    Blinky.posY = 14;
    Blinky.DefaultPosY = Blinky.posY;
    Blinky.minDotsToLeave = 1;
    Blinky.character = L'♥';

    Ghost Pinkie = {};
    Pinkie.posX = 13;
    Pinkie.DefaultPosX = Pinkie.posX;
    Pinkie.posY = 14;
    Pinkie.DefaultPosY = Pinkie.posY;
    Pinkie.minDotsToLeave = 10;
    Pinkie.character = L'♦';

    Ghost Inky = {};
    Inky.posX = 14;
    Inky.DefaultPosX = Inky.posX;
    Inky.posY = 14;
    Inky.DefaultPosY = Inky.posY;
    Inky.minDotsToLeave = 20;
    Inky.character = L'♣';

    Ghost Clyde = {};
    Clyde.posX = 15;
    Clyde.DefaultPosX = Clyde.posX;
    Clyde.posY = 14;
    Clyde.DefaultPosY = Clyde.posY;
    Clyde.minDotsToLeave = 30;
    Clyde.character = L'♠';

    ghosts[0] = &Blinky;
    ghosts[1] = &Pinkie;
    ghosts[2] = &Inky;
    ghosts[3] = &Clyde;

    mapPiece[0] = L'╔';
    mapPiece[1] = L'╗';
    mapPiece[2] = L'╚';
    mapPiece[3] = L'╝';
    mapPiece[4] = L'║';
    mapPiece[5] = L'═';
    mapPiece[6] = L'╦';
    mapPiece[7] = L'╠';
    mapPiece[8] = L'╩';
    mapPiece[9] = L'╣';
    mapPiece[10] = L'╬';
    mapPiece[11] = '-';
    mapPiece[12] = L'┌';
    mapPiece[13] = L'┐';
    mapPiece[14] = L'└';
    mapPiece[15] = L'┘';
    mapPiece[16] = L'─';
    mapPiece[17] = L'│';


    wstring map;
    
    map += L"╔════════════╗╔════════════╗";
    map += L"║            ║║            ║";
    map += L"║ ┌──┐ ┌───┐ ║║ ┌───┐ ┌──┐ ║";
    map += L"║@│xx│ │xxx│ ║║ │xxx│ │xx│@║";
    map += L"║ └──┘ └───┘ ╚╝ └───┘ └──┘ ║";
    map += L"║                          ║";
    map += L"║ ┌──┐ ┌┐ ┌──────┐ ┌┐ ┌──┐ ║";
    map += L"║ └──┘ ││ └──┐┌──┘ ││ └──┘ ║";
    map += L"║      ││    ││    ││      ║";
    map += L"╚════╗ │└──┐ ││ ┌──┘│ ╔════╝";
    map += L"xxxxx║ │┌──┘ └┘ └──┐│ ║xxxxx";
    map += L"xxxxx║ ││          ││ ║xxxxx";
    map += L"xxxxx║ ││ ╔══--══╗ ││ ║xxxxx";
    map += L"═════╝ └┘ ║xxxxxx║ └┘ ╚═════";
    map += L"x         ║xxxxxx║         x";
    map += L"═════╗ ┌┐ ║xxxxxx║ ┌┐ ╔═════";
    map += L"xxxxx║ ││ ╚══════╝ ││ ║xxxxx";
    map += L"xxxxx║ ││          ││ ║xxxxx";
    map += L"xxxxx║ ││ ┌──────┐ ││ ║xxxxx";
    map += L"╔════╝ └┘ └──┐┌──┘ └┘ ╚════╗";
    map += L"║            ││            ║";
    map += L"║ ┌──┐ ┌───┐ ││ ┌───┐ ┌──┐ ║";
    map += L"║ └─┐│ └───┘ └┘ └───┘ │┌─┘ ║";
    map += L"║@  ││                ││  @║";
    map += L"╚═╗ ││ ┌┐ ┌──────┐ ┌┐ ││ ╔═╝";
    map += L"╔═╝ └┘ ││ └──┐┌──┘ ││ └┘ ╚═╗";
    map += L"║      ││    ││    ││      ║";
    map += L"║ ┌────┘└──┐ ││ ┌──┘└────┐ ║";
    map += L"║ └────────┘ └┘ └────────┘ ║";
    map += L"║                          ║";
    map += L"╚══════════════════════════╝";
    
    auto cooldown = chrono::steady_clock::now();
    auto cooldownRate = chrono::steady_clock::now();

    for(int i = 0; i < mapWidth; i++){
        for(int j = 0; j < mapHeight; j++){
            if(map[j * mapWidth + i] == ' '){
                map[j * mapWidth + i] = '.';
                Dots++;
            }
        }
    }

    TotalDots = Dots;

    //clear screen
    for(int nx = 0; nx < iScreenWidth; nx++){
        for(int ny = 0; ny < iScreenHeight; ny++){
            screen[(int)ny * (int)iScreenWidth + (int)nx] = ' ';
        }
    }

    screen[iScreenWidth * iScreenHeight - 1] = '\0';
    WriteConsoleOutputCharacterW(hConsole, screen, iScreenWidth * iScreenHeight, { 0,0 }, &dwBytesWritten);

    while(RunningGame){
        cooldown = chrono::steady_clock::now();
        chrono::duration<float> dCooldownTime = cooldown - cooldownRate;
        float cooldownTime = dCooldownTime.count();

        // Player Input
        PlayerInput();

        // Move & Collision

        if(cooldownTime > 0.095f){

            if(!movePlayer(map, &cooldownRate, &moveBuffer)){
                if(!movePlayer(map, &cooldownRate, &lastMoveBuffer)){
                    moveBuffer = STOP;
                }
            }else{
                lastMoveBuffer = STOP;
            }

            if(pState == PAC){
                pState = MAN;
            }else{
                pState = PAC;
            }

            // Use Edge
            if(playerX == 0 && playerY == 14 && moveBuffer == LEFT){
                playerX = 27;
            }else if(playerX == 27 && playerY == 14 && moveBuffer == RIGHT){
                playerX = 0;
            }

            if(map[playerY * mapWidth + playerX] == '.'){
                map[playerY * mapWidth + playerX] = ' ';
                Dots--;
            }
            else if(map[playerY * mapWidth + playerX] == '@'){
                map[playerY * mapWidth + playerX] = ' ';
                ghostPhase = FRIGHTENED_PHASE;
            }
        
        }
        cooldown = cooldownRate;

        for(int nx = 0; nx < mapWidth; nx++){
            for(int ny = 0; ny < mapHeight; ny++){
                screen[ny * iScreenWidth + nx] = map[ny * mapWidth + nx];

                if(map[ny * mapWidth + nx] == 'x'){
                    screen[ny * iScreenWidth + nx] = ' ';
                }
            }
        }

        if(pState == PAC){
            screen[playerY * iScreenWidth + playerX] = 'C';
        }else{
            screen[playerY * iScreenWidth + playerX] = 'O';
        }


        // Update ghosts

        for(auto ghost : ghosts){
            screen[ghost->posY * iScreenWidth + ghost->posX] = ghost->character;

            if(playerX == ghost->posX && playerY == ghost->posY && ghostPhase == FRIGHTENED_PHASE){
                ghost->ResetGhost();
            }else if(playerX == ghost->posX && playerY == ghost->posY && ghostPhase && ghostPhase != FRIGHTENED_PHASE){
                ResetPlayer();
                Lives--;
            }

            if(ghost->minDotsToLeave <= TotalDots-Dots && !ghost->leftSpawn){
                ghost->posX = 13,
                ghost->posY = 11;
                ghost->leftSpawn = true;
            }

            //Move Ghost

        }



        const char* _LivesText = "Lives:";

        for(int i = 0; i <= sizeof(_LivesText)/sizeof(wchar_t)+1; i++){
            screen[(mapWidth + 3) * iScreenWidth + i + 2] = _LivesText[i];
        }

        for(int i = 1; i < maxLives+1; i++){
            if(i <= Lives){
                screen[(mapWidth + 3) * iScreenWidth + i + 9 ] = L'♥';
            }else{
                screen[(mapWidth + 3) * iScreenWidth + i + 9 ] = ' ';
            }
        }

        if(Dots == 0){
            RunningGame = false;
        }else if(Lives == 0){
            RunningGame = false;
        }

        swprintf_s(screen, 40, L"Px:%d, Py:%d, Gx:%d", playerX, playerY, ghostPhase);

        screen[iScreenWidth * iScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacterW(hConsole, screen, iScreenWidth * iScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    return 0;
}

void PlayerInput(){
    if(GetAsyncKeyState((unsigned short)'W') & 0x8000){
        if(moveBuffer != UP){
            lastMoveBuffer = moveBuffer;
        }
        moveBuffer = UP;
    }
    else if(GetAsyncKeyState((unsigned short)'S') & 0x8000){
        if(moveBuffer != DOWN){
            lastMoveBuffer = moveBuffer;
        }
        moveBuffer = DOWN;
    }
    else if(GetAsyncKeyState((unsigned short)'A') & 0x8000){
        if(moveBuffer != LEFT){
            lastMoveBuffer = moveBuffer;
        }
        moveBuffer = LEFT;
    }
    else if(GetAsyncKeyState((unsigned short)'D') & 0x8000){
        if(moveBuffer != RIGHT){
            lastMoveBuffer = moveBuffer;
        }
        moveBuffer = RIGHT;
    }
}

bool movePlayer(wstring map, std::chrono::steady_clock::time_point* cooldownRate, Direction* moveBuffer){
    switch(*moveBuffer){
        case UP:
            {
                auto obj = std::find(std::begin(mapPiece), std::end(mapPiece), map[(playerY- 1 ) * mapWidth + playerX]);

                if(obj != std::end(mapPiece)){
                    return false;
                }else{
                    playerY--;
                }

                *cooldownRate = chrono::steady_clock::now();
                break;
            }
        case LEFT:
            {
                auto obj = std::find(std::begin(mapPiece), std::end(mapPiece), map[(playerY) * mapWidth + playerX - 1]);
                
                if(obj != std::end(mapPiece)){
                    return false;
                }else{
                    playerX--;
                }

                *cooldownRate = chrono::steady_clock::now();
                break;
            }
        case DOWN:
            {
                auto obj = std::find(std::begin(mapPiece), std::end(mapPiece), map[(playerY + 1) * mapWidth + playerX]);
                
                if(obj != std::end(mapPiece)){
                    return false;
                }else{
                    playerY++;
                }

                *cooldownRate = chrono::steady_clock::now();
                break;
            }
        case RIGHT:
            {
                auto obj = std::find(std::begin(mapPiece), std::end(mapPiece), map[(playerY) * mapWidth + playerX + 1]);
                
                if(obj != std::end(mapPiece)){
                    return false;
                }else{
                    playerX++;
                }

                *cooldownRate = chrono::steady_clock::now();
                break;
            }
        default:
            return false;
    }

    return true;
}

void ResetPlayer(){
    playerX = 15;
    playerY = 23;
}


