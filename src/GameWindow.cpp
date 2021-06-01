#include "GameWindow.h"

GameWindow::GameWindow(bool status, int32_t width, int32_t height) : m_Status(status), m_Width(width), m_Height(height),
                                                                     m_VertexBufferID(0) {
    setupGL();
    setupImageBuffers();
    setupArena();

    // enemyInstances.reserve(enemyCount);
    this->m_QuadTree = std::make_unique<t_quadTree>(f_quadBox(0, 0, (float) width, (float) height), GetQuadBox);

    // Configurations
    this->m_EnemyConfig = std::make_unique<EnemyConfig>();

    if (m_ObservationType == Image) {
        m_ImageBuffer.set_capacity(IMAGE_OBS_COUNT);
    }

    generate();
}

GameWindow::~GameWindow() {
    m_Tiles.clear();
    m_EnemyInstances.clear();
    m_BulletInstances.clear();

    glDeleteBuffers(1, &m_VertexBufferID);
    glDeleteTextures(1, &m_TextureBufferID);
    glDeleteTextures(1, &m_TextureBulletID);
    glDeleteTextures(1, &m_TextureEnemyID);
}

void GameWindow::play(GLFWwindow *window) {
    double lastTime = glfwGetTime();
    double deltaTime = 0.0f;
    double currentTime = 0.0f;
    int64_t fps = 0;
    int32_t episode_count = 0;

    while (getGameStatus()) {
        render(window);

        deltaTime += (glfwGetTime() - lastTime) * UPDATES_PER_SECOND;
        lastTime = glfwGetTime();
        ++fps;

        while (deltaTime >= DELTA_TIME_THRESHOLD) {
            update(window);
            --deltaTime;

            if (INFO_LOG) {
                spdlog::info("FPS : {}", fps);
            }

            fps = 0;
        }

        updateGameStatus(window);

        if (m_Player->getHealth() <= 0) {
            restart(window);

            if (episode_count == 2) {
                break;
            }
//            episode_count++;
        }
    }

    close(window);
}

void GameWindow::restart(GLFWwindow *window) {
    spdlog::info("Restarting the Game...");

    // Empty all cached object data
    m_BulletInstances.clear();
    m_EnemyInstances.clear();

    if (m_MapRegeneration) {
        m_Tiles.clear();
        m_EmptyTiles.clear();
        m_Grid.clear();

        // Reset Maze
        setupArena();
    }

    // Re-generate everything
    generate();

    m_EnemyConfig->chaserCount = 3;
    m_EnemyConfig->sniperCount = 3;
}

void GameWindow::setupGL() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, m_Width, m_Height); //Set Size

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, m_Width, 0, m_Height); //Origin
    glMatrixMode(GL_MODELVIEW);

    glGenBuffers(1, &m_VertexBufferID);

    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexData), (GLvoid *) offsetof(VertexData, positionCoordinates));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData), (GLvoid *) offsetof(VertexData, textureCoordinates));

    m_MaxTileCount = static_cast<int32_t>(((float) (m_Width * m_Height) / (SQUARE_SIZE * SQUARE_SIZE)) * m_TilePercent);
}

void GameWindow::setupImageBuffers() {
    m_TextureBufferID = loadAndBufferImage("files/player.png", 100, 100);
    m_TextureBulletID = loadAndBufferImage("files/bullet_2.png", 100, 100);
    m_TextureEnemyID = loadAndBufferImage("files/boss.png", 100, 100);
    m_TextureTileID = loadAndBufferImage("files/tile.png", 100, 100);
    m_TextureTpID = loadAndBufferImage("files/blue_tile_2.png", 100, 100);
}

void GameWindow::setupArena() {
    m_GameArena = std::make_unique<Area>(0.0f, m_Height, 0.0f, m_Width);
    m_Maze = std::make_unique<Maze>(static_cast<int32_t>((float) m_Height / SQUARE_SIZE),
                                    static_cast<int32_t>((float) m_Width / SQUARE_SIZE));

    //Init Grid for PathFinding
    for (int32_t row = 0; row < m_Maze->m_Height + 1; ++row) {
        std::vector<std::shared_ptr<Point>> inner_vec;
        for (int32_t col = 0; col < m_Maze->m_Width + 1; ++col) {
            inner_vec.push_back(std::make_shared<Point>(Vector2<int32_t>(row, col)));
        }
        m_Grid.push_back(inner_vec);
    }
}

