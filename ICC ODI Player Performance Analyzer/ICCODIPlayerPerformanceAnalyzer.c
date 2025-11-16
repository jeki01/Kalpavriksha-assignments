#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include<math.h>
#include"Players_data.h"

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
}PlayerData;

typedef struct PlayerNode
{
    PlayerData data;
    struct PlayerNode *next;
}PlayerNode;

typedef struct TeamData
{
    int teamId;
    char teamName[50];
    int totalPlayers;
    float averageBattingStrikeRate;
    PlayerNode *playersList;

    PlayerNode *batsman;
    int totalBatsman;

    PlayerNode *bowlers;
    int totalBowlers;

    PlayerNode *allRounders;
    int totalAllRounders;
}TeamData;

void initialiseTeamsData( TeamData teamsData[]);
int findTeamIndex( const char teamName[] );
int computePerformanceIndex ( PlayerData playerData);
void insertPlayerToMainList( PlayerNode **head , PlayerData playerData);
void insertPlayerIntoRoleList ( PlayerNode **head , PlayerData playerData);
void loadInitialPlayers ( TeamData teamsData[]);


void printMenu();
bool validChoice(int choice);
void clearInputBuffer(void);
bool isEmptyorSpaces(const char input[]);
bool isNumber(const char input[]) ;
bool inputValidation( char input[] , int inputSize);
bool inputNameValidation(char input[] , int inputSize);
bool isFloatNumber(const char *input);
bool inputFloatValidation(char input[] , int inputSize);

void addNewPlayers(TeamData teamsData[]);
bool isDuplicatePlayerID(TeamData teamsData[], int playerID);
void recalculateTeamAvgStrikeRate(TeamData *team);

void displayPlayerOFTeam(TeamData teamsData[], int teamID);
void printNodes( PlayerNode *head);
void displayTeamsByAvgBattingSR( TeamData teamsData[] );
void displayTopKPlayers(TeamData teamsData[]);

void displayPlayersAccordingToRole( TeamData teamsData[]);
void heapifyDown(PlayerNode **heap, int size, int index);
void buildMaxHeap(PlayerNode **heap, int size);
PlayerNode* extractMax(PlayerNode **heap, int *size);
int createHeapFromLinkedList(PlayerNode *head, PlayerNode **heap);

void freeLinkedList(PlayerNode *head);
void freeAllMemory(TeamData teamsData[]);


int main()
{
    TeamData teamsData[10];
    initialiseTeamsData(teamsData);
    loadInitialPlayers(teamsData);

    printMenu();
    int choice;
    char inputID[10];
    int id;

    while(1)
    {
        char inputChoice[10];
        printf("\nEnter your choice: ");
        
        if( !inputValidation(inputChoice , sizeof(inputChoice)))
        {
            continue;
        }

        choice = atoi(inputChoice);

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
                printf("Enter Team ID :");
                if( !inputValidation(inputID , sizeof(inputID)))
                {
                    continue;
                }
                id = atoi(inputID);
                displayPlayerOFTeam( teamsData , id);
                printf("\n");
                break;
            case 3:
                displayTeamsByAvgBattingSR(teamsData);
                printf("\n");
                break;
            case 4:
                printf("\n");
                displayTopKPlayers(teamsData);
                break;
            case 5:
                printf("\n");
                displayPlayersAccordingToRole(teamsData);
                break;
            case 6:
                printf("exiting program!!!");
                freeAllMemory(teamsData);
                exit(0);
            default:
                printf("Invalid choice! Try again.\n");
                break;
        }
        printMenu();
    }
    return 0;
}

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
    if(choice < 1 || choice > 6)
    {
        return false;
    }
    return true;
}

void clearInputBuffer(void) 
{
    int userInput;
    while( (userInput = getchar()) != '\n'  && userInput!=EOF);
}

bool isEmptyorSpaces(const char input[]) 
{
    if (input[0] == '\0') 
    {
        return true;
    }
     for(int index=0 ; input[index] != '\0' ; index++)
    {
        if(!isspace(input[index]))
        {  
            return false; 
        }
    }
    return true;
}

bool isNumber(const char input[]) 
{
    int index=0 ;   
    while( input[index] && isspace((unsigned char) input[index]))
    {
        index++;
    }
    if( input[index] && !isdigit((unsigned char) input[index]))
    {
        return false;
    }
    while( input[index] && isdigit((unsigned char) input[index]))
    {
        index++;
    }
    while( input[index] && isspace((unsigned char) input[index]))
    {
        index++;
    }
    return input[index] == '\0'; 
}

