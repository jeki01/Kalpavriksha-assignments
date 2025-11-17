
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "Players_data.h"   

#define MAX_TEAMS 10

/* --- Data structures --- */

typedef struct PlayerData
{
    int playerId;
    char name[50];
    char teamName[50];
    char role[50];
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    int performanceIndex;
} PlayerData;

typedef struct PlayerNode
{
    PlayerData data;
    struct PlayerNode *next;  
} PlayerNode;

/* Role wrapper nodes */
typedef struct RoleNode
{
    PlayerNode *player;        
    struct RoleNode *next;     
} RoleNode;

typedef struct TeamData
{
    int teamId;
    char teamName[50];
    int totalPlayers;
    float averageBattingStrikeRate;
    PlayerNode *playersList;   

    RoleNode *batsman;         
    int totalBatsman;

    RoleNode *bowlers;
    int totalBowlers;

    RoleNode *allRounders;
    int totalAllRounders;
} TeamData;

/* Team index for O(log t) search */
typedef struct TeamIndex {
    char name[50];
    int idx;
} TeamIndex;

TeamIndex teamIndexArr[MAX_TEAMS];
int teamIndexCount = 0;

/* --- Prototypes --- */
/* initialization & load */
void initialiseTeamsData(TeamData teamsData[]);
int findTeamIndex(const char teamName[]);
int teamIndexCompare(const void *a, const void *b);
int computePerformanceIndex(PlayerData playerData);
PlayerNode* insertPlayerToMainList(PlayerNode **head, PlayerData playerData);
void insertPlayerIntoRoleList(RoleNode **head, PlayerNode *player);
void loadInitialPlayers(TeamData teamsData[]);

/* menu & input helpers */
void printMenu(void);
bool validChoice(int choice);
void clearInputBuffer(void);
bool isEmptyorSpaces(const char input[]);
bool isNumber(const char input[]);
bool inputValidation(char input[], int inputSize);
bool inputNameValidation(char input[], int inputSize);
bool isFloatNumber(const char *input);
bool inputFloatValidation(char input[], int inputSize);

int getValidatedInt(const char *prompt, int minVal, int maxVal);
float getValidatedFloat(const char *prompt);
void getValidatedName(char *out, int outSize);
int getValidatedRole(void);
int getValidatedTeamID(void);

/* operations */
void addNewPlayers(TeamData teamsData[]);
bool isDuplicatePlayerID(TeamData teamsData[], int playerID);
void recalculateTeamAvgStrikeRate(TeamData *team);

void displayPlayerOFTeam(TeamData teamsData[], int teamID);
void printNodes(PlayerNode *head);
void printPlayerTableHeader(void);

void displayTeamsByAvgBattingSR(TeamData teamsData[]);
void sortTeamsByStrikeRate(TeamData out[], int n);

void displayTopKPlayers(TeamData teamsData[]);

void displayPlayersAccordingToRole(TeamData teamsData[]);
void heapifyDown(RoleNode **heap, int size, int index);
void buildMaxHeap(RoleNode **heap, int size);
RoleNode* extractMaxRole(RoleNode **heap, int *size);

/* free */
void freeRoleList(RoleNode *head);
void freeLinkedList(PlayerNode *head);
void freeAllMemory(TeamData teamsData[]);

/* role helpers */
RoleNode** getRoleHeadPtr(TeamData *team, int role);
int getRoleCount(TeamData *team, int role);
const char* getRoleName(int role);

/* --- Implementation --- */

void printMenu()
{
    printf(" ==============================================================================\n");
    printf(" ICC ODI Player Performance Analyzer\n");
    printf(" ==============================================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display all Players of specific role Across All Teams by performance index\n");
    printf("6. Exit\n");
    printf(" ==============================================================================\n");
}

bool validChoice(int choice)
{
    return (choice >= 1 && choice <= 6);
}

void clearInputBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

bool isEmptyorSpaces(const char input[])
{
    if (input[0] == '\0') return true;
    for (int i = 0; input[i] != '\0'; ++i)
    {
        if (!isspace((unsigned char)input[i])) return false;
    }
    return true;
}

