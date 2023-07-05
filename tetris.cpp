#include <SDL.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <optional>
#include <queue>
#include <random>
#include <vector>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TILE_WIDTH = 10;
const int TILE_HEIGHT = 10;
const int GRAVITY_DURATION_SECONDS = 1;

enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Color
{
    int red = 0;
    int green = 0;
    int blue = 0;
    int alpha = 255;
};
const Color BLACK{ 0,0,0,255 };
const Color RED{ 255,0,0,255 };
const Color GREEN{ 0,255,0,255 };
const Color BLUE{ 0,0,255,255 };

const Color DEFAULT_COLORS[]
{
    RED,
    GREEN,
    BLUE
};

void drawTile(SDL_Renderer* renderer, Color color, int row, int col);

class Board;

class Piece
{
public:
    static constexpr int NUM_DEFAULT_PIECES = 5;
    static constexpr int MAX_HEIGHT = 4;
    static constexpr int MAX_WIDTH = 4;

    using TileGridType = std::array<std::array<bool, MAX_WIDTH>, MAX_HEIGHT>;
    static constexpr std::array<TileGridType, NUM_DEFAULT_PIECES> DEFAULT_PIECES
    { {
        {{
            {1,1,1,1},
            {0,0,0,0},
            {0,0,0,0},
            {0,0,0,0}
        }},
        {{
            {0,1,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        }},
        {{
            {1,1,0,0},
            {1,0,0,0},
            {1,0,0,0},
            {0,0,0,0}
        }},
        {{
            {1,1,0,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        }},
        {{
            {1,1,0,0},
            {1,1,0,0},
            {0,0,0,0},
            {0,0,0,0}
        }}
    }};

    TileGridType tileGrid = { false };
    Color color = RED;
    int row = 0;
    int col = 0;

    Piece();
    void rotate(Board& board);
    bool move(Direction direction, Board& board);
    bool moveTo(int newRow, int newCol, TileGridType& newTileGrid, Board& board);
    void draw(SDL_Renderer* renderer, std::optional<Color> color);
    void removeTile(SDL_Renderer* renderer, Color backgroundColor, int rowToRemove, int colToRemove);

private:
};

class Board
{
public:
    static constexpr int NUM_ROWS = 20;
    static constexpr int NUM_COLS = 12;
    static constexpr int START_X_PIXELS = SCREEN_WIDTH / 6;
    static constexpr int START_Y_PIXELS = SCREEN_HEIGHT / 6;
    static constexpr int BOARD_WIDTH_PIXELS = NUM_COLS * TILE_WIDTH;
    static constexpr int BOARD_HEIGHT_PIXELS = NUM_ROWS * TILE_HEIGHT;

    std::array<std::array<Piece*, NUM_COLS>, NUM_ROWS> pieceGrid = { nullptr };
    std::vector<Piece> pieces;
    Piece* activePiece = nullptr;
    int rowsCompleted = 0;

    Board(SDL_Renderer* renderer);
    void draw();
    bool isRowFull(int row);
    void collapseFullRows();
    bool update(Direction direction);
private:
    SDL_Renderer* renderer;

    void drawBorders();
};

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL init error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cout << "SDL create window error: " << SDL_GetError() << std::endl;
        return -2;
    }

    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cout << "Error creating Renderer: " << SDL_GetError() << std::endl;
        return -3;
    }

    Board board(renderer);
    board.draw();

    // TODO levels of difficulty
    auto gravityDeadline = std::chrono::steady_clock::now();

    bool quit = false;
    bool updateSuccess = true;
    while(!quit)
    {
        auto now = std::chrono::steady_clock::now();
        if(now > gravityDeadline)
        {
            gravityDeadline = now + std::chrono::seconds(GRAVITY_DURATION_SECONDS);
            board.update(Direction::DOWN);
        }

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_UP:
                    updateSuccess = board.update(Direction::UP);
                    break;
                case SDLK_DOWN:
                    updateSuccess = board.update(Direction::DOWN);
                    break;
                case SDLK_LEFT:
                    updateSuccess = board.update(Direction::LEFT);
                    break;
                case SDLK_RIGHT:
                    updateSuccess = board.update(Direction::RIGHT);
                    break;
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }

            if(!updateSuccess)
            {
                quit = true;
            }
        }

        if (!updateSuccess)
        {
            std::cout << "Game Over!" << std::endl;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void drawTile(SDL_Renderer* renderer, Color color, int row, int col)
{
    SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, color.alpha);

    SDL_Rect rect;
    rect.x = Board::START_X_PIXELS + col * TILE_WIDTH;
    rect.y = Board::START_Y_PIXELS + row * TILE_HEIGHT;
    rect.w = TILE_WIDTH;
    rect.h = TILE_HEIGHT;

    SDL_RenderFillRect(renderer, &rect);
}