uint32_t GameWindow::loadAndBufferImage(const char *filename, int32_t width, int32_t height) {
    uint32_t textureBufferID;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_COLOR);
    glGenTextures(1, &textureBufferID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBufferID);

    u_char *image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGBA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    SOIL_free_image_data(image);

    return textureBufferID;
}

void GameWindow::generate() {
    if (m_MapRegeneration) {
        spdlog::info("Generating A New Map...");
        generateRandomMap();

        spdlog::info("Generating Teleportation Cells...");
        generateExitPoints();

        spdlog::info("Generation is completed!");
        m_MapRegeneration = REGENERATE_MAP;
    }

    spdlog::info("Initializing The Player..");
    initPlayer();

    spdlog::info("End of Generation");
}

void GameWindow::generateRandomMap() {
    std::stack<Vector2<int32_t>> stack;

    int32_t min_row = 1;
    int32_t min_col = 0;

    auto max_row = static_cast<int32_t>((float) m_Width / SQUARE_SIZE);
    auto max_col = static_cast<int32_t>((float) m_Height / SQUARE_SIZE) - 1;

    //First Random Tile
    Vector2<int32_t> firstRandomPos((int32_t) arc4random() % max_row + min_row,
                                    (int32_t) arc4random() % max_col + min_col);

    while (!isPositionValid(firstRandomPos, min_row, max_row, min_col, max_col)) {
        firstRandomPos.x = (int32_t) arc4random() % max_row + min_row;
        firstRandomPos.y = (int32_t) arc4random() % max_col + min_col;
    }

    //DFS Based Procedural Generation
    stack.push(firstRandomPos);
    m_EmptyTiles.push_back(firstRandomPos);

    //DFS GENERATION
    while (m_EmptyTiles.size() <= (size_t) m_MaxTileCount) {
        Vector2<int32_t> curr = stack.top();

        //Select Random Point32_t Around
        std::vector<Vector2<int32_t>> neighbour_tiles;
        for (int32_t n_r = -1; n_r <= 1; ++n_r) {
            for (int32_t n_c = -1; n_c <= 1; ++n_c) {
                if ((n_r != 0 || n_c != 0) && (std::abs(n_r) != std::abs(n_c))) {
                    Vector2<int32_t> neighbour_tile(curr.x + n_r, curr.y + n_c);
                    if (isPositionValid(neighbour_tile, min_row, max_row, min_col, max_col)) {
                        neighbour_tiles.push_back(neighbour_tile);
                    }
                }
            }
        }

        if (!neighbour_tiles.empty()) {
            stack.push(neighbour_tiles.at(arc4random() % neighbour_tiles.size()));
            m_EmptyTiles.push_back(stack.top());
            m_Maze->m_Matrix[stack.top().x][stack.top().y] = Empty;
        } else {
            stack.pop();
        }

        neighbour_tiles.clear();
    }
}

