#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <sstream>
#include <cstdint>
#include <ctime>
#include <math.h>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>

using namespace std;


/*----------------------------------------------------------------------
 *
 * LeaderBoardNode --
 *
 *      Struct containing user's info to be sorted displayed
 *      in leader board.
 *
 *----------------------------------------------------------------------
 */

struct LeaderBoardNode {
   string user;
   float score;
};


/*----------------------------------------------------------------------
 *
 * Argument --
 *
 *      Struct containing info to be passed into playHangman().
 *
 *----------------------------------------------------------------------
 */

struct Argument {
   int arg1;
   string arg2;
};

string leaderBoardString = "Leader Board\n\n";
pthread_mutex_t m;
LeaderBoardNode leaderBoard[4];
int numEntries;


/*----------------------------------------------------------------------
 *
 * sorter --
 *
 *      Sorts leader board nodes based on score.
 *
 * Results:
 *      Leader board nodes are sorted.
 *
 *----------------------------------------------------------------------
 */

bool
sorter(LeaderBoardNode const& lhs, LeaderBoardNode const& rhs)
{
   return lhs.score < rhs.score;
}


/*----------------------------------------------------------------------
 *
 * itoa --
 *
 *      Converts based-10 integers into based-36 integers.
 *      Used to ensure that length of word is 1 byte.
 *
 * Results:
 *      Based-10 integers become based-36.
 *
 *----------------------------------------------------------------------
 */

char *
itoa(int val, int base)
{
   static char buf[32] = {0};
   int i = 30;
   for (; val && i; --i, val /= base)
      buf[i] = "0123456789abcdefghijklmnopqrstuvwxyz"[val % base];
   return &buf[i + 1];
}


/*----------------------------------------------------------------------
 *
 * stringFormat --
 *
 *      Adds trailing zero's to a string to ensure that length is
 *      exactly 3.
 *
 * Results:
 *      String becomes 3-character long.
 *
 *----------------------------------------------------------------------
 */

string
stringFormat(int input)
{
   string number = to_string(input);
   string res;
   if (number.length() > 3) {
      res = "999";
   } else if (number.length() == 3) {
      res = number;
   } else if (number.length() == 2) {
      res = string(1, '0').append(number);
   } else if (number.length() == 1) {
      res = string(2, '0').append(number);
   } else {
      res = "000";
   }

   return res;
}


/*----------------------------------------------------------------------
 *
 * sendStr --
 *
 *      Function used to send string to client following a format.
 *
 * Results:
 *      String send to client.
 *
 * Message Format (Server to Client):
 *
 *        __ __ __     __          __      __ __ __ __ __
 *      | __ __ __ |   __    |     __    | __ __ ...__ __|
 *      | Msg flag |  Word   |    Num    |  Data         |
 *      |          | Length  | Incorrect |               |
 *      | (3 bytes)| (1 byte)| (1 byte)  |(unconstrained)|
 *      | __ __ __ |   __    |     __    | __ __ __ __ __|
 *
 *----------------------------------------------------------------------
 */

void
sendStr(int socket, string str, int wordLengthgth, int numIncorrect,
        bool isCtrl)
{
   string sendString = itoa(wordLengthgth, 36) + to_string(numIncorrect) + str;
   string flag = (isCtrl) ? "000" : stringFormat(sendString.length() + 3);
   sendString = flag + sendString;

   char buffer[2000];
   strcpy(buffer, sendString.c_str());
   int status = send(socket, buffer, 2000, 0);
   if (status == -1) {
      cout << "Error: Send failed" << endl;
      close(socket);
      exit(1);
   }
}


/*----------------------------------------------------------------------
 *
 * receive --
 *
 *      Function used to receive string from client.
 *
 * Results:
 *      String received from client.
 *
 *----------------------------------------------------------------------
 */

string
receive(int socket)
{
   char buffer[2000];
   int bytesRem = 2000;
   while (bytesRem > 0) {
      int recvBytes = recv(socket, buffer, bytesRem, 0);
      if (recvBytes < 0) {
         cout << "Error: Failed to receive bytes" << endl;
         close(socket);
         exit(1);
      }
      bytesRem -= recvBytes;
   }
   return buffer;
}


/*----------------------------------------------------------------------
 *
 * getWord --
 *
 *      Picks a word for the player to guess.
 *
 * Results:
 *      Word is returned based on input file.
 *
 *----------------------------------------------------------------------
 */