bool Piece::moveTo(int newRow, int newCol, TileGridType& newTileGrid, Board& board)
{
    for (int subRow = 0; subRow < MAX_HEIGHT; subRow++)
    {
        for (int subCol = 0; subCol < MAX_WIDTH; subCol++)
        {
            if (!newTileGrid[subRow][subCol])
            {
                // not an actual part of the piece so ignore
                continue;
            }

            // where this tile lands of the full board
            int absoluteRow = newRow + subRow;
            int absoluteCol = newCol + subCol;

            if (absoluteRow < 0 || absoluteRow >= Board::NUM_ROWS
                || absoluteCol < 0 || absoluteCol >= Board::NUM_COLS)
            {
                // piece lands out of bounds of the full board
                return false;
            }

            if (board.pieceGrid[absoluteRow][absoluteCol] != nullptr
                && board.pieceGrid[absoluteRow][absoluteCol] != this)
            {
                // this spot of the full board is already taken
                return false;
            }
        }
    }

    for (int subRow = 0; subRow < MAX_HEIGHT; subRow++)
    {
        for (int subCol = 0; subCol < MAX_WIDTH; subCol++)
        {
            if (tileGrid[subRow][subCol])
            {
                int r = row + subRow;
                int c = col + subCol;
                board.pieceGrid[r][c] = nullptr;
            }
        }
    }

    for (int subRow = 0; subRow < MAX_HEIGHT; subRow++)
    {
        for (int subCol = 0; subCol < MAX_WIDTH; subCol++)
        {
            if (newTileGrid[subRow][subCol])
            {
                int r = newRow + subRow;
                int c = newCol + subCol;
                board.pieceGrid[r][c] = this;
            }
        }
    }

    row = newRow;
    col = newCol;

    return true;
}

Piece::Piece()
{
    static std::random_device rd;
    tileGrid = DEFAULT_PIECES[rd() % NUM_DEFAULT_PIECES];
    static int colorIndex = 0;
    color = DEFAULT_COLORS[colorIndex];
    colorIndex = (colorIndex + 1) % (sizeof(DEFAULT_COLORS) / sizeof(Color));
}

void Piece::rotate(Board& board)
{
    TileGridType newTileGrid = { false };

    for (int sourceRow = 0; sourceRow < MAX_HEIGHT; sourceRow++)
    {
        for (int sourceCol = 0; sourceCol < MAX_WIDTH; sourceCol++)
        {
            int destRow = sourceCol;
            int destCol = MAX_HEIGHT - sourceRow - 1;
            newTileGrid[destRow][destCol] = tileGrid[sourceRow][sourceCol];
        }
    }

    if (moveTo(row, col, newTileGrid, board))
    {
        tileGrid = newTileGrid;
    }
}

bool Piece::move(Direction direction, Board& board)
{
    int newRow = row + (direction == Direction::DOWN) - (direction == Direction::UP);
    int newCol = col + (direction == Direction::RIGHT) - (direction == Direction::LEFT);
    return !moveTo(newRow, newCol, tileGrid, board) && direction == Direction::DOWN;
}

