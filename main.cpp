#include <SFML/Graphics.hpp>    
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;

#define GRID_SIZE_X 15
#define GRID_SIZE_Y 30

#define TILE_SIZE 30.f
#define TILE_TEXTURE_SIZE 10

#define TEXTURE_SIZE 4

#define BG_SIZE_X 2084
#define BG_SIZE_Y 1152

#define BG_COUNT 8

#define MUSIC_COUNT 11

enum Key {
    A, W, D,

    SIZE
};

Keyboard::Key keyboard_keys[Key::SIZE];
bool keyboard_pressed[Key::SIZE];
bool keyboard_hold[Key::SIZE];

void KeyboardInit() {
    fill(keyboard_pressed, keyboard_pressed + Key::SIZE, false);
    fill(keyboard_hold, keyboard_hold + Key::SIZE, false);

    keyboard_keys[0] = Keyboard::A;
    keyboard_keys[1] = Keyboard::W;
    keyboard_keys[2] = Keyboard::D;
}

void KeyboardUpdate() {
    for (int i = 0; i < Key::SIZE; i++) {
        if (Keyboard::isKeyPressed(keyboard_keys[i])) {
            if (keyboard_hold[i]) {
                keyboard_pressed[i] = false;
            } else {
                keyboard_pressed[i] = true;
                keyboard_hold[i] = true;
            }
        } else keyboard_hold[i] = false;
    }
}

struct Tile {
    Sprite sprite;
    bool occupied;
};

struct Block {
    static const int max_size = 4;

    Tile tiles[max_size][max_size];

    Vector2i size;
    Vector2i pos;

    void Init(string str) {
        int y = 0;
        int x;

        for (int i = 0; i < max_size; i++) {
            for (int j = 0; j < max_size; j++) {
                tiles[i][j].occupied = false;
            }
        }

        for (int i = 0; i < str.length(); i++) {
            x = i - y * (size.x + 1);

            if (str[i] == 'x') {
                tiles[x][y].occupied = true;
            } else if (str[i] == '/') {
                if (!y++) size.x = x;
            }
        }

        size.y = y;
    }

    void Rotate(bool right) {
        bool temp[size.x][size.y];

        for (int i = 0; i < size.x; i++) {
            for (int j = 0; j < size.y; j++) {
                temp[i][j] = tiles[i][j].occupied;
            }
        }

        swap(size.x, size.y);

        for (int i = 0; i < size.x; i++) {
            for (int j = 0; j < size.y; j++) {
                tiles[i][j].occupied = right ? temp[j][size.x - i - 1] : temp[size.y - j - 1][i];
            }
        }
    }

    enum Dir {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    const Vector2i dir_vals[4] = { { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 } };

