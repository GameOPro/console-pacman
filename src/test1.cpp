#include <chrono>

#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <Windows.h>
#include <math.h>
#include <random>

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

bool energizerOn = false;

using namespace std;

chrono::steady_clock::time_point startTime;
chrono::steady_clock::time_point playerCooldown;
chrono::steady_clock::time_point phaseCooldown;
chrono::steady_clock::time_point energizerCooldown;

constexpr int phaseCooldownRate = 7; // seconds
constexpr int energizerCooldownRate = 10; // seconds
constexpr int playerCooldownRate = 100; // milliseconds
constexpr int ghostCooldownRate = 150; // milliseconds
constexpr int ghostRespawnTime = 5; // seconds




enum PlayerState{
    PAC,
    MAN
};

enum Direction{
    UP,
    LEFT,
    DOWN,
    RIGHT,
    STOP
};

enum GhostPhase{
    CHASE_PHASE,
    SCATTER_PHASE,
    FRIGHTENED_PHASE
};

Direction lastMoveBuffer = STOP;
Direction moveBuffer = STOP;
PlayerState pState = PAC;

int stage = 1;
wchar_t mapPiece[18];


struct GhostNode{
    int posX;
    int posY;
    int isSpecial = false;
};

struct Ghost{
    const string name;
    const int defaultPosX;
    const int defaultPosY;
    int posX;
    int posY;
    const wchar_t character;
    int minDotsToLeave;
    bool leftSpawn = false;
    Direction lastDirection;
    GhostPhase phase = SCATTER_PHASE;
    int targetX = 0;
    int targetY = 0;
    chrono::steady_clock::time_point ghostCooldown;
    chrono::steady_clock::time_point deathCooldown;

    void ResetGhost(){
        posX = defaultPosX;
        posY = defaultPosY;
        leftSpawn = false;
        lastDirection = STOP;
        deathCooldown = startTime + chrono::seconds(ghostRespawnTime);

        if(startTime >= phaseCooldown){
            phase = CHASE_PHASE;
        }else{
            phase = SCATTER_PHASE;
        }
    }
};

void PlayerInput();

bool movePlayer(wstring map, Direction* moveBuffer);

void moveGhost(unique_ptr<Ghost>& ghost, GhostPhase phase, wstring map);

void ResetPlayer();

float FindDistance(const int x, const int y, const int DistX, const int DistY);

void setTarget(unique_ptr<Ghost>& ghost, GhostPhase phase);
bool isValidDirection(Direction direction, wstring map,  int Y, int X);

vector<unique_ptr<Ghost>> ghosts(4);
GhostNode nodes[34];


