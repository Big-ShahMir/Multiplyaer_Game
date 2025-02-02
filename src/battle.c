/*
 * socket demonstrations:
 * This is the server side of an "internet domain" socket connection, for
 * communicating over the network.
 *
 * In this case we are willing to wait for chatter from the client
 * _or_ for a new connection.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef PORT
    #define PORT 52688
#endif

# define SECONDS 10

#define MAXHP 30
#define MINHP 20
int rangeHP = MAXHP - MINHP + 1;

#define MINDMG 2
#define MAXDMG 6
int rangeDMG = MAXDMG - MINDMG + 1;

#define MINPWRDMG 6
#define MAXPWRDMG 18
#define MAXPWR 3
#define MINPWR 1
int rangePWR = MAXPWR - MINPWR + 1;

int playerCount = 0;

struct client {
    int fd;
    struct in_addr ipaddr;
    struct client *next;
    struct client *prevMatch;
    struct client *currMatch;
    char name[32];
    int inMatch;
    int hp;
    int powermoves;
    int isTurn;
    int wins;
    int speak;
};

static struct client *addclient(struct client *top, int fd, struct in_addr addr);
static struct client *removeclient(struct client *top, int fd);
static void broadcast(struct client *top, char *s, int size, int fd);
int handleclient(struct client *p, struct client *top);
void printInstructions(struct client *top, struct client *p1, struct client *p2);
void matchplayers(struct client *top, struct client *player1, struct client *player2);
void makeLeaderboard(char* arr[], int arr2[], struct client *head);
void print_leaderboard(struct client *top, struct client *p);


int bindandlisten(void);

int main(void) {

    int clientfd, maxfd, nready;
    struct client *p;
    struct client *head = NULL;
    socklen_t len;
    struct sockaddr_in q;
    struct timeval tv;
    fd_set allset;
    fd_set rset;

    int i;

    // srand(time(NULL));

    int listenfd = bindandlisten();
    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;
        /* timeout in seconds (You may not need to use a timeout for
        * your assignment)*/
        tv.tv_sec = SECONDS;
        tv.tv_usec = 0;  /* and microseconds */

        nready = select(maxfd + 1, &rset, NULL, NULL, &tv);
        if (nready == 0) {
            printf("No response from clients in %d seconds\n", SECONDS);
            continue;
        }

        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)){
            printf("a new client is connecting\n");
            len = sizeof(q);
            if ((clientfd = accept(listenfd, (struct sockaddr *)&q, &len)) < 0) {
                perror("accept");
                exit(1);
            }
            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("connection from %s\n", inet_ntoa(q.sin_addr));
            head = addclient(head, clientfd, q.sin_addr);
        }

        for(i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &rset)) {
                for (p = head; p != NULL; p = p->next) {
                    if (p->fd == i) {
                        int result = handleclient(p, head);
                        if (result == -1) {
                            int tmp_fd = p->fd;
                            head = removeclient(head, p->fd);
                            FD_CLR(tmp_fd, &allset);
                            close(tmp_fd);
                        }
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

int damage(struct client *top, struct client *player1, struct client *player2, char type){
    // printInstructions(top, player1, player2);
    // if (type == 's'){
    //     // speaking
    //     ;
    // }
    if (type == 'a'){
        int dmg = rand() % rangeDMG + MINDMG;
        player2->hp -= dmg;

        player1->isTurn = 0;
        player2->isTurn = 1;
        
        char broadcastMessage[512];
        sprintf(broadcastMessage, "You hit %s for %d damage! \n",player2->name,dmg);
        write(player1->fd, broadcastMessage, strlen(broadcastMessage));

        // broadcastMessage[512];
        sprintf(broadcastMessage, "Your hitpoints: %d \nYour powermoves: %d \n%s's hitpoints: %d \n",player1->hp, player1->powermoves, player2->name,player2->hp);
        write(player1->fd, broadcastMessage, strlen(broadcastMessage));

        // broadcastMessage[512];
        sprintf(broadcastMessage, "%s hits you for %d damage! \n",player1->name,dmg);
        write(player2->fd, broadcastMessage, strlen(broadcastMessage));

        // broadcastMessage[512];
        sprintf(broadcastMessage, "Your hitpoints: %d \nYour powermoves: %d \n%s's hitpoints: %d \n",player2->hp, player2->powermoves, player1->name,player1->hp);
        write(player2->fd, broadcastMessage, strlen(broadcastMessage));

    }
    else if (type == 'p'){
        player1->powermoves -= 1;
        int roll = rand() % 100;
        if (roll >= 50){

            int dmg = rand() % rangePWR + MINPWR;
            player2->hp -= dmg;
            
            char broadcastMessage[512];
            sprintf(broadcastMessage, "You hit %s for %d damage! \n",player2->name,dmg);
            write(player1->fd, broadcastMessage, strlen(broadcastMessage));

            // broadcastMessage[512];
            sprintf(broadcastMessage, "Your hitpoints: %d \nYour powermoves: %d \n%s's hitpoints: %d \n",player1->hp, player1->powermoves, player2->name,player2->hp);
            write(player1->fd, broadcastMessage, strlen(broadcastMessage));

            // broadcastMessage[512];
            sprintf(broadcastMessage, "%s powermoves you for %d damage! \n",player1->name,dmg);
            write(player2->fd, broadcastMessage, strlen(broadcastMessage));

            // broadcastMessage[512];
            sprintf(broadcastMessage, "Your hitpoints: %d \nYour powermoves: %d \n%s's hitpoints: %d \n",player2->hp, player2->powermoves, player1->name,player1->hp);
            write(player2->fd, broadcastMessage, strlen(broadcastMessage));
        }
        else {
            char broadcastMessage[512];
            sprintf(broadcastMessage, "Power attack missed\n");
            write(player1->fd, broadcastMessage, strlen(broadcastMessage));

            char broadcastMessage2[512];
            sprintf(broadcastMessage2, "%s missed\n",player1->name);
            write(player2->fd, broadcastMessage, strlen(broadcastMessage));
        }
        player1->isTurn = 0;
        player2->isTurn = 1;
    }
    if (player2->hp <=0){
        player1->wins += 1;
        print_leaderboard(top, player1);
        return 1;
    }
    return 0;
}

int handleclient(struct client *p, struct client *top) {
    // if (p->isTurn){
    //     printInstructions(top, p, (*p).currMatch);
    // }
    char buf[256];
    char outbuf[512];
    int res;
    int len = read(p->fd, buf, sizeof(buf) - 1);\
    fflush(stdout);
    if (len > 0) {
        buf[len] = '\0';

        if (p->inMatch == 1 && (*p).isTurn == 1 && p->speak == 1){
            (*p).speak = 0;
            buf[len] = '\0';
            write((*p).fd, "You Said: ", 10);
            write((*p).fd, buf, len);
            write((*p).fd, "\n", 1);

            char msg[300];
            sprintf(msg, "%s said %s \n", (*p).name, buf);
            write((*p).currMatch->fd, msg, strlen(msg));
            printInstructions(top, (*p).currMatch, p);


            }
        else if (p->inMatch == 1 && (*p).isTurn == 1 && (p->speak) == 0 && len == 2){
            if (buf[0] == 'a'){
               res = damage(top, p, p->currMatch, 'a');
               printInstructions(top, (*p).currMatch, p);

            }
            else if (buf[0] == 'p' && p->powermoves > 0)
            {
                res = damage(top, p, p->currMatch, 'p');
                printInstructions(top, (*p).currMatch, p);
            }
            else if (buf[0] == 's')
            {
                write((*p).fd, "\nSpeak: ", 8);
                p->speak = 1;
                //printInstructions(top, p, (*p).currMatch);
            }


            if (res == 1) {
                char msg3[100];
                sprintf(msg3, "%s gives up. You win!\n", (*p).currMatch->name);
                write((*p).fd, msg3, strlen(msg3));
                char msg2[100];
                sprintf(msg2, "You are no match for %s. You scurry away...\n", (*p).name);
                write((*p).currMatch->fd, msg2, strlen(msg2));

                printf("%s and %s no longer matched", (*p).name, (*p).currMatch->name);
                (*p).prevMatch = (*p).currMatch;
                (*p).inMatch = 0;
                struct client *p2 = (*p).currMatch;
                (*p2).currMatch = NULL;
                (*p2).prevMatch = p;
                (*p2).inMatch = 0;
                (*p).currMatch = NULL;
                // printf("%s and %s no longer matched", (*p).name, (*p).currMatch->name);

                // (*p).currMatch->next = p;
                // (*p).next = top;
                // top = p;
                // struct client arr[2];
                struct client **p1;
               for (p1 = &top; *p1; p1 = &((*p1)->next)){
                if ((*p1)->inMatch == 0 && (*p1)->fd != p->fd){
                if ((*p1)->prevMatch == NULL){
                    matchplayers(top, *p1, p);
                    break;
                }
                else if (((*p1)->prevMatch)->fd != p->fd) {
                    matchplayers(top, *p1, p);
                    break;
                }

            }
            // count1++;
    }

            for (p1 = &top; *p1; p1 = &((*p1)->next)){
                if ((*p1)->inMatch == 0 && (*p1)->fd != p2->fd){
                if ((*p1)->prevMatch == NULL){
                    matchplayers(top, *p1, p2);
                    break;
                }
                else if (((*p1)->prevMatch)->fd != p2->fd) {
                    matchplayers(top, *p1, p2);
                    break;
                }

            }

               }

               }

        }

        // printf("Received %d bytes: %s", len, buf);
        // sprintf(outbuf, "%s says: %s", inet_ntoa(p->ipaddr), buf);
        // broadcast(top, outbuf, strlen(outbuf));

        return 0;
    } else if (len <= 0) {
        // socket is closed
        // printf("Disconnect from %s\n", inet_ntoa(p->ipaddr));
        // sprintf(outbuf, "Goodbye %s\r\n", inet_ntoa(p->ipaddr));
        printf("Disconnect from %s\n", (*p).name);
        sprintf(outbuf, "Goodbye %s\r\n", (*p).name);
        broadcast(top, outbuf, strlen(outbuf), -2);

        char msg3[100];
        sprintf(msg3, "--%s Dropped. You win!\n", (*p).currMatch->name);
        write((*p).fd, msg3, strlen(msg3));
        // struct client arr[2];
        struct client **p1;
        struct client *p2 = p->currMatch;

        (*p).prevMatch = (*p).currMatch;
        (*p).inMatch = 0;
        (*p2).currMatch = NULL;
        (*p2).prevMatch = p;
        (*p2).inMatch = 0;
        (*p).currMatch = NULL;

        for (p1 = &top; *p1; p1 = &((*p1)->next)){
                if ((*p1)->inMatch == 0 && (*p1)->fd != p2->fd){
                if ((*p1)->prevMatch == NULL){
                    matchplayers(top, *p1, p2);
                    break;
                }
                else if (((*p1)->prevMatch)->fd != p2->fd) {
                    matchplayers(top, *p1, p2);
                    break;
                }
            }       
        }

        return -1;
    }
    return 0;
}


 /* bind and listen, abort on error
  * returns FD of listening socket
  */
int bindandlisten(void) {
    struct sockaddr_in r;
    int listenfd;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    int yes = 1;
    if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
        perror("setsockopt");
    }
    memset(&r, '\0', sizeof(r));
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, 5)) {
        perror("listen");
        exit(1);
    }
    return listenfd;
}