void GameWindow::generateExitPoints() {
    std::vector<Vector2<int32_t>> exit_tiles;
    std::vector<Vector2<int32_t>> close_tiles;

    int32_t min_row = 1;
    int32_t min_col = 0;

    auto max_row = static_cast<int32_t>((float) m_Width / SQUARE_SIZE);
    auto max_col = static_cast<int32_t>((float) m_Height / SQUARE_SIZE) - 1;

    std::random_device rd;                                    // obtain a random number from hardware
    std::mt19937 gen(rd());                                   // seed the generator
    std::uniform_int_distribution<> dist_col(1, max_col - 2); // define the range
    std::uniform_int_distribution<> dist_row(1, max_row - 1); // define the range

    // Random Teleport Tile Selection
    auto vertical_teleport_pos = static_cast<int32_t>(dist_col(gen));
    auto horizontal_teleport_pos = static_cast<int32_t>(dist_row(gen));

    Vector2<int32_t> up_exit_tile(min_row, vertical_teleport_pos);
    Vector2<int32_t> down_exit_tile(max_row, vertical_teleport_pos);

    Vector2<int32_t> left_exit_tile(horizontal_teleport_pos, min_col);
    Vector2<int32_t> right_exit_tile(horizontal_teleport_pos, max_col);

    exit_tiles.push_back(up_exit_tile);
    exit_tiles.push_back(down_exit_tile);
    exit_tiles.push_back(right_exit_tile);
    exit_tiles.push_back(left_exit_tile);

    // Find closest tiles to teleport points
    for (auto tile : exit_tiles) {
        std::vector<float> distances;

        // In-efficient asF
        std::for_each(m_EmptyTiles.begin(), m_EmptyTiles.end(),
                      [&](const Vector2<int32_t> &selected_tile) {
                          distances.push_back(utility::dist_euc<int32_t>(tile, selected_tile));
                      });
        close_tiles.push_back(m_EmptyTiles[std::min_element(distances.begin(), distances.end()) - distances.begin()]);
    }

    // Create a way to teleport starting from closest point
    for (size_t close_tile_idx = 0; close_tile_idx < close_tiles.size(); ++close_tile_idx) {
        int32_t row_difference = exit_tiles[close_tile_idx].x - close_tiles[close_tile_idx].x;
        int32_t col_difference = exit_tiles[close_tile_idx].y - close_tiles[close_tile_idx].y;

        if ((size_t) close_tile_idx >= (exit_tiles.size() / 2)) {
            // Left-Right Teleport Points
            if (std::abs(col_difference) > 0) {
                for (auto step = 1; step <= std::abs(col_difference); ++step) {
                    (col_difference > 0) ? m_Maze->m_Matrix[exit_tiles[close_tile_idx].x][exit_tiles[close_tile_idx].y -
                                                                                          step] = Empty
                                         : m_Maze->m_Matrix[exit_tiles[close_tile_idx].x][exit_tiles[close_tile_idx].y +
                                                                                          step] = Empty;
                }
            }

            if (std::abs(row_difference) > 0) {
                for (auto step = 1; step <= std::abs(row_difference); ++step) {
                    (row_difference > 0) ? m_Maze->m_Matrix[close_tiles[close_tile_idx].x +
                                                            step][close_tiles[close_tile_idx].y] = Empty
                                         : m_Maze->m_Matrix[close_tiles[close_tile_idx].x -
                                                            step][close_tiles[close_tile_idx].y] = Empty;
                }
            }
        } else {
            // Top-Bottom Teleport Points
            if (std::abs(row_difference) > 0) {
                for (auto step = 1; step <= std::abs(row_difference); ++step) {
                    (row_difference > 0) ? m_Maze->m_Matrix[exit_tiles[close_tile_idx].x -
                                                            step][exit_tiles[close_tile_idx].y] = Empty
                                         : m_Maze->m_Matrix[exit_tiles[close_tile_idx].x +
                                                            step][exit_tiles[close_tile_idx].y] = Empty;
                }
            }

            if (std::abs(col_difference) > 0) {
                for (auto step = 1; step <= std::abs(col_difference); ++step) {
                    (col_difference > 0) ? m_Maze->m_Matrix[close_tiles[close_tile_idx].x][
                                                   close_tiles[close_tile_idx].y + step] = Empty
                                         : m_Maze->m_Matrix[close_tiles[close_tile_idx].x][
                                                   close_tiles[close_tile_idx].y - step] = Empty;
                }
            }
        }
    }

    // Empty 2 tiles to be able to jump
    m_Maze->m_Matrix[up_exit_tile.x + 2][up_exit_tile.y] = Empty;
    m_Maze->m_Matrix[up_exit_tile.x + 1][up_exit_tile.y] = Empty;
    m_Maze->m_Matrix[down_exit_tile.x - 2][down_exit_tile.y] = Empty;
    m_Maze->m_Matrix[down_exit_tile.x - 1][down_exit_tile.y] = Empty;
    m_Maze->m_Matrix[left_exit_tile.x][left_exit_tile.y + 1] = Empty;
    m_Maze->m_Matrix[left_exit_tile.x][left_exit_tile.y + 2] = Empty;
    m_Maze->m_Matrix[right_exit_tile.x][right_exit_tile.y - 1] = Empty;
    m_Maze->m_Matrix[right_exit_tile.x][right_exit_tile.y - 2] = Empty;

    /* Setup Exit Points */
    m_Maze->m_Matrix[up_exit_tile.x][up_exit_tile.y] = Teleport;
    m_Maze->m_Matrix[up_exit_tile.x - 1][up_exit_tile.y] = Teleport;
    m_Maze->m_Matrix[down_exit_tile.x][down_exit_tile.y] = Teleport;
    m_Maze->m_Matrix[left_exit_tile.x][left_exit_tile.y] = Teleport;
    m_Maze->m_Matrix[right_exit_tile.x][right_exit_tile.y] = Teleport;
    m_Maze->m_Matrix[right_exit_tile.x][right_exit_tile.y + 1] = Teleport;

    // Last
    for (int32_t row = 0; row < static_cast<int32_t>((float) m_Height / SQUARE_SIZE) + 1; ++row) {
        for (int32_t col = 0; col < static_cast<int32_t>((float) m_Width / SQUARE_SIZE) + 1; ++col) {
            if (m_Maze->m_Matrix[row][col] == Wall) {
                std::unique_ptr<Sprite> tile = std::make_unique<Sprite>(m_TextureTileID,
                                                                        Vector2<float>(SQUARE_SIZE * (float) col,
                                                                                       (float) m_Height -
                                                                                       SQUARE_SIZE * (float) row));
                m_Tiles.push_back(std::move(tile));
            } else if (m_Maze->m_Matrix[row][col] == Teleport) {
                std::unique_ptr<Sprite> teleport_tile = std::make_unique<Sprite>(m_TextureTpID,
                                                                                 Vector2<float>(
                                                                                         SQUARE_SIZE * (float) col,
                                                                                         (float) m_Height -
                                                                                         SQUARE_SIZE * (float) row));
                m_Tiles.push_back(std::move(teleport_tile));
            }
        }
    }
}