bool inputValidation( char input[] ,  int inputSize)
{
    while(1)
    {
        if (fgets(input, inputSize, stdin) == NULL)
        {
            printf("Input error!\n");
            printf("Enter again : ");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        if (isEmptyorSpaces(input))
        {
            printf("Input cannot be empty!\n");
            printf("Enter again : ");
            continue;
        }

        if (!isNumber(input))
        {
            printf("Invalid input! Enter a valid number \n");
            printf("Enter again : ");
            continue;
        }

        return true;
    }
}

bool inputNameValidation(char input[], int inputSize)
{
    fgets(input, inputSize, stdin);
    input[strcspn(input, "\n")] = '\0';  

    if (isEmptyorSpaces(input))
    {
        printf("Name cannot be empty!\n");
        return false;
    }

    int index = 0;
    while (input[index] && isspace((unsigned char)input[index]))
    {
        index++;
    }

    while (input[index] && input[index] != '\0')
    {
        char current_char = input[index];

        if (!isalpha((unsigned char)current_char) && !isspace((unsigned char)current_char))
        {
            printf("Invalid name! Only alphabets and spaces allowed.\n");
            return false;
        }

        index++;
    }
    return true;
}

bool isFloatNumber(const char *input) 
{
    int index=0 ;
    bool hasDecimalPoint = false;   
    bool hasDigit = false;
    while( input[index] && isspace((unsigned char) input[index]))
    {
        index++;
    }
    if( input[index] !='.' && !isdigit((unsigned char) input[index]))
    {
        return false;
    }
    while( input[index])
    {
        if(isdigit((unsigned char) input[index]))
        {
            hasDigit = true;
        }
        else if (  input[index] =='.')
        {
            if( hasDecimalPoint)
            {
                return false;
            }
            hasDecimalPoint = true;
        }
        else if (isspace((unsigned char)input[index]))
        {
            while (input[index])
            {
                if (!isspace((unsigned char)input[index]))
                    return false;
                index++;
            }
            break;
        }
        else
        {
            return false;
        }  
        index++;
    }
    return hasDigit; 
}   

bool inputFloatValidation(char input[] , int inputSize)
{
    if (!fgets(input, inputSize, stdin))
    {
        return false;
    }

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

void initialiseTeamsData( TeamData teamsData[])
{
    for( int index=0 ; index<10 ; index++)
    {
        teamsData[index].teamId = index + 1;
        strcpy(teamsData[index].teamName , teams[index]);
        teamsData[index].totalPlayers = 0;
        teamsData[index].averageBattingStrikeRate = 0;

        teamsData[index].playersList = NULL;

        teamsData[index].batsman = NULL;
        teamsData[index].totalBatsman = 0;

        teamsData[index].bowlers = NULL;
        teamsData[index].totalBowlers = 0;

        teamsData[index].allRounders = NULL;
        teamsData[index].totalAllRounders = 0;
    }
}

int findTeamIndex(const char teamName[])
{
    for (int index = 0; index < teamCount; ++index)
    {
        if (strcmp(teams[index], teamName) == 0)
            return index;
    }
    return -1;
}

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

    if (performance_index < 0)
    {
        performance_index = 0;
    }

    return (int)performance_index;
}


void insertPlayerToMainList( PlayerNode **head , PlayerData playerData)
{
    PlayerNode *newPlayer = malloc( sizeof(PlayerNode)) ;
    if( newPlayer == NULL)
    {
        printf(" Mmemory allocation failed \n");
        return;
    }

    newPlayer->data = playerData;
    newPlayer->next = NULL;

    if (*head == NULL)
    {
        *head = newPlayer;
        return;
    }

    PlayerNode *currentPlayer = *head;
    while( currentPlayer->next != NULL )
    {
        currentPlayer = currentPlayer->next;
    }

    currentPlayer->next = newPlayer;
}

void insertPlayerIntoRoleList( PlayerNode **head , PlayerData playerData)
{
    PlayerNode *newPlayer = malloc( sizeof(PlayerNode));
    if( newPlayer == NULL)
    {
        printf(" Memory allocation failed \n");
        return;
    }

    newPlayer->data = playerData;
    newPlayer->next = NULL;

    if( *head == NULL || (playerData.performanceIndex > (*head)->data.performanceIndex))
    {
        newPlayer->next = *head;
        *head = newPlayer;
        return;
    }

    PlayerNode *currentPlayer = *head;
    while( currentPlayer->next != NULL && currentPlayer->next->data.performanceIndex > playerData.performanceIndex )
    {
        currentPlayer = currentPlayer->next;
    }
    newPlayer->next = currentPlayer->next;
    currentPlayer->next = newPlayer;
}

void loadInitialPlayers ( TeamData teamsData[])
{
    for( int index=0 ; index<playerCount ; index++)
    {
        Player headerPlayer = players[index];
        PlayerData playerData ;

        playerData.playerId = players[index].id;
        strcpy(playerData.name , players[index].name );
        strcpy(playerData.teamName , players[index].team );
        strcpy(playerData.role , players[index].role);
        playerData.totalRuns = players[index].totalRuns;
        playerData.battingAverage = players[index].battingAverage;
        playerData.strikeRate = players[index].strikeRate;
        playerData.wickets = players[index].wickets;
        playerData.economyRate = players[index].economyRate;
        playerData.performanceIndex = computePerformanceIndex( playerData);
        
        int teamIndex = findTeamIndex(playerData.teamName);
        if( teamIndex == -1 )
        {
            continue;
        }

        TeamData *team_ptr = &teamsData[teamIndex];

        team_ptr->totalPlayers++;
        insertPlayerToMainList( &team_ptr->playersList , playerData);

        if( strcmp ( playerData.role , "Batsman") == 0) 
        {
            insertPlayerIntoRoleList( &team_ptr->batsman , playerData);
            team_ptr->totalBatsman++;
        }
        else if( (strcmp(playerData.role, "Bowler") == 0) )
        {
            insertPlayerIntoRoleList( &team_ptr->bowlers , playerData);
            team_ptr->totalBowlers++;
        }
        else
        {
            insertPlayerIntoRoleList( &team_ptr->allRounders , playerData);
            team_ptr->totalAllRounders++;
        }
    }


    for (int index = 0; index < teamCount; index++)
    {
        recalculateTeamAvgStrikeRate(&teamsData[index]);
    }
  
}

void displayPlayerOFTeam( TeamData teamsData[] , int teamID)
{
    if (teamID < 1 || teamID > teamCount) 
    {
        printf("Invalid Team ID!\n");
        return;
    }

    int index = teamID - 1; 
    printf("Players of Team %s: \n" , teamsData[index].teamName);

    printf(" ====================================================================================\n");
    printf("%-5s %-20s %-12s %6s %6s %7s %6s %6s %10s\n",
            "ID", "Name", "Role", "Runs" , "Avg", "SR", "Wkts", "ER", "PerfIdx"
    );

    printNodes( teamsData[index].playersList);
    printf(" ====================================================================================\n");
    printf("Total Player: %d\n" , teamsData[index].totalPlayers);
    printf("Average Batting Strike Rate: %f\n",teamsData[index].averageBattingStrikeRate);
}

void printNodes( PlayerNode *head)
{
    PlayerNode* currentPlayer = head;
    while( currentPlayer != NULL)
    {
        printf("%-5d %-20s %-12s %6d %6.2f %7.2f %6d %6.2f %10d\n" , 
                currentPlayer->data.playerId , currentPlayer->data.name , currentPlayer->data.role , currentPlayer->data.totalRuns , currentPlayer->data.battingAverage,
                currentPlayer->data.strikeRate , currentPlayer->data.wickets , currentPlayer->data.economyRate , currentPlayer->data.performanceIndex
        );
        currentPlayer = currentPlayer->next;
    }
}

void displayTeamsByAvgBattingSR( TeamData teamsData[] )
{
    printf("Teams Sorted by Average Batting Strike Rate\n");
    printf(" ====================================================================================\n");
    printf("%-5s %-15s %-12s %-14s\n",
            "ID", "Team Name", "Avg Bat SR", "Total Players"
    );
    printf(" ====================================================================================\n");

    TeamData sortedTeams[10]; 
    memcpy(sortedTeams, teamsData, sizeof(TeamData) * teamCount);

    for (int pass = 0; pass < teamCount; pass++)
    {
        for (int current = 0; current < teamCount - pass - 1; current++)
        {
            if (sortedTeams[current].averageBattingStrikeRate < sortedTeams[current + 1].averageBattingStrikeRate)
            {
                TeamData temporaryTeam  = sortedTeams[current];
                sortedTeams[current] = sortedTeams[current + 1];
                sortedTeams[current + 1] = temporaryTeam;
            }
        }
    }

    for(int index = 0; index < teamCount; index++)
    {
        printf("%-5d %-15s %-12.2f %-14d\n",
               sortedTeams[index].teamId,
               sortedTeams[index].teamName,
               sortedTeams[index].averageBattingStrikeRate,
               sortedTeams[index].totalPlayers
        );
    }
    printf(" ====================================================================================\n");
}

void displayTopKPlayers(TeamData teamsData[])
{
    char inputID[10];
    char inputRole[10];
    char numberOfPlayers[10];
    int topKPlayers;
    int teamID;
    int role;
    char role_name[50];
    while(1)
    {
        printf("Enter Team ID:");
        if( !inputValidation(inputID , sizeof(inputID)))
        {
            continue;
        }
        teamID = atoi(inputID);
        if (teamID < 1 || teamID > teamCount) 
        {
            printf("Invalid Team ID!\n");
            continue;
        }
        break;
    }
    
    PlayerNode *roleList = NULL;
    int totalRolePlayers = 0;
    while(1)
    {
        printf("Enter Role ( 1-Batsman , 2-Bowler , 3-All-rounder): ");
        if( !inputValidation(inputRole , sizeof(inputRole)))
        {
            continue;
        }
        role = atoi(inputRole);
            
        if (role < 1 || role > 3) 
        {
            printf("Invalid role ID!\n");
            continue;
        }
        if( role == 1)
        {
            strcpy(role_name , "Batsman");
            roleList = teamsData[teamID-1].batsman;
            totalRolePlayers = teamsData[teamID-1].totalBatsman;
        }
        else if( role == 2)
        {
            strcpy(role_name , "Bowler");
            roleList = teamsData[teamID-1].bowlers;
            totalRolePlayers = teamsData[teamID-1].totalBowlers;
        }
        else
        {
            strcpy(role_name ,"All-rounder");
            roleList = teamsData[teamID-1].allRounders;
            totalRolePlayers = teamsData[teamID-1].totalAllRounders;
        }
        break;
    }

    while(1)
    {   printf("Enter number of players: ");
        if( !inputValidation(numberOfPlayers , sizeof(numberOfPlayers)))
        {
            continue;
        }
        topKPlayers = atoi(numberOfPlayers);
        if (topKPlayers < 1 || topKPlayers > totalRolePlayers) 
        {
            printf("K should be between 1 and %d\n", totalRolePlayers);
            continue;
        }
        break;
    }

    printf("Top %d %s of %s :\n" , topKPlayers , role_name , teamsData[teamID-1].teamName);
    printf(" ====================================================================================\n");
    printf("%-5s %-20s %-12s %6s  %6s %7s %6s %6s %10s\n",
            "ID", "Name", "Role", "Runs" , "Avg", "SR", "Wkts", "ER", "PerfIdx"
    );
    printf(" ====================================================================================\n");

    PlayerNode *head = roleList;
    int count =0;
    while(head != NULL && count < topKPlayers )
    {
        PlayerData *player_node = &head->data;

        printf("%-5d %-20s %-12s %6d %6.2f %7.2f %6d %6.2f %10d\n",
               player_node->playerId, player_node->name, player_node->role, player_node->totalRuns,
               player_node->battingAverage, player_node->strikeRate, player_node->wickets,
               player_node->economyRate, player_node->performanceIndex);

        head = head->next;
        count++;
    }
}

void displayPlayersAccordingToRole(TeamData teamsData[])
{
    char inputRole[10];
    int role;
    char role_name[50];

    while (1)
    {
        printf("Enter Role ( 1-Batsman , 2-Bowler , 3-All-rounder): ");
        if (!inputValidation(inputRole, sizeof(inputRole)))
        {
            continue;
        }

        role = atoi(inputRole);

        if (role == 1)      
        {
            strcpy(role_name, "Batsman");
        }
        else if (role == 2) 
        {
            strcpy(role_name, "Bowler");
        }
        else if (role == 3) 
        {
            strcpy(role_name, "All-rounder");
        }
        else
        {
            printf("Invalid role ID!\n");
            continue;
        }
        break;
    }

    printf("\n%s of all teams:\n", role_name);
    printf(" ====================================================================================\n");
    printf("%-5s %-20s %-12s %-12s %6s %6s %7s %6s %10s\n",
           "ID", "Name", "Team", "Role", "Runs", "Avg",
           "SR", "Wkts", "PerfIdx");
    printf(" ====================================================================================\n");

    PlayerNode *heap[500];
    int heapSize = 0;

    for (int teamIndex = 0; teamIndex  < teamCount; teamIndex ++)
    {
        PlayerNode *currentPlayer;

        if (role == 1) 
        {
            currentPlayer = teamsData[teamIndex ].batsman;
        }
        else if (role == 2) 
        {
            currentPlayer = teamsData[teamIndex ].bowlers;
        }
        else 
        {
            currentPlayer = teamsData[teamIndex ].allRounders;
        }

        while (currentPlayer != NULL)
        {
            heap[heapSize++] = currentPlayer;
            currentPlayer = currentPlayer->next;
        }
    }

    buildMaxHeap(heap, heapSize);

    while (heapSize > 0)
    {
        PlayerNode *currentPlayerNode = extractMax(heap, &heapSize);
        PlayerData *player_node = &currentPlayerNode->data;

        printf("%-5d %-20s %-12s %-12s %6d %6.2f %7.2f %6d %10d\n",
               player_node->playerId, player_node->name, player_node->teamName, player_node->role,
               player_node->totalRuns, player_node->battingAverage, player_node->strikeRate,
               player_node->wickets, player_node->performanceIndex
            );
    }

    printf(" ====================================================================================\n");
}

void heapifyDown(PlayerNode **heap, int size, int index)
{
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < size && heap[left]->data.performanceIndex > heap[largest]->data.performanceIndex)
    {
        largest = left;
    }

    if (right < size && heap[right]->data.performanceIndex > heap[largest]->data.performanceIndex)
    {
        largest = right;
    }

    if (largest != index)
    {
        PlayerNode *currentNode = heap[index];
        heap[index] = heap[largest];
        heap[largest] = currentNode;

        heapifyDown(heap, size, largest);
    }
}

