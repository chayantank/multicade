#include "chess_main.h"
#include "chess_input.h"
#include "chess_display.h"
#include <Arduino.h>

#define BUZZER_PIN 14

namespace Chess {

const int EMPTY = 0;
const int PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6;
const int WHITE = 1, BLACK = -1;

int board[64];

enum GameState { STATE_INTRO, STATE_PLAYER_TURN, STATE_AI_TURN, STATE_GAMEOVER };
GameState state = STATE_INTRO;

int cursorX = 4, cursorY = 4;
int selectedSq = -1;

// Bitmaps
const unsigned char bmp_w_pawn[8]   = {0x00, 0x18, 0x24, 0x24, 0x18, 0x24, 0x42, 0xFF};
const unsigned char bmp_b_pawn[8]   = {0x00, 0x18, 0x3C, 0x3C, 0x18, 0x3C, 0x7E, 0xFF};
const unsigned char bmp_w_knight[8] = {0x00, 0x38, 0x44, 0x4C, 0x50, 0x20, 0x42, 0xFF};
const unsigned char bmp_b_knight[8] = {0x00, 0x38, 0x7C, 0x7C, 0x70, 0x30, 0x7E, 0xFF};
const unsigned char bmp_w_bishop[8] = {0x18, 0x24, 0x42, 0x42, 0x24, 0x18, 0x42, 0xFF};
const unsigned char bmp_b_bishop[8] = {0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x7E, 0xFF};
const unsigned char bmp_w_rook[8]   = {0x00, 0xAA, 0xAA, 0x42, 0x42, 0x42, 0x42, 0xFF};
const unsigned char bmp_b_rook[8]   = {0x00, 0xAA, 0xAA, 0x7E, 0x7E, 0x7E, 0x7E, 0xFF};
const unsigned char bmp_w_queen[8]  = {0x81, 0xA5, 0xA5, 0x5A, 0x24, 0x24, 0x42, 0xFF};
const unsigned char bmp_b_queen[8]  = {0x81, 0xA5, 0xA5, 0x7E, 0x3C, 0x3C, 0x7E, 0xFF};
const unsigned char bmp_w_king[8]   = {0x18, 0x3C, 0x18, 0x5A, 0x24, 0x24, 0x42, 0xFF};
const unsigned char bmp_b_king[8]   = {0x18, 0x3C, 0x18, 0x7E, 0x3C, 0x3C, 0x7E, 0xFF};

struct Move {
    uint8_t from;
    uint8_t to;
    uint8_t promote;
};

void resetGame() {
    int initialBoard[64] = {
        -4, -2, -3, -5, -6, -3, -2, -4,
        -1, -1, -1, -1, -1, -1, -1, -1,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         1,  1,  1,  1,  1,  1,  1,  1,
         4,  2,  3,  5,  6,  3,  2,  4
    };
    for(int i=0; i<64; i++) board[i] = initialBoard[i];
    state = STATE_PLAYER_TURN;
    cursorX = 4; cursorY = 6;
    selectedSq = -1;
}

void setup() {
    display_setup();
    input_setup();
    state = STATE_INTRO;
}

int generateMoves(int side, Move* moves) {
    int count = 0;
    for (int i=0; i<64; i++) {
        int piece = board[i];
        if (piece * side <= 0) continue;
        
        int r = i / 8, c = i % 8;
        int pType = abs(piece);
        
        if (pType == PAWN) {
            int dir = (side == WHITE) ? -1 : 1;
            int nr = r + dir;
            if (nr >= 0 && nr < 8) {
                int to = nr * 8 + c;
                if (board[to] == 0) {
                    bool promote = (nr == 0 || nr == 7);
                    moves[count++] = { (uint8_t)i, (uint8_t)to, promote ? (uint8_t)QUEEN : (uint8_t)0 };
                    if ((side == WHITE && r == 6) || (side == BLACK && r == 1)) {
                        int to2 = (r + 2*dir) * 8 + c;
                        if (board[to2] == 0) {
                            moves[count++] = { (uint8_t)i, (uint8_t)to2, 0 };
                        }
                    }
                }
                for (int dc = -1; dc <= 1; dc += 2) {
                    if (c + dc >= 0 && c + dc < 8) {
                        int to_cap = nr * 8 + (c + dc);
                        if (board[to_cap] * side < 0) {
                            bool promote = (nr == 0 || nr == 7);
                            moves[count++] = { (uint8_t)i, (uint8_t)to_cap, promote ? (uint8_t)QUEEN : (uint8_t)0 };
                        }
                    }
                }
            }
        } else if (pType == KNIGHT) {
            int dr[] = {-2, -2, -1, -1, 1, 1, 2, 2};
            int dc[] = {-1, 1, -2, 2, -2, 2, -1, 1};
            for (int d=0; d<8; d++) {
                int nr = r + dr[d], nc = c + dc[d];
                if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                    int to = nr * 8 + nc;
                    if (board[to] * side <= 0) moves[count++] = { (uint8_t)i, (uint8_t)to, 0 };
                }
            }
        } else if (pType == KING) {
            int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
            int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};
            for (int d=0; d<8; d++) {
                int nr = r + dr[d], nc = c + dc[d];
                if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                    int to = nr * 8 + nc;
                    if (board[to] * side <= 0) moves[count++] = { (uint8_t)i, (uint8_t)to, 0 };
                }
            }
        } else {
            int r_dirs[8], c_dirs[8], n_dirs = 0;
            if (pType == ROOK || pType == QUEEN) {
                r_dirs[0]=-1; c_dirs[0]=0; r_dirs[1]=1; c_dirs[1]=0;
                r_dirs[2]=0; c_dirs[2]=-1; r_dirs[3]=0; c_dirs[3]=1;
                n_dirs = 4;
            }
            if (pType == BISHOP || pType == QUEEN) {
                r_dirs[n_dirs]=-1; c_dirs[n_dirs]=-1; n_dirs++;
                r_dirs[n_dirs]=-1; c_dirs[n_dirs]=1; n_dirs++;
                r_dirs[n_dirs]=1; c_dirs[n_dirs]=-1; n_dirs++;
                r_dirs[n_dirs]=1; c_dirs[n_dirs]=1; n_dirs++;
            }
            for (int d=0; d<n_dirs; d++) {
                int nr = r + r_dirs[d], nc = c + c_dirs[d];
                while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
                    int to = nr * 8 + nc;
                    if (board[to] * side > 0) break;
                    moves[count++] = { (uint8_t)i, (uint8_t)to, 0 };
                    if (board[to] * side < 0) break;
                    nr += r_dirs[d]; nc += c_dirs[d];
                }
            }
        }
    }
    return count;
}