string
getWord(int clientSocket, int wordOrder, string fileName)
{
   srand(time(NULL));
   string path = (fileName != "NULL") ? fileName : "words.txt";
   ifstream myfile(path.c_str());
   if (!myfile) {
      cout << "Error: Cannot open input file" << endl;
      close(clientSocket);
      exit(1);
   }

   string word = "";
   string wordLengthStr, wordCountStr;
   myfile >> wordLengthStr >> wordCountStr;
   int wordLength, wordCount;
   wordLength = stoi(wordLengthStr, nullptr, 10);
   wordCount = stoi(wordCountStr, nullptr, 10);
   int index = (wordOrder == 0) ? (rand() % wordCount + 1) : wordOrder;
   for (int i = 0; i < index; i++) {
      myfile >> word;
   }

   myfile.close();
   return word;
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

void *
playHangman(void *input)
{
   struct Argument *args = (struct Argument *)input;
   int clientSocket = args->arg1;
   string fileName = args->arg2;

   float score;

   int valid;
   uint16_t converter;
   int status;

   string username;
   bool done = false;
   int turnCounter = 1;

   string word;
   int wordLength;
   string guess;
   int wordOrder;

   int guessedPointer = 0;
   string guessedCharacters[26];
   bool guessedChar;
   bool correct;
   string reply = "";

   unsigned int numIncorrect = 0;

   string userProgress = "";
   string incorrectGuesses = "";
   int userProgressCount = 0;

   /* Receive username. */
   username = receive(clientSocket);
   cout << "User " << username << " has connected." << endl;

   /* Receive word order. */
   status = recv(clientSocket, &converter, sizeof(uint16_t), 0);
   if (status <= 0) {
      cout << "Error: Failed to receive bytes" << endl;
      close(clientSocket);
      exit(1);
   }
   wordOrder = ntohs(converter);
   word = getWord(clientSocket, wordOrder, fileName);
   wordLength = word.length();

   cout << "The word is " << word << " for user " << username << endl;

   for (int i = 0; i < wordLength; i++) {
      userProgress += "_ ";
   }

   while (!done) {
      guessedChar = false;

      sendStr(clientSocket, to_string(turnCounter), wordLength, numIncorrect,
              0);

      sendStr(clientSocket, userProgress, wordLength, numIncorrect, 0);

      guess = receive(clientSocket);

      cout << "User: " << username << " guess: " << guess[0] << endl;

      if ((guess.length() != 1) || (isalpha(guess[0]) == 0)) {
         cout << "Error: user guess invalid" << endl;
         pthread_detach(pthread_self());
         close(clientSocket);
         exit(1);
      }

      /* Ensure that guess is not duplicated. */
      for (int i = 0; i < guessedPointer; i++) {
         if (guess == guessedCharacters[i]) {
            cout << username << " already guessed character "
                 << guessedCharacters[i] << "." << endl;
            guessedChar = true;
         }
      }

      if (guessedChar) {
         valid = 0;
         converter = htons(valid);
         status = send(clientSocket, &converter, sizeof(uint16_t), 0);
         if (status <= 0) {
            cout << "Error: send valid" << endl;
            pthread_detach(pthread_self());
            close(clientSocket);
            exit(1);
         }
      } else {
         guessedCharacters[guessedPointer] = guess;
         guessedPointer++;
         correct = false;
         valid = 1;
         converter = htons(valid);
         status = send(clientSocket, &converter, sizeof(uint16_t), 0);
         if (status <= 0) {
            cout << "Error: send valid" << endl;
            pthread_detach(pthread_self());
            close(clientSocket);
            exit(1);
         }

         /* Update user's progress */
         for (int i = 0; i < wordLength; i++) {
            if (guess[0] == word[i]) {
               userProgress[i * 2] = guess[0];
               correct = true;
               userProgressCount++;
            }
         }

         if (correct)
            reply = "Correct! \nIncorrect Guesses: " + incorrectGuesses;
         else {
            incorrectGuesses = incorrectGuesses + guess + " ";
            reply = "Incorrect! \nIncorrect Guesses: " + incorrectGuesses;
            numIncorrect++;
         }
         sendStr(clientSocket, reply, wordLength, numIncorrect, 0);

         if (userProgressCount == wordLength) {

            done = true;
            valid = 1;
            converter = htons(valid);
            status = send(clientSocket, &converter, sizeof(uint16_t), 0);
            if (status <= 0) {
               cout << "Error: send valid" << endl;
               pthread_detach(pthread_self());
               close(clientSocket);
               exit(1);
            }

            reply = "\nYou win! You guessed the word " + word + " correctly!";
            sendStr(clientSocket, reply, wordLength, numIncorrect, 0);

            score = (float)wordLength / (float)turnCounter;
            score = floor(score * 100.00 + 0.5) / 100.00;

            stringstream stream;

            stream << fixed << setprecision(2) << score;
            string s = stream.str();

            cout << username << "'s score is " << s << endl;

            pthread_mutex_lock(&m);
            reply = "";
            reply += leaderBoardString;
            numEntries++;

            if (numEntries > 3)
               numEntries = 3;

            leaderBoard[3].score = score;
            leaderBoard[3].user = username;

            sort(begin(leaderBoard), end(leaderBoard), &sorter);

            for (int i = 0; i < numEntries; i++) {
               stream.str(string());
               stream << leaderBoard[i].score;
               s = stream.str();

               reply = reply + leaderBoard[i].user + " " + s + "\n";
            }

            sendStr(clientSocket, reply, wordLength, numIncorrect, 0);
            pthread_mutex_unlock(&m);

         } else if (numIncorrect > 5) {
            done = true;
            valid = 1;
            converter = htons(valid);
            status = send(clientSocket, &converter, sizeof(uint16_t), 0);
            if (status <= 0) {
               cout << "Error: send valid" << endl;
               pthread_detach(pthread_self());
               close(clientSocket);
               exit(1);
            }

            reply = "\n\nYou lose :( Let's try again!";
            sendStr(clientSocket, reply, wordLength, numIncorrect, 0);

            score = 0;

            stringstream stream;

            stream << fixed << setprecision(2) << score;
            string s = stream.str();

            cout << username << "'s score is " << s << endl;

            pthread_mutex_lock(&m);
            reply = "";
            reply += leaderBoardString;
            numEntries++;

            if (numEntries > 3)
               numEntries = 3;

            leaderBoard[3].score = score;
            leaderBoard[3].user = username;

            sort(begin(leaderBoard), end(leaderBoard), &sorter);

            for (int i = 0; i < numEntries; i++) {
               stream.str(string());
               stream << leaderBoard[i].score;
               s = stream.str();

               reply = reply + leaderBoard[i].user + " " + s + "\n";
            }

            sendStr(clientSocket, reply, wordLength, numIncorrect, 0);
            pthread_mutex_unlock(&m);
         }

         else {
            valid = 0;
            converter = htons(valid);
            status = send(clientSocket, &converter, sizeof(uint16_t), 0);
            if (status <= 0) {
               cout << "Error: send valid" << endl;
               pthread_detach(pthread_self());
               close(clientSocket);
               exit(1);
            }
         }

         turnCounter++;
      }
   }

   pthread_detach(pthread_self());
   close(clientSocket);
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

   if (argc < 2) {
      cout << "Error: Port Number not entered" << endl;
      exit(1);
   }

   string fileName = (argc == 3) ? argv[2] : "NULL";

   if (argc == 3) {
      ifstream myfile(fileName.c_str());
      if (!myfile) {
         cout << "Error: Cannot open input file" << endl;
         exit(1);
      }
   }


   for (int i = 0; i < 4; i++) {
      leaderBoard[i].user = "";
      leaderBoard[i].score = 999999999;
   }

   numEntries = 0;

   pthread_mutex_init(&m, NULL);
   int status;
   unsigned short portNum = atoi(argv[1]);

   /* Create socket for server. */
   int sockfd;
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   /* Specify socket. */
   struct sockaddr_in serverAddr;
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(portNum);
   serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

   /* Bind socket to specified IP address and port number. */
   status = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
   if (status == -1) {
      cout << "Error: Failed to bind server socket" << endl;
      exit(1);
   }


   listen(sockfd, 128);
   cout << "Server Listening..." << endl;

   /* Declare Client Socket. */
   int clientSocket;
   struct sockaddr_in clientAddr;

   pthread_t tid;

   struct Argument args;

   args.arg1 = clientSocket;
   args.arg2 = fileName;


   while (1) {
      socklen_t addrLen = sizeof(clientAddr);
      int clientSocket =
         accept(sockfd, (struct sockaddr *)&clientAddr, &addrLen);
      if (clientSocket == -1) {
         cout << "Error: Client failed to connect" << endl;
         close(clientSocket);
         exit(1);
      }
      cout << "Connection Accepted" << endl;

      /* Create threads for multi-player support. */
      struct Argument args;
      args.arg1 = clientSocket;
      args.arg2 = fileName;
      status = pthread_create(&tid, NULL, playHangman, (void *)&args);
      if (status != 0) {
         cout << "Error: Failed to create thread" << endl;
         close(clientSocket);
         exit(1);
      }
   }

   close(clientSocket);
   return 1;
}