void buildMaxHeap(PlayerNode **heap, int size)
{
    for (int index = size / 2 - 1; index >= 0; index--)
    {
        heapifyDown(heap, size, index);
    }
}

PlayerNode* extractMax(PlayerNode **heap, int *size)
{
    if (*size <= 0)
    {
        return NULL;
    }

    PlayerNode *maxNode = heap[0];
    heap[0] = heap[*size - 1];
    (*size)--;

    heapifyDown(heap, *size, 0);

    return maxNode;
}

int createHeapFromLinkedList(PlayerNode *head, PlayerNode **heap)
{
    int count = 0;
    while (head != NULL)
    {
        heap[count++] = head;
        head = head->next;
    }
    return count;
}

bool isDuplicatePlayerID(TeamData teamsData[], int playerID)
{
    for (int index = 0;  index< teamCount; index++)
    {
        PlayerNode *lists[3] = {
            teamsData[index].batsman,
            teamsData[index].bowlers,
            teamsData[index].allRounders
        };

        for (int roleIndex = 0; roleIndex < 3; roleIndex++)
        {
            PlayerNode *currentNode = lists[roleIndex];
            while (currentNode != NULL)
            {
                if (currentNode->data.playerId == playerID)
                {
                    return true;
                }
                currentNode = currentNode->next;
            }
        }
    }
    return false;
}

