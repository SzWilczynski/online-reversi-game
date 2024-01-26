#include "reversi.cpp"
#include "data_buffer.cpp"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

int main(int argc, char** argv)
{
  if(argc < 3)
  {
    printf("Not enough input arguments provided. Please provide port and address to connect to.\n");
    return 1;
  }
  int clientSocket;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(atoi(argv[1]));
  serverAddr.sin_addr.s_addr = inet_addr(argv[2]);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
  addr_size = sizeof serverAddr;
  connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
  if(clientSocket == -1)
  {
    puts("Failed to create socket");
    return 1;
  }

  DataBuffer buffer = DataBuffer(clientSocket, 512);

  ReversiBoard board = ReversiBoard();
  board.PrintBoard();
  PieceType playerSide;

  while(1)
  {
    if(buffer.ReadFromSocket() <= 0)
    {
      printf("Communication ended\n");
      break;
    }
    while(1)
    {
      char* command = buffer.FindCommand();
      if(command == NULL)
      {
        break;
      }
      else if(strcmp(command, "MSG") == 0)
      {
        puts(buffer.buffer);
      }
      else if(strcmp(command, "MOV") == 0)
      {
        char* move = buffer.buffer;
        PieceType side = WHITE;
        if (move[0] == 'B')
          side = BLACK;
        int xPos = (int)(move[1]-'0');
        int yPos = (int)(move[2]-'0');
        board.MakeMove(side, xPos-1, yPos-1);
        board.PrintBoard();
      }
      else if(strcmp(command, "REQ") == 0)
      {
        char move[9] = "W00[MOV]";
        int xPos = 0;
        int yPos = 0;
        do{
          puts("Please input correct move coordinates");
          board.CanSideMove(playerSide, true);
          fflush(stdin);
          if(scanf("%d %d", &xPos, &yPos) == EOF)
          {
            xPos = 0;
            yPos = 0;
          };
        }while(!(xPos>0 && xPos<9 && yPos>0 && yPos<9));

        if(playerSide == BLACK)
          move[0] = 'B';

        move[1] = ((char)xPos) + '0';
        move[2] = ((char)yPos) + '0';
        if(buffer.SendToSocket(move) < 0)
        {
          break;
        }
      }
      else if(strcmp(command, "SIDE") == 0)
      {
        if(strcmp(buffer.buffer, "WHITE") == 0)
        {
          playerSide = WHITE;
        }else
          playerSide = BLACK;
      }
      else
      {
        printf("Invalid command detected! (%s)\n", command);
      }
    }
  }

  close(clientSocket);
    
  return 0;
}