void GameWindow::initPlayer() {
    //Get Random Point
    Vector2<int32_t> randomPosition = m_EmptyTiles[arc4random() % m_EmptyTiles.size()];
    if (m_Player == nullptr) {
        // Create Player Object
        m_Player = std::make_unique<Player>(m_TextureBufferID, Vector2<float>((float) randomPosition.y * SQUARE_SIZE,
                                                                              (float) m_Width -
                                                                              (float) randomPosition.x * SQUARE_SIZE),
                                            AI_AGENT, m_ObservationType);
        m_Player->setVelocity(Vector2<float>(PLAYER_SPEED, PLAYER_SPEED));
        m_Player->setOffset(OBJECT_OFFSET);
    } else {
        m_Player->reset();
    }
    // Initialize Player
    m_Player->setPosition(Vector2<float>((float) randomPosition.y * SQUARE_SIZE,
                                         (float) m_Width - (float) randomPosition.x * SQUARE_SIZE));
    m_Player->setArea(m_GameArena->Copy());
    m_Player->setMaze(m_Maze->Copy());
}

bool GameWindow::isPositionValid(Vector2<int32_t> &position, int32_t min_row, int32_t max_row, int32_t min_col,
                                 int32_t max_col) {
    return (position.x > min_row) && (position.x < max_row) &&
           (position.y > min_col) && (position.y < max_col) &&
           m_Maze->m_Matrix[position.x][position.y] == Wall;
}

void GameWindow::firePlayerBullet() {
    Vector2<float> bulletPosition =
            m_Player->getPosition() + Vector2<float>(std::cos(m_Player->getRotation()) * (SQUARE_SIZE / 2),
                                                     std::sin(m_Player->getRotation()) * (SQUARE_SIZE / 2));

    std::unique_ptr<Projectile> bullet = std::make_unique<Projectile>(m_TextureBulletID, bulletPosition, PlayerBullet);

    Vector2<float> bullet_relative_speed(std::cos(utility::angle2Rad(m_Player->getRotation())) * BULLET_SPEED,
                                         std::sin(utility::angle2Rad(m_Player->getRotation())) * BULLET_SPEED);

    bullet->setVelocity(bullet_relative_speed);
    bullet->setRotation(m_Player->getRotation());
    bullet->setArea(m_GameArena->Copy());
    bullet->setMaze(m_Maze->Copy());
    bullet->setOffset(OBJECT_OFFSET);

    m_BulletInstances.push_back(std::move(bullet));
}

void GameWindow::fireEnemyBullet(const Vector2<float> &enemyPosition, const Vector2<float> &direction) {
    std::unique_ptr<Projectile> enemyBullet = std::make_unique<Projectile>(m_TextureBulletID,
                                                                           enemyPosition + direction * 10.0f,
                                                                           EnemyBullet);
    Vector2<float> bullet_relative_speed = direction * 4.0f;

    enemyBullet->setVelocity(bullet_relative_speed);
    enemyBullet->setRotation(m_Player->getRotation());
    enemyBullet->setArea(m_GameArena->Copy());
    enemyBullet->setMaze(m_Maze->Copy());
    enemyBullet->setOffset(OBJECT_OFFSET);

    enemyBullet->setRotation(direction.angle());

    m_EnemyBulletInstances.push_back(std::move(enemyBullet));
}

