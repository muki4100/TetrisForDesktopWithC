#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "TetrisManager.h"
#include "Util.h"
#include "Constant.h"

#define INITAL_SPEED 300
#define SPEED_LEVEL_OFFSET 40
#define LEVELP_UP_CONDITION 3
#define STATUS_POSITION_X_TO_PRINT 38
#define STATUS_POSITION_Y_TO_PRINT 3

#define LINES_TO_DELETE_HIGHTING_COUNT 3
#define LINES_TO_DELETE_HIGHTING_MILLISECOND 100

static void _TetrisManager_PrintStatus(TetrisManager* tetrisManager, int* x, int* y);
static void _TetrisManager_PrintKeys(TetrisManager* tetrisManager, int* x, int* y);
static void _TetrisManager_ClearBoard(TetrisManager* tetrisManager);
static void _TetrisManager_ChangeBoardByStatus(TetrisManager* tetrisManager, int status);
static void _TetrisManager_UpSpeedLevel(TetrisManager* tetrisManager);
static void _TetrisManager_SearchLineIndexesToDelete(TetrisManager* tetrisManager, int* indexes, int* count);
static void _TetrisManager_DeleteLines(TetrisManager* tetrisManager, int* indexes, int count);
static void _TetrisManager_HighlightLinesToDelete(TetrisManager* tetrisManager, int* indexes, int count);

void TetrisManager_Init(TetrisManager* tetrisManager, int speedLevel){
	Block block;
	block.current = -1;
	memset(tetrisManager->board, 0, sizeof(char)* BOARD_ROW_SIZE * BOARD_COL_SIZE);
	_TetrisManager_ClearBoard(tetrisManager);
	tetrisManager->block = Block_Make(True, block);
	tetrisManager->deletedLineCount = 0;
	tetrisManager->speedLevel = speedLevel;
	tetrisManager->score = 0;
}

int TetrisManager_CheckValidPosition(TetrisManager* tetrisManager, int direction){
	Block temp = Block_Move(tetrisManager->block, direction);
	int i;
	for (i = 0; i < POSITIONS_SIZE; i++){
		int x = Block_GetPositions(temp)[i].x;
		int y = Block_GetPositions(temp)[i].y;
		if (!(tetrisManager->board[x][y] == EMPTY || tetrisManager->board[x][y] == MOVING_BLOCK)){
			return tetrisManager->board[x][y];
		}
	}
	return EMPTY;
}

void TetrisManager_ChangeBoardByDirection(TetrisManager* tetrisManager, int direction){
	int tempDirection = DOWN;
	int tempCheckResult = EMPTY;
	_TetrisManager_ClearBoard(tetrisManager);
	int checkResult = TetrisManager_CheckValidPosition(tetrisManager, direction);
	if (checkResult == EMPTY){
		tetrisManager->block = Block_Move(tetrisManager->block, direction);
	}
	else{
		if (direction == UP && checkResult != FIXED_BLOCK){
			if (checkResult == TOP_WALL){
				tempDirection = DOWN;
				tempCheckResult = TOP_WALL;
			}
			else if (checkResult == RIGHT_WALL){
				tempDirection = LEFT;
				tempCheckResult = RIGHT_WALL;
			}
			else if (checkResult == LEFT_WALL){
				tempDirection = RIGHT;
				tempCheckResult = LEFT_WALL;
			}
			do{
				tetrisManager->block = Block_Move(tetrisManager->block, tempDirection);
			} while (TetrisManager_CheckValidPosition(tetrisManager, direction) == tempCheckResult);
			tetrisManager->block = Block_Move(tetrisManager->block, direction);
		}
	}
	_TetrisManager_ChangeBoardByStatus(tetrisManager, MOVING_BLOCK);
}

void TetrisManager_ChangeBoardByAuto(TetrisManager* tetrisManager){
	TetrisManager_ChangeBoardByDirection(tetrisManager, DOWN);
}

void TetrisManager_ProcessDirectDown(TetrisManager* tetrisManager){
	while (!TetrisManager_IsReachedToBottom(tetrisManager)){
		TetrisManager_ChangeBoardByDirection(tetrisManager, DOWN);
	}
}

void TetrisManager_ProcessDeletingLines(TetrisManager* tetrisManager){
	int indexes[BOARD_ROW_SIZE];
	int count;
	_TetrisManager_SearchLineIndexesToDelete(tetrisManager, indexes, &count);
	if (count > 0){
		_TetrisManager_HighlightLinesToDelete(tetrisManager, indexes, count);
		_TetrisManager_DeleteLines(tetrisManager, indexes, count);
	}
}

int TetrisManager_IsReachedToBottom(TetrisManager* tetrisManager){
	int i;
	for (i = 0; i < POSITIONS_SIZE; i++){
		int x = Block_GetPositions(tetrisManager->block)[i].x;
		int y = Block_GetPositions(tetrisManager->block)[i].y;
		if (tetrisManager->board[x + 1][y] != EMPTY && tetrisManager->board[x + 1][y] != MOVING_BLOCK){
			return True;
		}
	}
	return False;
}

