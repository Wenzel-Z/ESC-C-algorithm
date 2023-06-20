// Zachary Wenzel
// Use -pthread flag to compile

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>

int balance = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct node {
    int useBit;
    int protected;
    int R;
    int M;
    long int owner;
    struct node *next;
};

struct node* last = NULL;

void insertNode() {
    struct node* temp;
    temp = (struct node*)malloc(sizeof(struct node));
 
   
    if (last == NULL) {
        temp->useBit = 0;
        temp->protected = 0;
        temp->R = 0;
        temp->M = 0;
        temp->next = temp;
        last = temp;
    }
 
    else {
        temp->useBit = 0;
        temp->protected = 0;
        temp->R = 0;
        temp->M = 0;
        temp->next = last->next;
        last->next = temp;
    }
}

void viewList()
{
    if (last == NULL)
        printf("\nList is empty\n");
    else {
        struct node* temp;
        temp = last->next;
        do {
            printf("\nR: %d M: %d Owner: %li", temp->R, temp->M, temp->owner);
            temp = temp->next;
        } while (temp != last->next);
    }
}

void handlePageFault(struct node* cur) {
    struct node* temp;
    temp = last;

    int class2 = 0;
    int class3 = 0;
    int class4 = 0;
    int i = 0;

    while(i<6) {
        if (temp->owner == 0) 
        {
            printf("Page Evicted: R: %d M: %d Owner: %li\n\n", temp->R, temp->M, temp->owner);
            
            temp->useBit = 1;
            temp->R = 0;
            temp->M = 0;
            temp->protected = 1;
            temp->owner = cur->owner;
            last = temp;
            return;
        }
        temp = temp->next;
        i++;
    }

    i = 0;
    while(i<6) {
        if (temp->M == 0 && temp->R == 0 && temp->protected == 1) 
        {
            printf("Page Evicted: R: %d M: %d Owner: %li\n\n", temp->R, temp->M, temp->owner);
            
            temp->useBit = 1;
            temp->R = 0;
            temp->M = 0;
            temp->protected = 1;
            temp->owner = cur->owner;
            last = temp;
            return;
        }
        else if (temp->M == 1 && temp->R == 0) 
        {
            class2 = class2 + 1;
        }
        else if (temp->M == 0 && temp->R == 1) 
        {
            class3 = class3 + 1;
        }
        else if (temp->M == 1 && temp->R == 1) 
        {
            class4 = class4 + 1;
        }
        temp = temp->next;
        i++;
    }

    i = 0;
    if (class2 >= 1) 
    {
        while(i < 6) 
        {
            if(temp->M == 1 && temp->R == 0) 
            {
            printf("Page Evicted: R: %d M: %d Owner: %li\n\n", temp->R, temp->M, temp->owner);
            
            temp->useBit = 1;
            temp->R = 0;
            temp->M = 0;
            temp->protected = 0;
            temp->owner = cur->owner;
            last = temp;
            return;               
            }
            temp = temp->next;
            i++;
        }
    }
    else if (class3 >= 1) 
    {
        while(i < 6) 
        {
            if(temp->M == 0 && temp->R == 1) 
            {
            printf("Page Evicted: R: %d M: %d Owner: %li\n\n", temp->R, temp->M, temp->owner);
            
            temp->useBit = 1;
            temp->R = 0;
            temp->M = 0;
            temp->protected = 1;
            temp->owner = cur->owner;
            last = temp;
            return;               
            }
            temp = temp->next;
            i++;
        }
    }
    else if (class4 >= 1) 
    {
        while(i < 6) 
        {
            if(temp->M == 1 && temp->R == 1) 
            {
            printf("Page Evicted: R: %d M: %d Owner: %li\n\n", temp->R, temp->M, temp->owner);
            
            temp->useBit = 1;
            temp->R = 0;
            temp->M = 0;
            temp->protected = 1;
            temp->owner = cur->owner;
            last = temp;
            return;               
            }
            temp = temp->next;
            i++;
        }
    }
}

void resetOwnedBits(struct node* cur) {
    long int owner = cur->owner;
    int i = 0;
    while (i < 6) {
        if (cur->owner == owner) {
            cur->M = 0;
            cur->R = 0;
        }
        cur = cur->next;
        i++;
    }
}

void resetAllRBits(struct node* cur) {
    int i = 0;
    while (i < 6) {
        cur->M = 0;
        cur->R = 0;
        cur = cur->next;
        i++;
    }
}

struct node* requestPage() {
    struct node* temp;
    temp = last;
    while(1) {
        if(temp->useBit == 0 && temp->protected == 0) {
            //printf("R: %d M: %d Owner: %li\n", temp->R, temp->M, temp->owner);
            temp->useBit = 1;
            temp->protected = 1;
            last = temp;
            return temp;
        } else {
            temp->useBit = 0;
            temp = temp->next;
        }
    }
}

int checkOwnership(struct node* cur) {
    long int owner = pthread_self();
    int i = 0;
    while (i < 6) {
        if (owner == cur->owner) {
            return 1;
        } else {
            cur = cur->next;
            i++;
        }
    }
    return 0;
}

void *readFile(void *name) {
    pthread_detach(pthread_self());

    char c;
    int num;
    long int accBalance = 0;

    struct node *node = requestPage();
    node->owner = pthread_self();

    FILE *file;
    file = fopen(name, "r");

    while(!(feof(file))) {
        fscanf(file, "%s\n", &c);
        if (c == 'R') {
            pthread_mutex_lock(&mutex);
            resetOwnedBits(node);
            while (c != 'W') {
                fscanf(file, "%s\n", &c);
                if(isalpha(c)) 
                {
                    int val = (rand() % 25 + 1);
                    int check = checkOwnership(node);

                    if(val == 7 || check == 0) 
                    {
                        printf("Page fault in thread %li\n", pthread_self());
                        handlePageFault(node);
                    } else if (val == 17) 
                    {
                        resetAllRBits(node);
                    }

                    if (accBalance < 0) 
                    {
                        node->R = 1;
                        node->M = 1;
                    } 
                    else 
                    {
                        node->R = 1;
                    }
                    
                    balance = balance + accBalance;
                    printf("Accont balance after thread %li is %d\n", pthread_self(), balance);
                    pthread_mutex_unlock(&mutex);
                    sleep(.15);
                    break;
                }
                
                num = atoi(&c);
                accBalance += num;
            }
            sleep(.15);
        }
    }
    fclose(file);
    return NULL;
}

int main() {
    pthread_t thread_id[5];
    char name1[9] = "data1.in";
    char name2[9] = "data2.in";
    char name3[9] = "data3.in";
    char name4[9] = "data4.in";
    char name5[9] = "data5.in";

    // Create circular linked list
    insertNode();
    insertNode();
    insertNode();
    insertNode();
    insertNode();
    insertNode();

    // Open thread and write
    pthread_create(&(thread_id[0]), NULL, readFile, (void *) name1);
    pthread_create(&(thread_id[1]), NULL, readFile, (void *) name2);
    pthread_create(&(thread_id[2]), NULL, readFile, (void *) name3);
    pthread_create(&(thread_id[3]), NULL, readFile, (void *) name4);
    pthread_create(&(thread_id[4]), NULL, readFile, (void *) name5);
    sleep(3);
    printf("\n\n");
    printf("Final balance: %d\n", balance);
}