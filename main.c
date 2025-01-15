#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h> // Include for size_t

#define MAX_WORD_LENGTH 100
#define MAX_LINE_LENGTH 1024
#define HASH_SIZE 1000

typedef struct Node {
    char word[MAX_WORD_LENGTH];
    float score;
    struct Node* next;
} Node;

Node* hashTable[HASH_SIZE];

unsigned int hash(char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % HASH_SIZE;
}

void insert(char* word, float score) {
    unsigned int index = hash(word);
    Node* newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->word, word);
    newNode->score = score;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

float search(char* word) {
    unsigned int index = hash(word);
    Node* node = hashTable[index];
    while (node) {
        if (strcmp(node->word, word) == 0) {
            return node->score;
        }
        node = node->next;
    }
    return 0.0;
}

void load_lexicon(char* filename) {
    FILE* file = fopen(filename, "r");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char* word = strtok(line, "\t");
        float score = atof(strtok(NULL, "\t"));
        insert(word, score);
    }
    fclose(file);
}

float sentiment_analysis(char* sentence) {
    char* word = strtok(sentence, " \t\n");
    float sum = 0.0;
    int count = 0;
    while (word) {
        // Convert word to lowercase and remove punctuation
        for(int i = 0; word[i]; i++){
            word[i] = tolower(word[i]);
            if (ispunct(word[i])) {
                word[i] = '\0';
            }
        }
        sum += search(word);
        count++;
        word = strtok(NULL, " \t\n");
    }
    return count ? sum / count : 0.0;
}

void process_comments(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }
    
    char line[MAX_LINE_LENGTH];
    printf("\t\tstring sample\t\t\t\t\t\t\tscores\n");
    printf("---------------------------------------------------------------------------------------------------------\n");
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character at the end of the line
        line[strcspn(line, "\n")] = '\0';
        printf("%-100s", line);
        // Calculate sentiment score
        float sentiment_score = sentiment_analysis(line);

        // Print the sentiment score
        printf("%.2f\n", sentiment_score);
    }
    fclose(file);
}

typedef struct {
  char *word;
  float score;
  float SD;
  int SIS_array[10];
} word_t;

word_t* read_lexicon(const char *filename, size_t *num_words, size_t *allocated) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Error opening file: %s\n", filename);
    return NULL;
  }

  *num_words = 0;  // Initialize num_words
  *allocated = 0;  // Initialize allocated

  word_t *words = NULL;

  char line[MAX_LINE_LENGTH];
  while (fgets(line, MAX_LINE_LENGTH, fp)) {
    // Remove trailing newline
    line[strcspn(line, "\n")] = '\0';

    // Allocate space for a new word
    if (*num_words == *allocated) {
      *allocated = *allocated == 0 ? 10 : *allocated * 2;
      words = (word_t*)realloc(words, *allocated * sizeof(word_t));
      if (words == NULL) {
        fprintf(stderr, "Error allocating memory for words\n");
        fclose(fp);
        return NULL;
      }
    }

    // Parse the line
    words[*num_words].word = malloc(strlen(line) + 1);
    if (words[*num_words].word == NULL) {
      fprintf(stderr, "Error allocating memory for word\n");
      fclose(fp);
      return NULL;
    }
    sscanf(line, "%s %f %f %*[[]", words[*num_words].word, &(words[*num_words].score), &(words[*num_words].SD));

    // Handle potential missing intensity scores
    int i;
    for (i = 0; i < 10; i++) {
      if (sscanf(line, "%d%*[,]", &(words[*num_words].SIS_array[i])) != 1) {
        words[*num_words].SIS_array[i] = 0; // Set missing values to 0
      }
    }

    (*num_words)++;
  }

  fclose(fp);
  return (word_t*)realloc(words, *num_words * sizeof(word_t)); // Reallocate to exact size
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <lexicon_file> <comments_file>\n", argv[0]);
        return 1;
    }

    size_t num_words = 0;
    size_t allocated = 0;

    word_t *words = read_lexicon(argv[1], &num_words, &allocated);
    if (words == NULL) {
        return 1;
    }

    // Load lexicon into hash table
    for (size_t i = 0; i < num_words; i++) {
        insert(words[i].word, words[i].score);
    }

    // Process comments and perform sentiment analysis
    process_comments(argv[2]);

    // Free allocated memory
    for (size_t i = 0; i < num_words; i++) {
        free(words[i].word);
    }
    free(words);

    return 0;
}