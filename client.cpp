#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <unistd.h>
#include <locale>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;


/*----------------------------------------------------------------------
 *
 * sendStr --
 *
 *      Function used to send string to server.
 *      The message sent to server does not contain any header.
 *
 * Results:
 *      String send to server.
 *
 *----------------------------------------------------------------------
 */

void
sendStr(int serverSocket, string str)
{
   char buffer[2000];
   strcpy(buffer, str.c_str());
   int status = send(serverSocket, buffer, 2000, 0);
   if (status == -1) {
      cout << "Error: Send failed" << endl;
      close(serverSocket);
      exit(1);
   }
}


/*----------------------------------------------------------------------
 *
 * receive --
 *
 *      Function used to receive string from server.
 *      The function unpacks the message and only return the
 *      actual string sent by server.
 *
 * Results:
 *      String received from server.
 *
 *----------------------------------------------------------------------
 */

string
receive(int serverSocket)
{
   char buffer[2000];
   int bytesRem = 2000;
   while (bytesRem > 0) {
      int recvBytes = recv(serverSocket, buffer, bytesRem, 0);
      if (recvBytes < 0) {
         cout << "Error: Failed to receive bytes" << endl;
         close(serverSocket);
         exit(1);
      }
      bytesRem-= recvBytes;
   }

   string rawMsg = (string)buffer;

   /* Extract header's info */
   int flag = stoi(rawMsg.substr(0, 3), nullptr, 10);
   int wordLength = stoi(rawMsg.substr(3, 1), nullptr, 36);
   int numIncorrect = stoi(rawMsg.substr(4, 1), nullptr, 10);

   /* Extract string */
   string str = rawMsg.substr(5, rawMsg.length());

   return str;
}


/*----------------------------------------------------------------------
 *
 * playHangman --
 *
 *      The logic of the game is in this function.
 *      Word is generated, and the user has a to guess it.
 *      A score is given to the user at the end.
 *
 * Results:
 *      Hangman game is started.
 *
 *----------------------------------------------------------------------
 */

void
playGame(int serverSocket, string username, int wordOrder)
{

   int valid;
   int status;

   uint16_t converter;
   int sockfd = serverSocket;

   string reply;
   string word;
   string guess;
   string turn;

   bool guessed = false;

   /* Converts word order to little endian used in TCP/IP networks. */
   converter = htons(wordOrder);
   status = send(serverSocket, &converter, sizeof(uint16_t), 0);
   if (status <= -1) {
      cout << "Error: Send failed" << endl;
      close(serverSocket);
      exit(1);
   }

   while (!guessed) {
      turn = receive(sockfd);
      cout << "\nTurn " << turn << endl;

      word = receive(sockfd);
      cout << word << endl;

      do {
         if ((guess.length() != 1) && (guess.length() != 0)) {
            cout << "Error! Please guess ONE letter." << endl;
         }
         if ((isalpha(guess[0]) == 0) && (guess != "")) {
            cout << "Error! Please guess one LETTER." << endl;
         }
         cout << "Letter to guess: ";
         cin >> guess;
      } while (guess.length() != 1 || isalpha(guess[0]) == 0);

      char char_array[2];
      strcpy(char_array, guess.c_str());
      char_array[1] = '\0';
      locale loc;
      if (!islower(char_array[0]))
         char_array[0] = tolower(char_array[0]);
      sendStr(sockfd, char_array);

      status = recv(sockfd, &converter, sizeof(uint16_t), 0);
      if (status <= 0) {
         cout << "Error: Failed to receive bytes" << endl;
         close(sockfd);
         exit(1);
      }
      valid = ntohs(converter);

      if (valid != 1) {
         cout << "Error! Letter " << guess[0]
              << " has been guessed before, please guess another letter."
              << endl;
      }

      else {
         reply = receive(sockfd);

         cout << reply << endl;

         status = recv(sockfd, &converter, sizeof(uint16_t), 0);
         if (status <= 0) {
            cout << "Error: Failed to receive bytes" << endl;
            close(sockfd);
            exit(1);
         }

         valid = ntohs(converter);

         if (valid == 1) {
            guessed = true;
            reply = receive(sockfd);
            cout << reply << endl;
            guessed = true;

            cout << "\n";
            reply = receive(sockfd);
            cout << reply << endl;
         }
      }
   }
}


/*----------------------------------------------------------------------
 *
 * main --
 *      Takes in arguments user inputted in command line and call
 *      helper functions to start the game.
 *
 * Results:
 *      Returns a 1 after executing
 *
 *----------------------------------------------------------------------
 */

int
main(int argc, char *argv[])
{
   if (argc < 3) {
      cout << "Error: Port Number or IP Address not entered." << endl;
      exit(1);
   }

   if (argc > 4) {
      cout << "Error: Too many arguments." << endl;
      exit(1);
   }

   int sockfd;
   string ipAddr = argv[1];
   char ip[ipAddr.size() + 1];
   strcpy(ip, ipAddr.c_str());

   int wordOrder;

   unsigned short portNum = atoi(argv[2]);

   /* Create socket for client. */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   /* Specify socket. */
   struct sockaddr_in serverAddr;
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(portNum);
   serverAddr.sin_addr.s_addr = inet_addr(ip);


   int status =
      connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));


   if (status == -1) {
      cout << "Error: Connection could not be established" << endl;
      close(sockfd);
      exit(1);
   }

start:
   cout << "Welcome to Hangman!" << endl;
   string username;
   cout << "Enter username: ";
   cin >> username;

back:
   string ready;
   cout << "Read to play? (Y/N): ";
   cin >> ready;

   if (ready.length() > 1) {
      cout << "Enter only ONE character/digit." << endl;
      goto back;
   }

   char tokenizedReady[2];
   strcpy(tokenizedReady, ready.c_str());

   if (isalpha(tokenizedReady[0])) {
      if (ready.length() > 1) {
         cout << "Enter only ONE character." << endl;
         goto back;
      } else if (tolower(tokenizedReady[0] == 'y')) {
         wordOrder = 0;
      } else {
         goto start;
      }
   } else if (isdigit(tokenizedReady[0])) {
      wordOrder = atoi(tokenizedReady);
      cout << "Entered: " << wordOrder << endl;
   } else {
      cout << "Enter a valid character/ digit." << endl;
      goto back;
   }

   sendStr(sockfd, username);
   playGame(sockfd, username, wordOrder);

   close(sockfd);

   return 1;
}