    void Move(Dir dir) {
        pos.x += dir_vals[dir].x;
        pos.y += dir_vals[dir].y;
    }
};

void TetrisInit(Tile tetris[GRID_SIZE_X][GRID_SIZE_Y], Texture& texture) {
    for (int i = 0; i < GRID_SIZE_X; i++) {
        for (int j = 0; j < GRID_SIZE_Y; j++) {
            tetris[i][j].occupied = false;
            tetris[i][j].sprite.setTexture(texture);
            tetris[i][j].sprite.scale(TILE_SIZE / TILE_TEXTURE_SIZE, TILE_SIZE / TILE_TEXTURE_SIZE);
            tetris[i][j].sprite.setPosition(TILE_SIZE * i, TILE_SIZE * j);
        }
    }
}

void TetrisInitBlock(Block& block, Texture& texture) {
    for (int i = 0; i < Block::max_size; i++) {
        for (int j = 0; j < Block::max_size; j++) {
            block.tiles[i][j].sprite.setTexture(texture);
            block.tiles[i][j].sprite.scale(TILE_SIZE / TILE_TEXTURE_SIZE, TILE_SIZE / TILE_TEXTURE_SIZE);
        }
    }
}

void TetrisRespawnBlock(Block& block, string* blocks, int blocks_size) {
    srand(time(0));

    block.Init(blocks[rand() % blocks_size]);
    block.pos = Vector2i(GRID_SIZE_X / 2, 0);

    int rand_x = rand() % TEXTURE_SIZE * TILE_TEXTURE_SIZE;
    int rand_y = rand() % TEXTURE_SIZE * TILE_TEXTURE_SIZE;
    
    for (int i = 0; i < Block::max_size; i++) {
        for (int j = 0; j < Block::max_size; j++) {
            block.tiles[i][j].sprite.setTextureRect(IntRect(rand_x, rand_y, TILE_TEXTURE_SIZE, TILE_TEXTURE_SIZE));
        }
    }
}

bool TetrisBlockCollided(Tile tetris[GRID_SIZE_X][GRID_SIZE_Y], Block& block) {
    if (GRID_SIZE_Y <= block.pos.y + block.size.y - 1) return true;
    if (block.pos.x < 0) return true;
    if (block.pos.x + block.size.x - 1 >= GRID_SIZE_X) return true;


    for (int i = 0; i < block.size.x; i++) {
        for (int j = 0; j < block.size.y; j++) {
            if (tetris[block.pos.x + i][block.pos.y + j].occupied && block.tiles[i][j].occupied) return true;
        }
    }

    return false;
}

void TetrisMoveBlock(Tile tetris[GRID_SIZE_X][GRID_SIZE_Y], Block& block) {
    if (keyboard_pressed[Key::A] && block.pos.x > 0) {
        block.Move(Block::Dir::LEFT);

        if (TetrisBlockCollided(tetris, block)) {
            block.Move(Block::Dir::RIGHT);
        }
    }

    if (keyboard_pressed[Key::D] && block.pos.x + block.size.x - 1 < GRID_SIZE_X) {
        block.Move(Block::Dir::RIGHT);

        if (TetrisBlockCollided(tetris, block)) {
            block.Move(Block::Dir::LEFT);
        }
    }
}

void TetrisRotateBlock(Tile tetris[GRID_SIZE_X][GRID_SIZE_Y], Block& block) {
    if (keyboard_pressed[Key::W]) {
        block.Rotate(true);

        if (TetrisBlockCollided(tetris, block)) block.Rotate(false);
    }
}

void TetrisUpdateBlockTextures(Tile tetris[GRID_SIZE_X][GRID_SIZE_Y], Block& block) {
    for (int i = 0; i < block.size.x; i++) {
        for (int j = 0; j < block.size.y; j++) {
            if (block.tiles[i][j].occupied) block.tiles[i][j].sprite.setPosition(Vector2f((block.pos.x + i) * TILE_SIZE, (block.pos.y + j) * TILE_SIZE));
        }
    }
}

void TetrisFixBlock(Tile tetris[GRID_SIZE_X][GRID_SIZE_Y], Block& block) {
    for (int i = 0; i < block.size.x; i++) {
        for (int j = 0; j < block.size.y; j++) {
            if (block.tiles[i][j].occupied) {
                tetris[block.pos.x + i][block.pos.y + j].occupied = true;
                tetris[block.pos.x + i][block.pos.y + j].sprite.setTextureRect(block.tiles[i][j].sprite.getTextureRect());
            }
        }
    }
}

void TetrisCheckLines(Tile tetris[GRID_SIZE_X][GRID_SIZE_Y], Sound& tnt_sound) {
    int cleared = 0;

    for (int j = GRID_SIZE_Y - 1; j >= 0; j--) {
        int sum = 0;

        for (int i = 0; i < GRID_SIZE_X; i++) {
            sum += tetris[i][j].occupied;
        }

        if (sum == GRID_SIZE_X) {
            for (int i = 0; i < GRID_SIZE_X; i++) {
                tetris[i][j].occupied = false;
            }

            cleared++;
        } else {
            for (int i = 0; i < GRID_SIZE_X; i++) {
                tetris[i][j + cleared].occupied = tetris[i][j].occupied;
                tetris[i][j + cleared].sprite.setTextureRect(tetris[i][j].sprite.getTextureRect());
            }
        }
    }

    if (cleared) tnt_sound.play();
}

int main() {
    Vector2f window_size(GRID_SIZE_X * TILE_SIZE, GRID_SIZE_Y * TILE_SIZE);
    //float window_ratio = window_size.x / window_size.y;

    Event event;

    string effect_names[] = {
        "cow", "chicken", "pig", "sheep",
        "cow", "villager", "irongolem", "steve",
        "creeper", "skeleton", "zombie", "enderman",
        "spider", "blaze", "slime", "slime"
    };

    int effect_names_count = TEXTURE_SIZE * TEXTURE_SIZE;

    SoundBuffer effect_buffers[effect_names_count];

    for (int i = 0; i < effect_names_count; i++) {
        effect_buffers[i].loadFromFile("R/" + effect_names[i] + ".ogg");
    }

    Sound mob_sound;
    mob_sound.setVolume(70);

    SoundBuffer tnt_sound_buffer;
    tnt_sound_buffer.loadFromFile("R/tnt.ogg");

    Sound tnt_sound;
    tnt_sound.setBuffer(tnt_sound_buffer);
    tnt_sound.setVolume(70);

    Tile tetris[GRID_SIZE_X][GRID_SIZE_Y];
    string blocks[] = {
        "xxxx/",
        "xx./.xx/",
        ".xx/xx./",
        ".x./xxx/",
        "xx/xx/",
        "x../xxx/",
        "xxx/x../"
        //extra
        /*"xxx/.xx/",
        "x/",
        "x.x/xxx/.x./",
        "x../x../xxx/",
        ".x./.x./.x./xxx/"*/
    };

    int blocks_size = sizeof blocks / sizeof blocks[0];

    Texture texture;
    texture.loadFromFile("R/minecraft.png");

    Block block;

    KeyboardInit();

    TetrisInit(tetris, texture);

    TetrisInitBlock(block, texture);
    TetrisRespawnBlock(block, blocks, blocks_size);
    TetrisUpdateBlockTextures(tetris, block);

    float delay_tyme = .5f;
    float fastah_delay_tyme = .1f;
    float c_delay_tyme = 0;

    Texture bg_t[BG_COUNT];

    for (int i = 0; i < BG_COUNT; i++) {
        bg_t[i].loadFromFile("R/bg" + to_string(i) + ".png");
    }

    Sprite bg;
    bg.scale(Vector2f(window_size.x / (BG_SIZE_X * .5f), window_size.y / BG_SIZE_Y));
    bg.setTextureRect(IntRect(BG_SIZE_X * .21f, 0, BG_SIZE_X, BG_SIZE_Y));

    int cur_bg_t = 0;
    float bg_change_delay = .5f;
    float c_bg_change_delay = bg_change_delay;

    Music musics[MUSIC_COUNT];

    for (int i = 0; i < MUSIC_COUNT; i++) {
        musics[i].openFromFile("R/music" + to_string(i) + ".ogg");
    }

    float extra_music_change_delay = 3.f;
    float music_change_delay = extra_music_change_delay;
    float c_music_change_delay = 0;

    RenderWindow window(VideoMode(window_size.x, window_size.y), "Tetris", Style::Default & ~Style::Resize);
    View view(Vector2f(0, 0), Vector2f(500, 500));

    window.setFramerateLimit(60);

    Clock clock;

    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();

                break;
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Space)) continue;

        KeyboardUpdate();

        float delta = clock.restart().asSeconds();

        c_delay_tyme += delta;
        c_bg_change_delay += delta;
        c_music_change_delay += delta;

        if (c_music_change_delay >= music_change_delay) {
            c_music_change_delay = 0;
            srand(time(0));
            int ind = rand() % MUSIC_COUNT;
            musics[ind].play();
            music_change_delay = extra_music_change_delay + musics[ind].getDuration().asSeconds();
        }

        TetrisMoveBlock(tetris, block);
        TetrisRotateBlock(tetris, block);

        if (c_delay_tyme >= (Keyboard::isKeyPressed(Keyboard::S) ? fastah_delay_tyme : delay_tyme)) {
            c_delay_tyme = 0;

            block.Move(Block::Dir::DOWN);

            if (TetrisBlockCollided(tetris, block)) {
                block.Move(Block::Dir::UP);

                IntRect rect = block.tiles[0][0].sprite.getTextureRect();
                int x = rect.left / TILE_TEXTURE_SIZE;
                int y = rect.top / TILE_TEXTURE_SIZE;
                mob_sound.setBuffer(effect_buffers[y * TEXTURE_SIZE + x]);
                mob_sound.play();

                TetrisFixBlock(tetris, block);
                TetrisCheckLines(tetris, tnt_sound);
                TetrisRespawnBlock(block, blocks, blocks_size);

                if (TetrisBlockCollided(tetris, block)) {
                    window.close();
                }
            }
        }

        TetrisUpdateBlockTextures(tetris, block);

        if (c_bg_change_delay >= bg_change_delay) {
            c_bg_change_delay = 0;
            bg.setTexture(bg_t[cur_bg_t]);
            cur_bg_t = (cur_bg_t + 1) % BG_COUNT;
        }

        window.clear();

        window.draw(bg);

        for (int i = 0; i < GRID_SIZE_X; i++) {
            for (int j = 0; j < GRID_SIZE_Y; j++) {
                if (tetris[i][j].occupied) window.draw(tetris[i][j].sprite);
            }
        }

        for (int i = 0; i < block.size.x; i++) {
            for (int j = 0; j < block.size.y; j++) {
                if (block.tiles[i][j].occupied) window.draw(block.tiles[i][j].sprite);
            }
        }

        window.display();
    }

    return 0;
}