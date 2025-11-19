// ICCODIPlayerPerformanceAnalyzer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "Players_data.h"   // must provide players[], playerCount, teams[], teamCount

/* ---------------------- MACROS ---------------------- */
#define MAX_NAME_LEN 50
#define MAX_TEAMS 10
#define MAX_HEAP_SIZE 512   /* safe upper bound for temporary heaps */

/* ---------------------- ENUMS & TYPES ---------------------- */
typedef enum {
    ROLE_BATSMAN = 1,
    ROLE_BOWLER  = 2,
    ROLE_ALL_ROUNDER = 3
} RoleType;

typedef struct PlayerData {
    int playerId;
    char name[MAX_NAME_LEN];
    char teamName[MAX_NAME_LEN];
    RoleType role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    int performanceIndex;
} PlayerData;

/* Each RoleNode owns the PlayerData (no duplication). */
typedef struct RoleNode {
    PlayerData data;
    struct RoleNode *next;
} RoleNode;

typedef struct TeamData {
    int teamId;
    char teamName[MAX_NAME_LEN];

    int totalPlayers;

    /* Maintain cumulative strike rate (batters + all-rounders) for O(1) updates */
    float cumulativeStrikeRate;
    int strikeRatePlayersCount;
    float averageBattingStrikeRate;

    /* Three role lists (sorted descending by performanceIndex) */
    RoleNode *batsman;
    int totalBatsman;

    RoleNode *bowlers;
    int totalBowlers;

    RoleNode *allRounders;
    int totalAllRounders;
} TeamData;

/* Index structures for O(log t) lookups */
typedef struct TeamIndexById {
    int teamId;
    int idx; /* index in teamsData[] */
} TeamIndexById;

typedef struct TeamIndexByName {
    char name[MAX_NAME_LEN];
    int idx;
} TeamIndexByName;

/* Global indices */
TeamIndexById teamIdIndex[MAX_TEAMS];
int teamIdIndexCount = 0;
TeamIndexByName teamNameIndex[MAX_TEAMS];
int teamNameIndexCount = 0;

/* ---------------------- FUNCTION PROTOTYPES ---------------------- */
/* init / load */
void initialiseTeamsData(TeamData teamsData[]);
int findTeamIndexByName(const char teamName[]);
int findTeamIndexById(int teamId);
int compareTeamNameIndex(const void *a, const void *b);
int compareTeamIdIndex(const void *a, const void *b);
int computePerformanceIndex(const PlayerData *p);
void loadInitialPlayers(TeamData teamsData[]);

/* role list insertion (maintain sorted by performanceIndex desc) */
void insertIntoRoleList(RoleNode **head, PlayerData pdata);

/* printing and display */
void printMenu(void);
void printPlayerTableHeader(void);
void printRoleTableHeader(void);
void displayPlayerOfTeam(TeamData teamsData[], int teamID);
void displayTeamsByAvgBattingSR(TeamData teamsData[]);
void displayTopKPlayers(TeamData teamsData[]);
void displayPlayersAccordingToRole(TeamData teamsData[]);

/* heap (RoleNode* heap used for k-way merge) */
void heapifyDown(RoleNode **heap, int size, int index);
void buildMaxHeap(RoleNode **heap, int size);
RoleNode* extractMaxRole(RoleNode **heap, int *size);

/* helpers & input validation */
bool isEmptyorSpaces(const char *input);
bool isNumber(const char *input);
bool inputValidation(char input[], int inputSize);
bool isFloatNumber(const char *input);
bool inputFloatValidation(char input[], int inputSize);
bool inputNameValidation(char input[], int inputSize);

int getValidatedInt(const char *prompt, int minVal, int maxVal);
float getValidatedFloat(const char *prompt);
void getValidatedName(char *out, int outSize);
RoleType getValidatedRole(void);

/* misc operations */
bool isDuplicatePlayerID(TeamData teamsData[], int playerID);
void recalculateTeamAvgStrikeRate_O1_onInsert(TeamData *team, const PlayerData *p);
void freeRoleList(RoleNode *head);
void freeAllMemory(TeamData teamsData[]);

/* ---------------------- IMPLEMENTATION ---------------------- */

void printMenu(void)
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

bool isEmptyorSpaces(const char *input)
{
    if (!input || input[0] == '\0') return true;
    for (int i = 0; input[i]; ++i) {
        if (!isspace((unsigned char)input[i])) return false;
    }
    return true;
}

