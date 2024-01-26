#pragma once

#include <stdio.h>

#define REVERSI_BOARD_SIZE 8

enum PieceType : char {EMPTY, WHITE, BLACK};

class ReversiBoard {
    private:
    PieceType board[REVERSI_BOARD_SIZE][REVERSI_BOARD_SIZE];

    bool AreCoordinatesValid(int xPos, int yPos)
    {
        return xPos >= 0 && xPos <= REVERSI_BOARD_SIZE && yPos >= 0 && yPos <= REVERSI_BOARD_SIZE;
    }

    bool IsMoveDirectionValid(PieceType side, int xPos, int yPos, int xDirection, int yDirection)
    {
        if(xDirection == 0 && yDirection == 0) return false;

        bool willCapture = false;
        xPos += xDirection;
        yPos += yDirection;
        while(AreCoordinatesValid(xPos, yPos))
        {
            if (board[xPos][yPos] == EMPTY) return false;

            if (board[xPos][yPos] == side)
            {
                if (willCapture) return true;
                return false;
            }

            willCapture = true;
            xPos += xDirection;
            yPos += yDirection;
        }
        return false;
    }

    void ConvertInLine(PieceType side, int xPos, int yPos, int xDirection, int yDirection)
    {
        if(!IsMoveDirectionValid(side, xPos, yPos, xDirection, yDirection)) return;

        xPos += xDirection;
        yPos += yDirection;
        while(board[xPos][yPos] != side)
        {
            board[xPos][yPos] = side;
            xPos += xDirection;
            yPos += yDirection;
        }
    }

    public:
    ReversiBoard()
    {
        for(int x = REVERSI_BOARD_SIZE-1; x >= 0; x--) for(int y = REVERSI_BOARD_SIZE-1; y >= 0; y--)
        board[x][y] = EMPTY;
        
        board[3][3] = BLACK;
        board[4][4] = BLACK;
        board[3][4] = WHITE;
        board[4][3] = WHITE;
    }

    void PrintBoard()
    {
        for(int y = REVERSI_BOARD_SIZE-1; y >= 0; y--)
        {
            printf("%d", y+1);
            for(int x = 0; x < REVERSI_BOARD_SIZE; x++)
            {
                char symbol = ' ';
                PieceType tile = board[x][y];
                switch(tile)
                {
                    case EMPTY:
                        symbol = '.';
                        break;
                    case WHITE:
                        symbol = 'X';
                        break;
                    case BLACK:
                        symbol = '#';
                        break;
                }
                printf("%c", symbol);
            }
            printf("\n");
        }
        printf("012345678\n");
    }

    bool IsMoveValid(PieceType side, int xPos, int yPos)
    {
        if(AreCoordinatesValid(xPos, yPos) && board[xPos][yPos] != EMPTY) return false;

        for(int xDirection = -1; xDirection <= 1; xDirection++) for(int yDirection = -1; yDirection <= 1; yDirection++)
            if(IsMoveDirectionValid(side, xPos, yPos, xDirection, yDirection)) return true;

        return false;
    }

    bool CanSideMove(PieceType side, bool printOptions)
    {
        if(printOptions)
            printf("Possible moves: ");
        bool canMove = false;
        for(int x = REVERSI_BOARD_SIZE-1; x >= 0; x--) for(int y = REVERSI_BOARD_SIZE-1; y >= 0; y--)
            if(IsMoveValid(side, x, y))
            {
                canMove = true;
                if(printOptions)
                    printf("[%d %d],", x+1, y+1);
                else
                    break;
            }
        if(printOptions)
            printf("\n");

        return canMove;
    }

    bool MakeMove(PieceType side, int xPos, int yPos)
    {
        if (!IsMoveValid(side, xPos, yPos)) return false;
        
        board[xPos][yPos] = side;
        for(int xDirection = -1; xDirection <= 1; xDirection++) for(int yDirection = -1; yDirection <= 1; yDirection++)
            ConvertInLine(side, xPos, yPos, xDirection, yDirection);
        return true;
    }

    int PieceCount(PieceType side)
    {
        int count = 0;
        for(int x = REVERSI_BOARD_SIZE-1; x >= 0; x--) for(int y = REVERSI_BOARD_SIZE-1; y >= 0; y--)
            if(board[x][y] == side) count++;
        return count;
    }
};