int TetrisManager_ProcessReachedCase(TetrisManager* tetrisManager){
	_TetrisManager_ChangeBoardByStatus(tetrisManager, FIXED_BLOCK);
	tetrisManager->block = Block_Make(False, tetrisManager->block);
	if (TetrisManager_IsReachedToBottom(tetrisManager)){
		return END;
	}
	else{
		return PLAYING;
	}
}

void TetrisManager_Sleep(TetrisManager* tetrisManager){ 
	Sleep(TetrisManager_GetDownMilliSecond(tetrisManager));
}

void TetrisManager_Print(TetrisManager* tetrisManager){
	int i;
	int j;
	int x;
	int y;
	CursorUtil_GotoXY(0, 0);
	for (i = 0; i < BOARD_ROW_SIZE; i++){
		for (j = 0; j < BOARD_COL_SIZE; j++){
			switch (tetrisManager->board[i][j]){
			case LEFT_TOP_EDGE:
				FontUtil_ChangeFontColor(LIGHT_WHITE);
				printf("灰");
				FontUtil_ChangeFontColor(WHITE);
				break;
			case RIGHT_TOP_EDGE:
				FontUtil_ChangeFontColor(LIGHT_WHITE);
				printf("汐");
				FontUtil_ChangeFontColor(WHITE);
				break;
			case LEFT_BOTTOM_EDGE:
				FontUtil_ChangeFontColor(LIGHT_WHITE);
				printf("汍");
				FontUtil_ChangeFontColor(WHITE);
				break;
			case RIGHT_BOTTOM_EDGE:
				FontUtil_ChangeFontColor(LIGHT_WHITE);
				printf("污");
				FontUtil_ChangeFontColor(WHITE);
				break;
			case EMPTY:
				printf("  ");
				break;
			case MOVING_BLOCK:
				FontUtil_ChangeFontColor(tetrisManager->block.color);
				printf("﹥");
				FontUtil_ChangeFontColor(WHITE);
				break;
			case FIXED_BLOCK:
				FontUtil_ChangeFontColor(JADE);
				printf("〨");
				FontUtil_ChangeFontColor(WHITE);
				break;
			case LEFT_WALL:
			case RIGHT_WALL:
				FontUtil_ChangeFontColor(LIGHT_WHITE);
				printf("��");
				FontUtil_ChangeFontColor(WHITE);
				break;
			case TOP_WALL:
			case BOTTOM_WALL:
				FontUtil_ChangeFontColor(LIGHT_WHITE);
				printf("〞");
				FontUtil_ChangeFontColor(WHITE);
				break;
			}
		}
		printf("\n");
	}
	x = STATUS_POSITION_X_TO_PRINT;
	y = STATUS_POSITION_Y_TO_PRINT;
	_TetrisManager_PrintStatus(tetrisManager, &x, &y);
	x += 6;
	y += 2;
	_TetrisManager_PrintKeys(tetrisManager, &x, &y);
	x += 3;
	y += 2;
	Block_PrintNext(tetrisManager->block, x, y);
	CursorUtil_Hide();
}

DWORD TetrisManager_GetDownMilliSecond(TetrisManager* tetrisManager){
	int i;
	DWORD milliSecond = INITAL_SPEED;
	for (i = MIN_SPEED_LEVEL; i < tetrisManager->speedLevel; i++){
		if (i < MAX_SPEED_LEVEL / 2){
			milliSecond -= SPEED_LEVEL_OFFSET;
		}
		else{
			milliSecond -= (SPEED_LEVEL_OFFSET / 5);
		}
	}
	return milliSecond;
}

static void _TetrisManager_PrintStatus(TetrisManager* tetrisManager, int* x, int* y){
	CursorUtil_GotoXY(*x, (*y)++);
	printf("旨 Lv 旬  旨 Line 旬  旨 TotalScore 旬");
	CursorUtil_GotoXY(*x, (*y)++);
	printf("早%3d 早  早%4d  早  早%7d     早", tetrisManager->speedLevel, tetrisManager->deletedLineCount, tetrisManager->score);
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("曲收收旭  曲收收收旭  曲收收收收收收旭");
}

static void _TetrisManager_PrintKeys(TetrisManager* tetrisManager, int* x, int* y){
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("旨收收收收 Keys 收收收收旬");
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("早∠       早move left  早");
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("早⊥       早move right 早");
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("早⊿       早move down  早");
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("早∟       早rotate     早");
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("早SpaceBar 早direct down早");
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("早ESC      早pause      早");
	CursorUtil_GotoXY(*x, (*y)++); 
	printf("曲收收收收收收收收收收收旭");
}