static struct client *addclient(struct client *top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    playerCount += 1;

    p->fd = fd;
    p->ipaddr = addr;
    p->next = top;
    top = p;
    p->prevMatch = NULL;
    p->currMatch = NULL;
    p->inMatch = 0;
    p->isTurn = 0;
    p->wins = 0;
    p->speak = 0;
    // p->name = malloc(33);  // CAN BE REPLACED WITH STRING ARRAY [33]
    //strcpy(p->name, "");

    char broadcastMessage[512];
    sprintf(broadcastMessage, "Enter your username (maximum 32 chars): ");
    write(top->fd, broadcastMessage, strlen(broadcastMessage));

    char buf[32];
    int len = read(fd, buf, 31*sizeof(char));
    if (len <= 0) {
        perror("read");
        exit(1);
    }
    buf[len-1] = '\0';
    strncpy(p->name, buf, 32);

    sprintf(broadcastMessage, "**%s entered the server**, %d players online\n", p->name, playerCount);
    broadcast(top, broadcastMessage, strlen(broadcastMessage), -2);

    // playerCount += 1;

    if (playerCount % 2 == 0){

        // struct client arr[2];
        struct client **p1;
        // struct client *p2;
        // struct client *p3;
        // //int check=0;

        // int count1 = 0;
        for (p1 = &top; *p1; p1 = &((*p1)->next)){
            // if (count1 == playerCount-1){
            //     p2 = *p1;
            // }
            // if (count1 == playerCount-2){
            //     p3 = *p1;
            // }
            // if ((*p1)->inMatch == 0 && (*p1)->prevMatch != NULL && ((*p1)->prevMatch)->fd != p->fd && (*p1)->fd != p->fd){
            //     printf("2");
            //     matchplayers(top, *p1, p);
            //     break;
            // }
            if ((*p1)->inMatch == 0 && (*p1)->fd != p->fd){
                if ((*p1)->prevMatch == NULL){
                    matchplayers(top, *p1, p);
                    break;
                }
                else if (((*p1)->prevMatch)->fd != p->fd) {
                    matchplayers(top, *p1, p);
                    break;
                }

            }
            // count1++;
    }
    //matchplayers(top, p2, p3);
    }
    return top;
}