bool isNumber(const char *input)
{
    int idx = 0;
    while (input[idx] && isspace((unsigned char)input[idx])) idx++;
    if (input[idx] && !isdigit((unsigned char)input[idx])) return false;
    while (input[idx] && isdigit((unsigned char)input[idx])) idx++;
    while (input[idx] && isspace((unsigned char)input[idx])) idx++;
    return input[idx] == '\0';
}

bool inputValidation(char input[], int inputSize)
{
    while (1) {
        if (fgets(input, inputSize, stdin) == NULL) {
            printf("Input error! Enter again : ");
            continue;
        }
        input[inputSize - 1] = '\0';             /* ensure termination */
        input[strcspn(input, "\n")] = '\0';
        if (isEmptyorSpaces(input)) {
            printf("Input cannot be empty!\nEnter again : ");
            continue;
        }
        if (!isNumber(input)) {
            printf("Invalid input! Enter a valid number \nEnter again : ");
            continue;
        }
        return true;
    }
}

bool isFloatNumber(const char *input)
{
    int idx = 0; bool hasDecimal = false; bool hasDigit = false;
    while (input[idx] && isspace((unsigned char)input[idx])) idx++;
    if (input[idx] != '.' && !isdigit((unsigned char)input[idx])) return false;
    while (input[idx]) {
        if (isdigit((unsigned char)input[idx])) hasDigit = true;
        else if (input[idx] == '.') {
            if (hasDecimal) return false;
            hasDecimal = true;
        } else if (isspace((unsigned char)input[idx])) {
            while (input[idx]) {
                if (!isspace((unsigned char)input[idx])) return false;
                idx++;
            }
            break;
        } else return false;
        idx++;
    }
    return hasDigit;
}

bool inputFloatValidation(char input[], int inputSize)
{
    if (fgets(input, inputSize, stdin) == NULL) return false;
    input[inputSize - 1] = '\0';
    input[strcspn(input, "\n")] = '\0';
    if (isEmptyorSpaces(input)) {
        printf("Input cannot be empty!\n");
        return false;
    }
    if (!isFloatNumber(input)) {
        printf("Invalid input! Enter a valid decimal number.\n");
        return false;
    }
    return true;
}

bool inputNameValidation(char input[], int inputSize)
{
    if (fgets(input, inputSize, stdin) == NULL) return false;
    input[inputSize - 1] = '\0';
    input[strcspn(input, "\n")] = '\0';
    if (isEmptyorSpaces(input)) {
        printf("Name cannot be empty!\n");
        return false;
    }
    int i = 0;
    while (input[i] && isspace((unsigned char)input[i])) i++;
    for (; input[i]; ++i) {
        if (!isalpha((unsigned char)input[i]) && !isspace((unsigned char)input[i])) {
            printf("Invalid name! Only alphabets and spaces allowed.\n");
            return false;
        }
    }
    return true;
}

/* Reusable prompt helpers */
int getValidatedInt(const char *prompt, int minVal, int maxVal)
{
    char buf[64];
    int val;
    while (1) {
        if (prompt) printf("%s", prompt);
        if (!inputValidation(buf, sizeof(buf))) continue;
        val = atoi(buf);
        if (val < minVal || val > maxVal) {
            printf("Value must be between %d and %d. Try again.\n", minVal, maxVal);
            continue;
        }
        return val;
    }
}

float getValidatedFloat(const char *prompt)
{
    char buf[64];
    float val;
    while (1) {
        if (prompt) printf("%s", prompt);
        if (!inputFloatValidation(buf, sizeof(buf))) continue;
        val = atof(buf);
        return val;
    }
}

void getValidatedName(char *out, int outSize)
{
    while (1) {
        if (fgets(out, outSize, stdin) == NULL) { printf("Input error\n"); continue; }
        out[outSize - 1] = '\0';
        out[strcspn(out, "\n")] = '\0';
        if (isEmptyorSpaces(out)) { printf("Name cannot be empty! Enter again: "); continue; }
        int i = 0;
        while (out[i] && isspace((unsigned char)out[i])) i++;
        bool ok = true;
        for (; out[i]; ++i) {
            if (!isalpha((unsigned char)out[i]) && !isspace((unsigned char)out[i])) { ok = false; break; }
        }
        if (!ok) { printf("Invalid name! Only alphabets and spaces allowed. Enter again: "); continue; }
        return;
    }
}