bool isSquareAttacked(int sq, int bySide) {
    int r = sq / 8, c = sq % 8;
    int pawnDir = (bySide == WHITE) ? 1 : -1;
    for (int dc = -1; dc <= 1; dc += 2) {
        int nr = r + pawnDir, nc = c + dc;
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            if (board[nr*8+nc] == PAWN * bySide) return true;
        }
    }
    int dr_n[] = {-2, -2, -1, -1, 1, 1, 2, 2}, dc_n[] = {-1, 1, -2, 2, -2, 2, -1, 1};
    for (int d=0; d<8; d++) {
        int nr = r + dr_n[d], nc = c + dc_n[d];
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            if (board[nr*8+nc] == KNIGHT * bySide) return true;
        }
    }
    int dr_k[] = {-1, -1, -1, 0, 0, 1, 1, 1}, dc_k[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    for (int d=0; d<8; d++) {
        int nr = r + dr_k[d], nc = c + dc_k[d];
        if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            if (board[nr*8+nc] == KING * bySide) return true;
        }
    }
    int r_dirs[] = {-1, 1, 0, 0, -1, -1, 1, 1}, c_dirs[] = {0, 0, -1, 1, -1, 1, -1, 1};
    for (int d=0; d<8; d++) {
        int nr = r + r_dirs[d], nc = c + c_dirs[d];
        while (nr >= 0 && nr < 8 && nc >= 0 && nc < 8) {
            int to = nr * 8 + nc;
            int piece = board[to];
            if (piece != 0) {
                if (piece * bySide > 0) {
                    int pt = abs(piece);
                    if (d < 4 && (pt == ROOK || pt == QUEEN)) return true;
                    if (d >= 4 && (pt == BISHOP || pt == QUEEN)) return true;
                }
                break;
            }
            nr += r_dirs[d]; nc += c_dirs[d];
        }
    }
    return false;
}

bool inCheck(int side) {
    int kingSq = -1;
    for (int i=0; i<64; i++) {
        if (board[i] == KING * side) { kingSq = i; break; }
    }
    if (kingSq == -1) return true;
    return isSquareAttacked(kingSq, -side);
}

