#include <iostream>
#include <iomanip>
#include <cmath>
#include <cctype>                  // isspace() is in cctype header
using namespace std;

char grid[16][16];
int  reachable[16][16], dist[16][16];
long long rngState;
int  cap, best;                    // cap: backpack capacity, best: theoretical max value
int  tW[9], tV[9];                 // treasure weight, value: 0-5 treasures, 6=Z drop, 7=V drop, 8=W drop
int  backpack[9];                  // 1 if item collected
int  weapon[3];                    // 0=z, 1=v, 2=w
int  curRow, curCol;
int  heart, stamina, maxStamina, stepcounter;
int  weight, value;
int  gameOver, resultType;         // resultType: 0 win, 1 hearts, 2 stamina


// Get random number
int getRandomNum(){
    rngState = (rngState * 1103515245 + 12345) % 2147483648;
    return rngState >> 16; // right shift 16 places
}

// Read Reply (a whole word counts as ONE reply)
int readReply(char &ans){          // returns 0 if there is no input left
    if (!(cin >> ans)) return 0;  // if the first input character is EOF, then return 0
    while (cin.peek() != EOF && !isspace(cin.peek())) //cin.peek : look at the next character without reading it
    {
        cin.get();                // read the next character
        ans = '?';               // more than one character: invalid answer
    }
    return 1;
}

// Initialize Game
void initGame(){
    long long seed;
    cout << "=============\nTomb Explorer\n=============\n" << endl;
    cout << "Seed: \n";
    cin >> seed;
    rngState = seed % 2147483648;
    cout << "Backpack capacity: ";
    cin >> cap;
    for (int i = 0; i <= 5; i++)
    {
        cout << "Treasure " << i + 1 << " weight and value: ";
        cin >> tW[i] >> tV[i];
    }
    cout << "Monster Z drop weight and value: ";
    cin >> tW[6] >> tV[6];
    cout << "Monster V drop weight and value: ";
    cin >> tW[7] >> tV[7];
    cout << "Monster W drop weight and value: ";
    cin >> tW[8] >> tV[8];
    heart = 3; stepcounter = 0; weight = 0; value = 0;
    curRow = 0; curCol = 0; gameOver = 0;
    for (int i = 0; i < 9; i++) backpack[i] = 0; // clear backpack
    for (int i = 0; i < 3; i++) weapon[i] = 0;   // clear weapons owned
}

// Check Map Reachable and Find Shortest Distance (BFS)
int reachableMap(){                // returns shortest distance, or -1 if E unreachable
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
        {
            reachable[i][j] = 0;
            dist[i][j] = 0;
        }
    int qr[256], qc[256], head = 0, tail = 0; // qr: queue of row, qc: queue of column
    int dr[4] = {1, -1, 0, 0}, dc[4] = {0, 0, 1, -1};
    qr[0] = 0; qc[0] = 0; tail = 1;
    reachable[0][0] = 1; dist[0][0] = 0;
    while (head < tail)           // have to travel through every point to make sure every items are put in blocks that are reachable
    {
        int r = qr[head], c = qc[head]; head++;
        for (int k = 0; k <= 3; k++)
        {
            int nr = r + dr[k], nc = c + dc[k];
            if (nr < 0 || nr > 15 || nc < 0 || nc > 15) continue;   // out of bounds
            if (reachable[nr][nc]) continue;                        // already visited
            if (grid[nr][nc] == '#') continue;                      // wall
            reachable[nr][nc] = 1;
            dist[nr][nc] = dist[r][c] + 1;
            qr[tail] = nr; qc[tail] = nc; tail++;
        }
    }
    if (reachable[15][15]) return dist[15][15]; // [15][15] is the position of the exit
    else return -1;
}

// Fill in Map
void fillMap(){
    for (int i = 0; i <= 15; i++)
        for (int j = 0; j <= 15; j++)
        {
            if (getRandomNum() % 100 < 40) grid[i][j] = '#';
            else                           grid[i][j] = '.';
        }
    grid[0][0]   = 'S';
    grid[15][15] = 'E';
}

// Generate Valid Map
void generateMap(){
    int D;
    while (true)
    {
        fillMap();
        D = reachableMap();
        if (D != -1) break;       // fill the map again when not reachable
    }
    maxStamina = ceil(1.5 * D);
    stamina = maxStamina;
}

// Place Objects
void placeOne(char symbol){        // place assigned item
    while (true)
    {
        int row = getRandomNum() % 16;
        int col = getRandomNum() % 16;
        if (reachable[row][col] && grid[row][col] == '.')
        {
            grid[row][col] = symbol;
            return;
        }
    }
}

