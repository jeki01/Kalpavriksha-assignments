#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "users.txt"

typedef struct {
    unsigned int id;       // id cannot be negative
    char name[50];
    unsigned short age;    // age cannot be negative
} User;

// Function prototypes
void addUser();
void displayUsers();
void updateUser();
void deleteUser();
unsigned int getUnsignedInt(const char *prompt);

int main() {
    int choice;

    while (1) {
        printf("\n--- User Management System ---\n");
        printf("1. Add User\n");
        printf("2. Display Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");

        choice = getUnsignedInt("Enter your choice: ");

        switch (choice) {
            case 1: addUser(); break;
            case 2: displayUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 5: exit(0);
            default: printf("Invalid choice!\n");
        }
    }
    return 0;
}

// Safe input function for unsigned integers
unsigned int getUnsignedInt(const char *prompt) {
    char buffer[20];
    unsigned int num;
    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("Error reading input. Try again.\n");
            continue;
        }
        if (sscanf(buffer, "%u", &num) != 1) {
            printf("Invalid input. Please enter a valid number.\n");
            continue;
        }
        return num;
    }
}

// Add user
void addUser() {
    FILE *fp = fopen(FILE_NAME, "a");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }

    User u;
    u.id = getUnsignedInt("Enter ID: ");

    printf("Enter Name: ");
    fgets(u.name, sizeof(u.name), stdin);
    u.name[strcspn(u.name, "\n")] = 0;

    u.age = (unsigned short)getUnsignedInt("Enter Age: ");

    fprintf(fp, "%u %s %hu\n", u.id, u.name, u.age);
    fclose(fp);
    printf("User added successfully!\n");
}

// Display users
void displayUsers() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("Error opening file.\n"); // Updated message
        return;
    }

    User u;
    printf("\n--- User Records ---\n");
    while (fscanf(fp, "%u %49s %hu", &u.id, u.name, &u.age) == 3) {
        printf("ID: %u | Name: %s | Age: %hu\n", u.id, u.name, u.age);
    }

    fclose(fp);
}

// Update user
void updateUser() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("Error opening file.\n"); // Updated message
        return;
    }

    FILE *temp = fopen("temp.txt", "w"); // Keeping temp file as in old structure
    if (temp == NULL) {
        printf("Error creating temp file.\n");
        fclose(fp);
        return;
    }

    unsigned int id;
    printf("Enter ID to update: ");
    id = getUnsignedInt("");

    User u;
    int found = 0;
    while (fscanf(fp, "%u %49s %hu", &u.id, u.name, &u.age) == 3) {
        if (u.id == id) {
            found = 1;
            printf("Enter new Name: ");
            fgets(u.name, sizeof(u.name), stdin);
            u.name[strcspn(u.name, "\n")] = 0;
            u.age = (unsigned short)getUnsignedInt("Enter new Age: ");
        }
        fprintf(temp, "%u %s %hu\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (found)
        printf("User updated successfully!\n");
    else
        printf("User with ID %u not found.\n", id);
}

// Delete user
void deleteUser() {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        printf("Error opening file.\n"); // Updated message
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Error creating temp file.\n");
        fclose(fp);
        return;
    }

    unsigned int id;
    printf("Enter ID to delete: ");
    id = getUnsignedInt("");

    User u;
    int found = 0;
    while (fscanf(fp, "%u %49s %hu", &u.id, u.name, &u.age) == 3) {
        if (u.id == id) {
            found = 1;
            continue;
        }
        fprintf(temp, "%u %s %hu\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (found)
        printf("User deleted successfully!\n");
    else
        printf("User with ID %u not found.\n", id);
}
