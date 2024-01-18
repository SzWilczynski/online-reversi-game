#include "data_buffer.cpp"
#include "reversi.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <algorithm>

struct MatchData {
  int player1sock;
  int player2sock;
  unsigned int matchNumber;
};

void * socketThread(void *arg)
{
  struct MatchData* match = ((MatchData*)arg);

  int matchNumber = match->matchNumber;
  int player1 = match->player1sock;
  int player2 = match->player2sock;
  DataBuffer* currentPlayer = new DataBuffer(match->player1sock, 128);
  DataBuffer* otherPlayer = new DataBuffer(match->player2sock, 128);

  free(arg);

  printf("Starting match#%d\n", matchNumber);

  ReversiBoard board = ReversiBoard();

  currentPlayer->SendToSocket("You're playing as white (X) and will go first[MSG]WHITE[SIDE]");
  otherPlayer->SendToSocket("You're playing as black (#) and will go second[MSG]BLACK[SIDE]");

  PieceType currentSide = BLACK;
  std::swap(currentPlayer, otherPlayer);

  bool couldMoveLastTurn = true;
  bool playerDisconnected = false;
  while(1)
  {
    std::swap(currentPlayer, otherPlayer);
    if(currentSide == WHITE)
      currentSide = BLACK;
    else
      currentSide = WHITE;

    if(!board.CanSideMove(currentSide, false)){
      if(!couldMoveLastTurn)
      {
        currentPlayer->SendToSocket("Out of options for both sides! Ending game.[MSG]");
        otherPlayer->SendToSocket("Out of options for both sides! Ending game.[MSG]");
        break;
      }
      currentPlayer->SendToSocket("Current side cannot make a move! Skipping turn.[MSG]");
      otherPlayer->SendToSocket("Current side cannot make a move! Skipping turn.[MSG]");
      couldMoveLastTurn = false;
      continue;
    }

    couldMoveLastTurn = true;

    if(currentPlayer->SendToSocket("[REQ]") <= 0)
    {
      playerDisconnected = true;
      break;
    }
    char* command = NULL;
    while(1)
    {
      ssize_t receivedMessageSize;
      do
      {
        receivedMessageSize = currentPlayer->ReadFromSocket();
      }
      while(receivedMessageSize < 0);
      if(receivedMessageSize == 0)
      {
        playerDisconnected = true;
        break;
      }
      command = currentPlayer->FindCommand();
      if(command == NULL)
      {
        continue;
      }
      else if(strcmp(command, "MOV") == 0)
      {
        char* move = currentPlayer->buffer;
        puts(move);
        int xPos = (int)(move[1]-'0');
        int yPos = (int)(move[2]-'0');
        if(board.MakeMove(currentSide, xPos-1, yPos-1))
        {
          currentPlayer->SendToSocket(move);
          currentPlayer->SendToSocket("[MOV]");
          otherPlayer->SendToSocket("Opponent made their move.[MSG]");
          otherPlayer->SendToSocket(move);
          otherPlayer->SendToSocket("[MOV]");
          break;
        }else
          currentPlayer->SendToSocket("[REQ]");
      }
      else
      {
        printf("Invalid command detected! (%s)\n", command);
      }
    }
    
    if(playerDisconnected)
    {
      otherPlayer->SendToSocket("Opponent has disconnected. Ending game.[MSG]");
      break;
    }
  }

  int whiteScore = board.PieceCount(WHITE);
  int blackScore = board.PieceCount(BLACK);

  char endMessage[256];

  char score[3] = "00";

  strcpy(endMessage, "Final results:");
  strcat(endMessage, "\nWHITE - ");
  score[0] = (char)(whiteScore/10 + '0');
  score[1] = (char)(whiteScore%10 + '0');
  strcat(endMessage, score);
  strcat(endMessage, "\nBLACK - ");
  score[0] = (char)(blackScore/10 + '0');
  score[1] = (char)(blackScore%10 + '0');
  strcat(endMessage, score);
  if(whiteScore > blackScore)
  {
    strcat(endMessage, "\nWHITE WINS!");
  }
  else if(whiteScore < blackScore)
  {
    strcat(endMessage, "\nBLACK WINS!");
  }
  else
  {
    strcat(endMessage, "\nA DRAW?");
  }
  strcat(endMessage, "[MSG]");

  currentPlayer->SendToSocket(endMessage);
  otherPlayer->SendToSocket(endMessage);

  close(player1);
  close(player2);

  delete currentPlayer;
  delete otherPlayer;

  printf("Ending match #%d\n", matchNumber);
  return NULL;
}

int main(int argc, char** argv){
  if(argc < 2)
  {
    printf("Not enough input arguments provided. Please provide port to connect to.\n");
    return 1;
  }
  int serverSocket, newSocket;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  serverSocket = socket(PF_INET, SOCK_STREAM, 0);
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(atoi(argv[1]));
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
  bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  if(listen(serverSocket,50)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  pthread_t thread_id;
  unsigned int startedMatches = 0;

  while(1)
  {
    struct MatchData* match = (MatchData*)malloc(sizeof(MatchData));

    addr_size = sizeof serverStorage;
    newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
    match->player1sock = newSocket;
    printf("First player connected\n");

    newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
    match->player2sock = newSocket;
    printf("Second player connected\n");

    match->matchNumber = ++startedMatches;

    if( pthread_create(&thread_id, NULL, socketThread, match) != 0 )
      printf("Failed to create thread\n");

    pthread_detach(thread_id);
  }
  return 0;
}