int main(){
    // Create Screen Buffer
	wchar_t *screen = new wchar_t[iScreenWidth*iScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
    
    // Init stuff
    srand((unsigned) time(NULL));

    Ghost Blinky = {"Blinky", 12, 14, 12, 14, L'♥', 1, false, STOP, SCATTER_PHASE, 0, 0, startTime + chrono::milliseconds(ghostCooldownRate), startTime + chrono::seconds(ghostRespawnTime)};

    Ghost Pinkie = {"Pinkie", 13, 14, 13, 14, L'♦', 10, false, STOP, SCATTER_PHASE, 0, 0, startTime + chrono::milliseconds(ghostCooldownRate), startTime + chrono::seconds(ghostRespawnTime)};

    Ghost Inky = {"Inky", 14, 14, 14, 14, L'♣', 20, false, STOP, SCATTER_PHASE, 0, 0, startTime + chrono::milliseconds(ghostCooldownRate), startTime + chrono::seconds(ghostRespawnTime)};

    Ghost Clyde = {"Clyde", 15, 14, 15, 14, L'♠', 30, false, STOP, SCATTER_PHASE, 0, 0, startTime + chrono::milliseconds(ghostCooldownRate), startTime + chrono::seconds(ghostRespawnTime)};

    ghosts[0] = make_unique<Ghost>(Blinky);
    ghosts[1] = make_unique<Ghost>(Pinkie);
    ghosts[2] = make_unique<Ghost>(Inky);
    ghosts[3] = make_unique<Ghost>(Clyde);

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

    // Set all 34 nodes
    
    nodes[0] = {6, 1};
    nodes[1] = {21, 1};
    nodes[2] = {1, 5};
    nodes[3] = {6, 5};
    nodes[4] = {9, 5};
    nodes[5] = {12, 5};
    nodes[6] = {15, 5};
    nodes[7] = {18, 5};
    nodes[8] = {21, 5 };
    nodes[9] = {26, 5};
    nodes[10] = {6, 8};
    nodes[11] = {21, 8};
    nodes[12] = {12, 11, true};
    nodes[13] = {15, 11, true};
    nodes[14] = {6, 14};
    nodes[15] = {9, 14};
    nodes[16] = {18, 14};
    nodes[17] = {21, 14};
    nodes[18] = {9, 17};
    nodes[19] = {18, 17};
    nodes[20] = {6, 20};
    nodes[21] = {9, 20};
    nodes[22] = {18, 20};
    nodes[23] = {21, 20};
    nodes[24] = {6, 23};
    nodes[25] = {9, 23};
    nodes[26] = {12, 23, true};
    nodes[27] = {15, 23, true};
    nodes[28] = {18, 23};
    nodes[29] = {21, 23};
    nodes[30] = {3, 26};
    nodes[31] = {24, 26};
    nodes[32] = {12, 29};
    nodes[33] = {15, 29};

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
    map += L"╚════╗ │└──┐x││x┌──┘│ ╔════╝";
    map += L"xxxxx║ │┌──┘x└┘x└──┐│ ║xxxxx";
    map += L"xxxxx║ ││xxxxxxxxxx││ ║xxxxx";
    map += L"xxxxx║ ││x╔══--══╗x││ ║xxxxx";
    map += L"═════╝ └┘x║xxxxxx║x└┘ ╚═════";
    map += L"xxxxxx xxx║xxxxxx║xxx xxxxxx";
    map += L"═════╗ ┌┐x║xxxxxx║x┌┐ ╔═════";
    map += L"xxxxx║ ││x╚══════╝x││ ║xxxxx";
    map += L"xxxxx║ ││xxxxxxxxxx││ ║xxxxx";
    map += L"xxxxx║ ││x┌──────┐x││ ║xxxxx";
    map += L"╔════╝ └┘x└──┐┌──┘x└┘ ╚════╗";
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

    startTime = chrono::steady_clock::now();
    playerCooldown = startTime + chrono::milliseconds(playerCooldownRate);
    phaseCooldown = startTime + chrono::seconds(phaseCooldownRate);
    energizerCooldown = startTime + chrono::seconds(energizerCooldownRate);

    for(int i = 0; i < mapWidth; i++){
        for(int j = 0; j < mapHeight; j++){
            if(map[j * mapWidth + i] == ' '){
                map[j * mapWidth + i] = '.';
                Dots++;
            }
        }
    }

    TotalDots = Dots;

    //clear screen (Needed for some reason)
    for(int nx = 0; nx < iScreenWidth; nx++){
        for(int ny = 0; ny < iScreenHeight; ny++){
            screen[(int)ny * (int)iScreenWidth + (int)nx] = ' ';
        }
    }

    screen[iScreenWidth * iScreenHeight - 1] = '\0';
    WriteConsoleOutputCharacterW(hConsole, screen, iScreenWidth * iScreenHeight, { 0,0 }, &dwBytesWritten);

    while(RunningGame){
        startTime = chrono::steady_clock::now();

        // Player Input
        PlayerInput();

        // Move & Collision

        if(startTime >= playerCooldown){

            if(!movePlayer(map, &moveBuffer)){
                if(!movePlayer(map, &lastMoveBuffer)){
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

            // Use Edge teleport
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
                energizerOn = true;
                for(auto& ghost : ghosts){
                    ghost->phase = FRIGHTENED_PHASE;
                }
                energizerCooldown = startTime + chrono::seconds(energizerCooldownRate);
            }
        }

        if(energizerOn && startTime >= energizerCooldown){
            energizerOn = false;
            for(auto& ghost : ghosts){
                if(startTime >= phaseCooldown){
                    ghost->phase = CHASE_PHASE;
                }else{
                    ghost->phase = SCATTER_PHASE;
                }
            }
        }

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

        for(auto& ghost : ghosts){
            screen[ghost->posY * iScreenWidth + ghost->posX] = ghost->character;

            if(startTime >= phaseCooldown && (ghost->phase != CHASE_PHASE && ghost->phase != FRIGHTENED_PHASE) && !energizerOn){
                ghost->phase = CHASE_PHASE;
            }

            if(playerX == ghost->posX && playerY == ghost->posY && ghost->phase == FRIGHTENED_PHASE){
                ghost->ResetGhost();
            }else if(playerX == ghost->posX && playerY == ghost->posY && ghost->phase != FRIGHTENED_PHASE){
                ResetPlayer();
                phaseCooldown = startTime + chrono::seconds(phaseCooldownRate);
                for(auto& ghost : ghosts){
                    ghost->ResetGhost();
                }
                Lives--;
            }

            if(ghost->minDotsToLeave <= TotalDots-Dots && !ghost->leftSpawn && startTime >= ghost->deathCooldown){
                ghost->posX = 13,
                ghost->posY = 11;
                ghost->leftSpawn = true;
                ghost->lastDirection = RIGHT;
            }

            //Move Ghost
            if(startTime >= ghost->ghostCooldown && ghost->lastDirection != STOP){
                moveGhost(ghost, ghost->phase, map);

                 // Use Edge teleport
                if(ghost->posX == 0 && ghost->posY == 14 && ghost->lastDirection == LEFT){
                    ghost->posX = 27;
                }else if(ghost->posX == 27 && ghost->posY == 14 && ghost->lastDirection == RIGHT){
                    ghost->posX = 0;
                }
            }
            
        }

        const string _LivesText = "Lives:";

        for(unsigned long long i = 0; i <= _LivesText.length(); i++){
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

        swprintf_s(screen, 40, L"Energized:%d", energizerOn);

        screen[iScreenWidth * iScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacterW(hConsole, screen, iScreenWidth * iScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    delete[] screen;
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

bool movePlayer(wstring map, Direction* moveBuffer){
    switch(*moveBuffer){
        case UP:
            {
                if(!isValidDirection(UP, map, playerY, playerX)){
                    return false;
                }
                playerY--;
                break;
            }
        case LEFT:
            {
                if(!isValidDirection(LEFT, map, playerY, playerX)){
                    return false;
                }
                playerX--;
                break;
            }
        case DOWN:
            {
                if(!isValidDirection(DOWN, map, playerY, playerX)){
                    return false;
                }
                playerY++;
                break;
            }
        case RIGHT:
            {
                if(!isValidDirection(RIGHT, map, playerY, playerX)){
                    return false;
                }
                playerX++;
                break;
            }
        default:
            return false;
    }

    playerCooldown = startTime + chrono::milliseconds(playerCooldownRate);
    return true;
}

bool isValidDirection(Direction direction, wstring map, int Y, int X){
    wchar_t* obj = nullptr;

    switch(direction){
        case UP:
            obj = std::find(std::begin(mapPiece), std::end(mapPiece), map[(Y - 1) * mapWidth + X]);
            
            break;
        case LEFT:
            obj = std::find(std::begin(mapPiece), std::end(mapPiece), map[(Y) * mapWidth + X - 1]);

            break;
        case DOWN:
            obj = std::find(std::begin(mapPiece), std::end(mapPiece), map[(Y + 1) * mapWidth + X]);

            break;
        case RIGHT:
            obj = std::find(std::begin(mapPiece), std::end(mapPiece), map[(Y) * mapWidth + X + 1]);

            break;
        default:
            return false;
    }

    if(obj != std::end(mapPiece)){
        return false;
    }

    return true;
}

void setTarget(unique_ptr<Ghost>& ghost, GhostPhase phase){
    switch(phase){
        case CHASE_PHASE:
            switch(ghost->character){
                case L'♥':
                    ghost->targetX = playerX;
                    ghost->targetY = playerY;
                    break;
                case L'♦':
                    switch(moveBuffer){
                    case UP:
                        ghost->targetX = playerX - 4;
                        ghost->targetY = playerY - 4;
                        break;
                    case LEFT:
                        ghost->targetX = playerX - 4;
                        ghost->targetY = playerY;
                        break;
                    case DOWN:
                        ghost->targetX = playerX;
                        ghost->targetY = playerY + 4;
                        break;
                    case RIGHT:
                        ghost->targetX = playerX + 4;
                        ghost->targetY = playerY;
                        break;
                    default:
                        break;
                    }
                    break;
                case L'♣':
                    switch(moveBuffer){
                    case UP:
                        {
                        int dist = FindDistance(ghosts[0]->posX, ghosts[0]->posY, playerX, playerY - 2) * 2;
                        ghost->targetX = playerX + dist;
                        ghost->targetY = playerY + dist;
                        break;
                        }
                    case LEFT:
                        {
                        int dist = FindDistance(ghosts[0]->posX, ghosts[0]->posY, playerX - 2, playerY) * 2;
                        ghost->targetX = playerX + dist;
                        ghost->targetY = playerY + dist;
                        break;
                        }
                    case DOWN:
                        {
                        int dist = FindDistance(ghosts[0]->posX, ghosts[0]->posY, playerX, playerY + 2) * 2;
                        ghost->targetX = playerX + dist;
                        ghost->targetY = playerY + dist;
                        break;
                        }
                    case RIGHT:
                        {
                        int dist = FindDistance(ghosts[0]->posX, ghosts[0]->posY, playerX + 2, playerY) * 2;
                        ghost->targetX = playerX + dist;
                        ghost->targetY = playerY + dist;
                        break;
                        }
                    default:
                        break;
                    }
                    break;
                case L'♠':
                    if(FindDistance(ghost->posX, ghost->posY, playerX, playerY) >= 8){
                        ghost->targetX = playerX;
                        ghost->targetY = playerY;
                    }else{
                        ghost->targetX = 0;
                        ghost->targetY = 35;
                    }
                    break;
            }
            break;
        case SCATTER_PHASE:
            switch(ghost->character){
                case L'♥':
                    ghost->targetX = 2;
                    ghost->targetY = 0;
                    break;
                case L'♦':
                    ghost->targetX = 26;
                    ghost->targetY = 0;
                    break;
                case L'♣':
                    ghost->targetX = 28;
                    ghost->targetY = 35;
                    break;
                case L'♠':
                    ghost->targetX = 0;
                    ghost->targetY = 35;
                    break;
            }
            break;
        case FRIGHTENED_PHASE:
            break;
        default:
            break;
    };
}

void moveGhost(unique_ptr<Ghost>& ghost, GhostPhase phase, wstring map){
    bool nodeFound = false;
    bool special = false;
    Direction invalidDirection = STOP;

    for(auto node : nodes){
        if(ghost->posX == node.posX && ghost->posY == node.posY){
            setTarget(ghost, phase);
            nodeFound = true;
            if(node.isSpecial){
                special = true;
            }
        }
    }

    switch(ghost->lastDirection){
        case UP:
            invalidDirection = DOWN;
            break;
        case LEFT:
            invalidDirection = RIGHT;
            break;
        case DOWN:
            invalidDirection = UP;
            break;
        case RIGHT:
            invalidDirection = LEFT;
            break;
        default:
            break;
    }

    if(nodeFound){
        float up = 999;
        float left = 999;
        float down = 999;
        float right = 999;
        float minimum = 999;
        for(int i = UP; i != STOP; i++){
            if(i != invalidDirection){
                switch(i){
                    case UP:
                        if(isValidDirection(UP, map, ghost->posY, ghost->posX) && (!special || ghost->phase == FRIGHTENED_PHASE)){
                            up = FindDistance(ghost->posX, ghost->posY - 1, ghost->targetX, ghost->targetY);
                        }
                        break;
                    case LEFT:
                        if(isValidDirection(LEFT, map, ghost->posY, ghost->posX)){
                            left = FindDistance(ghost->posX - 1, ghost->posY, ghost->targetX, ghost->targetY);
                        }
                        break;
                    case DOWN:
                        if(isValidDirection(DOWN, map, ghost->posY, ghost->posX)){
                            down = FindDistance(ghost->posX, ghost->posY + 1, ghost->targetX, ghost->targetY);
                        }
                        break;
                    case RIGHT:
                        if(isValidDirection(RIGHT, map, ghost->posY, ghost->posX)){
                            right = FindDistance(ghost->posX + 1, ghost->posY, ghost->targetX, ghost->targetY);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        minimum = min(min(up, left), min(down, right));

        if(minimum == up){
            ghost->lastDirection = UP;
        }else if(minimum == left){
            ghost->lastDirection = LEFT;
        }else if(minimum == down){
            ghost->lastDirection = DOWN;
        }else if(minimum == right){
            ghost->lastDirection = RIGHT;
        }
    }



    if(!isValidDirection(ghost->lastDirection, map, ghost->posY, ghost->posX) && !nodeFound){
        for(int i = UP; i != STOP; i++){
            if(i != ghost->lastDirection){
                if(isValidDirection(static_cast<Direction>(i), map, ghost->posY, ghost->posX) && i != invalidDirection){
                    ghost->lastDirection = static_cast<Direction>(i);
                    break;
                }
            }
        }
    }

    switch(ghost->lastDirection){
        case UP:
            ghost->posY--;
            break;
        case LEFT:
            ghost->posX--;
            break;
        case DOWN:
            ghost->posY++;
            break;
        case RIGHT:
            ghost->posX++;
            break;
        default:
            break;
    }
    ghost->ghostCooldown = startTime + chrono::milliseconds(ghostCooldownRate);
}

void ResetPlayer(){
    playerX = 15;
    playerY = 23;
}

float FindDistance(const int x, const int y, const int DistX, const int DistY){
    float dist = sqrt(pow((float)DistX - (float)x, 2) + pow((float)DistY - (float)y, 2));
    return dist;
}