int apply(Move m) {
    int captured = board[m.to];
    int piece = board[m.from];
    board[m.to] = m.promote ? (m.promote * (piece > 0 ? 1 : -1)) : piece;
    board[m.from] = 0;
    return captured;
}

void unapply(Move m, int originalPiece, int captured) {
    board[m.from] = originalPiece;
    board[m.to] = captured;
}

int evaluate() {
    int score = 0;
    for (int i=0; i<64; i++) {
        int p = board[i];
        if (p == 0) continue;
        int pt = abs(p);
        int val = 0;
        if (pt == PAWN) val = 10;
        else if (pt == KNIGHT) val = 30;
        else if (pt == BISHOP) val = 30;
        else if (pt == ROOK) val = 50;
        else if (pt == QUEEN) val = 90;
        else if (pt == KING) val = 900;
        
        int r = i / 8, c = i % 8;
        if (r >= 2 && r <= 5 && c >= 2 && c <= 5) val += 1;
        if (r >= 3 && r <= 4 && c >= 3 && c <= 4) val += 1;
        
        if (p > 0) score += val; else score -= val;
    }
    return score;
}

int minimax(int depth, int alpha, int beta, int side) {
    if (depth == 0) return evaluate();
    
    Move moves[250];
    int count = generateMoves(side, moves);
    if (count > 240) count = 240;
    
    int bestVal = (side == WHITE) ? -99999 : 99999;
    bool hasValidMove = false;
    
    for (int i=0; i<count; i++) {
        yield(); // Prevent Watchdog Timer Reset on ESP32
        Move m = moves[i];
        int original = board[m.from];
        int captured = apply(m);
        
        if (!inCheck(side)) {
            hasValidMove = true;
            int val = minimax(depth - 1, alpha, beta, -side);
            if (side == WHITE) {
                if (val > bestVal) bestVal = val;
                if (bestVal > alpha) alpha = bestVal;
                unapply(m, original, captured);
                if (beta <= alpha) break;
            } else {
                if (val < bestVal) bestVal = val;
                if (bestVal < beta) beta = bestVal;
                unapply(m, original, captured);
                if (beta <= alpha) break;
            }
        } else {
            unapply(m, original, captured);
        }
    }
    
    if (!hasValidMove) {
        if (inCheck(side)) return (side == WHITE) ? -9000 : 9000;
        return 0;
    }
    
    return bestVal;
}

Move getBestMove(int depth, int side) {
    Move moves[250];
    int count = generateMoves(side, moves);
    if (count > 240) count = 240;
    
    int bestVal = (side == WHITE) ? -99999 : 99999;
    Move bestMove = {0, 0, 0};
    bool found = false;
    
    for (int i=0; i<count; i++) {
        yield(); // Prevent WDT
        Move m = moves[i];
        int original = board[m.from];
        int captured = apply(m);
        
        if (!inCheck(side)) {
            if (!found) { bestMove = m; found = true; }
            int val = minimax(depth - 1, -99999, 99999, -side);
            
            if (side == WHITE) {
                if (val > bestVal) { bestVal = val; bestMove = m; }
            } else {
                if (val < bestVal) { bestVal = val; bestMove = m; }
            }
        }
        unapply(m, original, captured);
    }
    return bestMove;
}

void drawScaledBitmap(int x, int y, const unsigned char* bmp, int scale, int color) {
    for (int r = 0; r < 8; r++) {
        unsigned char row = bmp[r];
        for (int c = 0; c < 8; c++) {
            if (row & (0x80 >> c)) {
                fillRect(x + c * scale, y + r * scale, scale, scale, color);
            }
        }
    }
}