static struct client *removeclient(struct client *top, int fd) {
    struct client **p;

    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }

    playerCount -= 1;

    return top;
}


static void broadcast(struct client *top, char *s, int size, int fd) { //ALTERED BY SHAH
    struct client *p;
    int check;
    for (p = top; p; p = p->next) {
        if ((*p).fd != fd){
        check = write(p->fd, s, size);
        if (check == -1){
            perror("Writing Error");
            continue;
            }
        }
    }
    /* should probably check write() return value and perhaps remove client */
}

void printInstructions(struct client *top, struct client *p1, struct client *p2){
    char broadcastMessage2[512];
    sprintf(broadcastMessage2, "Waiting for %s to strike.... \n",p1->name);
    write(p2->fd, broadcastMessage2, strlen(broadcastMessage2));

    if (p1->powermoves <= 0){
        char broadcastMessage[512];
        sprintf(broadcastMessage, "Your hitpoints: %d \n \n%s's hitpoints: %d \n",p1->hp,p2->name,p2->hp);
        write(p1->fd, broadcastMessage, strlen(broadcastMessage));

        // broadcastMessage[512];
        sprintf(broadcastMessage, "(s) to speak to the other player\n(a) to do a regular attack (2-6 damage)\n");
        write(p1->fd, broadcastMessage, strlen(broadcastMessage));
    }
    else{
        char broadcastMessage[512];
        sprintf(broadcastMessage, "Your hitpoints: %d \nYour powermoves: %d \n \n%s's hitpoints: %d \n",p1->hp,p1->powermoves,p2->name,p2->hp);    
        write(p1->fd, broadcastMessage, strlen(broadcastMessage));

        // broadcastMessage[512];
        sprintf(broadcastMessage, "(s) to speak to the other player\n(a) to do a regular attack (2-6 damage)\n(p) to do a powermove with a 50%% of hitting (6-18 damage)\n(q) to forfeit the match \n\n");
        write(p1->fd, broadcastMessage, strlen(broadcastMessage));
    }
}

