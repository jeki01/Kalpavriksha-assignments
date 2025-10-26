#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define max length for product name [cite: 54]
#define MAX_NAME_LENGTH 50

//structure definition
typedef struct {
    int productID;
    char productName[MAX_NAME_LENGTH + 1]; // 50 chars + 1 null terminator
    float price;
    int quantity;
} Product;

int getValidatedInt(const char* prompt, int min, int max) {
    int value;
    int scan_result;
    do {
        printf("%s (%d-%d): ", prompt, min, max);
        scan_result = scanf("%d", &value);

        // Clear the input buffer (to catch letters or extra input)
        while(getchar() != '\n'); 
        
        if (scan_result != 1 || value < min || value > max) {
            printf("Invalid input. Please enter a number between %d and %d.\n", min, max);
        }
    } while (scan_result != 1 || value < min || value > max);
    return value;
}

float getValidatedFloat(const char* prompt, float min, float max) {
    float value;
    int scan_result;
    do {
        printf("%s (%.0f-%.0f): ", prompt, min, max);
        scan_result = scanf("%f", &value);

        while(getchar() != '\n');
        
        if (scan_result != 1 || value < min || value > max) {
            printf("Invalid input. Please enter a number between %.0f and %.0f.\n", min, max);
        }
    } while (scan_result != 1 || value < min || value > max);
    return value;
}



// Displays the main menu
void displayMenu() {
    printf("\n========= INVENTORY MENU =========\n");
    printf("1. Add New Product\n");
    printf("2. View All Products\n");
    printf("3. Update Quantity\n");
    printf("4. Search Product by ID\n");
    printf("5. Search Product by Name\n");
    printf("6. Search Product by Price Range\n");
    printf("7. Delete Product\n");
    printf("8. Exit\n");
    printf("==================================\n");
    printf("Enter your choice: ");
}

// Adds a new product by reallocating memory
void addProduct(Product **inventory, int *count) {
    *inventory = (Product*)realloc(*inventory, (*count + 1) * sizeof(Product));

    if (*inventory == NULL) {
        printf("Error: Memory allocation failed!\n");
        return;
    }

    Product *newProduct = &((*inventory)[*count]);
    int scan_result;

    printf("Enter new product details:\n");

    // Use helpers for validated input
    newProduct->productID = getValidatedInt("Product ID", 1, 10000); // [cite: 52]
    
    // Special validation for string
    do {
        printf("Product Name (1-50 chars): "); 
        scan_result = scanf(" %50[^\n]", newProduct->productName);
        while(getchar() != '\n');
        if (scan_result != 1) {
            printf("Name cannot be empty. Please try again.\n");
            newProduct->productName[0] = '\0';
        }
    } while (scan_result != 1);

    newProduct->price = getValidatedFloat("Product Price", 0, 100000); 
    newProduct->quantity = getValidatedInt("Product Quantity", 0, 1000000);

    (*count)++;
    printf("Product added successfully!\n");
}

// Displays all products currently in the inventory
void viewAllProducts(Product *inventory, int count) {
    printf("\n========= PRODUCT LIST =========\n");
    if (count == 0) {
        printf("Inventory is empty.\n");
    } else {
        for (int i = 0; i < count; i++) {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   inventory[i].productID,
                   inventory[i].productName,
                   inventory[i].price,
                   inventory[i].quantity);
        }
    }
    printf("================================\n");
}

// Finds a product by ID and updates its quantity
void updateQuantity(Product *inventory, int count) {
    int searchID;
    int found = 0;
    printf("Enter Product ID to update quantity: ");
    scanf("%d", &searchID);
    while(getchar() != '\n');

    for (int i = 0; i < count; i++) {
        if (inventory[i].productID == searchID) {
    
            inventory[i].quantity = getValidatedInt("Enter new Quantity", 0, 1000000);
            printf("Quantity updated successfully!\n");
            found = 1;
            break;
        }
    }
    
    if (!found) {
        printf("Error: Product with ID %d not found.\n", searchID);
    }
}

// Finds and displays a single product by its ID
void searchByID(Product *inventory, int count) {
    int searchID;
    int found = 0;
    printf("Enter Product ID to search: ");
    scanf("%d", &searchID);
    while(getchar() != '\n'); 

    for (int i = 0; i < count; i++) {
        if (inventory[i].productID == searchID) {
            printf("Product Found:\n");
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   inventory[i].productID,
                   inventory[i].productName,
                   inventory[i].price,
                   inventory[i].quantity);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Error: Product with ID %d not found.\n", searchID);
    }
}