void addNewPlayers(TeamData teamsData[])
{
    char inputID[10];
    char inputRole[10];
    char userInput[50];

    int teamID, role;
    char roleName[20];

    while (1)
    {
        printf("Enter Team ID to add player: ");
        if (!inputValidation(inputID, sizeof(inputID)))
        {
            continue;
        }

        teamID = atoi(inputID);

        if (teamID < 1 || teamID > teamCount)
        {
            printf("Invalid Team ID! Try again.\n");
            continue;
        }
        break;
    }

    TeamData *team = &teamsData[teamID - 1];

    printf("Enter Player Details:\n");
    int playerID;
    while (1)
    {
        printf("Player ID: ");
        if (!inputValidation(userInput, sizeof(userInput)))
        {
            continue;
        }

        playerID = atoi(userInput);

        if (playerID <= 0)
        {
            printf("Invalid Player ID! Must be positive.\n");
            continue;
        }

        if (isDuplicatePlayerID(teamsData, playerID))
        {
            printf("Error: Player ID %d already exists! Try again.\n", playerID);
            continue; 
        }
        break;
    }

    char playerName[50];
    printf("Name: ");
    while (!inputNameValidation(playerName, sizeof(playerName)))
    {
        printf("Enter again: ");
    }

    while (1)
    {
        printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
        if (!inputValidation(inputRole, sizeof(inputRole)))
        {
            continue;
        }

        role = atoi(inputRole);

        if (role < 1 || role > 3)
        {
            printf("Invalid Role! Try again.\n");
            continue;
        }

        if (role == 1) 
        {
            strcpy(roleName, "Batsman");
        }
        else if (role == 2) 
        {
            strcpy(roleName, "Bowler");
        }
        else 
        {
            strcpy(roleName, "All-rounder");
        }

        break;
    }

    int runs, wickets;
    float average, strikeRate, economyRate;

    printf("Total Runs: ");
    inputValidation(userInput, sizeof(userInput));
    runs = atoi(userInput);

    printf("Batting Average: ");
    inputFloatValidation(userInput, sizeof(userInput));
    average = atof(userInput);

    printf("Strike Rate: ");
    inputFloatValidation(userInput, sizeof(userInput));
    strikeRate = atof(userInput);

    printf("Wickets: ");
    inputValidation(userInput, sizeof(userInput));
    wickets = atoi(userInput);

    printf("Economy Rate: ");
    inputFloatValidation(userInput, sizeof(userInput));
    economyRate = atof(userInput);

    PlayerData player_data;

    player_data.playerId = playerID;
    strcpy(player_data.name, playerName);
    strcpy(player_data.teamName, team->teamName);
    strcpy(player_data.role, roleName);

    player_data.totalRuns = runs;
    player_data.battingAverage = average;
    player_data.strikeRate =strikeRate;
    player_data.wickets = wickets;
    player_data.economyRate = economyRate;

    player_data.performanceIndex = computePerformanceIndex(player_data);

    insertPlayerToMainList(&team->playersList, player_data);
    team->totalPlayers++;

    if (role == 1)
    {
        insertPlayerIntoRoleList(&team->batsman, player_data);
        team->totalBatsman++;
    }
    else if (role == 2)
    {
        insertPlayerIntoRoleList(&team->bowlers, player_data);
        team->totalBowlers++;
    }
    else
    {
        insertPlayerIntoRoleList(&team->allRounders, player_data);
        team->totalAllRounders++;
    }

    recalculateTeamAvgStrikeRate(team);

    printf("\nPlayer added successfully to Team %s!\n", team->teamName);
}

