#include "monty.h"
int LoadSession(char *file) {
  GameThread *loadedGameTable[MAXCORES] = {0};
  GameScore gameScoreEntry = {0};
  FILE *fp;
  int i;
  if ((fp = fopen(file, "r")) == NULL) return -1;

  for (i=0; fread(&gameScoreEntry, sizeof(GameScore), 1, fp) == sizeof(GameScore) ;i++) {
    loadedGameTable[i] = calloc(1, sizeof(GameThread));
    loadedGameTable->score = gameScoreEntry;
  }

  fclose(fp);
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


