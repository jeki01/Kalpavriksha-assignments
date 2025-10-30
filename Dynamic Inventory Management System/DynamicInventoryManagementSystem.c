#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LENGTH 50

// ----- STRUCT -----
typedef struct {
    unsigned short productID;             // 1–10000
    char productName[MAX_NAME_LENGTH + 1];
    float price;
    unsigned int quantity;                // non-negative
} Product;

// ----- INPUT HELPERS -----
int getValidatedInt(const char *prompt, int min, int max) {
    int value;
    while (1) {
        printf("%s (%d-%d): ", prompt, min, max);
        if (scanf("%d", &value) == 1 && value >= min && value <= max) {
            while (getchar() != '\n');
            return value;
        }
        printf("Invalid input. Try again!\n");
        while (getchar() != '\n');
    }
}

float getValidatedFloat(const char *prompt, float min, float max) {
    float value;
    while (1) {
        printf("%s (%.0f-%.0f): ", prompt, min, max);
        if (scanf("%f", &value) == 1 && value >= min && value <= max) {
            while (getchar() != '\n');
            return value;
        }
        printf("Invalid input. Try again!\n");
        while (getchar() != '\n');
    }
}

// ----- UTILITIES -----
int idExists(const Product *inventory, int count, int id) {
    for (int i = 0; i < count; i++)
        if (inventory[i].productID == id) return 1;
    return 0;
}

void strToLower(const char *src, char *dst, size_t size) {
    size_t i;
    for (i = 0; i + 1 < size && src[i] != '\0'; i++)
        dst[i] = (char)tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

// ----- PRODUCT INPUT -----
void inputProductRest(Product *p) {
    printf("Product Name (1-50 chars): ");
    scanf(" %50[^\n]", p->productName);
    while (getchar() != '\n');

    p->price = getValidatedFloat("Price", 0, 100000);
    p->quantity = getValidatedInt("Quantity", 0, 1000000);
}

// ----- ADD PRODUCT -----
void addProduct(Product **inventory, int *count) {
    int id;
    do {
        id = getValidatedInt("Product ID", 1, 10000);
        if (idExists(*inventory, *count, id))
            printf("ID exists. Enter unique ID.\n");
        else break;
    } while (1);

    Product *temp = realloc(*inventory, (*count + 1) * sizeof(Product));
    if (!temp) {
        printf("Memory allocation failed!\n");
        return;
    }
    *inventory = temp;

    (*inventory)[*count].productID = (unsigned short)id;
    inputProductRest(&((*inventory)[*count]));
    (*count)++;

    printf("Product added successfully!\n");
}

// ----- VIEW PRODUCTS -----
void viewAllProducts(const Product *inventory, int count) {
    if (count == 0) {
        printf("No products in inventory.\n");
        return;
    }
    printf("\n========= PRODUCT LIST =========\n");
for (int i = 0; i < count; i++) {
    printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %u\n",
           inventory[i].productID,
           inventory[i].productName,
           inventory[i].price,
           inventory[i].quantity);
}
printf("\n========= PRODUCT LIST =========\n");
for (int i = 0; i < count; i++) {
    printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %u\n",
           inventory[i].productID,
           inventory[i].productName,
           inventory[i].price,
           inventory[i].quantity);
}
}

// ----- SEARCH BY ID -----
void searchByID(const Product *inventory, int count) {
    int id = getValidatedInt("Enter Product ID", 1, 10000);
    for (int i = 0; i < count; i++) {
        if (inventory[i].productID == id) {
            printf("Found: %s | Price: %.2f | Qty: %u\n",
                   inventory[i].productName,
                   inventory[i].price,
                   inventory[i].quantity);
            return;
        }
    }
    printf("Product not found.\n");
}