void GameWindow::handleSpawns(GLFWwindow *window) {
    if (m_Player->getFireStatus() &&
        m_Player->m_BulletCount > 0 &&
        m_FireTicks >= PLAYER_FIRE_TICKS) {

        firePlayerBullet();
        m_FireTicks = 0;
        --m_Player->m_BulletCount;
    }

    m_Player->setFireStatus(false); // Reset player fire order

    if (m_SpawnTicks >= ENEMY_SPAWN_TICKS) {
        spawnEnemies();
        m_SpawnTicks = 0;
    }

    ++m_FireTicks;
    ++m_SpawnTicks;
}

void GameWindow::spawnEnemies() {
    if (m_EnemyInstances.size() < m_EnemyConfig->enemyCount) {
        Vector2<int32_t> playerTile = m_Maze->pos2MtrCoord(m_Player->getPosition());

        //Get Random Point
        std::vector<Vector2<int32_t>> spanwableTiles;

        std::for_each(m_EmptyTiles.begin(), m_EmptyTiles.end(),
                      [&](const Vector2<int32_t> &selected_tile) {
                          if (selected_tile != playerTile) {
                              float dist = utility::dist_euc<int32_t>(playerTile, selected_tile);
                              if (dist < (float) m_EnemyConfig->enemySpawnDistance) {
                                  spanwableTiles.push_back(selected_tile);
                              }
                          }
                      });

        Vector2<int32_t> randomPosition = spanwableTiles[arc4random() % spanwableTiles.size()];

        // Decide to which enemy type is going to be generated
        EnemyType targetEnemyType = Chaser;

        if (m_EnemyConfig->sniperCount > 0) {
            --m_EnemyConfig->sniperCount;
            targetEnemyType = Sniper;
        } else if (m_EnemyConfig->suicideBomberCount > 0) {
            --m_EnemyConfig->suicideBomberCount;
            targetEnemyType = SuicideBomber;
        }

        // spdlog::info("Spawned Enemy Type : {}", targetEnemyType);

        // Spawn
        std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(m_TextureEnemyID,
                                                               Vector2<float>((float) randomPosition.y * SQUARE_SIZE,
                                                                              (float) m_Width -
                                                                              (float) randomPosition.x * SQUARE_SIZE),
                                                               targetEnemyType);

        enemy->setArea(m_GameArena->Copy());
        enemy->setMaze(m_Maze->Copy());
        enemy->setOffset(OBJECT_OFFSET);

        m_EnemyInstances.push_back(std::move(enemy));

        m_EnemySpawnDistance = (uint32_t) std::max((float) (m_EnemySpawnDistance * 0.90), MIN_ENEMY_SPAWN_DISTANCE);
        m_EnemyCount = std::min(m_EnemyCount + 1, MAX_ENEMY_COUNT);
    }
}

bool GameWindow::checkStaticWallCollision(const Vector2<float> &position) {
    // Get Intersecting Tiles
    TileType bottomLeftTile = this->m_Maze->pos2Tile(position);

    if (bottomLeftTile == Wall || bottomLeftTile == Obstacle) {
        return true;
    }

    TileType bottomRightTile = this->m_Maze->pos2Tile(Vector2<float>(position.y, position.x + SQUARE_SIZE));

    if (bottomRightTile == Wall || bottomRightTile == Obstacle) {
        return true;
    }

    TileType topLeftTile = this->m_Maze->pos2Tile(Vector2<float>(position.y + SQUARE_SIZE, position.x));

    if (topLeftTile == Wall || topLeftTile == Obstacle) {
        return true;
    }

    TileType topRightTile = this->m_Maze->pos2Tile(Vector2<float>(position.y + SQUARE_SIZE, position.x + SQUARE_SIZE));

    if (topRightTile == Wall || topRightTile == Obstacle) {
        return true;
    }

    return false;
}

bool GameWindow::checkObjectCollision(const std::unique_ptr<Area> &objectX,
                                      const std::unique_ptr<Area> &objectY) {
    //Intersection of Two Rectangle
    bool res = !(objectX->m_Box->left >= objectY->m_Box->right ||
                 objectX->m_Box->right <= objectY->m_Box->left ||
                 objectX->m_Box->top <= objectY->m_Box->bottom ||
                 objectX->m_Box->bottom >= objectY->m_Box->top);

    return res;
}

bool GameWindow::checkObjectCollision(const std::shared_ptr<Area> &objectX, const std::shared_ptr<Area> &objectY) {
    //Intersection of Two Rectangle
    bool res = !(objectX->m_Box->left >= objectY->m_Box->right ||
                 objectX->m_Box->right <= objectY->m_Box->left ||
                 objectX->m_Box->top <= objectY->m_Box->bottom ||
                 objectX->m_Box->bottom >= objectY->m_Box->top);

    //Avoid Memory Leak
    // free(objectX->Box);
    // free(objectY->Box);
    return res;
}

