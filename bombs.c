#include "stdio.h"
#include "stdbool.h"
#include "stdlib.h"
#include "termios.h"
#include "unistd.h"
#include "time.h"

int bombspercent = 15;

struct State
{
    bool Bomb;
    bool Flag;
    bool Closed;
};
 
struct Map
{
    int CursorRow;
    int CursorCol;
    int Rows;
    int Cols;
    bool first;
    struct State **tiles;
};

void initMap(struct Map *m, int Rows, int Cols){
    m->tiles = (struct State**)malloc(Rows * sizeof(struct State*));
    for (int i=0; i < Rows; i++) m->tiles[i] = (struct State*)malloc(Cols * sizeof(struct State));
    for(int Row=0; Row < Rows; Row++){
        for(int Col=0; Col < Cols; Col++){
            m->tiles[Row][Col].Closed = true;
            m->tiles[Row][Col].Bomb = false;
            m->tiles[Row][Col].Flag = false;
            m->first = true;
        }
    }

    m->Rows = Rows;
    m->Cols = Cols;
    m->CursorCol = 0;
    m->CursorRow = 0; 
}
int neighboringBombs(struct Map *m, int Row, int Col){
    int count = 0;
    for(int i1=-1; i1<=1; i1++){
        for(int i2=-1; i2<=1; i2++){
            int posRow = Row + i1;
            int posCol = Col + i2;
            if((posRow >= 0) && (posRow < m->Rows) && (posCol >= 0) && (posCol < m->Cols)){
                if(m->tiles[posRow][posCol].Bomb){
                    count++;
                }
            }
        }
    }
    return count;
}

bool isAtCursor(struct Map *m, int Row, int Col){
    if((m->CursorRow == Row) && (m->CursorCol == Col)) return true;
    return false;
}

struct State* getStateAtCursor(struct Map *m){
    return &(m->tiles[m->CursorRow][m->CursorCol]); 
}

void randomizeMap(struct Map *m){
    int bombs = m->Rows * m->Cols * bombspercent/100;
    while(bombs >= 0){
        int Row = rand() % m->Rows;
        int Col = rand() % m->Cols;
        if(!isAtCursor(m,Row, Col)){
        if(!m->tiles[Row][Col].Bomb){
            m->tiles[Row][Col].Bomb = true;
            bombs--;
        }
        }
    }
}

void openAllNeighbors(struct Map *m, int Row, int Col){
    if(neighboringBombs(m, Row, Col) > 0) return; 
    for(int i1=-1; i1<=1; i1++){
        for(int i2=-1; i2<=1; i2++){
            int posRow = Row + i1;
            int posCol = Col + i2;
            if((posRow >= 0) && (posRow < m->Rows) && 
               (posCol >= 0) && (posCol < m->Cols) &&
               !((posRow == Row) && (posCol == Col)))
                {
                struct State *s = &(m->tiles[posRow][posCol]);
                if((!s->Bomb) && (s->Closed) && (!s->Flag)){
                    if(neighboringBombs(m, posRow, posCol) > 0){
                        s->Closed = false;
                    }else{
                        s->Closed = false;
                        openAllNeighbors(m, posRow, posCol);
                    }
                }
            }
        }
    }
    return;
}

void printMap(struct Map *m){
    for(int Row=0; Row < m->Rows; Row++){
        for(int Col=0; Col < m->Cols; Col++){
            if(isAtCursor(m, Row, Col)){
                printf("(");
            }else{
                printf(" ");
            }
            struct State s =  m->tiles[Row][Col];
            if(s.Closed){
                printf(":");
            }else if (s.Flag){
                printf("?");
            }else if (s.Bomb){
                printf("X");
            }else{
                if(neighboringBombs(m, Row, Col) > 0){
                    printf("%d", neighboringBombs(m, Row, Col));
                }else{
                    printf(" ");
                }
            }
            if(isAtCursor(m, Row, Col)){
                printf(")");
            }else{
                printf(" ");
            }
        }
        printf("\n");
    }
}

void printAllOpen(struct Map *m){
    for(int Row=0; Row < m->Rows; Row++){
        for(int Col=0; Col < m->Cols; Col++){
            printf(" ");
            struct State s =  m->tiles[Row][Col];
            if (s.Bomb){
                printf("X");
            }else{
                if(neighboringBombs(m, Row, Col) > 0){
                    printf("%d", neighboringBombs(m, Row, Col));
                }else{
                    printf(" ");
                }
            }
            printf(" ");
        }
        printf("\n");
    }
}

bool win(struct Map *m){
    bool win = true;
    for(int Row=0; Row < m->Rows; Row++){
        for(int Col=0; Col < m->Cols; Col++){
            if(!(m->tiles[Row][Col].Bomb && m->tiles[Row][Col].Flag)){
                win = false;
            }
        }
    }
    return win;
}

bool input(struct Map *m){
    char c = getchar();
    bool end = false;
    struct State *s = getStateAtCursor(m);
    switch(c){
        case 'w': if(m->CursorRow > 0) m->CursorRow--; break;
        case 's': if(m->CursorRow < m->Rows-1) m->CursorRow++; break;
        case 'a': if(m->CursorCol > 0) m->CursorCol--; break;
        case 'd': if(m->CursorCol < m->Cols-1) m->CursorCol++; break;
        case 'f':
            if(s->Closed){ s->Closed = false; s->Flag=true;  break;}
            if(s->Flag)  { s->Closed = true;  s->Flag=false; break;}
        case ' ':
            if(!s->Flag && s->Closed){
                if(m->first){
                    m->first = false;
                    randomizeMap(m);
                }
                if(s->Bomb){
                    end = true;
                }else{
                    openAllNeighbors(m, m->CursorRow, m->CursorCol);
                    s->Closed = false;
                }
            }
            break;
    }
    return end;
}

void clear(){
    for(int i=0; i<1000; i++){
        printf("\n");
    }
    printf("\33[100A");
    printf("\33[100A");
}

int main(){
    ;srand(time(NULL));
    static struct termios oldT, newT;

    tcgetattr(STDIN_FILENO, &oldT);
    newT = oldT;
    newT.c_lflag &= ~(ICANON | ECHO);
    tcsetattr( STDIN_FILENO, TCSANOW, &newT);

    struct Map m; 
    initMap(&m, 10, 10);
    clear();
    printMap(&m);
    while(!input(&m)){
        printf("\33[%dA",m.Rows);
        printf("\33[%dA",m.Cols);
        printMap(&m);
        if(win(&m)){
            printf("You Win!\n");
            tcsetattr( STDIN_FILENO, TCSANOW, &oldT);
            return 0;
        }
    }
    printf("\33[%dA",m.Rows);
    printf("\33[%dA",m.Cols);
    printAllOpen(&m);

    tcsetattr( STDIN_FILENO, TCSANOW, &oldT);
    return 0;
}