void drawBoardUI() {
    for(int i=0; i<=8; i++) {
        drawLine(i*8, 0, i*8, 64, 1);
        drawLine(0, i*8, 64, i*8, 1);
    }
    for(int r=0; r<8; r++) {
        for(int c=0; c<8; c++) {
            int p = board[r*8+c];
            if (p != 0) {
                int px = c*8;
                int py = r*8;
                int pt = abs(p);
                const unsigned char* bmp = nullptr;
                if (p > 0) {
                    if (pt == PAWN) bmp = bmp_w_pawn;
                    if (pt == KNIGHT) bmp = bmp_w_knight;
                    if (pt == BISHOP) bmp = bmp_w_bishop;
                    if (pt == ROOK) bmp = bmp_w_rook;
                    if (pt == QUEEN) bmp = bmp_w_queen;
                    if (pt == KING) bmp = bmp_w_king;
                } else {
                    if (pt == PAWN) bmp = bmp_b_pawn;
                    if (pt == KNIGHT) bmp = bmp_b_knight;
                    if (pt == BISHOP) bmp = bmp_b_bishop;
                    if (pt == ROOK) bmp = bmp_b_rook;
                    if (pt == QUEEN) bmp = bmp_b_queen;
                    if (pt == KING) bmp = bmp_b_king;
                }
                if (bmp) drawBitmap(px, py, bmp, 8, 8, 1);
            }
        }
    }
    
    if (selectedSq != -1) {
        Move moves[250];
        int count = generateMoves(WHITE, moves);
        if (count > 240) count = 240;
        for(int i=0; i<count; i++) {
            if (moves[i].from == selectedSq) {
                int original = board[moves[i].from];
                int captured = apply(moves[i]);
                if (!inCheck(WHITE)) {
                    int to_r = moves[i].to / 8;
                    int to_c = moves[i].to % 8;
                    drawCircle(to_c*8 + 4, to_r*8 + 4, 2, 1);
                }
                unapply(moves[i], original, captured);
            }
        }
    }
    
    drawRect(cursorX*8, cursorY*8, 8, 8, 1);
    drawRect(cursorX*8+1, cursorY*8+1, 6, 6, 1);
    
    // Magnified View
    int magX = 74;
    int magY = 8;
    int magS = 16;
    
    drawRect(magX - 2, magY - 2, magS*3 + 4, magS*3 + 4, 1);
    
    for (int r_off = -1; r_off <= 1; r_off++) {
        for (int c_off = -1; c_off <= 1; c_off++) {
            int br = cursorY + r_off;
            int bc = cursorX + c_off;
            int dr = r_off + 1;
            int dc = c_off + 1;
            int drawX = magX + dc * magS;
            int drawY = magY + dr * magS;
            
            drawRect(drawX, drawY, magS, magS, 1);
            
            if (br >= 0 && br < 8 && bc >= 0 && bc < 8) {
                int p = board[br * 8 + bc];
                if (p != 0) {
                    int pt = abs(p);
                    const unsigned char* bmp = nullptr;
                    if (p > 0) {
                        if (pt == PAWN) bmp = bmp_w_pawn;
                        if (pt == KNIGHT) bmp = bmp_w_knight;
                        if (pt == BISHOP) bmp = bmp_w_bishop;
                        if (pt == ROOK) bmp = bmp_w_rook;
                        if (pt == QUEEN) bmp = bmp_w_queen;
                        if (pt == KING) bmp = bmp_w_king;
                    } else {
                        if (pt == PAWN) bmp = bmp_b_pawn;
                        if (pt == KNIGHT) bmp = bmp_b_knight;
                        if (pt == BISHOP) bmp = bmp_b_bishop;
                        if (pt == ROOK) bmp = bmp_b_rook;
                        if (pt == QUEEN) bmp = bmp_b_queen;
                        if (pt == KING) bmp = bmp_b_king;
                    }
                    if (bmp) {
                        drawScaledBitmap(drawX, drawY, bmp, 2, 1);
                    }
                }
                
                if (r_off == 0 && c_off == 0) {
                    drawRect(drawX + 1, drawY + 1, magS - 2, magS - 2, 1);
                    drawRect(drawX + 2, drawY + 2, magS - 4, magS - 4, 1);
                }
            } else {
                drawLine(drawX, drawY, drawX + magS, drawY + magS, 1);
                drawLine(drawX + magS, drawY, drawX, drawY + magS, 1);
            }
        }
    }
}

unsigned long stateTimer = 0;