bool isNumber(const char input[])
{
    int index = 0;
    while (input[index] && isspace((unsigned char)input[index])) index++;
    if (input[index] && !isdigit((unsigned char)input[index])) return false;
    while (input[index] && isdigit((unsigned char)input[index])) index++;
    while (input[index] && isspace((unsigned char)input[index])) index++;
    return input[index] == '\0';
}

bool inputValidation(char input[], int inputSize)
{
    while (1)
    {
        if (fgets(input, inputSize, stdin) == NULL)
        {
            printf("Input error!\nEnter again : ");
            continue;
        }
        input[strcspn(input, "\n")] = '\0';
        if (isEmptyorSpaces(input))
        {
            printf("Input cannot be empty!\nEnter again : ");
            continue;
        }
        if (!isNumber(input))
        {
            printf("Invalid input! Enter a valid number \nEnter again : ");
            continue;
        }
        return true;
    }
}

bool inputNameValidation(char input[], int inputSize)
{
    if (fgets(input, inputSize, stdin) == NULL) return false;
    input[strcspn(input, "\n")] = '\0';
    if (isEmptyorSpaces(input))
    {
        printf("Name cannot be empty!\n");
        return false;
    }
    int i = 0;
    while (input[i] && isspace((unsigned char)input[i])) i++;
    while (input[i] && input[i] != '\0')
    {
        char c = input[i];
        if (!isalpha((unsigned char)c) && !isspace((unsigned char)c))
        {
            printf("Invalid name! Only alphabets and spaces allowed.\n");
            return false;
        }
        i++;
    }
    return true;
}

bool isFloatNumber(const char *input)
{
    int index = 0;
    bool hasDecimal = false;
    bool hasDigit = false;
    while (input[index] && isspace((unsigned char)input[index])) index++;
    if (input[index] != '.' && !isdigit((unsigned char)input[index])) return false;
    while (input[index])
    {
        if (isdigit((unsigned char)input[index])) hasDigit = true;
        else if (input[index] == '.')
        {
            if (hasDecimal) return false;
            hasDecimal = true;
        }
        else if (isspace((unsigned char)input[index]))
        {
            while (input[index])
            {
                if (!isspace((unsigned char)input[index])) return false;
                index++;
            }
            break;
        }
        else return false;
        index++;
    }
    return hasDigit;
}

bool inputFloatValidation(char input[], int inputSize)
{
    if (fgets(input, inputSize, stdin) == NULL) return false;
    input[strcspn(input, "\n")] = '\0';
    if (isEmptyorSpaces(input))
    {
        printf("Input cannot be empty!\n");
        return false;
    }
    if (!isFloatNumber(input))
    {
        printf("Invalid input! Enter a valid decimal number.\n");
        return false;
    }
    return true;
}

/* ---------- Helpers for validated prompts (DRY) ---------- */

int getValidatedInt(const char *prompt, int minVal, int maxVal)
{
    char buf[64];
    int val;
    while (1)
    {
        if (prompt) printf("%s", prompt);
        if (!inputValidation(buf, sizeof(buf))) continue;
        val = atoi(buf);
        if ((minVal != INT_MIN && val < minVal) || (maxVal != INT_MAX && val > maxVal))
        {
            if (minVal != INT_MIN || maxVal != INT_MAX)
                printf("Value must be between %d and %d. Try again.\n", minVal, maxVal);
            else
                printf("Invalid value. Try again.\n");
            continue;
        }
        return val;
    }
}

float getValidatedFloat(const char *prompt)
{
    char buf[64];
    float val;
    while (1)
    {
        if (prompt) printf("%s", prompt);
        if (!inputFloatValidation(buf, sizeof(buf))) continue;
        val = atof(buf);
        return val;
    }
}