// Finds products by a partial name match (substring)
void searchByName(Product *inventory, int count) {
    char searchName[MAX_NAME_LENGTH + 1];
    int found = 0;

    printf("Enter name to search (partial allowed): ");
    int scan_result = scanf(" %50[^\n]", searchName);
    while(getchar() != '\n'); 

    if(scan_result != 1) {
        printf("No name entered. Aborting search.\n");
        return;
    }

    printf("Products Found:\n");
    for (int i = 0; i < count; i++) {
        if (strstr(inventory[i].productName, searchName) != NULL) {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   inventory[i].productID,
                   inventory[i].productName,
                   inventory[i].price,
                   inventory[i].quantity);
            found = 1;
        }
    }

    if (!found) {
        printf("No products found matching '%s'.\n", searchName);
    }
}

// Finds and displays products within a given price range
void searchByPriceRange(Product *inventory, int count) {
    float minPrice, maxPrice;
    int found = 0;

    // Use helpers to get valid price range
    minPrice = getValidatedFloat("Enter minimum price", 0, 100000); 
    maxPrice = getValidatedFloat("Enter maximum price", minPrice, 100000); 

    printf("Products in price range (%.2f - %.2f):\n", minPrice, maxPrice);

    for (int i = 0; i < count; i++) {
        if (inventory[i].price >= minPrice && inventory[i].price <= maxPrice) {
            printf("Product ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   inventory[i].productID,
                   inventory[i].productName,
                   inventory[i].price,
                   inventory[i].quantity);
            found = 1;
        }
    }

    if (!found) {
        printf("No products found in this price range.\n");
    }
}

// Deletes a product by ID
void deleteProduct(Product **inventory, int *count) {
    int deleteID;
    printf("Enter Product ID to delete: ");
    scanf("%d", &deleteID);
    while(getchar() != '\n');

    int foundIndex = -1;
    for (int i = 0; i < *count; i++) {
        if ((*inventory)[i].productID == deleteID) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) {
        printf("Error: Product with ID %d not found.\n", deleteID);
        return;
    }

    for (int i = foundIndex; i < *count - 1; i++) {
        (*inventory)[i] = (*inventory)[i + 1];
    }

    (*count)--;

    if (*count > 0) {
        *inventory = (Product*)realloc(*inventory, (*count) * sizeof(Product));
    } else {
        free(*inventory);
        *inventory = NULL;
    }
    
    printf("Product deleted successfully!\n");
}


// --- Main Function ---

int main() {
    Product *inventory = NULL;
    int productCount = 0;
    int initialCount = 0;
    int choice = 0;

    // 1. Get initial number of products (with validation)
    initialCount = getValidatedInt("Enter initial number of products", 1, 100); // [cite: 51]
    productCount = initialCount;

    // 2. Allocate initial memory using calloc
    inventory = (Product*)calloc(initialCount, sizeof(Product));
    if (inventory == NULL) {
        printf("Initial memory allocation failed!\n");
        return 1;
    }

    // 3. Get details for the initial products (with validation)
    for (int i = 0; i < productCount; i++) {
        printf("Enter details for product %d:\n", i + 1);
        int scan_result;
        
        inventory[i].productID = getValidatedInt("Product ID", 1, 10000); // [cite: 52]

        do {
            printf("Product Name (1-50 chars): "); // [cite: 54]
            scan_result = scanf(" %50[^\n]", inventory[i].productName);
            while(getchar() != '\n');
            if (scan_result != 1) {
                printf("Name cannot be empty. Please try again.\n");
                inventory[i].productName[0] = '\0';
            }
        } while (scan_result != 1);

        inventory[i].price = getValidatedFloat("Product Price", 0, 100000); // [cite: 55]
        inventory[i].quantity = getValidatedInt("Product Quantity", 0, 1000000); // [cite: 56]
    }

    // 4. Main menu loop
    do {
        displayMenu();
        
        // Get user choice for menu
        int scan_result = scanf("%d", &choice);
        while(getchar() != '\n'); // Clear buffer

        if (scan_result != 1) {
            choice = -1; // Set to invalid choice
        }

        switch (choice) {
            case 1: addProduct(&inventory, &productCount); break;
            case 2: viewAllProducts(inventory, productCount); break;
            case 3: updateQuantity(inventory, productCount); break;
            case 4: searchByID(inventory, productCount); break;
            case 5: searchByName(inventory, productCount); break;
            case 6: searchByPriceRange(inventory, productCount); break;
            case 7: deleteProduct(&inventory, &productCount); break;
            case 8:
                free(inventory);
                printf("Memory released successfully. Exiting program...\n");
                break;
            default:
                printf("Invalid choice. Please enter a number from 1 to 8.\n");
        }

    } while (choice != 8);

    return 0;
}