void matchplayers(struct client *top, struct client *player1, struct client *player2){
    // struct client *player1 = removeclient(top, p1->fd);
    // struct client *player2 = removeclient(top, p2->fd);

    if (player1->inMatch){
        printf("player 1: %s, is already in a match\n", player1->name);
    }
    else if (player2->inMatch){
        printf("player 2: %s, is already in a match\n", player2->name);
        return;
    }
    else if (player1->prevMatch == player2 && player2->prevMatch == player1){
        printf("players: %s & %s cannot be matched up\n", player1->name, player2->name);
        return;
    }
    else { // match up and send to play (not fully implemented)

        char broadcastMessage[512];
        sprintf(broadcastMessage, "%s engages %s!\n",player1->name, player2->name);
        broadcast(top, broadcastMessage, strlen(broadcastMessage), player1->fd);

        player1->currMatch = player2;
        player1->hp = rand() % rangeHP + MINHP;
        player1->powermoves = rand() % rangePWR + MINPWR;
        player1->isTurn = 1;
        player1->inMatch = 1;

        player2->currMatch = player1;
        player2->hp = rand() % rangeHP + MINHP;
        player2->powermoves = rand() % rangePWR + MINPWR;
        player2->inMatch = 1;

        // broadcastMessage[512];
        sprintf(broadcastMessage, "You engage %s!\n\n",player1->currMatch->name);
        write(player1->fd, broadcastMessage, strlen(broadcastMessage));

        // broadcastMessage[512];
        sprintf(broadcastMessage, "You engage %s!\n\n",player2->currMatch->name);
        write(player2->fd, broadcastMessage, strlen(broadcastMessage));

        printInstructions(top, player1,player2);
    }

    // player1->currMatch = NULL;
    // player1->prevMatch = player2;
    // player1->inMatch = 0;

    // player2->currMatch = NULL;
    // player2->prevMatch = player1;
    // player2->inMatch = 0;
    // printf("%s and %s no longer matched", player1->name, player2->name);
    // addclient(player1, player1->fd, in_addr);
    // addclient(player2, player2->fd, in_addr);
}