void getValidatedName(char *out, int outSize)
{
    while (1)
    {
        if (fgets(out, outSize, stdin) == NULL) { printf("Input error\n"); continue; }
        out[strcspn(out, "\n")] = '\0';
        if (isEmptyorSpaces(out)) { printf("Name cannot be empty! Enter again: "); continue; }
        int i = 0;
        while (out[i] && isspace((unsigned char)out[i])) i++;
        bool ok = true;
        for (; out[i]; ++i)
        {
            char c = out[i];
            if (!isalpha((unsigned char)c) && !isspace((unsigned char)c)) { ok = false; break; }
        }
        if (!ok) { printf("Invalid name! Only alphabets and spaces allowed. Enter again: "); continue; }
        return;
    }
}

int getValidatedRole(void)
{
    while (1)
    {
        int r = getValidatedInt("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3);
        return r;
    }
}

int getValidatedTeamID(void)
{
    while (1)
    {
        int tid = getValidatedInt("Enter Team ID: ", 1, teamCount);
        return tid;
    }
}

/* ---------- Team index for O(log t) ---------- */

int teamIndexCompare(const void *a, const void *b)
{
    const TeamIndex *A = a;
    const TeamIndex *B = b;
    return strcmp(A->name, B->name);
}

int findTeamIndex(const char teamName[])
{
    int lo = 0, hi = teamIndexCount - 1;
    while (lo <= hi)
    {
        int mid = lo + (hi - lo) / 2;
        int cmp = strcmp(teamIndexArr[mid].name, teamName);
        if (cmp == 0) return teamIndexArr[mid].idx;
        if (cmp < 0) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

/* ---------- Performance computation ---------- */

int computePerformanceIndex(PlayerData playerData)
{
    float performance_index = 0.0f;

    if (strcmp(playerData.role, "Batsman") == 0)
    {
        performance_index = (playerData.battingAverage * playerData.strikeRate) / 100.0f;
    }
    else if (strcmp(playerData.role, "Bowler") == 0)
    {
        performance_index = (playerData.wickets * 2.0f) + (100.0f - playerData.economyRate);
    }
    else
    {
        performance_index = ((playerData.battingAverage * playerData.strikeRate) / 100.0f) + (playerData.wickets * 2.0f);
    }

    if (performance_index < 0) performance_index = 0;
    return (int)performance_index;
}

/* ---------- Insert main node (returns pointer to created node) ---------- */

PlayerNode* insertPlayerToMainList(PlayerNode **head, PlayerData playerData)
{
    PlayerNode *newPlayer = malloc(sizeof(PlayerNode));
    if (newPlayer == NULL)
    {
        printf("Memory allocation failed\n");
        return NULL;
    }
    newPlayer->data = playerData;
    newPlayer->next = NULL;

    if (*head == NULL)
    {
        *head = newPlayer;
        return newPlayer;
    }

    PlayerNode *cur = *head;
    while (cur->next != NULL) cur = cur->next;
    cur->next = newPlayer;
    return newPlayer;
}

/* ---------- Insert wrapper into role list (sorted descending by performanceIndex) ---------- */

void insertPlayerIntoRoleList(RoleNode **head, PlayerNode *player)
{
    if (player == NULL) return;
    RoleNode *newRole = malloc(sizeof(RoleNode));
    if (newRole == NULL)
    {
        printf("Memory allocation failed for role node\n");
        return;
    }
    newRole->player = player;
    newRole->next = NULL;

    if (*head == NULL || player->data.performanceIndex > (*head)->player->data.performanceIndex)
    {
        newRole->next = *head;
        *head = newRole;
        return;
    }

    RoleNode *cur = *head;
    while (cur->next != NULL && cur->next->player->data.performanceIndex > player->data.performanceIndex)
        cur = cur->next;

    newRole->next = cur->next;
    cur->next = newRole;
}

/* ---------- initialise and load players ---------- */

void initialiseTeamsData(TeamData teamsData[])
{
    for (int i = 0; i < teamCount; ++i)
    {
        teamsData[i].teamId = i + 1;
        strcpy(teamsData[i].teamName, teams[i]);
        teamsData[i].totalPlayers = 0;
        teamsData[i].averageBattingStrikeRate = 0.0f;
        teamsData[i].playersList = NULL;
        teamsData[i].batsman = NULL;
        teamsData[i].totalBatsman = 0;
        teamsData[i].bowlers = NULL;
        teamsData[i].totalBowlers = 0;
        teamsData[i].allRounders = NULL;
        teamsData[i].totalAllRounders = 0;
    }

    /* build team index for O(log t) search */
    teamIndexCount = teamCount < MAX_TEAMS ? teamCount : MAX_TEAMS;
    for (int i = 0; i < teamIndexCount; ++i)
    {
        strncpy(teamIndexArr[i].name, teams[i], sizeof(teamIndexArr[i].name) - 1);
        teamIndexArr[i].name[sizeof(teamIndexArr[i].name) - 1] = '\0';
        teamIndexArr[i].idx = i;
    }
    qsort(teamIndexArr, teamIndexCount, sizeof(TeamIndex), teamIndexCompare);
}

void loadInitialPlayers(TeamData teamsData[])
{
    for (int i = 0; i < playerCount; ++i)
    {
        Player headerPlayer = players[i];
        PlayerData pd;
        pd.playerId = headerPlayer.id;
        strcpy(pd.name, headerPlayer.name);
        strcpy(pd.teamName, headerPlayer.team);
        strcpy(pd.role, headerPlayer.role);
        pd.totalRuns = headerPlayer.totalRuns;
        pd.battingAverage = headerPlayer.battingAverage;
        pd.strikeRate = headerPlayer.strikeRate;
        pd.wickets = headerPlayer.wickets;
        pd.economyRate = headerPlayer.economyRate;
        pd.performanceIndex = computePerformanceIndex(pd);

        int tindex = findTeamIndex(pd.teamName);
        if (tindex == -1) continue;
        TeamData *team = &teamsData[tindex];
        team->totalPlayers++;
        PlayerNode *mainNode = insertPlayerToMainList(&team->playersList, pd);

        if (strcmp(pd.role, "Batsman") == 0)
        {
            insertPlayerIntoRoleList(&team->batsman, mainNode);
            team->totalBatsman++;
        }
        else if (strcmp(pd.role, "Bowler") == 0)
        {
            insertPlayerIntoRoleList(&team->bowlers, mainNode);
            team->totalBowlers++;
        }
        else
        {
            insertPlayerIntoRoleList(&team->allRounders, mainNode);
            team->totalAllRounders++;
        }
    }

    for (int i = 0; i < teamCount; ++i) recalculateTeamAvgStrikeRate(&teamsData[i]);
}

/* ---------- Display team players (main list) ---------- */

void printPlayerTableHeader(void)
{
    printf(" ====================================================================================\n");
    printf("%-5s %-20s %-12s %6s %6s %7s %6s %6s %10s\n",
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "PerfIdx");
    printf(" ====================================================================================\n");
}

void displayPlayerOFTeam(TeamData teamsData[], int teamID)
{
    if (teamID < 1 || teamID > teamCount)
    {
        printf("Invalid Team ID!\n");
        return;
    }
    int idx = teamID - 1;
    printf("Players of Team %s: \n", teamsData[idx].teamName);
    printPlayerTableHeader();
    printNodes(teamsData[idx].playersList);
    printf(" ====================================================================================\n");
    printf("Total Player: %d\n", teamsData[idx].totalPlayers);
    printf("Average Batting Strike Rate: %f\n", teamsData[idx].averageBattingStrikeRate);
}

void printNodes(PlayerNode *head)
{
    PlayerNode *cur = head;
    while (cur != NULL)
    {
        printf("%-5d %-20s %-12s %6d %6.2f %7.2f %6d %6.2f %10d\n",
               cur->data.playerId, cur->data.name, cur->data.role, cur->data.totalRuns,
               cur->data.battingAverage, cur->data.strikeRate, cur->data.wickets,
               cur->data.economyRate, cur->data.performanceIndex);
        cur = cur->next;
    }
}

/* ---------- Teams sorted by avg SR ---------- */

void sortTeamsByStrikeRate(TeamData out[], int n)
{
    /* simple bubble sort as earlier to preserve behavior */
    for (int pass = 0; pass < n; ++pass)
    {
        for (int i = 0; i < n - pass - 1; ++i)
        {
            if (out[i].averageBattingStrikeRate < out[i + 1].averageBattingStrikeRate)
            {
                TeamData tmp = out[i];
                out[i] = out[i + 1];
                out[i + 1] = tmp;
            }
        }
    }
}

void displayTeamsByAvgBattingSR(TeamData teamsData[])
{
    printf("Teams Sorted by Average Batting Strike Rate\n");
    printf(" ====================================================================================\n");
    printf("%-5s %-15s %-12s %-14s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf(" ====================================================================================\n");

    TeamData sortedTeams[MAX_TEAMS];
    memcpy(sortedTeams, teamsData, sizeof(TeamData) * teamCount);
    sortTeamsByStrikeRate(sortedTeams, teamCount);

    for (int i = 0; i < teamCount; ++i)
    {
        printf("%-5d %-15s %-12.2f %-14d\n",
               sortedTeams[i].teamId,
               sortedTeams[i].teamName,
               sortedTeams[i].averageBattingStrikeRate,
               sortedTeams[i].totalPlayers);
    }
    printf(" ====================================================================================\n");
}

/* ---------- Top K players of a team by role (O(K)) ---------- */

RoleNode** getRoleHeadPtr(TeamData *team, int role)
{
    if (role == 1) return &team->batsman;
    if (role == 2) return &team->bowlers;
    return &team->allRounders;
}

int getRoleCount(TeamData *team, int role)
{
    if (role == 1) return team->totalBatsman;
    if (role == 2) return team->totalBowlers;
    return team->totalAllRounders;
}

const char* getRoleName(int role)
{
    if (role == 1) return "Batsman";
    if (role == 2) return "Bowler";
    return "All-rounder";
}

/* prints header used earlier for TopK */
void displayTopKPlayers(TeamData teamsData[])
{
    int teamID;
    while (1)
    {
        teamID = getValidatedInt("Enter Team ID:", 1, teamCount);
        break;
    }

    int role = getValidatedRole();

    RoleNode *roleList = NULL;
    int totalRolePlayers = 0;
    if (role == 1) { roleList = teamsData[teamID - 1].batsman; totalRolePlayers = teamsData[teamID - 1].totalBatsman; }
    else if (role == 2) { roleList = teamsData[teamID - 1].bowlers; totalRolePlayers = teamsData[teamID - 1].totalBowlers; }
    else { roleList = teamsData[teamID - 1].allRounders; totalRolePlayers = teamsData[teamID - 1].totalAllRounders; }

    int topK;
    while (1)
    {
        topK = getValidatedInt("Enter number of players: ", 1, totalRolePlayers > 0 ? totalRolePlayers : 1);
        if (topK < 1 || topK > totalRolePlayers)
        {
            printf("K should be between 1 and %d\n", totalRolePlayers);
            continue;
        }
        break;
    }

    printf("Top %d %s of %s :\n", topK, getRoleName(role), teamsData[teamID - 1].teamName);
    printPlayerTableHeader();

    RoleNode *cur = roleList;
    int count = 0;
    while (cur != NULL && count < topK)
    {
        PlayerData *p = &cur->player->data;
        printf("%-5d %-20s %-12s %6d %6.2f %7.2f %6d %6.2f %10d\n",
               p->playerId, p->name, p->role, p->totalRuns,
               p->battingAverage, p->strikeRate, p->wickets,
               p->economyRate, p->performanceIndex);
        cur = cur->next;
        count++;
    }
}

/* ---------- Display all players across all teams of a role using k-way merge (O(N log t)) ---------- */

void displayPlayersAccordingToRole(TeamData teamsData[])
{
    int role = getValidatedRole();
    const char *role_name = getRoleName(role);

    printf("\n%s of all teams:\n", role_name);
    printf(" ====================================================================================\n");
    printf("%-5s %-20s %-12s %-12s %6s %6s %7s %6s %10s\n",
           "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "PerfIdx");
    printf(" ====================================================================================\n");

    /* Build initial heap of RoleNode* heads (size <= teamCount) */
    RoleNode *heap[MAX_TEAMS];
    int heapSize = 0;
    for (int ti = 0; ti < teamCount; ++ti)
    {
        RoleNode *head = NULL;
        if (role == 1) head = teamsData[ti].batsman;
        else if (role == 2) head = teamsData[ti].bowlers;
        else head = teamsData[ti].allRounders;

        if (head != NULL) heap[heapSize++] = head;
    }

    if (heapSize == 0)
    {
        printf("No players found for role %s.\n", role_name);
        printf(" ====================================================================================\n");
        return;
    }

   
    buildMaxHeap(heap, heapSize);

    while (heapSize > 0)
    {
        RoleNode *topRoleNode = extractMaxRole(heap, &heapSize);
        PlayerData *p = &topRoleNode->player->data;
        printf("%-5d %-20s %-12s %-12s %6d %6.2f %7.2f %6d %10d\n",
               p->playerId, p->name, p->teamName, p->role,
               p->totalRuns, p->battingAverage, p->strikeRate,
               p->wickets, p->performanceIndex);

        
        if (topRoleNode->next != NULL)
        {
            heap[heapSize++] = topRoleNode->next;
            
            int i = heapSize - 1;
            while (i > 0)
            {
                int parent = (i - 1) / 2;
                if (heap[parent]->player->data.performanceIndex < heap[i]->player->data.performanceIndex)
                {
                    RoleNode *tmp = heap[parent];
                    heap[parent] = heap[i];
                    heap[i] = tmp;
                    i = parent;
                }
                else break;
            }
        }
    }

    printf(" ====================================================================================\n");
}

/* Heap helpers for RoleNode* heap (max-heap by performanceIndex) */

void heapifyDown(RoleNode **heap, int size, int index)
{
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < size && heap[left]->player->data.performanceIndex > heap[largest]->player->data.performanceIndex)
        largest = left;
    if (right < size && heap[right]->player->data.performanceIndex > heap[largest]->player->data.performanceIndex)
        largest = right;

    if (largest != index)
    {
        RoleNode *tmp = heap[index];
        heap[index] = heap[largest];
        heap[largest] = tmp;
        heapifyDown(heap, size, largest);
    }
}

void buildMaxHeap(RoleNode **heap, int size)
{
    for (int i = size / 2 - 1; i >= 0; --i)
        heapifyDown(heap, size, i);
}

RoleNode* extractMaxRole(RoleNode **heap, int *size)
{
    if (*size <= 0) return NULL;
    RoleNode *maxNode = heap[0];
    heap[0] = heap[*size - 1];
    (*size)--;
    heapifyDown(heap, *size, 0);
    return maxNode;
}

/* ---------- Duplicate check across all teams (check main player nodes) ---------- */

bool isDuplicatePlayerID(TeamData teamsData[], int playerID)
{
    for (int i = 0; i < teamCount; ++i)
    {
        PlayerNode *cur = teamsData[i].playersList;
        while (cur != NULL)
        {
            if (cur->data.playerId == playerID) return true;
            cur = cur->next;
        }
    }
    return false;
}

/* ---------- Add new player  ---------- */

void addNewPlayers(TeamData teamsData[])
{
    int teamID = getValidatedInt("Enter Team ID to add player: ", 1, teamCount);
    TeamData *team = &teamsData[teamID - 1];

    printf("Enter Player Details:\n");

    int playerID;
    while (1)
    {
        playerID = getValidatedInt("Player ID: ", 1, 1000);
        if (isDuplicatePlayerID(teamsData, playerID))
        {
            printf("Error: Player ID %d already exists! Try again.\n", playerID);
            continue;
        }
        break;
    }

    char playerName[50];
    printf("Name: ");
    getValidatedName(playerName, sizeof(playerName));

    int role = getValidatedRole();
    char roleName[20];
    if (role == 1) strcpy(roleName, "Batsman");
    else if (role == 2) strcpy(roleName, "Bowler");
    else strcpy(roleName, "All-rounder");

    int runs = getValidatedInt("Total Runs: ", 0, 1000000);
    float average = getValidatedFloat("Batting Average: ");
    float strikeRate = getValidatedFloat("Strike Rate: ");
    int wickets = getValidatedInt("Wickets: ", 0, 1000000);
    float economyRate = getValidatedFloat("Economy Rate: ");

    PlayerData pd;
    pd.playerId = playerID;
    strcpy(pd.name, playerName);
    strcpy(pd.teamName, team->teamName);
    strcpy(pd.role, roleName);
    pd.totalRuns = runs;
    pd.battingAverage = average;
    pd.strikeRate = strikeRate;
    pd.wickets = wickets;
    pd.economyRate = economyRate;
    pd.performanceIndex = computePerformanceIndex(pd);

    PlayerNode *mainNode = insertPlayerToMainList(&team->playersList, pd);
    if (mainNode == NULL)
    {
        printf("Failed to add player due to allocation error.\n");
        return;
    }
    team->totalPlayers++;

    if (role == 1) { insertPlayerIntoRoleList(&team->batsman, mainNode); team->totalBatsman++; }
    else if (role == 2) { insertPlayerIntoRoleList(&team->bowlers, mainNode); team->totalBowlers++; }
    else { insertPlayerIntoRoleList(&team->allRounders, mainNode); team->totalAllRounders++; }

    recalculateTeamAvgStrikeRate(team);

    printf("\nPlayer added successfully to Team %s!\n", team->teamName);
}

/* ---------- Average strike rate recalculation ---------- */

void recalculateTeamAvgStrikeRate(TeamData *team)
{
    float totalSR = 0.0f;
    int count = 0;
    PlayerNode *cur = team->playersList;
    while (cur != NULL)
    {
        if (strcmp(cur->data.role, "Batsman") == 0 || strcmp(cur->data.role, "All-rounder") == 0)
        {
            totalSR += cur->data.strikeRate;
            count++;
        }
        cur = cur->next;
    }
    if (count > 0) team->averageBattingStrikeRate = totalSR / count;
    else team->averageBattingStrikeRate = 0.0f;
}

/* ---------- Freeing memory ---------- */

void freeLinkedList(PlayerNode *head)
{
    PlayerNode *tmp;
    while (head != NULL)
    {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

void freeRoleList(RoleNode *head)
{
    RoleNode *tmp;
    while (head != NULL)
    {
        tmp = head;
        head = head->next;
        free(tmp); 
    }
}

void freeAllMemory(TeamData teamsData[])
{
    for (int i = 0; i < teamCount; ++i)
    {
       
        freeRoleList(teamsData[i].batsman);
        freeRoleList(teamsData[i].bowlers);
        freeRoleList(teamsData[i].allRounders);

        teamsData[i].batsman = NULL;
        teamsData[i].bowlers = NULL;
        teamsData[i].allRounders = NULL;

        
        freeLinkedList(teamsData[i].playersList);
        teamsData[i].playersList = NULL;
    }
}

/* ---------- main ---------- */

int main()
{
    TeamData teamsData[MAX_TEAMS];
    initialiseTeamsData(teamsData);
    loadInitialPlayers(teamsData);

    printMenu();

    while (1)
    {
        int choice = getValidatedInt("\nEnter your choice: ", 1, 6);
        if (!validChoice(choice))
        {
            printf("Invalid choice! Try again.\n");
            continue;
        }

        switch (choice)
        {
            case 1:
                addNewPlayers(teamsData);
                break;
            case 2:
            {
                int id = getValidatedInt("Enter Team ID :", 1, teamCount);
                displayPlayerOFTeam(teamsData, id);
                break;
            }
            case 3:
                displayTeamsByAvgBattingSR(teamsData);
                break;
            case 4:
                displayTopKPlayers(teamsData);
                break;
            case 5:
                displayPlayersAccordingToRole(teamsData);
                break;
            case 6:
                printf("exiting program!!!");
                freeAllMemory(teamsData);
                exit(0);
            default:
                printf("Invalid choice! Try again.\n");
        }
        printMenu();
    }

    return 0;
}