// ----- SEARCH BY NAME (case-insensitive) -----
void searchByName(const Product *inventory, int count) {
    char search[MAX_NAME_LENGTH + 1], tempName[MAX_NAME_LENGTH + 1], tempSearch[MAX_NAME_LENGTH + 1];
    printf("Enter product name: ");
    scanf(" %50[^\n]", search);
    while (getchar() != '\n');

    strToLower(search, tempSearch, sizeof(tempSearch));

    int found = 0;
    for (int i = 0; i < count; i++) {
        strToLower(inventory[i].productName, tempName, sizeof(tempName));
        if (strstr(tempName, tempSearch)) {
            printf("ID:%d | Name:%s | Price:%.2f | Qty:%u\n",
                   inventory[i].productID,
                   inventory[i].productName,
                   inventory[i].price,
                   inventory[i].quantity);
            found = 1;
        }
    }
    if (!found) printf("No matching product found.\n");
}

// ----- SEARCH BY PRICE RANGE -----
void searchByPriceRange(const Product *inventory, int count) {
    float min = getValidatedFloat("Min price", 0, 100000);
    float max = getValidatedFloat("Max price", min, 100000);
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (inventory[i].price >= min && inventory[i].price <= max) {
            printf("ID:%d | %s | Price:%.2f | Qty:%u\n",
                   inventory[i].productID,
                   inventory[i].productName,
                   inventory[i].price,
                   inventory[i].quantity);
            found = 1;
        }
    }
    if (!found) printf("No products in this range.\n");
}

// ----- UPDATE QUANTITY -----
void updateQuantity(Product *inventory, int count) {
    int id = getValidatedInt("Enter Product ID to update", 1, 10000);
    for (int i = 0; i < count; i++) {
        if (inventory[i].productID == id) {
            inventory[i].quantity = getValidatedInt("New Quantity", 0, 1000000);
            printf("Quantity updated.\n");
            return;
        }
    }
    printf("Product not found.\n");
}

// ----- DELETE PRODUCT -----
void deleteProduct(Product **inventory, int *count) {
    int id = getValidatedInt("Enter Product ID to delete", 1, 10000);
    int idx = -1;
    for (int i = 0; i < *count; i++)
        if ((*inventory)[i].productID == id) { idx = i; break; }

    if (idx == -1) { printf("Product not found.\n"); return; }

    int newCount = *count - 1;
    if (newCount == 0) {
        free(*inventory);
        *inventory = NULL;
    } else {
        Product *temp = malloc(newCount * sizeof(Product));
        if (!temp) { printf("Memory error.\n"); return; }
        for (int i = 0, j = 0; i < *count; i++)
            if (i != idx) temp[j++] = (*inventory)[i];
        free(*inventory);
        *inventory = temp;
    }
    *count = newCount;
    printf("Product deleted successfully!\n");
}

// ----- MENU -----
void displayMenu() {
    printf("\n========= INVENTORY MENU =========\n"
           "1. Add Product\n"
           "2. View Products\n"
           "3. Update Quantity\n"
           "4. Search by ID\n"
           "5. Search by Name\n"
           "6. Search by Price Range\n"
           "7. Delete Product\n"
           "8. Exit\n"
           "==================================\n"
           "Enter choice: ");
}

// ----- MAIN -----
int main() {
    Product *inventory = NULL;
    int productCount = 0;
    int initial = getValidatedInt("Enter number of initial products", 1, 100);

    inventory = calloc(initial, sizeof(Product));
    if (!inventory) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < initial; i++) {
        printf("\nEnter details for product %d:\n", i + 1);
        int id;
        do {
            id = getValidatedInt("Product ID", 1, 10000);
            if (idExists(inventory, productCount, id))
                printf("Duplicate ID! Try again.\n");
            else break;
        } while (1);
        inventory[i].productID = (unsigned short)id;
        inputProductRest(&inventory[i]);
        productCount++;
    }

    int choice;
    do {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid choice.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1: addProduct(&inventory, &productCount); break;
            case 2: viewAllProducts(inventory, productCount); break;
            case 3: updateQuantity(inventory, productCount); break;
            case 4: searchByID(inventory, productCount); break;
            case 5: searchByName(inventory, productCount); break;
            case 6: searchByPriceRange(inventory, productCount); break;
            case 7: deleteProduct(&inventory, &productCount); break;
            case 8: printf("Exiting...\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 8);

    free(inventory);
    return 0;
}
