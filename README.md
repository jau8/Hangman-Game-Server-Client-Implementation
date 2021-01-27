# Hangman Game -- C++ Implementation

## Author: Jason Au

## How to Run the Game?
### Compiling
Open terminal and go to the main directory that contains server.cpp and client.cpp.
Compile the cpp files by running 'make'. Once done, client and server should appear
in the file directory.

### Running
#### Server
To run the server, use the following command:
'./server [PORT NUMBER] [Name or directory of TEXT FILE (optional)]'
If the user chooses to input a text file, then the server will extract words from
that file and use as guess words for the player.
If the user does not input a text file, then the server will use the default file
named 'words.txt', which is included in the folder.
The format of the text file is as follow:
Line 1: WordLength WordCount
Line 2: Word 1
Line 3: Word 2
...
Line n: Word n
Even though word length is specified, the program can support any word lengths.
#### Client
To run client, use the following command:
'./client 127.0.0.1 [Backdoor Place of the word in the file (optional)]'
If the user decides to input a backdoor argument, then the program will choose
that word as the guess word.
If the user does not input a backdoor argument, then the program will generate a
random word in the text file.

### High Level Description of Implementation
#### Intro
This program implements the classic hangman game as a console application with 
C++, socket programming, and multithreaded programming. 
The program will generate a word from a dictionary file. The player then has
to try to guess the word by suggesting letters within 6 tries. A score is
calculated and given to the user at the end of the game based on if the user
has guessed the word correct, how many tries, and how long the word is.
## Multi-player Support
This program support multiple players using multithreaded programming. Each
user is a thread. Since the users do not compete with each other directly,
the players do not have to wait for each other to start the game. One player
can play at a time for multiple players can play at once.
At the end of the game, each user will be given a score based on their 
performance and ranked in a leader board. 

### Messages Printed by Server and Client during the Program Flow
#### Server
./server 2017
Server Listening...
Connection Accepted
User Jason has connected.
The word is jinx for user Jason
User: Jason guess: l
User: Jason guess: q
Connection Accepted
User Jingfan has connected.
The word is buzz for user Jingfan
User: Jingfan guess: w
User: Jingfan guess: j
User: Jingfan guess: b
User: Jingfan guess: u
User: Jason guess: j
User: Jason guess: i
User: Jason guess: x
User: Jingfan guess: z
Jingfan's score is 0.80
User: Jason guess: y
User: Jason guess: q
Jason already guessed character q.
User: Jason guess: c
User: Jason guess: m
User: Jason guess: n
Jason's score is 0.44
Connection Accepted
User Arthur has connected.
The word is hajj for user Arthur
User: Arthur guess: q
User: Arthur guess: w
User: Arthur guess: e
User: Arthur guess: r
User: Arthur guess: t
User: Arthur guess: y
Arthur's score is 0.00

#### Client (Player 1)
./client 127.0.0.1 2017
Welcome to Hangman!
Enter username: Jingfan
Read to play? (Y/N): 2
Entered: 2

Turn 1
_ _ _ _ 
Letter to guess: W
Incorrect! 
Incorrect Guesses: w 

Turn 2
_ _ _ _ 
Letter to guess: j
Incorrect! 
Incorrect Guesses: w j 

Turn 3
_ _ _ _ 
Letter to guess: b
Correct! 
Incorrect Guesses: w j 

Turn 4
b _ _ _ 
Letter to guess: u
Correct! 
Incorrect Guesses: w j 

Turn 5
b u _ _ 
Letter to guess: z
Correct! 
Incorrect Guesses: w j 

You win! You guessed the word buzz correctly!

Leader Board

Jingfan 0.80

#### Client (Player 2)
./client 127.0.0.1 2017
Welcome to Hangman!
Enter username: Jason
Read to play? (Y/N): y

Turn 1
_ _ _ _ 
Letter to guess: l
Incorrect! 
Incorrect Guesses: l 

Turn 2
_ _ _ _ 
Letter to guess: Q
Incorrect! 
Incorrect Guesses: l q 

Turn 3
_ _ _ _ 
Letter to guess: j
Correct! 
Incorrect Guesses: l q 

Turn 4
j _ _ _ 
Letter to guess: I
Correct! 
Incorrect Guesses: l q 

Turn 5
j i _ _ 
Letter to guess: x
Correct! 
Incorrect Guesses: l q 

Turn 6
j i _ x 
Letter to guess: y
Incorrect! 
Incorrect Guesses: l q y 

Turn 7
j i _ x 
Letter to guess: q
Error! Letter q has been guessed before, please guess another letter.

Turn 7
j i _ x 
Letter to guess: c
Incorrect! 
Incorrect Guesses: l q y c 

Turn 8
j i _ x 
Letter to guess: m
Incorrect! 
Incorrect Guesses: l q y c m 

Turn 9
j i _ x 
Letter to guess: n
Correct! 
Incorrect Guesses: l q y c m 

You win! You guessed the word jinx correctly!

Leader Board

Jason 0.44
Jingfan 0.80

#### Client (Player 3)
./client 127.0.0.1 2017
Welcome to Hangman!
Enter username: Arthur
Read to play? (Y/N): n
Welcome to Hangman!
Enter username: Arthur
Read to play? (Y/N): y

Turn 1
_ _ _ _ 
Letter to guess: q
Incorrect! 
Incorrect Guesses: q 

Turn 2
_ _ _ _ 
Letter to guess: w
Incorrect! 
Incorrect Guesses: q w 

Turn 3
_ _ _ _ 
Letter to guess: e
Incorrect! 
Incorrect Guesses: q w e 

Turn 4
_ _ _ _ 
Letter to guess: r
Incorrect! 
Incorrect Guesses: q w e r 

Turn 5
_ _ _ _ 
Letter to guess: t
Incorrect! 
Incorrect Guesses: q w e r t 

Turn 6
_ _ _ _ 
Letter to guess: y
Incorrect! 
Incorrect Guesses: q w e r t y 


You lose :( Let's try again!

Leader Board

Arthur 0.00
Jason 0.44
Jingfan 0.80

### Edge Cases
#### Incorrect Input
Welcome to Hangman!
Enter username: Jason
Read to play? (Y/N): y

Turn 1
_ _ _ _ 
Letter to guess: q
Incorrect! 
Incorrect Guesses: q 

Turn 2
_ _ _ _ 
Letter to guess: 2
Error! Please guess one LETTER.
Letter to guess: 5
Error! Please guess one LETTER.
Letter to guess: qs
Error! Please guess ONE letter.
Letter to guess: 23dsds
Error! Please guess ONE letter.
Error! Please guess one LETTER.
Letter to guess: #$#
Error! Please guess ONE letter.
Error! Please guess one LETTER.
Letter to guess: 

#### Dictionary File Does Not Exist
./server 2017 Hello.txt
Error: Cannot open input file

#### Working Dictionary File
./server 2018 words.txt
Server Listening...