void GameWindow::handleCollisions() {
    // COLLISIONS

    // Delete Bullets if they pass the borders or hit the any tile
    std::vector<uint32_t> bulletsToDestroy;

    for (uint32_t bulletIdx = 0; bulletIdx < m_BulletInstances.size(); ++bulletIdx) {
        Vector2<float> bulletPosition = m_BulletInstances[bulletIdx]->getPosition();

        if (bulletPosition.x > ((float) m_Width + SQUARE_SIZE) || bulletPosition.y > ((float) m_Height + SQUARE_SIZE) ||
            bulletPosition.x < (-SQUARE_SIZE) || bulletPosition.y < (-SQUARE_SIZE) ||
            m_BulletInstances[bulletIdx]->checkFullCollision(m_BulletInstances[bulletIdx]->getVelocity())) {
            bulletsToDestroy.push_back(bulletIdx);
        }
    }

    // Remove player bullets
    removeBullets(bulletsToDestroy, false);

    std::unique_ptr<Area> rocketArea = std::make_unique<Area>(m_Player->getPosition().y,
                                                              m_Player->getPosition().y + SQUARE_SIZE,
                                                              m_Player->getPosition().x,
                                                              m_Player->getPosition().x + SQUARE_SIZE);

    // Enemy Bullet Collisions
    for (uint32_t bulletIdx = 0; bulletIdx < m_EnemyBulletInstances.size(); ++bulletIdx) {
        Vector2<float> bulletPosition = m_EnemyBulletInstances[bulletIdx]->getPosition();

        std::unique_ptr<Area> enemyBulletArea = std::make_unique<Area>(bulletPosition.y,
                                                                       bulletPosition.y + SQUARE_SIZE,
                                                                       bulletPosition.x,
                                                                       bulletPosition.x + SQUARE_SIZE);
        // Sniper bullet & player colllision
        if (checkObjectCollision(enemyBulletArea, rocketArea)) {
            bulletsToDestroy.push_back(bulletIdx);
            m_Player->setHealth(m_Player->getHealth() - HEALTH_LOSS_AFTER_HIT);

            if (DEBUG_LOG) {
                spdlog::critical("Current Player Health After Bullet Hit : {}", m_Player->getHealth());
            }

            continue;
        }

        // Tile Collision and Map Check
        if (bulletPosition.x > ((float) m_Width + SQUARE_SIZE) || bulletPosition.y > ((float) m_Height + SQUARE_SIZE) ||
            bulletPosition.x < (-SQUARE_SIZE) || bulletPosition.y < (-SQUARE_SIZE) ||
            m_EnemyBulletInstances[bulletIdx]->checkFullCollision(m_EnemyBulletInstances[bulletIdx]->getVelocity())) {
            bulletsToDestroy.push_back(bulletIdx);
        }
    }

    // Remove sniper bullets
    removeBullets(bulletsToDestroy, true);

    // Monster Enemy Collisions [Necessary !]
    std::vector<uint32_t> enemiesToDestroy;
    for (uint32_t enemyIdx = 0; enemyIdx < m_EnemyInstances.size(); ++enemyIdx) {

        Vector2<float> enemyPosition = m_EnemyInstances[enemyIdx]->getPosition();

        // Player Collision
        std::unique_ptr<Area> enemyArea = std::make_unique<Area>(enemyPosition.y,
                                                                 enemyPosition.y + SQUARE_SIZE,
                                                                 enemyPosition.x,
                                                                 enemyPosition.x + SQUARE_SIZE);

        if (checkObjectCollision(enemyArea, rocketArea)) {
            enemiesToDestroy.push_back(enemyIdx);
            m_Player->setHealth(m_Player->getHealth() - 10);

            // Check type and increment re-spawnable enemy count
            switch (m_EnemyInstances[enemyIdx]->getType()) {
                case Chaser: {
                    ++m_EnemyConfig->chaserCount;
                }
                    break;
                case Sniper: {
                    ++m_EnemyConfig->sniperCount;
                }
                    break;
                case SuicideBomber: {
                    ++m_EnemyConfig->suicideBomberCount;
                }
                case AI: {
                    spdlog::error("AI Enemy is not ready yet!");
                }
                    break;
            }

            if (DEBUG_LOG) {
                spdlog::critical("Current Player Health After Enemy Collision : {}", m_Player->getHealth());
            }

            continue;
        }

        // Bullet Collision and Cleanup
        for (uint32_t bulletIdx = 0; bulletIdx < m_BulletInstances.size(); ++bulletIdx) {

            Vector2<float> bulletPosition = m_BulletInstances[bulletIdx]->getPosition();
            std::unique_ptr<Area> bulletArea = std::make_unique<Area>(bulletPosition.y,
                                                                      bulletPosition.y + SQUARE_SIZE,
                                                                      bulletPosition.x,
                                                                      bulletPosition.x + SQUARE_SIZE);
            if (checkObjectCollision(bulletArea, enemyArea)) {
                m_Player->addScore(ENEMY_DESTROY_POINTS);

                spdlog::info("Current Player Score : {}", m_Player->getScore());

                // Remove a bullet
                m_BulletInstances[bulletIdx] = std::move(m_BulletInstances.back());
                m_BulletInstances.pop_back();

                enemiesToDestroy.push_back(enemyIdx);
                break;
            }
        }
    }

    removeEnemies(enemiesToDestroy);
}

