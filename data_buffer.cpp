#pragma once

#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

class DataBuffer
{
    char* bufferEnd;
    char* bufferCap;
    char* lastFoundCommand;
    int bufferSpaceLeft;
    int socket;

    public:
        char* buffer;
        DataBuffer(int inSocket, int bufferSize);
        ~DataBuffer();
        ssize_t ReadFromSocket();
        ssize_t SendToSocket(const char* message);
        ssize_t SendToSocket(const char* message, int flags);
        char* FindCommand();
        void EraseFromFront(char* point);  
};

ssize_t DataBuffer::ReadFromSocket()
{
    ssize_t receivedMessageSize = recv(socket, bufferEnd, bufferSpaceLeft, 0);
    if(receivedMessageSize > 0)
    {
        bufferSpaceLeft -= receivedMessageSize;
        bufferEnd = bufferEnd+receivedMessageSize;
    }
    return receivedMessageSize;
};

ssize_t DataBuffer::SendToSocket(const char* message)
{
    ssize_t size = send(socket, message, strlen(message), MSG_NOSIGNAL);
    return size;
};

char* DataBuffer::FindCommand()
{
    if(lastFoundCommand != NULL)
    {
        EraseFromFront(lastFoundCommand);
        lastFoundCommand = NULL;
    }
    char* commandEnd = strchr(buffer, (int)']');
    if(commandEnd == 0)
    {
        return NULL;
    }
    char* commandStart = strchr(buffer, (int)'[');
    *commandStart = '\0';
    *commandEnd = '\0';

    lastFoundCommand = commandEnd;
    return commandStart+1;
};

void DataBuffer::EraseFromFront(char* point)
{
    memmove(buffer, point+1, bufferCap-point);
    bufferSpaceLeft += point+1-buffer;
    bufferEnd -= point+1-buffer;
    memset(bufferCap-bufferSpaceLeft+1, '\0', bufferSpaceLeft);
};

DataBuffer::DataBuffer(int inSocket, int bufferSize)
{
    buffer = (char *)malloc(bufferSize);
    lastFoundCommand = NULL;
    socket = inSocket;
    bufferEnd = buffer;
    bufferSpaceLeft = bufferSize-1;
    bufferCap = buffer+bufferSize-1;
    memset(buffer, '\0', bufferSize);
};

DataBuffer::~DataBuffer()
{
    free(buffer);
};