void recalculateTeamAvgStrikeRate(TeamData *team)
{
    float totalSR = 0.0f;
    int count = 0;

    PlayerNode *currentPlayer = team->playersList;
    while (currentPlayer != NULL)
    {
        if (strcmp(currentPlayer->data.role, "Batsman") == 0 ||
            strcmp(currentPlayer->data.role, "All-rounder") == 0)
        {
            totalSR += currentPlayer->data.strikeRate;
            count++;
        }
        currentPlayer = currentPlayer->next;
    }

    if (count > 0)
    {
        team->averageBattingStrikeRate = totalSR / count;
    }
    else
    {
        team->averageBattingStrikeRate = 0;
    }
}

void freeLinkedList(PlayerNode *head)
{
    PlayerNode *playerNodeToFree;
    while (head != NULL)
    {
        playerNodeToFree = head;
        head = head->next;
        free(playerNodeToFree);
    }
}

void freeAllMemory(TeamData teamsData[])
{
    for (int index = 0; index < teamCount; index++)
    {
        freeLinkedList(teamsData[index].playersList);
        freeLinkedList(teamsData[index].batsman);
        freeLinkedList(teamsData[index].bowlers);
        freeLinkedList(teamsData[index].allRounders);

        teamsData[index].playersList   = NULL;
        teamsData[index].batsman = NULL;
        teamsData[index].bowlers = NULL;
        teamsData[index].allRounders = NULL;
    }
}