void GameWindow::setGameStatus(bool status) {
    this->m_Status = status;
}

bool GameWindow::getGameStatus() const {
    return m_Status;
}

void GameWindow::updateGameStatus(GLFWwindow *window) {
    setGameStatus((bool) (!glfwWindowShouldClose(window)));
    glfwGetFramebufferSize(window, &m_Width, &m_Height); //GetCurrent Buffer Size
}

void GameWindow::render(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto &instance : m_BulletInstances) {
        instance->render();
    }

    for (const auto &enemy : m_EnemyInstances) {
        enemy->render();
    }

    for (const auto &enemyBullet : m_EnemyBulletInstances) {
        enemyBullet->render();
    }

    for (const auto &tile : m_Tiles) {
        tile->render();
    }

    m_Player->render();
    glfwSwapBuffers(window);

    glfwPollEvents(); //Get All Drawing Events
}

void GameWindow::update(GLFWwindow *window) {
    handleCollisions();
    handleSpawns(window);

    // Update Instances
    for (const auto &instance : m_BulletInstances) {
        instance->update(window);
    }

    for (const auto &enemy : m_EnemyInstances) {
        if (enemy->getType() == Sniper && enemy->onAction()) {

            Vector2<float> bulletDirection = (m_Player->getPosition() - enemy->getPosition()).normalize();

            fireEnemyBullet(enemy->getPosition(), bulletDirection);

            enemy->stopAction();
        }
        enemy->update(m_Player->getPosition(), m_Grid);
    }

    for (const auto &enemyBullet : m_EnemyBulletInstances) {
        enemyBullet->update(window);
    }

    // Image observations
    if (m_ObservationType == Image) {
        m_ImageBuffer.push_back(getCurrentImageGameState(window));
        if (m_ImageBuffer.size() >= IMAGE_OBS_COUNT) {
            m_Player->update(window, this->getCurrentGameState(window));
        }
    } else {
        // Internal Observations
        m_Player->update(window, this->getCurrentGameState(window));
    }

    // Random Restart Check!
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_TRUE) {
        restart(window);
    }
}

void GameWindow::removeBullets(std::vector<uint32_t> &bulletsToDestroy, bool enemy) {
    // Remove bullets from the render list
    if (bulletsToDestroy.empty())
        return;

    if (enemy) {
        for (auto bulletIdx : bulletsToDestroy) {
            m_EnemyBulletInstances[bulletIdx] = std::move(m_EnemyBulletInstances.back());
            m_EnemyBulletInstances.pop_back();
        }
    } else {
        for (auto bulletIdx : bulletsToDestroy) {
            m_BulletInstances[bulletIdx] = std::move(m_BulletInstances.back());
            m_BulletInstances.pop_back();
        }
    }

    bulletsToDestroy.clear();
}

void GameWindow::removeEnemies(std::vector<uint32_t> &enemiesToDestroy) {
    // Remove collided enemies
    if (enemiesToDestroy.empty())
        return;

    for (auto enemyIdx : enemiesToDestroy) {
        m_EnemyInstances[enemyIdx] = std::move(m_EnemyInstances.back());
        m_EnemyInstances.pop_back();
    }

    enemiesToDestroy.clear();
}