static void _TetrisManager_ClearBoard(TetrisManager* tetrisManager){
	int i;
	int j;
	for (i = 0; i < BOARD_ROW_SIZE; i++){
		tetrisManager->board[i][0] = LEFT_WALL;
		tetrisManager->board[i][BOARD_COL_SIZE - 1] = RIGHT_WALL;
	}
	for (i = 0; i < BOARD_COL_SIZE; i++){
		tetrisManager->board[0][i] = TOP_WALL;
		tetrisManager->board[BOARD_ROW_SIZE - 1][i] = BOTTOM_WALL;
	}
	for (i = 1; i < BOARD_ROW_SIZE - 1; i++){
		for (j = 1; j < BOARD_COL_SIZE - 1; j++){
			if (tetrisManager->board[i][j] != FIXED_BLOCK){
				tetrisManager->board[i][j] = EMPTY;
			}
		}
	}
	tetrisManager->board[0][0] = LEFT_TOP_EDGE;
	tetrisManager->board[0][BOARD_COL_SIZE - 1] = RIGHT_TOP_EDGE;
	tetrisManager->board[BOARD_ROW_SIZE - 1][0] = LEFT_BOTTOM_EDGE;
	tetrisManager->board[BOARD_ROW_SIZE - 1][BOARD_COL_SIZE - 1] = RIGHT_BOTTOM_EDGE;
}

static void _TetrisManager_ChangeBoardByStatus(TetrisManager* tetrisManager, int status){
	int i;
	for (i = 0; i < POSITIONS_SIZE; i++){
		int x = Block_GetPositions(tetrisManager->block)[i].x;
		int y = Block_GetPositions(tetrisManager->block)[i].y;
		tetrisManager->board[x][y] = status;
	}
}

static void _TetrisManager_UpSpeedLevel(TetrisManager* tetrisManager){
	if (tetrisManager->speedLevel < MAX_SPEED_LEVEL){
		tetrisManager->speedLevel++;
	}
}

static void _TetrisManager_SearchLineIndexesToDelete(TetrisManager* tetrisManager, int* indexes, int* count){
	int i;
	int j;
	int toDelete;
	memset(indexes, -1, sizeof(int)* (BOARD_ROW_SIZE - 2));
	*count = 0;
	for (i = 1; i < BOARD_ROW_SIZE - 1; i++){
		toDelete = True;
		for (j = 1; j < BOARD_COL_SIZE - 1; j++){
			if (tetrisManager->board[i][j] != FIXED_BLOCK){
				toDelete = False;
				break;
			}
		}
		if (toDelete){
			indexes[(*count)++] = i;
		}
	}
}

static void _TetrisManager_DeleteLines(TetrisManager* tetrisManager, int* indexes, int count){
	int i;
	int j;
	int k = BOARD_ROW_SIZE - 2;
	int toDelete;
	char temp[BOARD_ROW_SIZE][BOARD_COL_SIZE] = { EMPTY };
	for (i = BOARD_ROW_SIZE - 2; i > 0; i--){
		toDelete = False;
		for (j = 0; j < BOARD_COL_SIZE; j++){
			if (i == indexes[j]){
				toDelete = True;
				break;
			}
		}
		if (!toDelete){
			for (j = 0; j < BOARD_COL_SIZE; j++){
				temp[k][j] = tetrisManager->board[i][j];
			}
			k--;
		}
	}
	for (i = 1; i < BOARD_ROW_SIZE - 1; i++){
		for (j = 1; j < BOARD_COL_SIZE - 1; j++){
			tetrisManager->board[i][j] = temp[i][j];
		}
	}
	for (i = 0; i < count; i++){
		tetrisManager->score += tetrisManager->speedLevel * 100;
		tetrisManager->deletedLineCount++;
		if (tetrisManager->deletedLineCount % LEVELP_UP_CONDITION == 0){
			_TetrisManager_UpSpeedLevel(tetrisManager);
		}
	}
}

static void _TetrisManager_HighlightLinesToDelete(TetrisManager* tetrisManager, int* indexes, int count){
	int i;
	int j;
	int k;
	for (i = 0; i < LINES_TO_DELETE_HIGHTING_COUNT; i++){
		FontUtil_ChangeFontColor(JADE);
		Sleep(LINES_TO_DELETE_HIGHTING_MILLISECOND);
		for (j = 0; j < count; j++){
			CursorUtil_GotoXY(2, indexes[j]);
			for (k = 0; k < BOARD_COL_SIZE - 2; k++){
				printf("〨");
			}
		}
		FontUtil_ChangeFontColor(WHITE);
		Sleep(LINES_TO_DELETE_HIGHTING_MILLISECOND);
		for (j = 0; j < count; j++){
			CursorUtil_GotoXY(2, indexes[j]);
			for (k = 0; k < BOARD_COL_SIZE - 2; k++){
				printf("  ");
			}
		}
	}
}