void loop() {
    unsigned long now = millis();
    
    if (state == STATE_INTRO) {
        display_clear();
        drawRect(0, 0, 128, 64, 1);
        drawRect(2, 2, 124, 60, 1);
        drawText(40, 20, "CHESS");
        
        if ((now / 500) % 2 == 0) {
            drawText(20, 45, "CLICK TO START");
        }
        display_render();
        
        if (input_action()) {
            tone(BUZZER_PIN, 800, 100);
            delay(200);
            resetGame();
        }
        return;
    }
    
    if (state == STATE_PLAYER_TURN) {
        int jx = input_x() - 2048;
        int jy = input_y() - 2048;
        static unsigned long lastMove = 0;
        if (now - lastMove > 150) {
            if (jy < -1000) { cursorY--; lastMove = now; tone(BUZZER_PIN, 400, 10); }
            if (jy > 1000) { cursorY++; lastMove = now; tone(BUZZER_PIN, 400, 10); }
            if (jx < -1000) { cursorX--; lastMove = now; tone(BUZZER_PIN, 400, 10); }
            if (jx > 1000) { cursorX++; lastMove = now; tone(BUZZER_PIN, 400, 10); }
            if (cursorX < 0) cursorX = 0; if (cursorX > 7) cursorX = 7;
            if (cursorY < 0) cursorY = 0; if (cursorY > 7) cursorY = 7;
        }
        
        if (input_action()) {
            int sq = cursorY * 8 + cursorX;
            if (selectedSq == -1) {
                if (board[sq] > 0) {
                    selectedSq = sq;
                    tone(BUZZER_PIN, 800, 30);
                }
            } else {
                if (sq == selectedSq) {
                    selectedSq = -1;
                    tone(BUZZER_PIN, 300, 30);
                } else if (board[sq] > 0) {
                    selectedSq = sq;
                    tone(BUZZER_PIN, 800, 30);
                } else {
                    Move moves[250];
                    int count = generateMoves(WHITE, moves);
                    if (count > 240) count = 240;
                    bool valid = false;
                    for(int i=0; i<count; i++) {
                        if (moves[i].from == selectedSq && moves[i].to == sq) {
                            int original = board[moves[i].from];
                            int captured = apply(moves[i]);
                            if (!inCheck(WHITE)) {
                                valid = true;
                                break;
                            } else {
                                unapply(moves[i], original, captured);
                            }
                        }
                    }
                    if (valid) {
                        tone(BUZZER_PIN, 1200, 50);
                        selectedSq = -1;
                        state = STATE_AI_TURN;
                        
                        Move aiMoves[250];
                        int aiCount = generateMoves(BLACK, aiMoves);
                        if (aiCount > 240) aiCount = 240;
                        bool aiHasMoves = false;
                        for(int j=0; j<aiCount; j++) {
                            int orig = board[aiMoves[j].from];
                            int cap = apply(aiMoves[j]);
                            if (!inCheck(BLACK)) aiHasMoves = true;
                            unapply(aiMoves[j], orig, cap);
                            if(aiHasMoves) break;
                        }
                        if (!aiHasMoves) {
                            state = STATE_GAMEOVER;
                            stateTimer = now;
                            tone(BUZZER_PIN, 200, 500);
                        }
                    } else {
                        selectedSq = -1;
                        tone(BUZZER_PIN, 300, 30);
                    }
                }
            }
        }
    }
    
    if (state == STATE_AI_TURN) {
        display_clear();
        drawBoardUI();
        drawText(70, 20, "AI");
        drawText(70, 35, "THINKING");
        display_render();
        
        Move best = getBestMove(2, BLACK);
        if (best.from == 0 && best.to == 0) {
            state = STATE_GAMEOVER;
            stateTimer = now;
        } else {
            apply(best);
            tone(BUZZER_PIN, 600, 50);
            state = STATE_PLAYER_TURN;
            
            Move playerMoves[250];
            int pCount = generateMoves(WHITE, playerMoves);
            if (pCount > 240) pCount = 240;
            bool pHasMoves = false;
            for(int j=0; j<pCount; j++) {
                int orig = board[playerMoves[j].from];
                int cap = apply(playerMoves[j]);
                if (!inCheck(WHITE)) pHasMoves = true;
                unapply(playerMoves[j], orig, cap);
                if (pHasMoves) break;
            }
            if (!pHasMoves) {
                state = STATE_GAMEOVER;
                stateTimer = now;
                tone(BUZZER_PIN, 200, 500);
            }
        }
        return; 
    }
    
    display_clear();
    drawBoardUI();
    
    if (state == STATE_GAMEOVER) {
        drawText(70, 20, "GAME");
        drawText(70, 30, "OVER");
        if ((now / 500) % 2 == 0) drawText(68, 50, "CLICK");
        if (input_action() && now - stateTimer > 1000) {
            tone(BUZZER_PIN, 800, 100); delay(200);
            resetGame();
        }
    } else if (state == STATE_PLAYER_TURN) {
        drawText(75, 10, "CHESS");
        drawText(75, 30, "YOUR");
        drawText(75, 45, "TURN");
    }
    
    display_render();
}

}