void placeObjects(){               // assign object to place
    char order[12] = {'1', '2', '3', '4', '5', '6', 'Z', 'V', 'W', 'z', 'v', 'w'};   // exact order
    for (int i = 0; i < 12; i++)
        placeOne(order[i]);
}

// Find Maximum Possible Value
void maxValue(int idx, int curW, int curV){
    if (idx == 9)
    {
        if (curV > best) best = curV;
        return;
    }
    maxValue(idx + 1, curW, curV); // skip current one (must consider this situation)
    if (curW + tW[idx] <= cap)
        maxValue(idx + 1, curW + tW[idx], curV + tV[idx]); // take current one (consider if not exceeding backpack capacity)
}

// Print Map (show player position)
void printMap(){
    cout << "\n\n";
    for (int j = 0; j < 18; j++) cout << '#';
    cout << "\n";
    for (int i = 0; i <= 15; i++)
    {
        cout << '#';
        for (int j = 0; j <= 15; j++)
        {
            if (i == curRow && j == curCol) cout << 'U'; // player's position
            else cout << grid[i][j];
        }
        cout << '#' << "\n";
    }
    for (int j = 0; j < 18; j++) cout << '#';
    cout << "\n";
}

// Print Status
void printStatus(){
    cout << "\n========== Status ==========" << endl;
    cout << "Hearts: " << heart << " / 3" << endl;
    cout << "Stamina: " << stamina << " / " << maxStamina << endl;
    cout << "Weapons: z[" << (weapon[0] ? 'Y' : 'N') << "] v[" << (weapon[1] ? 'Y' : 'N')
         << "] w[" << (weapon[2] ? 'Y' : 'N') << "]" << endl;   // (0 -> N, 1 -> Y)
    cout << "Total Value: " << value << endl;
    cout << "Backpack Weight: " << weight << " / " << cap << endl;
    cout << "Steps: " << stepcounter << endl;
    cout << "============================" << endl;
}

// Encounter Monster
int encounterMonster(char monster){ // 'Z', 'V' or 'W'
    int idx = (monster == 'Z') ? 0 : (monster == 'V') ? 1 : 2;
    // success
    if (weapon[idx] == 1){
        cout << "You defeated monster " << monster << "." << endl;
        return 1;
    }
    // blocked
    else{
        cout << "Monster " << monster << " blocked you." << endl;
        cout << "You need weapon " << (char)(monster - 'A' + 'a') << " and lost 1 heart." << endl;
        return 0;
    }
}

// Found Treasure
void foundTreasure(char treasure, int row, int col)
{
    int idx = treasure - '1';      // C++ does implicit conversion from char to int
    cout << "You found Treasure " << idx + 1 << "." << endl;
    cout << "Weight: " << tW[idx] << ", Value: " << tV[idx] << endl;
    while (true){
        char collect;
        cout << "Take it [y/n]: ";
        if (!readReply(collect)) return; // return if input is empty
        if (collect == 'y' || collect == 'Y'){
            if (weight + tW[idx] > cap)
                cout << "Your backpack is too heavy. You cannot take it." << endl;
            else{
                backpack[idx] = 1;
                weight += tW[idx]; value += tV[idx];
                grid[row][col] = '.'; // change grid status
                cout << "Item collected." << endl;
            }
            return;
        }
        else if (collect == 'n' || collect == 'N'){
            cout << "You left it here." << endl;
            return;
        }
        else                       // invalid input
            cout << "Please enter y or n." << endl;
    }
}

// Collect Drop
void collectDrop(char monster, int row, int col){ // 'Z', 'V' or 'W'
    int idx = (monster == 'Z') ? 6 : (monster == 'V') ? 7 : 8;
    cout << "You found Monster " << monster << " drop." << endl;
    cout << "Weight: " << tW[idx] << ", Value: " << tV[idx] << endl;
    while (true){
        char collect;
        cout << "Take it [y/n]: ";
        if (!readReply(collect)) return;
        if (collect == 'y' || collect == 'Y')
        {
            if (weight + tW[idx] > cap)
                cout << "Your backpack is too heavy. You cannot take it." << endl;
            else
            {
                backpack[idx] = 1;
                weight += tW[idx]; value += tV[idx];
                grid[row][col] = '.';
                cout << "Item collected." << endl;
            }
            return;
        }
        else if (collect == 'n' || collect == 'N')
        {
            cout << "You left it here." << endl;
            return;
        }
        else
            cout << "Please enter y or n." << endl;
    }
}

// Pick Weapon
void pickWeapon(char c, int row, int col){ // 'z', 'v' or 'w'
    int idx = (c == 'z') ? 0 : (c == 'v') ? 1 : 2;
    weapon[idx] = 1;
    grid[row][col] = '.';
    cout << "You picked up weapon " << c << "." << endl;
}

