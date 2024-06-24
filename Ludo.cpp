/*
 * i22-0518 Umer Farooq
 * i22-1697 Saif ur Rehman
 * i22-1632 Arsalan Javed
 */
#include <iostream>
#include <random>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

#define RESET "\u001B[0m"
#define RED "\u001B[31m"
#define GREEN "\u001B[32m"
#define YELLOW "\u001B[33m"
#define BLUE "\u001B[34m"
#define CYAN "\u001B[36m"

#define SIZE 15
#define CELL_NUMBER 52
#define MAX_PLAYER 4
#define MAX_DE 6

class Cell
{
public:
    int x;
    int y;
};
class token
{
public:
    int x;
    int y;
    int index;
    int id;
    int hr;
    bool revstep;
    token()
    {
        x = 0;
        y = 0;
        index = 0;
        id = 0;
        hr = 0;
        revstep = false;
    }
    bool isFinalWay(int turn)
    {
        // red = 51
        // g = 12
        // y = 25
        // b = 38
        switch (turn)
        {
        case 0:
            if (index == 51)
                return true;
            return false;

        case 1:
            if (index == 12)
                return true;
            return false;
        case 2:
            if (index == 38)
                return true;
            return false;
        case 3:
            if (index == 25)
                return true;
            return false;
        }
        return false;
    }

    bool isSafe()
    {
        int safe[6] = {1, 9, 14, 22, 27, 35};

        for (int i = 0; i < 6; i++)
        {
            if (index == safe[i])
            {
                return true;
            }
        }
        return false;
    }
};
class Player
{
public:
    int team;
    token goti[4];
    string color;
    int complete;
    int turnsWO6;
    Player()
    {
        team = 0;
        complete = 0;
        turnsWO6 = 0;
    }
    int gethitrate()
    {
        int sum = 0;
        for (int i = 0; i < 4; i++)
        {
            sum += goti[i].hr;
        }
        return sum;
    }
    void moveOnFinalWay(int idx)
    {
        switch (team)
        {
        case 0:
            goti[idx - 1].x += 1;
            break;
        case 1:
            goti[idx - 1].y += 1;
            break;
        case 2:
            goti[idx - 1].y -= 1;
            break;
        case 3:
            goti[idx - 1].x -= 1;
            break;
        }
    }
    int getIndexByTurn()
    {
        // red = 51
        // g = 12
        // b = 38
        // y = 25

        switch (team)
        {
        case 0:
            return 1;
        case 1:
            return 14;
        case 2:
            return 40;
        case 3:
            return 27;

        default:
            return 0;
        }
    }
    void setColor()
    {
        switch (team)
        {
        case 0:
            color = RED;
            break;
        case 1:
            color = GREEN;
            break;
        case 2:
            color = CYAN;
            break;
        case 3:
            color = YELLOW;
            break;
        default:
            return;
        }
        return;
    }
};

Cell cells[CELL_NUMBER];
Cell houses[MAX_PLAYER][4];
Player players[MAX_PLAYER];
vector<Player *> Winners;
vector<int> turn;
sem_t m1;
int DiceRolls[4][3] = {0};
int numTokens, nbPlayer;
bool gamefinished = false;
int killvalues[3] = {-1, -1, -1}; // for the count of tokens hit ,and  which player and which token hit

void *createMaster(int, int);
void *Masterthread(void *);
void initGame();
void shuff(vector<int> &);
void menu(int &, int &);
void play();
void *playerT(void *);
void displayCurrent();