RoleType getValidatedRole(void)
{
    while (1) {
        int r = getValidatedInt("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3);
        if (r == 1) return ROLE_BATSMAN;
        if (r == 2) return ROLE_BOWLER;
        return ROLE_ALL_ROUNDER;
    }
}

/* -------- team index helpers for O(log t) ---------- */
int compareTeamNameIndex(const void *a, const void *b)
{
    const TeamIndexByName *A = a;
    const TeamIndexByName *B = b;
    return strcmp(A->name, B->name);
}
int compareTeamIdIndex(const void *a, const void *b)
{
    const TeamIndexById *A = a;
    const TeamIndexById *B = b;
    if (A->teamId < B->teamId) return -1;
    if (A->teamId > B->teamId) return 1;
    return 0;
}

int findTeamIndexByName(const char teamName[])
{
    int lo = 0, hi = teamNameIndexCount - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int cmp = strcmp(teamNameIndex[mid].name, teamName);
        if (cmp == 0) return teamNameIndex[mid].idx;
        if (cmp < 0) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

int findTeamIndexById(int teamId)
{
    int lo = 0, hi = teamIdIndexCount - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (teamIdIndex[mid].teamId == teamId) return teamIdIndex[mid].idx;
        if (teamIdIndex[mid].teamId < teamId) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

/* Compute performance index as integer */
int computePerformanceIndex(const PlayerData *p)
{
    float perf = 0.0f;
    if (p->role == ROLE_BATSMAN) {
        perf = (p->battingAverage * p->strikeRate) / 100.0f;
    } else if (p->role == ROLE_BOWLER) {
        perf = (p->wickets * 2.0f) + (100.0f - p->economyRate);
    } else {
        perf = ((p->battingAverage * p->strikeRate) / 100.0f) + (p->wickets * 2.0f);
    }
    if (perf < 0) perf = 0;
    return (int)perf;
}

/* Insert into role list keeping descending performanceIndex order */
void insertIntoRoleList(RoleNode **head, PlayerData pdata)
{
    RoleNode *newNode = malloc(sizeof(RoleNode));
    if (!newNode) { printf("Memory allocation failed\n"); return; }
    newNode->data = pdata;
    newNode->next = NULL;

    if (*head == NULL || pdata.performanceIndex > (*head)->data.performanceIndex) {
        newNode->next = *head;
        *head = newNode;
        return;
    }
    RoleNode *cur = *head;
    while (cur->next != NULL && cur->next->data.performanceIndex > pdata.performanceIndex) cur = cur->next;
    newNode->next = cur->next;
    cur->next = newNode;
}

/* Initialise teams and build indices */
void initialiseTeamsData(TeamData teamsData[])
{
    for (int i = 0; i < teamCount && i < MAX_TEAMS; ++i) {
        teamsData[i].teamId = i + 1; /* initial id; we'll maintain index by id separately if header differs */
        strncpy(teamsData[i].teamName, teams[i], MAX_NAME_LEN-1);
        teamsData[i].teamName[MAX_NAME_LEN-1] = '\0';
        teamsData[i].totalPlayers = 0;
        teamsData[i].cumulativeStrikeRate = 0.0f;
        teamsData[i].strikeRatePlayersCount = 0;
        teamsData[i].averageBattingStrikeRate = 0.0f;
        teamsData[i].batsman = NULL; teamsData[i].totalBatsman = 0;
        teamsData[i].bowlers = NULL; teamsData[i].totalBowlers = 0;
        teamsData[i].allRounders = NULL; teamsData[i].totalAllRounders = 0;
    }

    /* Build name index for binary search (teamName -> index) */
    teamNameIndexCount = teamCount < MAX_TEAMS ? teamCount : MAX_TEAMS;
    for (int i = 0; i < teamNameIndexCount; ++i) {
        strncpy(teamNameIndex[i].name, teams[i], MAX_NAME_LEN-1);
        teamNameIndex[i].name[MAX_NAME_LEN-1] = '\0';
        teamNameIndex[i].idx = i;
    }
    qsort(teamNameIndex, teamNameIndexCount, sizeof(TeamIndexByName), compareTeamNameIndex);

    /* Build id index: assume team ids are 1..teamCount by default; if Players_data.h contains different IDs,
       you should fill teamIdIndex accordingly. We'll map teamId = (i+1) -> index i for now */
    teamIdIndexCount = teamCount < MAX_TEAMS ? teamCount : MAX_TEAMS;
    for (int i = 0; i < teamIdIndexCount; ++i) {
        teamIdIndex[i].teamId = i + 1;
        teamIdIndex[i].idx = i;
    }
    qsort(teamIdIndex, teamIdIndexCount, sizeof(TeamIndexById), compareTeamIdIndex);
}

/* Load initial players from Players_data.h into role lists */
void loadInitialPlayers(TeamData teamsData[])
{
    for (int i = 0; i < playerCount; ++i) {
        Player hdr = players[i]; /* type from header */
        PlayerData pd;
        pd.playerId = hdr.id;
        strncpy(pd.name, hdr.name, MAX_NAME_LEN-1); pd.name[MAX_NAME_LEN-1] = '\0';
        strncpy(pd.teamName, hdr.team, MAX_NAME_LEN-1); pd.teamName[MAX_NAME_LEN-1] = '\0';
        /* map hdr.role (string) to enum */
        if (strcmp(hdr.role, "Batsman") == 0) pd.role = ROLE_BATSMAN;
        else if (strcmp(hdr.role, "Bowler") == 0) pd.role = ROLE_BOWLER;
        else pd.role = ROLE_ALL_ROUNDER;
        pd.totalRuns = hdr.totalRuns;
        pd.battingAverage = hdr.battingAverage;
        pd.strikeRate = hdr.strikeRate;
        pd.wickets = hdr.wickets;
        pd.economyRate = hdr.economyRate;
        pd.performanceIndex = computePerformanceIndex(&pd);

        int tIndex = findTeamIndexByName(pd.teamName);
        if (tIndex == -1) continue;

        TeamData *team = &teamsData[tIndex];
        team->totalPlayers++;

        if (pd.role == ROLE_BATSMAN) {
            insertIntoRoleList(&team->batsman, pd);
            team->totalBatsman++;
            team->cumulativeStrikeRate += pd.strikeRate;
            team->strikeRatePlayersCount++;
        } else if (pd.role == ROLE_BOWLER) {
            insertIntoRoleList(&team->bowlers, pd);
            team->totalBowlers++;
        } else {
            insertIntoRoleList(&team->allRounders, pd);
            team->totalAllRounders++;
            team->cumulativeStrikeRate += pd.strikeRate;
            team->strikeRatePlayersCount++;
        }
        if (team->strikeRatePlayersCount > 0)
            team->averageBattingStrikeRate = team->cumulativeStrikeRate / team->strikeRatePlayersCount;
        else team->averageBattingStrikeRate = 0.0f;
    }
}

/* Print headers (kept same as original) */
void printPlayerTableHeader(void)
{
    printf(" ====================================================================================\n");
    printf("%-5s %-20s %-12s %6s %6s %7s %6s %6s %10s\n",
           "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "PerfIdx");
    printf(" ====================================================================================\n");
}

/* Display all players of a team (prints batsmen, bowlers, all-rounders sequentially)
   This preserves the previous column formatting. */
void displayPlayerOfTeam(TeamData teamsData[], int teamID)
{
    int idx = findTeamIndexById(teamID);
    if (idx == -1) {
        printf("Invalid Team ID!\n");
        return;
    }
    TeamData *team = &teamsData[idx];
    printf("Players of Team %s: \n", team->teamName);
    printPlayerTableHeader();

    /* print batsmen */
    RoleNode *cur = team->batsman;
    while (cur) {
        PlayerData *p = &cur->data;
        const char *roleStr = (p->role == ROLE_BATSMAN) ? "Batsman" : (p->role == ROLE_BOWLER ? "Bowler" : "All-rounder");
        printf("%-5d %-20s %-12s %6d %6.2f %7.2f %6d %6.2f %10d\n",
               p->playerId, p->name, roleStr, p->totalRuns,
               p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
        cur = cur->next;
    }
    /* print bowlers */
    cur = team->bowlers;
    while (cur) {
        PlayerData *p = &cur->data;
        const char *roleStr = (p->role == ROLE_BATSMAN) ? "Batsman" : (p->role == ROLE_BOWLER ? "Bowler" : "All-rounder");
        printf("%-5d %-20s %-12s %6d %6.2f %7.2f %6d %6.2f %10d\n",
               p->playerId, p->name, roleStr, p->totalRuns,
               p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
        cur = cur->next;
    }
    /* print all-rounders */
    cur = team->allRounders;
    while (cur) {
        PlayerData *p = &cur->data;
        const char *roleStr = (p->role == ROLE_BATSMAN) ? "Batsman" : (p->role == ROLE_BOWLER ? "Bowler" : "All-rounder");
        printf("%-5d %-20s %-12s %6d %6.2f %7.2f %6d %6.2f %10d\n",
               p->playerId, p->name, roleStr, p->totalRuns,
               p->battingAverage, p->strikeRate, p->wickets, p->economyRate, p->performanceIndex);
        cur = cur->next;
    }

    printf(" ====================================================================================\n");
    printf("Total Player: %d\n", team->totalPlayers);
    printf("Average Batting Strike Rate: %f\n", team->averageBattingStrikeRate);
}

/* Display teams sorted by average batting strike rate (descending) */
void displayTeamsByAvgBattingSR(TeamData teamsData[])
{
    printf("Teams Sorted by Average Batting Strike Rate\n");
    printf(" ====================================================================================\n");
    printf("%-5s %-15s %-12s %-14s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf(" ====================================================================================\n");

    /* Make a shallow copy of TeamData (we'll copy the struct values) */
    TeamData copy[MAX_TEAMS];
    int n = teamCount < MAX_TEAMS ? teamCount : MAX_TEAMS;
    memcpy(copy, teamsData, sizeof(TeamData) * n);

    /* bubble sort descending (keeps behavior similar to original) */
    for (int pass = 0; pass < n; ++pass) {
        for (int i = 0; i < n - pass - 1; ++i) {
            if (copy[i].averageBattingStrikeRate < copy[i + 1].averageBattingStrikeRate) {
                TeamData tmp = copy[i];
                copy[i] = copy[i + 1];
                copy[i + 1] = tmp;
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        printf("%-5d %-15s %-12.2f %-14d\n",
               copy[i].teamId,
               copy[i].teamName,
               copy[i].averageBattingStrikeRate,
               copy[i].totalPlayers);
    }
    printf(" ====================================================================================\n");
}

/* Top-K players of a specific team by role: lists are PRE-SORTED so traversal O(K) */
void displayTopKPlayers(TeamData teamsData[])
{
    int teamID = getValidatedInt("Enter Team ID: ", 1, INT_MAX);
    int idx = findTeamIndexById(teamID);
    if (idx == -1) { printf("Invalid Team ID!\n"); return; }
    TeamData *team = &teamsData[idx];

    RoleType role = getValidatedRole();
    RoleNode *head = NULL;
    int totalRole = 0;
    const char *roleName = (role == ROLE_BATSMAN) ? "Batsman" : (role == ROLE_BOWLER ? "Bowler" : "All-rounder");

    if (role == ROLE_BATSMAN) { head = team->batsman; totalRole = team->totalBatsman; }
    else if (role == ROLE_BOWLER) { head = team->bowlers; totalRole = team->totalBowlers; }
    else { head = team->allRounders; totalRole = team->totalAllRounders; }

    if (totalRole == 0) {
        printf("No players of role %s in team %s.\n", roleName, team->teamName);
        return;
    }

    int topK = getValidatedInt("Enter number of players: ", 1, totalRole);

    printf("Top %d %s of %s :\n", topK, roleName, team->teamName);
    printPlayerTableHeader();

    RoleNode *cur = head;
    int cnt = 0;
    while (cur && cnt < topK) {
        PlayerData *p = &cur->data;
        const char *roleStr = (p->role == ROLE_BATSMAN) ? "Batsman" : (p->role == ROLE_BOWLER ? "Bowler" : "All-rounder");
        printf("%-5d %-20s %-12s %6d %6.2f %7.2f %6d %6.2f %10d\n",
               p->playerId, p->name, roleStr, p->totalRuns,
               p->battingAverage, p->strikeRate, p->wickets,
               p->economyRate, p->performanceIndex);
        cur = cur->next;
        cnt++;
    }
}

/* Display all players across teams of a role using k-way merge (heap size <= t) */
void displayPlayersAccordingToRole(TeamData teamsData[])
{
    RoleType role = getValidatedRole();
    const char *roleName = (role == ROLE_BATSMAN) ? "Batsman" : (role == ROLE_BOWLER ? "Bowler" : "All-rounder");

    printf("\n%s of all teams:\n", roleName);
    printf(" ====================================================================================\n");
    printf("%-5s %-20s %-12s %-12s %6s %6s %7s %6s %10s\n",
           "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "PerfIdx");
    printf(" ====================================================================================\n");

    /* Build initial heap with head of each team's role list (<= teamCount entries) */
    RoleNode *heap[MAX_HEAP_SIZE];
    int heapSize = 0;
    int tcount = teamCount < MAX_TEAMS ? teamCount : MAX_TEAMS;

    for (int i = 0; i < tcount; ++i) {
        RoleNode *h = NULL;
        if (role == ROLE_BATSMAN) h = teamsData[i].batsman;
        else if (role == ROLE_BOWLER) h = teamsData[i].bowlers;
        else h = teamsData[i].allRounders;

        if (h) {
            heap[heapSize++] = h;
        }
    }

    if (heapSize == 0) {
        printf("No players found for role %s.\n", roleName);
        printf(" ====================================================================================\n");
        return;
    }

    buildMaxHeap(heap, heapSize);

    while (heapSize > 0) {
        RoleNode *top = extractMaxRole(heap, &heapSize);
        PlayerData *p = &top->data;
        const char *roleStr = (p->role == ROLE_BATSMAN) ? "Batsman" : (p->role == ROLE_BOWLER ? "Bowler" : "All-rounder");
        printf("%-5d %-20s %-12s %-12s %6d %6.2f %7.2f %6d %10d\n",
               p->playerId, p->name, p->teamName, roleStr,
               p->totalRuns, p->battingAverage, p->strikeRate,
               p->wickets, p->performanceIndex);

        /* push next node from same team's role list (since role lists are independent per team) */
        if (top->next) {
            heap[heapSize++] = top->next;
            /* sift-up */
            int i = heapSize - 1;
            while (i > 0) {
                int parent = (i - 1) / 2;
                if (heap[parent]->data.performanceIndex < heap[i]->data.performanceIndex) {
                    RoleNode *tmp = heap[parent];
                    heap[parent] = heap[i];
                    heap[i] = tmp;
                    i = parent;
                } else break;
            }
        }
    }

    printf(" ====================================================================================\n");
}

/* Heap helpers (max-heap by performanceIndex) */
void heapifyDown(RoleNode **heap, int size, int index)
{
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < size && heap[left]->data.performanceIndex > heap[largest]->data.performanceIndex)
        largest = left;
    if (right < size && heap[right]->data.performanceIndex > heap[largest]->data.performanceIndex)
        largest = right;

    if (largest != index) {
        RoleNode *tmp = heap[index];
        heap[index] = heap[largest];
        heap[largest] = tmp;
        heapifyDown(heap, size, largest);
    }
}
void buildMaxHeap(RoleNode **heap, int size)
{
    for (int i = size / 2 - 1; i >= 0; --i) heapifyDown(heap, size, i);
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

/* Duplicate ID check across all role lists (teams) */
bool isDuplicatePlayerID(TeamData teamsData[], int playerID)
{
    int tcount = teamCount < MAX_TEAMS ? teamCount : MAX_TEAMS;
    for (int i = 0; i < tcount; ++i) {
        RoleNode *r = teamsData[i].batsman;
        while (r) { if (r->data.playerId == playerID) return true; r = r->next; }
        r = teamsData[i].bowlers;
        while (r) { if (r->data.playerId == playerID) return true; r = r->next; }
        r = teamsData[i].allRounders;
        while (r) { if (r->data.playerId == playerID) return true; r = r->next; }
    }
    return false;
}

/* Update strike rate summary on insert (O(1)) */
void recalculateTeamAvgStrikeRate_O1_onInsert(TeamData *team, const PlayerData *p)
{
    if (p->role == ROLE_BATSMAN || p->role == ROLE_ALL_ROUNDER) {
        team->cumulativeStrikeRate += p->strikeRate;
        team->strikeRatePlayersCount++;
    }
    if (team->strikeRatePlayersCount > 0)
        team->averageBattingStrikeRate = team->cumulativeStrikeRate / team->strikeRatePlayersCount;
    else team->averageBattingStrikeRate = 0.0f;
}

/* Free helpers */
void freeRoleList(RoleNode *head)
{
    RoleNode *tmp;
    while (head) {
        tmp = head; head = head->next; free(tmp);
    }
}
void freeAllMemory(TeamData teamsData[])
{
    int tcount = teamCount < MAX_TEAMS ? teamCount : MAX_TEAMS;
    for (int i = 0; i < tcount; ++i) {
        freeRoleList(teamsData[i].batsman); teamsData[i].batsman = NULL;
        freeRoleList(teamsData[i].bowlers); teamsData[i].bowlers = NULL;
        freeRoleList(teamsData[i].allRounders); teamsData[i].allRounders = NULL;
    }
}

/* Add new player flow (uses helper prompts). Maintains O(1) avg SR update and sorted role lists. */
void addNewPlayerInteractive(TeamData teamsData[])
{
    int teamID = getValidatedInt("Enter Team ID to add player: ", 1, INT_MAX);
    int tindex = findTeamIndexById(teamID);
    if (tindex == -1) { printf("Invalid Team ID!\n"); return; }
    TeamData *team = &teamsData[tindex];

    printf("Enter Player Details:\n");
    int playerID;
    while (1) {
        playerID = getValidatedInt("Player ID: ", 1, 1000000);
        if (isDuplicatePlayerID(teamsData, playerID)) {
            printf("Error: Player ID %d already exists! Try again.\n", playerID);
            continue;
        }
        break;
    }

    char playerName[MAX_NAME_LEN];
    printf("Name: ");
    getValidatedName(playerName, sizeof(playerName));

    RoleType role = getValidatedRole();
    const char *roleStr = (role == ROLE_BATSMAN) ? "Batsman" : (role == ROLE_BOWLER ? "Bowler" : "All-rounder");

    int runs = getValidatedInt("Total Runs: ", 0, INT_MAX);
    float average = getValidatedFloat("Batting Average: ");
    float strikeRate = getValidatedFloat("Strike Rate: ");
    int wickets = getValidatedInt("Wickets: ", 0, INT_MAX);
    float economyRate = getValidatedFloat("Economy Rate: ");

    PlayerData pd;
    pd.playerId = playerID;
    strncpy(pd.name, playerName, MAX_NAME_LEN-1); pd.name[MAX_NAME_LEN-1] = '\0';
    strncpy(pd.teamName, team->teamName, MAX_NAME_LEN-1); pd.teamName[MAX_NAME_LEN-1] = '\0';
    pd.role = role;
    pd.totalRuns = runs;
    pd.battingAverage = average;
    pd.strikeRate = strikeRate;
    pd.wickets = wickets;
    pd.economyRate = economyRate;
    pd.performanceIndex = computePerformanceIndex(&pd);

    /* insert into correct role list (keeps sorted by perf) */
    if (role == ROLE_BATSMAN) {
        insertIntoRoleList(&team->batsman, pd);
        team->totalBatsman++;
    } else if (role == ROLE_BOWLER) {
        insertIntoRoleList(&team->bowlers, pd);
        team->totalBowlers++;
    } else {
        insertIntoRoleList(&team->allRounders, pd);
        team->totalAllRounders++;
    }

    team->totalPlayers++;
    recalculateTeamAvgStrikeRate_O1_onInsert(team, &pd);

    printf("\nPlayer added successfully to Team %s!\n", team->teamName);
}

/* ---------- main ---------- */
int main(void)
{
    TeamData teamsData[MAX_TEAMS];
    initialiseTeamsData(teamsData);
    loadInitialPlayers(teamsData);

    printMenu();
    while (1) {
        int choice = getValidatedInt("\nEnter your choice: ", 1, 6);
        switch (choice) {
            case 1: addNewPlayerInteractive(teamsData); break;
            case 2: {
                int id = getValidatedInt("Enter Team ID : ", 1, INT_MAX);
                displayPlayerOfTeam(teamsData, id);
                break;
            }
            case 3: displayTeamsByAvgBattingSR(teamsData); break;
            case 4: displayTopKPlayers(teamsData); break;
            case 5: displayPlayersAccordingToRole(teamsData); break;
            case 6:
                printf("exiting program!!!");
                freeAllMemory(teamsData);
                return 0;
            default: printf("Invalid choice! Try again.\n"); break;
        }
        printMenu();
    }
    return 0;
}