// // our own enhancement
// char *leaderboard[] = {"", "", ""};
// int winsNumber[] = {-1, -1, -1};

// void makeLeaderboard(char* arr[], int arr2[], struct client *head){
//     printf("554");
//     struct client *p;
//     printf("556");
//     for (p = head; p != NULL; p = p->next){
//         if (p->wins > arr2[2]) {
//             arr2[0] = arr2[1];
//             strncpy(arr[0], arr[1], strlen(arr[1]));
//             arr2[1] = arr2[2];
//             strncpy(arr[1], arr[2], strlen(arr[2]));
//             arr2[2] = p->wins;
//             strncpy(arr[0], p->name, strlen(p->name));
//         }
//         else if (p->wins > arr2[1]) {
//             arr2[0] = arr2[1];
//             strncpy(arr[0], arr[1], strlen(arr[1]));
//             arr2[1] = p->wins;
//             strncpy(arr[1], p->name, strlen(p->name));
//         }
//         else if (p->wins > arr2[0]) {
//             arr2[0] = p->wins;
//             strncpy(arr[0], p->name, strlen(p->name));
//         }
//     }
// }

// our own enhancement (a session based leader board)
char leaderboard[3][33] = {"", "", ""};
int winsNumber[] = {-1, -1, -1};

void print_leaderboard(struct client *top, struct client *p){
    if (p->wins > winsNumber[0]){
        winsNumber[2] = winsNumber[1];
        winsNumber[1] = winsNumber[0];
        winsNumber[0] = p->wins;
        strncpy(leaderboard[2], leaderboard[1], strlen(leaderboard[1]));
        strncpy(leaderboard[1], leaderboard[0], strlen(leaderboard[0]));
        // char name2[32];
        // strncpy(name2,p->name, strlen(p->name));
        strncpy(leaderboard[0], p->name, strlen(p->name));
    }
    else if (p->wins > winsNumber[1]){
        winsNumber[2] = winsNumber[1];
        winsNumber[1] = p->wins;
        strncpy(leaderboard[2], leaderboard[1], strlen(leaderboard[1]));
        strncpy(leaderboard[1], p->name, strlen(p->name));
    }
    else if (p->wins > winsNumber[2]){
        winsNumber[2] = p->wins;
        strncpy(leaderboard[2], p->name, strlen(p->name));
    }
    char broadcastMessage[1024];
    sprintf(broadcastMessage, "\n\nLeaderboard:\n1: %s with %d wins\n2: %s with %d wins\n3: %s with %d wins\n\n", leaderboard[0], winsNumber[0], leaderboard[1], winsNumber[1], leaderboard[2], winsNumber[2]);
    broadcast(top, broadcastMessage, strlen(broadcastMessage), -2);
}