char Grid[SIZE][SIZE] = {
    // 0   1    2    3    4    5    6    7    8    9   10   11   12   13   14
    {'1', '1', ' ', ' ', ' ', ' ', 'O', 'O', 'O', ' ', ' ', ' ', ' ', '2', '2'}, // 0
    {'1', '1', ' ', ' ', ' ', ' ', 'O', '2', '2', ' ', ' ', ' ', ' ', '2', '2'}, // 1
    {' ', ' ', ' ', ' ', ' ', ' ', '2', '2', 'O', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
    {' ', ' ', ' ', ' ', ' ', ' ', 'O', '2', 'O', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
    {' ', ' ', ' ', ' ', ' ', ' ', 'O', '2', 'O', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
    {' ', ' ', ' ', ' ', ' ', ' ', 'O', '2', 'O', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
    {'O', '1', 'O', 'O', 'O', 'O', ' ', ' ', ' ', 'O', 'O', 'O', '4', 'O', 'O'}, // 6
    {'O', '1', '1', '1', '1', '1', ' ', 'H', ' ', '4', '4', '4', '4', '4', 'O'}, // 7
    {'O', 'O', '1', 'O', 'O', 'O', ' ', ' ', ' ', 'O', 'O', 'O', 'O', '4', 'O'}, // 8
    {' ', ' ', ' ', ' ', ' ', ' ', 'O', '3', 'O', ' ', ' ', ' ', ' ', ' ', ' '}, // 9
    {' ', ' ', ' ', ' ', ' ', ' ', 'O', '3', 'O', ' ', ' ', ' ', ' ', ' ', ' '}, // 10
    {' ', ' ', ' ', ' ', ' ', ' ', 'O', '3', 'O', ' ', ' ', ' ', ' ', ' ', ' '}, // 11
    {' ', ' ', ' ', ' ', ' ', ' ', 'O', '3', '3', ' ', ' ', ' ', ' ', ' ', ' '}, // 12
    {'3', '3', ' ', ' ', ' ', ' ', '3', '3', 'O', ' ', ' ', ' ', ' ', '4', '4'}, // 13
    {'3', '3', ' ', ' ', ' ', ' ', 'O', 'O', 'O', ' ', ' ', ' ', ' ', '4', '4'}, // 14
}; // Ludo board is 15x15
int main()
{
    sem_init(&m1, 0, 1);
    for (int i = 0; i < 4; i++)
        Winners.push_back(NULL);

    while (true)
    {

        int choice = -1;
        int Numplayers = -1;
        menu(choice, Numplayers);

        if (choice == 2)
        {
            cout << "\nStopping the game ..." << endl;
            break;
        }
        nbPlayer = Numplayers;
        play();

        for (int i = 0; i < Winners.size(); i++)
        {
            if (Winners[i] != NULL)
                cout << i + 1 << " - " << Winners[i]->color << "Player " << Winners[i]->team + 1 << " won!" << RESET << "\n\n";
        }
    }
    sem_destroy(&m1);
    return 0;
}
void *createMaster(int turnval = 0, int choice = -1)
{
    pthread_t master;
    pair<int, int> args = make_pair(turnval, choice);
    pthread_create(&master, NULL, Masterthread, (void *)&args);
    void *status;
    pthread_join(master, &status);
    return status;
}
void initGame()
{
    //{y,x}
    int newCells[CELL_NUMBER][2] =
        {{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}, {7, 0}, {8, 0}, {8, 1}, {8, 2}, {8, 3}, {8, 4}, {8, 5}, {9, 6}, {10, 6}, {11, 6}, {12, 6}, {13, 6}, {14, 6}, {14, 7}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 9}, {8, 10}, {8, 11}, {8, 12}, {8, 13}, {8, 14}, {7, 14}, {6, 14}, {6, 13}, {6, 12}, {6, 11}, {6, 10}, {6, 9}, {5, 8}, {4, 8}, {3, 8}, {2, 8}, {1, 8}, {0, 8}, {0, 7}};

    int newHouses[MAX_PLAYER][4][2] = {
        {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
        {{13, 0}, {14, 0}, {13, 1}, {14, 1}},
        {{0, 13}, {0, 14}, {1, 13}, {1, 14}},
        {{13, 13}, {14, 13}, {13, 14}, {14, 14}}};

    for (int i = 0; i < CELL_NUMBER; i++)
    {
        cells[i].y = newCells[i][1];
        cells[i].x = newCells[i][0];
    }
    for (int i = 0; i < MAX_PLAYER; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            houses[i][j].y = newHouses[i][j][1];
            houses[i][j].x = newHouses[i][j][0];

            players[i].goti[j].x = houses[i][j].x;
            players[i].goti[j].y = houses[i][j].y;
            players[i].goti[j].index = -1;
            players[i].team = i;
            players[i].goti[j].id = 'a' + j;
        }
        players[i].setColor(); // set color string according to team
    }
}
void menu(int &choice, int &players)
{
    while (choice > 2 || choice < 1)
    {
        cout << endl
             << "1 - To Play" << endl;
        cout << "2 - To Quit " << endl
             << endl
             << ">";
        //  choice = 1;
        cin >> choice;
    }

    if (choice == 1)
    {
        while (players < 2 || players > 4)
        {
            cout << endl
                 << "How many players (between 2 and 4) ?" << endl
                 << endl
                 << ">";
            // players = 2;
            cin >> players;
        }
        numTokens = -1;
        while (numTokens < 1 || numTokens > 4)
        {
            cout << endl
                 << "How many Tokens (between 1 and 4) ?" << endl
                 << endl
                 << ">";
            // numTokens = 2;
            cin >> numTokens;
        }
    }
}
void play()
{

    initGame();
    pthread_t Playthreads[4];
    srand(time(NULL));

    for (int i = 0; i < nbPlayer; i++)
        turn.push_back(i); // Player turn : 0, 1, 2 or 3
    shuff(turn);           // generate random order for turns
    while (!gamefinished)
    {
        for (int i = 0; i < nbPlayer; i++)
        {
            pthread_create(&Playthreads[i], NULL, playerT, (void *)&turn[i]);
        }
        for (int i = 0; i < nbPlayer; i++)
        {
            pthread_join(Playthreads[i], NULL);
        }
        shuff(turn);
    }
    return;
}
void *Masterthread(void *args)
{
    int l = (*(pair<int, int> *)args).first; //which player 
    int m = (*(pair<int, int> *)args).second; //which goti
    Player &p = players[l];
    token *currtok = NULL;
    if (m != -1)
    {
        currtok = &p.goti[m];
    }

    int turnval = p.team;
    // exit game for that player if 20 turns W/O 6
    if (p.turnsWO6 >= 20)
    {
        cout << p.color << "\nPlayer " << p.team + 1 << RESET << " has played 20 turns without a 6. He is out of the game." << endl;
        turn.erase(remove(turn.begin(), turn.end(), turnval), turn.end());
        nbPlayer--;
        for (int i = Winners.size(); i >= 0; i--)
            if (Winners[i] == NULL)
            {
                Winners[i] = new Player(p);
                break;
            }

        if (nbPlayer == 1)
        {
            for (int i = Winners.size(); i >= 0; i--)
                if (Winners[i] == NULL)
                {
                    Winners[i] = new Player(players[turn[0]]);
                    break;
                }
            gamefinished = true;
        }

    }
    // Check for entry square in home coloumn
    if (currtok != NULL && currtok->isFinalWay(turnval) && p.gethitrate() >= 1)
    {
        currtok->index = 100;
    }

    if (currtok != NULL)
    {
        // check if token was hit

        killvalues[0] = 0;
        killvalues[1] = -1;
        killvalues[2] = -1;
        int &c = killvalues[0];
        int &xval = killvalues[1];
        int &yval = killvalues[2];
        for (int i = 0; i < MAX_PLAYER; i++)
        {
            if (i != turnval) // if it lands on oppenent piece take it
            {
                for (int j = 0; j < numTokens; j++)
                {
                    if (players[i].goti[j].x == currtok->x && players[i].goti[j].y == currtok->y && players[i].team != p.team)
                    {                                     // on same square
                        if (!players[i].goti[j].isSafe()) // not safe
                        {
                            /*
                             * Agar koi ek hi goti hai to simple last
                             * step ko dekho increment ke liye
                             * lekin agar jota hai to nai agay barhne dena
                             *
                             */

                            c++;
                            xval = i;
                            yval = j;
                        }
                        if (c >= 2) // make sure only one piece is there and not a block
                            break;
                    }
                }
                if (c >= 2)
                    break;
            }
        }
        // winner check code
        if (p.complete == numTokens)
        {
            bool already = false;
            for (int i = 0; i < Winners.size(); i++)
                if (Winners[i] != NULL && Winners[i]->team == p.team)
                {
                    already = true;
                    break;
                }
            if (!already)
            {
                for (int i = 0; i < Winners.size(); i++)
                {
                    if (Winners[i] == NULL)
                    {
                        cout << "Add normal " << p.color << p.team << RESET << " in winner pos : " << i << endl;

                        Winners[i] = new Player(p);
                        sleep(5);
                        break;
                    }
                }

                turn.erase(remove(turn.begin(), turn.end(), p.team), turn.end());

                nbPlayer--;

                if (nbPlayer == 1)
                {
                    for (int i = Winners.size(); i >= 0; i--)
                        if (Winners[i] == NULL)
                        {
                            cout << "Add Final " << players[turn[0]].color << players[turn[0]].team << RESET << " in winner pos : " << i << endl;
                            Winners[i] = new Player(players[turn[0]]);
                            sleep(5);
                            break;
                        }
                    gamefinished = true;
                }
            }
        }
    }

    return NULL;
}
void *playerT(void *arg)
{
    sem_wait(&m1);

    int l = *(int *)arg;
    Player &p = players[l];
    int turnval = p.team;

    int *MyRolls = DiceRolls[turnval];
    for (int i = 0; i < 3; i++)
        MyRolls[i] = 0; // clear rolls array before next own move

    displayCurrent();

    cout << "\nIt's up to player " + p.color << turnval + 1 << " to play." << RESET << endl;

    MyRolls[0] = rand() % MAX_DE + 1;
    if (MyRolls[0] == 6)
    {
        p.turnsWO6 = 0;
        MyRolls[1] = rand() % MAX_DE + 1;
        if (MyRolls[1] == 6)
        {
            MyRolls[2] = rand() % MAX_DE + 1;
            if (MyRolls[2] == 6)
            {
                cout << "Launch of the dice ... Result : 6,6,6" << endl;
                cout << "PASS TURN" << endl;
                for (int i = 0; i < 3; i++)
                    MyRolls[i] = 0;
                createMaster(turnval);
                sleep(1);
                sem_post(&m1);

                return NULL;
            }
        }
    }
    else
    {
        p.turnsWO6++;
    }

    cout << "Launch of the dice ... Result : ";
    for (int i = 0; i < 3; i++)
    {
        if (MyRolls[i] != 0)
            cout << MyRolls[i] << ",";
    }
    cout << "\b \n";

    if (MyRolls[0] != 6)
    {
        bool allin = true; // if all tokens are in house then pass turn if 6 is not rolled
        for (int i = 0; i < numTokens; i++)
        {
            if (p.goti[i].index != -1 && p.goti[i].id != 5)
            {
                allin = false;
                break;
            }
        }
        if (allin)
        {
            cout << "PASS TURN" << endl;
            sleep(1);
            createMaster(turnval);
            sem_post(&m1);

            return NULL;
        }
    }
    cout << p.color << "Player Hit Rate : " << RESET << p.gethitrate() << endl;
    bool pass = false;
    int choice = 0;
    for (int i = 0; i < 3; i++)
    {
        if (MyRolls[i] == 0)
            continue;
        bool allin = true; // if all tokens are in house then pass turn if 6 is not rolled
        for (int i = 0; i < numTokens; i++)
        {
            if (p.goti[i].index != -1 && p.goti[i].id != 5)
            {
                allin = false;
                break;
            }
        }
        if (allin && MyRolls[0] != 6)
        {
            cout << "PASS TURN" << endl;

            sleep(1);
            createMaster(turnval);
            sem_post(&m1);
            return NULL;
        }
        while (pass == false)
        {
            cout << "Which piece do you want to move a throw of " << MyRolls[i] << endl
                 << endl;
            cout << ">";
            cin >> choice;
            if (choice < 1 || choice > numTokens)
            {
                cout << "INVALID: Please enter a number between 1 and " << numTokens << "." << endl;
            }
            else if (p.goti[choice - 1].id == 5)
            {
                cout << "INVALID: This piece has already home." << endl;
            }
            else if (p.goti[choice - 1].index == -1 && MyRolls[i] != 6)
            {
                cout << "INVALID: This piece is in the house." << endl;
            }
            else
            {
                pass = true;
            }
        }
        pass = false;
        int result = MyRolls[i];
        token &curr = p.goti[choice - 1]; //   index is which goti chosen
        createMaster(turnval, choice - 1);

        if (curr.index == -1 && result == 6)
        {
            curr.index = p.getIndexByTurn();
            curr.x = cells[p.getIndexByTurn()].x;
            curr.y = cells[p.getIndexByTurn()].y;
        }
        else if (curr.index != -1)
        {
            if (curr.index + result > 106) // in final path need exact number to get in home
            {
                cout << "Cannot move without exact number." << endl;
                cout << "PASS TURN" << endl;
                createMaster(turnval, choice - 1);
                sleep(1);
                sem_post(&m1);

                return NULL;
            }

            int step = result;
            while (step > 0)
            {
                createMaster(turnval, choice - 1);

                if (curr.index < CELL_NUMBER)
                {
                    curr.index = (curr.index + 1) % CELL_NUMBER;
                    curr.x = cells[curr.index].x;
                    curr.y = cells[curr.index].y;
                }
                else
                {
                    curr.index++;

                    p.moveOnFinalWay(choice);

                    if (curr.index == 106) // final square for finish
                    {
                        displayCurrent();
                        cout << "MADE IT TO THE END" << endl;
                        p.complete++;
                        createMaster(turnval, choice - 1);
                        if (gamefinished)
                        {
                            sem_post(&m1);
                            return NULL;
                        }
                        curr.id = 5;
                        bool allin = true; // if all tokens are in house then pass turn
                        for (int i = 0; i < numTokens; i++)
                        {
                            if (p.goti[i].index != -1 && p.goti[i].id != 5)
                            {
                                allin = false;
                                break;
                            }
                        }
                        if (allin)
                        {
                            cout << "PASS TURN" << endl;
                            createMaster(turnval, choice - 1);
                            sem_post(&m1);

                            return NULL;
                        }
                    }
                }
                createMaster(turnval, choice - 1);
                if (killvalues[0] != 0)
                {
                    int c = killvalues[0];
                    int xval = killvalues[1];
                    int yval = killvalues[2];
                    if (c == 1 && step == 1) // if it is the final step only then kill the token
                    {
                        cout << "Token Hit" << endl;
                        cout << p.color << "Killer Team: " << p.team + 1 << RESET << players[xval].color << endl
                             << "Victim Team: " << players[xval].team + 1 << RESET << endl;
                        players[xval].goti[yval].index = -1;
                        players[xval].goti[yval].x = houses[xval][yval].x;
                        players[xval].goti[yval].y = houses[xval][yval].y;
                        curr.hr++;
                    }
                    if (c >= 2)
                    {
                        // reverse move if block is in front
                        if (curr.index < CELL_NUMBER)
                        {
                            curr.index = (curr.index - 1) % CELL_NUMBER;
                            curr.x = cells[curr.index].x;
                            curr.y = cells[curr.index].y;
                        }
                    }
                }
                step--;
            }
        }
    }
    createMaster(turnval, choice - 1);

    sem_post(&m1);
    return NULL;
}

void displayCurrent()
{
    char currentBoard[SIZE][SIZE];

    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            currentBoard[i][j] = Grid[i][j];
        }
    }
    for (int i = 0; i < MAX_PLAYER; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (players[i].goti[j].id != 5) // 5 means goti has reached house final
            {
                char &currposition = currentBoard[players[i].goti[j].y][players[i].goti[j].x];
                // check if 2 players are on same x and y

                if (currposition >= 'a' && currposition <= 'z')
                {
                    currposition = 'T';
                }
                else
                    currposition = players[i].goti[j].id;
            }
        }
    }

    cout << endl;
    for (int i = 0; i < SIZE; i++)
    {
        cout << " ";
        for (int j = 0; j < SIZE; j++)
        {
            string s = "";
            s = s + currentBoard[i][j] + " ";

            switch (currentBoard[i][j])
            {
            case '1':
                s = "O ";
                s = RED + s + RESET;
                break;
            case '2':
                s = "O ";
                s = GREEN + s + RESET;
                break;
            case '3':
                s = "O ";
                s = CYAN + s + RESET;
                break;
            case '4':
                s = "O ";
                s = YELLOW + s + RESET;
                break;
            default:
                s = currentBoard[i][j];
                s += " ";
                for (int a = 0; a < MAX_PLAYER; a++)
                {
                    for (int b = 0; b < 4; b++)
                    {
                        if (i == players[a].goti[b].y && j == players[a].goti[b].x)
                        {
                            switch (players[a].team)
                            {
                            case 0:
                                s = RED + s + RESET;
                                break;
                            case 1:
                                s = GREEN + s + RESET;
                                break;
                            case 2:
                                s = CYAN + s + RESET;
                                break;
                            case 3:
                                s = YELLOW + s + RESET;
                                break;
                            }
                        }
                    }
                }
                break;
            }
            cout << s;
        }
        cout << endl;
    }
    cout << endl;
}

void shuff(vector<int> &turn)
{
    mt19937 g((random_device())());
    shuffle(turn.begin(), turn.begin() + nbPlayer, g); // generate random order for turns again
}