// Player Move (Note: tryMove is new, tests implies whether player should move or not.(extracting what "move" functions should do, simplify them.))
int tryMove(int nr, int nc)        // nr, nc: position trying to move to
{
    if (nr < 0 || nr > 15 || nc < 0 || nc > 15) return 0; // curRow, curCol remains
    char c = grid[nr][nc];         // c : block object
    if (c == '#') return 2;        // curRow, curCol remains

    if (c == 'Z' || c == 'V' || c == 'W'){
        stamina--; stepcounter++;
        // success
        if (encounterMonster(c) == 1){
            grid[nr][nc] = (c == 'Z') ? 'a' : (c == 'V') ? 'b' : 'c';
            // actually moving to new position
            curRow = nr;
            curCol = nc;
            if (stamina > 0) collectDrop(c, nr, nc); // no stamina left: game ends before the drop
            return 1;
        }
        // blocked
        else{
            // curRow, curCol remains
            heart--;
            return 4;
        }
    }

    // actually moving to new position
    curRow = nr;
    curCol = nc;
    stamina--; stepcounter++;
    if (c == 'E') return 3;        // reaching E with the last stamina is still a win
    if (stamina == 0) return 1;    // no stamina left: game ends before any item / weapon event

    if (c == 'z' || c == 'v' || c == 'w') pickWeapon(c, nr, nc);
    else if (c >= '1' && c <= '6') foundTreasure(c, nr, nc);
    else if (c == 'a' || c == 'b' || c == 'c')
        collectDrop((c == 'a') ? 'Z' : (c == 'b') ? 'V' : 'W', nr, nc);
    return 1;
}

int moveUp()    { return tryMove(curRow - 1, curCol); }
int moveDown()  { return tryMove(curRow + 1, curCol); }
int moveLeft()  { return tryMove(curRow,   curCol - 1); }
int moveRight() { return tryMove(curRow,   curCol + 1); }

// Win
void win(){
    gameOver = 1; resultType = 0;
}

// Lose and Show Reason
void lose(int reason){             // 0 = hearts, 1 = stamina
    gameOver = 1; resultType = reason + 1;
    if (reason == 0) cout << "You ran out of hearts." << endl;
    else cout << "You ran out of stamina." << endl;
}

// Read Command
void readCommand(char cmd){
    int code;
    if (cmd == 'w') code = moveUp();
    else if (cmd == 'a') code = moveLeft();
    else if (cmd == 's') code = moveDown();
    else if (cmd == 'd') code = moveRight();
    else{ 
        cout << "Invalid command." << endl; 
        return;
    }
    if (code == 0){ 
        cout << "You cannot move outside the map." << endl;
        return;
    }
    if (code == 2){ 
        cout << "You hit a wall." << endl;
        return;
    }

    if (code == 3) win();
    else if (heart == 0) lose(0);  // send lose reason
    else if (stamina == 0) lose(1);
    return;
}

// Print Result at Last
void printResult(){
    cout << "\n====================================" << endl;
    cout << "Tomb Explorer Result" << endl;
    cout << "====================================" << endl;
    if (resultType == 0)      cout << "You escaped the tomb." << endl;
    else if (resultType == 1) cout << "Game Over. You ran out of hearts." << endl;
    else                      cout << "Game Over. You ran out of stamina." << endl;
    cout << "Final Value: " << value << endl;
    cout << "Theoretical Max Value: " << best << endl;
    double pct;
    if (best == 0) pct = 0.0;
    else           pct = 100.0 * value / best; // implicit conversion to float
    cout << "Mission Completeness: " << fixed << setprecision(1) << pct << "%" << endl;
    cout << "Backpack Weight: " << weight << " / " << cap << endl;
    cout << "Hearts: " << heart << " / 3" << endl;
    cout << "Stamina: " << stamina << " / " << maxStamina << endl;
    cout << "Weapons: z[" << (weapon[0] ? 'Y' : 'N') << "] v[" << (weapon[1] ? 'Y' : 'N')
         << "] w[" << (weapon[2] ? 'Y' : 'N') << "]" << endl;
    cout << "Total Steps: " << stepcounter << endl;
    cout << "====================================" << endl;
}

// Main Function
int main(){
    initGame();
    generateMap();
    placeObjects();
    best = 0;
    maxValue(0, 0, 0);
    printMap();
    printStatus();
    while (!gameOver){
        cout << "Command [w/a/s/d]: ";
        char cmd;
        if (!readReply(cmd)) return 0; // check full input, stop when no input
        readCommand(cmd);
        printMap();
        printStatus();
    }
    printResult();
    return 0;
}