void Piece::draw(SDL_Renderer* renderer, std::optional<Color> color)
{
    for (int subRow = 0; subRow < MAX_HEIGHT; subRow++)
    {
        for (int subCol = 0; subCol < MAX_WIDTH; subCol++)
        {
            if (tileGrid[subRow][subCol])
            {
                drawTile(renderer, color ? *color : this->color, row + subRow, col + subCol);
            }
        }
    }
}

void Piece::removeTile(SDL_Renderer* renderer, Color backgroundColor, int rowToRemove, int colToRemove)
{
    int subRow = rowToRemove - row;
    int subCol = colToRemove - col;
    if (!tileGrid[subRow][subCol])
    {
        return;
    }

    drawTile(renderer, backgroundColor, rowToRemove, colToRemove);
    tileGrid[subRow][subCol] = false;
}

Board::Board(SDL_Renderer* renderer) : renderer(renderer)
{
    pieces.reserve(NUM_ROWS * NUM_COLS);
    activePiece = &pieces.emplace_back();
}

void Board::draw()
{
    drawBorders();
    for (Piece& piece : pieces)
    {
        piece.draw(renderer, std::nullopt);
    }
    SDL_RenderPresent(renderer);
}

bool Board::isRowFull(int row)
{
    for (int col = 0; col < NUM_COLS; col++)
    {
        if (pieceGrid[row][col] == nullptr)
        {
            return false;
        }
    }

    rowsCompleted++;
    return true;
}

void Board::collapseFullRows()
{
    for (int row = 0; row < NUM_ROWS; row++)
    {
        if (!isRowFull(row))
        {
            continue;
        }

        for (int col = 0; col < NUM_COLS; col++)
        {
            pieceGrid[row][col]->removeTile(renderer, BLACK, row, col);
            pieceGrid[row][col] = nullptr;
        }

        // erase pieces that have no tiles left of the board
        pieces.erase(
            std::remove_if(
                pieces.begin(),
                pieces.end(),
                [](Piece& piece)
                {
                    for (int row = 0; row < Piece::MAX_HEIGHT; row++)
                    {
                        for (int col = 0; col < Piece::MAX_WIDTH; col++)
                        {
                            if (piece.tileGrid[row][col])
                            {
                                return false;
                            }
                        }
                    }

                    return true;
                }),
            pieces.end());

        // re-write pieceGrid since addresses will change if any piece were erased
        pieceGrid = { nullptr };
        for (Piece& piece : pieces)
        {
            piece.moveTo(piece.row, piece.col, piece.tileGrid, *this);
        }


        for (Piece& piece : pieces)
        {
            piece.draw(renderer, BLACK /* background color */);
            while (!piece.move(Direction::DOWN, *this)) {}
            piece.draw(renderer, std::nullopt);
        }

        // TODO piece breaks into 2 pieces (rare)
    }
}

bool Board::update(Direction direction)
{
    bool result = true;

    // TODO move draw function inside Piece::move
    activePiece->draw(renderer, BLACK /* background color */);

    bool shouldSpawn = false;
    if (direction == Direction::UP)
    {
        activePiece->rotate(*this);
    }
    else
    {
        shouldSpawn = activePiece->move(direction, *this);
    }
    activePiece->draw(renderer, std::nullopt);

    if (shouldSpawn)
    {
        collapseFullRows();
        activePiece = &pieces.emplace_back();

        if (!activePiece->moveTo(activePiece->row, activePiece->col, activePiece->tileGrid, *this))
        {
            std::cout << "Rows Completed: " << rowsCompleted << std::endl;
            pieces.pop_back();
            result = false;
        }
    }

    SDL_RenderPresent(renderer);
    return result;
}

void Board::drawBorders()
{
    SDL_Rect outlineRect = { START_X_PIXELS - 1, START_Y_PIXELS - 1, BOARD_WIDTH_PIXELS + 2, BOARD_HEIGHT_PIXELS + 2 };
    SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
    SDL_RenderDrawRect(renderer, &outlineRect);
}