void GameWindow::close(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GameWindow::test_path_finding(const Vector2<float> &target_pos) {
    m_Maze->print();

    Vector2<int32_t> player_pos_tile = m_Maze->pos2MtrCoord(target_pos);
    Vector2<int32_t> random_target(3, 3);

    std::vector<Vector2<int32_t>> route = utility::findRoute(m_Maze->m_Matrix, m_Grid, player_pos_tile, random_target);

    spdlog::critical("Player Row : {} Col : {} Route Size : {}", player_pos_tile.x, player_pos_tile.y, route.size());

    for (const auto &tile : route) {
        spdlog::critical("Tile row : {} col : {}", tile.x, tile.y);

        std::unique_ptr<Sprite> rand_tile = std::make_unique<Sprite>(m_TextureTpID,
                                                                     Vector2<float>(SQUARE_SIZE * (float) tile.y,
                                                                                    (float) m_Height -
                                                                                    SQUARE_SIZE * (float) tile.x));

        m_Tiles.push_back(std::move(rand_tile));
    }

    spdlog::critical("Done");
}

std::unique_ptr<GameState> GameWindow::getCurrentGameState(GLFWwindow *window) {

    if (m_ObservationType == Internal) {
        return getCurrentInternalGameState();
    }
    std::vector<BinaryImage> currentStateImages = std::vector<BinaryImage>(m_ImageBuffer.begin(), m_ImageBuffer.end());

    return std::make_unique<GameStateImage>(currentStateImages, m_Width / 8, m_Height / 8, currentStateImages.size());
}

std::unique_ptr<GameState> GameWindow::getCurrentInternalGameState() {
    std::vector<Vector2<float>> enemy_positions;
    std::vector<Vector2<float>> bullet_positions;
    std::vector<float> bullet_directions;
    std::vector<Vector2<float>> normal_tile_positions;
    std::vector<Vector2<float>> teleport_tile_positions;

    std::for_each(m_EnemyInstances.begin(), m_EnemyInstances.end(),
                  [&](auto &enemy) { enemy_positions.push_back(enemy->getPosition()); });

    std::for_each(m_EnemyBulletInstances.begin(), m_EnemyBulletInstances.end(), [&](auto &bullet) {
        bullet_positions.push_back(bullet->getPosition());
        bullet_directions.push_back(bullet->getRotation());
    });
    std::for_each(m_Tiles.begin(), m_Tiles.end(), [&](auto &tile) {
        TileType type = m_Maze->pos2Tile(tile->getPosition());
        if (type == Wall) {
            normal_tile_positions.push_back(tile->getPosition());
        } else if (type == Teleport) {
            normal_tile_positions.push_back(tile->getPosition());
        }
    });

    uint16_t remaining_tile_count = 900 - normal_tile_positions.size();

    // Standardize Normal Tile Count in State Description
    for (uint16_t remaining_tile = 0; remaining_tile < remaining_tile_count; ++remaining_tile) {
        normal_tile_positions.emplace_back(-1.0, -1.0);
    }

    return std::make_unique<GameStateInternal>(m_Player->getPosition(),
                                               m_Player->getRotation(),
                                               m_Player->getRemainingBulletCount(),
                                               m_Player->getTimeToTeleport(),
                                               m_Player->getHealth(),
                                               std::move(normal_tile_positions),
                                               std::move(teleport_tile_positions),
                                               std::move(enemy_positions),
                                               std::move(bullet_positions),
                                               std::move(bullet_directions));
}

BinaryImage GameWindow::getCurrentImageGameState(GLFWwindow *window) {
    glfwGetFramebufferSize(window, &m_Width, &m_Height);
    BinaryImage buffer(m_Width * m_Height * 3);
    glReadPixels(0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

    // Down-scale and convert it to gray
    // TODO : [SLOW]
    torch::Tensor outputTensor = torch::tensor(buffer).view({3, m_Width, m_Height}).to(
            torch::kFloat32).flipud().unsqueeze(0);
    outputTensor = interpolate(outputTensor, InterpolateFuncOptions().size(
                    std::vector<int64_t>({m_Width / 8, m_Height / 8}))
            .mode(torch::kBilinear)
            .align_corners(true))
            .mean(1);

    // Flatten the tensor and cast it
    // TODO : [SLOW]
    outputTensor = outputTensor.view({m_Width * m_Height / 64}).to(torch::kUInt8);
    buffer = std::vector<u_char>(outputTensor.data_ptr<uint8_t>(),
                                 outputTensor.data_ptr<uint8_t>() + outputTensor.numel());

    return buffer;
}