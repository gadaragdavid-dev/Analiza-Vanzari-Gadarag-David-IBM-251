#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RECORDS 11000
#define MAX_ITEMS 500 // Număr estimat de produse/categorii unice

typedef struct {
    char key[100];
    double total_revenue;
} Stat;

typedef struct {
    char date[20];
    char name[100], cat[100], subcat[100], country[100], city[100];
    double rev;
    int month;
} Sale;

// Funcție pentru a sorta descrescător după venit
int compareStats(const void *a, const void *b) {
    double diff = ((Stat*)b)->total_revenue - ((Stat*)a)->total_revenue;
    return (diff > 0) ? 1 : (diff < 0) ? -1 : 0;
}

// Funcție pentru a adăuga sau actualiza o valoare într-un tabel de statistici
void update_stat(Stat *stats, int *size, char *key, double val) {
    for (int i = 0; i < *size; i++) {
        if (strcmp(stats[i].key, key) == 0) {
            stats[i].total_revenue += val;
            return;
        }
    }
    strcpy(stats[*size].key, key);
    stats[*size].total_revenue = val;
    (*size)++;
}

int main() {
    FILE *f = fopen("sales.csv", "r");
    if (!f) return 1;

    Sale *s = malloc(MAX_RECORDS * sizeof(Sale));
    char line[1024];
    int count = 0;
    fgets(line, 1024, f); // Skip header

    // Tabele pentru agregare
    Stat prodStats[MAX_RECORDS], catStats[MAX_ITEMS], cityStats[MAX_RECORDS];
    int pSize = 0, cSize = 0, citySize = 0;
    double monthlyRev[13] = {0};

    while (fgets(line, 1024, f) && count < MAX_RECORDS) {
        char *token = strtok(line, ",");
        int y, m, d;
        sscanf(token, "%d-%d-%d", &y, &m, &d);
        
        strtok(NULL, ","); // skip ID
        char *name = strtok(NULL, ",");
        char *cat = strtok(NULL, ",");
        char *subcat = strtok(NULL, ",");
        double price = atof(strtok(NULL, ","));
        int qty = atoi(strtok(NULL, ","));
        char *country = strtok(NULL, ",");
        char *city = strtok(NULL, ",");
        if(city) city[strcspn(city, "\r\n")] = 0;

        double rev = price * qty;
        monthlyRev[m] += rev;
        
        update_stat(prodStats, &pSize, name, rev);
        update_stat(catStats, &cSize, cat, rev);
        
        char cityKey[200];
        sprintf(cityKey, "%s (%s)", city, country);
        update_stat(cityStats, &citySize, cityKey, rev);
        
        count++;
    }
    fclose(f);

    // --- 1. VENIT LUNAR ---
    printf("1. VENIT TOTAL PE LUNA:\n");
    for(int i=1; i<=12; i++) printf("   Luna %02d: %.2f\n", i, monthlyRev[i]);

    // --- 2. TOP 5 PRODUSE ---
    qsort(prodStats, pSize, sizeof(Stat), compareStats);
    printf("\n2. TOP 5 PRODUSE (Dupa venitul total):\n");
    for(int i=0; i<5 && i<pSize; i++) printf("   %d. %s: %.2f\n", i+1, prodStats[i].key, prodStats[i].total_revenue);

    // --- 3. DISTRIBUTIE CATEGORII ---
    qsort(catStats, cSize, sizeof(Stat), compareStats);
    printf("\n3. DISTRIBUTIE PE CATEGORII:\n");
    for(int i=0; i<cSize; i++) printf("   - %s: %.2f\n", catStats[i].key, catStats[i].total_revenue);

    // --- 4. TOP ORASE ---
    qsort(cityStats, citySize, sizeof(Stat), compareStats);
    printf("\n4. TOP ORASE (Cele mai mari vanzari):\n");
    for(int i=0; i<10 && i<citySize; i++) printf("   - %s: %.2f\n", cityStats[i].key, cityStats[i].total_revenue);

    // --- 5. TENDINTE (Exemplu Crestere/Scadere) ---
    printf("\n5. TENDINTE (Analiza Ianuarie vs Decembrie):\n");
    printf("   - Venitul in Ianuarie: %.2f\n", monthlyRev[1]);
    printf("   - Venitul in Decembrie: %.2f\n", monthlyRev[12]);
    if(monthlyRev[12] > monthlyRev[1]) printf("   => Tendinta generala: CRESTERE la final de an.\n");
    else printf("   => Tendinta generala: SCADERE la final de an.\n");

    free(s);
    return 0;
}