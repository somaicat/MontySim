#include "monty.h"
int LoadSession(char *file, int numSessions) {
  GameThread *loadedGameTable[MAXCORES] = {0};
  GameScore gameScoreEntry = {0};
  FILE *fp;
  int i;
  if ((fp = fopen(file, "r")) == NULL) return -1;

  for (i=0; fread(&gameScoreEntry, sizeof(GameScore), 1, fp) == 1;i++) {
    loadedGameTable[i] = calloc(1, sizeof(GameThread));
    loadedGameTable[i]->score = gameScoreEntry;
  }
  fclose(fp);
  if (i != numSessions) {
    for(;i>=0; free(loadedGameTable[i--]));
    return 1;
  }
  for(;i>=0;gameThreadTable[i] = loadedGameTable[i]) {
    i--;
  }
  return 0;
}

void SaveSession(char *file) {
  FILE *fp;
  int i;
  if ((fp = fopen(file, "w")) == NULL) return;

  for (i=0; i<MAXCORES && gameThreadTable[i] != NULL;i++) {
    fwrite(&gameThreadTable[i]->score, sizeof(GameScore), 1, fp);
  }
  fclose(fp);
  return;
}

void *SessionSaveThread(void *arg) {
  return NULL;
}


