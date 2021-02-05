#include "GameWindow.h"

GameWindow::GameWindow(bool status, int width, int height): status(status), width(width), height(height),
                                                            vertexBufferID(0){
    setupGL();
    setupImageBuffers();
    setupArena();

    // enemyInstances.reserve(enemyCount);
    this->quadTree = std::make_unique<t_quadTree>(f_quadBox(0, 0, width, height), GetQuadBox);

    // Configurations
    this->m_EnemyConfig = std::make_unique<EnemyConfig>();

    generate();
}

GameWindow::~GameWindow(){
    tiles.clear();
    enemyInstances.clear();
    bulletInstances.clear();

    glDeleteBuffers( 1, &vertexBufferID);
    glDeleteTextures(1, &textureBufferID);
    glDeleteTextures(1, &textureBulletID);
    glDeleteTextures(1, &textureEnemyID);

    free(game_area->Box);
}

void GameWindow::play(GLFWwindow* window){
    double lastTime    = glfwGetTime();
    double deltaTime   = 0.0f;
    double currentTime = 0.0f;
    long long num_frames = 0;
    
    while(getGameStatus()){

        if(rocket->getHealth() <= 0){
            restart(window);
        }
        else{
            render(window);

            deltaTime += (glfwGetTime() - lastTime) * UPDATES_PER_SECOND;
            lastTime = glfwGetTime();
            ++num_frames;
            
            while (deltaTime >= DELTA_TIME_THRESHOLD){
                update(window);
                --deltaTime;

                if(INFO_LOG){
                    std::cout << "FPS : " << num_frames << std::endl;
                }

                num_frames = 0;
            }
        }
        updateGameStatus(window);
    }

    close(window);
}

void GameWindow::restart(GLFWwindow* window){
    // Empty all cached object data
    bulletInstances.clear();
    enemyInstances.clear();
    tiles.clear();
    emptyTiles.clear();
    grid.clear();
    
    // Reset Maze
    setupArena();

    // Re-generate everything
    generate();
}

void GameWindow::setupGL(){
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, width, height); //Set Size

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, width, 0, height); //Origin
    glMatrixMode(GL_MODELVIEW);

    glGenBuffers(1, &vertexBufferID);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexData), (GLvoid *) offsetof(VertexData, positionCoordinates));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData), (GLvoid *)offsetof(VertexData, textureCoordinates));

    maxTileCount = static_cast<int>(((width * height) / (SQUARE_SIZE * SQUARE_SIZE)) * tile_percent);
}

void GameWindow::setupImageBuffers(){
    textureBufferID = loadAndBufferImage("files/enemy.png", 100, 100);
    textureBulletID = loadAndBufferImage("files/bullet_2.png", 100, 100);
    textureEnemyID  = loadAndBufferImage("files/boss.png", 100, 100);
    textureTileID   = loadAndBufferImage("files/tile.png", 100, 100);
    textureTpID     = loadAndBufferImage("files/blue_tile_2.png", 100, 100);
}

void GameWindow::setupArena(){
    game_area = std::make_unique<Area>(0.0f, height, 0.0f, width);
    maze      = std::make_unique<Maze>(static_cast<int>(height / SQUARE_SIZE),
                                       static_cast<int>(width / SQUARE_SIZE));
    
    //Init Grid for PathFinding
    for(size_t row = 0; row < maze->height + 1; ++row)
    {
        std::vector<std::shared_ptr<Point>> inner_vec;
        for(size_t col = 0; col < maze->height + 1; ++col)
        {
            inner_vec.push_back(std::make_shared<Point>(Vector2<int>(row, col)));
        }
        grid.push_back(inner_vec);
    }
}

uint GameWindow::loadAndBufferImage(const char *filename, int width, int height){
    uint textureBufferID;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_COLOR);
    glGenTextures(1, &textureBufferID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureBufferID);

    unsigned char *image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGBA);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    SOIL_free_image_data(image);

    return textureBufferID;
}

void GameWindow::generate(){
    std::cout << "Generate A Map" << std::endl;
    generateRandomMap();

    std::cout << "Generate Exit Point Map" << std::endl;
    generateExitPoints();

    std::cout << "Init Player" << std::endl;
    initPlayer();

    std::cout << "End of generation" << std::endl;
}

void GameWindow::generateRandomMap(){
    std::stack<Vector2<int>> stack;

    int min_row = 1;
    int min_col = 0;

    int max_row = static_cast<int>(width / SQUARE_SIZE);
    int max_col = static_cast<int>(height / SQUARE_SIZE) - 1;

    //First Random Tile
    Vector2<int> firstRandomPos(arc4random() % max_row + min_row,
                                arc4random() % max_col + min_col);

    while(!isPositionValid(firstRandomPos, min_row, max_row, min_col, max_col)){
        firstRandomPos.x = arc4random() % max_row + min_row;
        firstRandomPos.y = arc4random() % max_col + min_col;
    }

    //DFS Based Procedural Genaration
    stack.push(firstRandomPos);
    emptyTiles.push_back(firstRandomPos);


    //DFS GENERATION
    while(emptyTiles.size() <= maxTileCount){
        Vector2<int> curr = stack.top();


        //Select Random Point Around
        std::vector<Vector2<int>> neighbour_tiles;
        for(int n_r = -1; n_r <= 1; ++n_r){
            for(int n_c = -1; n_c <= 1; ++n_c){
                if((n_r != 0 || n_c != 0) && (std::abs(n_r) != std::abs(n_c))){
                    Vector2<int> neighbour_tile(curr.x + n_r, curr.y + n_c);
                    if(isPositionValid(neighbour_tile, min_row, max_row, min_col, max_col)){
                        neighbour_tiles.push_back(neighbour_tile);
                    }
                }
            }
        }

        if(neighbour_tiles.size() > 0){
            stack.push(neighbour_tiles.at(arc4random() % neighbour_tiles.size()));
            emptyTiles.push_back(stack.top());
            maze->matrix[stack.top().x][stack.top().y] = Empty;
        }
        else{
            stack.pop();
        }

        neighbour_tiles.clear();
    }
}

void GameWindow::generateExitPoints(){
    std::vector<Vector2<int>> exit_tiles;
    std::vector<Vector2<int>> close_tiles;

    int min_row = 1;
    int min_col = 0;

    int max_row = static_cast<int>(width / SQUARE_SIZE);
    int max_col = static_cast<int>(height / SQUARE_SIZE) - 1;

    // Random Teleport Tile Selection
    Vector2<int> up_exit_tile(min_row, static_cast<int>(arc4random() % (uint32_t)max_col - 2) + 1);
    Vector2<int> down_exit_tile(max_row, up_exit_tile.y);

    Vector2<int> left_exit_tile(static_cast<int>(arc4random() % (uint32_t)max_row), min_col);
    Vector2<int> right_exit_tile(left_exit_tile.x, max_col);
    
    exit_tiles.push_back(up_exit_tile);
    exit_tiles.push_back(down_exit_tile);
    exit_tiles.push_back(right_exit_tile);
    exit_tiles.push_back(left_exit_tile);

    // Find closest tiles to teleport points
    for(auto i = 0; i < exit_tiles.size(); ++i){
        Vector2<int> tile = exit_tiles[i];
        std::vector<float> distances;

        // In-efficient asF
        std::for_each(emptyTiles.begin(), emptyTiles.end(),
                      [&](Vector2<int> selected_tile){
                          distances.push_back(utility::dist_euc<int>(tile, selected_tile));
                      }
                     );
        close_tiles.push_back(emptyTiles[std::min_element(distances.begin(), distances.end()) - distances.begin()]);
    }

    // Create a way to teleport starting from closest point
    for(auto i = 0; i < close_tiles.size(); ++i){
        int row_difference = exit_tiles[i].x - close_tiles[i].x;
        int col_difference = exit_tiles[i].y - close_tiles[i].y;
        
        if(i >= (int)(exit_tiles.size() / 2)){
            // Left-Right Teleport Points
            if(std::abs(col_difference) > 0){
                for(auto step = 1; step <= std::abs(col_difference); ++step){
                    (col_difference > 0) ? maze->matrix[exit_tiles[i].x][exit_tiles[i].y - step] = Empty :
                                           maze->matrix[exit_tiles[i].x][exit_tiles[i].y + step] = Empty;
                }
            }

            if(std::abs(row_difference) > 0){
                for(auto step = 1; step <= std::abs(row_difference); ++step){
                    (row_difference > 0) ? maze->matrix[close_tiles[i].x + step][close_tiles[i].y] = Empty :
                                           maze->matrix[close_tiles[i].x - step][close_tiles[i].y]  = Empty;
                }
            }
        }
        else{
            // Top-Bottom Teleport Points
            if(std::abs(row_difference) > 0){
                for(auto step = 1; step <= std::abs(row_difference); ++step){
                    (row_difference > 0) ? maze->matrix[exit_tiles[i].x - step][exit_tiles[i].y] = Empty:
                                           maze->matrix[exit_tiles[i].x + step][exit_tiles[i].y] = Empty;
                }
            }

            if(std::abs(col_difference) > 0){
                for(auto step = 1; step <= std::abs(col_difference); ++step){
                    (col_difference > 0) ? maze->matrix[close_tiles[i].x][close_tiles[i].y + step] = Empty :
                                           maze->matrix[close_tiles[i].x][close_tiles[i].y - step]   = Empty;
                }
            }
        }
    }

    // Empty 2 tiles to be able to jump
    maze->matrix[up_exit_tile.x + 2][up_exit_tile.y]       = Empty;
    maze->matrix[up_exit_tile.x + 1][up_exit_tile.y]       = Empty;
    maze->matrix[down_exit_tile.x - 2][down_exit_tile.y]   = Empty;
    maze->matrix[down_exit_tile.x - 1][down_exit_tile.y]   = Empty;
    maze->matrix[left_exit_tile.x][left_exit_tile.y + 1]   = Empty;
    maze->matrix[left_exit_tile.x][left_exit_tile.y + 2]   = Empty;
    maze->matrix[right_exit_tile.x][right_exit_tile.y - 1] = Empty;
    maze->matrix[right_exit_tile.x][right_exit_tile.y - 2] = Empty;

    /*Setup Exit Points*/
    maze->matrix[up_exit_tile.x][up_exit_tile.y]           = Teleport;
    maze->matrix[up_exit_tile.x - 1][up_exit_tile.y]       = Teleport;
    maze->matrix[down_exit_tile.x][down_exit_tile.y]       = Teleport;
    maze->matrix[left_exit_tile.x][left_exit_tile.y]       = Teleport;
    maze->matrix[right_exit_tile.x][right_exit_tile.y]     = Teleport;
    maze->matrix[right_exit_tile.x][right_exit_tile.y + 1] = Teleport;

    // Last
    for(int row = 0; row < static_cast<int>(height / SQUARE_SIZE) + 1; ++row){
        for(int col = 0; col < static_cast<int>(width / SQUARE_SIZE) + 1; ++col){
            if(maze->matrix[row][col] == Wall){
                std::unique_ptr<Sprite> tile = std::make_unique<Sprite>(textureTileID, Vector2<float>(SQUARE_SIZE * col, height - SQUARE_SIZE * row));
                tiles.push_back(std::move(tile));
            }
            else if(maze->matrix[row][col] == Teleport){
                std::unique_ptr<Sprite> teleport_tile = std::make_unique<Sprite>(textureTpID, Vector2<float>(SQUARE_SIZE * col, height - SQUARE_SIZE * row));
                tiles.push_back(std::move(teleport_tile));
            }
        }
    }

}

void GameWindow::initPlayer(){
    //Get Random Point
    Vector2<int> randomPosition = emptyTiles[arc4random() % emptyTiles.size()];

    // Create Player Object
    rocket = std::make_unique<Player>(textureBufferID, Vector2<float>(randomPosition.y * SQUARE_SIZE,
                                                                      width - randomPosition.x * SQUARE_SIZE));

    // Initialize Player
    rocket->setArea(game_area->Copy());
    rocket->setMaze(maze->Copy());
    rocket->setVelocity(Vector2<float>(PLAYER_SPEED, PLAYER_SPEED));
    rocket->setHealth(100);
    rocket->setOffset(OBJECT_OFFSET);
}

bool GameWindow::isPositionValid(Vector2<int> &position, int min_row, int max_row, int min_col, int max_col){
    return (position.x > min_row) && (position.x < max_row) &&
           (position.y > min_col) && (position.y < max_col) &&
           maze->matrix[position.x][position.y] == Wall;
}

void GameWindow::firePlayerBullet(){
    std::unique_ptr<Projectile> bullet = std::make_unique<Projectile>(textureBulletID,
                                                              rocket->getPosition() + Vector2<float>(20.0f, 0.0f), PlayerBullet);
    Vector2<float> bullet_relative_speed(std::cos(utility::angle2Rad(rocket->getRotation())) * 4.0f,
                                         std::sin(utility::angle2Rad(rocket->getRotation())) * 4.0f);

    bullet->setVelocity(bullet_relative_speed);
    bullet->setRotation(rocket->getRotation());
    bullet->setArea(game_area->Copy());
    bullet->setMaze(maze->Copy());
    bullet->setOffset(OBJECT_OFFSET);

    bulletInstances.push_back(std::move(bullet));
}

void GameWindow::fireEnemyBullet(Vector2<float> enemyPosition, Vector2<float> direction){
    std::unique_ptr<Projectile> enemyBullet = std::make_unique<Projectile>(textureBulletID,  enemyPosition + direction * 10.0f, EnemyBullet);
    Vector2<float> bullet_relative_speed = direction * 4.0f;

    enemyBullet->setVelocity(bullet_relative_speed);
    enemyBullet->setRotation(rocket->getRotation());
    enemyBullet->setArea(game_area->Copy());
    enemyBullet->setMaze(maze->Copy());
    enemyBullet->setOffset(OBJECT_OFFSET);

    enemyBulletInstances.push_back(std::move(enemyBullet));
}

void GameWindow::handleSpawns(GLFWwindow* window){
    /*Basic Spawn Trick*/
    // static int spawnTicks = 0;
    // static int fireTicks  = 0;
    // static int enemyTicks = 0;

    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS &&
                  rocket->bullet_count > 0 &&
                  fireTicks >= PLAYER_FIRE_TICKS){

        firePlayerBullet();
        fireTicks = 0;
        --rocket->bullet_count;
    }

    if(spawnTicks >= ENEMY_SPAWN_TICKS){
        spawnEnemies();
        spawnTicks = 0;
    }

    ++fireTicks;
    ++spawnTicks;
}

void GameWindow::spawnEnemies(){
    if(enemyInstances.size() < m_EnemyConfig->enemyCount){
        Vector2<int> playerTile = maze->pos2MtrCoord(rocket->getPosition());

        //Get Random Point
        std::vector<Vector2<int>> spanwableTiles;

        std::for_each(emptyTiles.begin(), emptyTiles.end(),
                      [&](Vector2<int> selected_tile){
                            if(selected_tile != playerTile){
                                float dist = utility::dist_euc<int>(playerTile, selected_tile);
                                if(dist < m_EnemyConfig->enemySpawnDistance){
                                    spanwableTiles.push_back(selected_tile);
                                } 
                          }
                        }
                    );

        Vector2<int> randomPosition = spanwableTiles[arc4random() % spanwableTiles.size()];

        // Decide to which enemy type is going to be generated
        EnemyType targetEnemyType = Chaser;

        if(m_EnemyConfig->sniperCount > 0){
            --m_EnemyConfig->sniperCount;
            targetEnemyType = Sniper;
        }
        else if(m_EnemyConfig->suicideBomberCount > 0){
            --m_EnemyConfig->suicideBomberCount;
            targetEnemyType = SuicideBomber;
        }

        std::cout << "Spawn Enemy Type : " << targetEnemyType << std::endl;

        // Spawn
        std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(textureEnemyID, 
                                                                Vector2<float>(randomPosition.y * SQUARE_SIZE, 
                                                                                width - randomPosition.x * SQUARE_SIZE), targetEnemyType);

        // enemy->setVelocity(Vector2<float>(-5.0f, 0.0f));
        enemy->setArea(game_area->Copy());
        enemy->setMaze(maze->Copy());
        enemy->setOffset(OBJECT_OFFSET);
        
        enemyInstances.push_back(std::move(enemy));

        enemySpawnDistance = std::max((float)(enemySpawnDistance * 0.90), MIN_ENEMY_SPAWN_DISTANCE);
        enemyCount = std::min(enemyCount + 1, MAX_ENEMY_COUNT);
    }
}

bool GameWindow::checkStaticWallCollision(Vector2<float> position){
    // Get Intersecting Tiles
    TileType bottomLeftTile = this->maze->pos2Tile(position);
    
    if(bottomLeftTile == Wall || bottomLeftTile == Obstacle){
        return true;
    }

    TileType bottomRightTile = this->maze->pos2Tile(Vector2<float>(position.y, position.x + SQUARE_SIZE));
    
    if(bottomRightTile == Wall || bottomRightTile == Obstacle){
        return true;
    }

    TileType topLeftTile = this->maze->pos2Tile(Vector2<float>(position.y + SQUARE_SIZE, position.x));

    if(topLeftTile == Wall || topLeftTile == Obstacle){
        return true;
    }

    TileType topRightTile = this->maze->pos2Tile(Vector2<float>(position.y + SQUARE_SIZE, position.x + SQUARE_SIZE));

    if(topRightTile == Wall || topRightTile == Obstacle){
        return true;
    }

    return false;
}

bool GameWindow::checkObjectCollision(std::unique_ptr<Area> objectX, std::unique_ptr<Area> objectY){
    //Intersection of Two Rectangle
    bool res =  !(objectX->Box->left   >= objectY->Box->right ||
                  objectX->Box->right  <= objectY->Box->left ||
                  objectX->Box->top    <= objectY->Box->bottom  ||
                  objectX->Box->bottom >= objectY->Box->top);

    //Avoid Memory Leak
    // free(objectX->Box);
    // free(objectY->Box);
    return res;
}

bool GameWindow::checkObjectCollision(std::shared_ptr<Area> objectX, std::shared_ptr<Area> objectY){
    //Intersection of Two Rectangle
    bool res =  !(objectX->Box->left   >= objectY->Box->right ||
                  objectX->Box->right  <= objectY->Box->left ||
                  objectX->Box->top    <= objectY->Box->bottom  ||
                  objectX->Box->bottom >= objectY->Box->top);

    //Avoid Memory Leak
    // free(objectX->Box);
    // free(objectY->Box);
    return res;
}

void GameWindow::handleCollisions(){
    // COLLISIONS

    // Delete Bullets if they pass the borders or hit the any tile
    std::vector<unsigned int> bulletsToDestroy;

    for(unsigned int bulletIdx = 0; bulletIdx < bulletInstances.size(); ++bulletIdx){
        Vector2<float> bulletPosition = bulletInstances[bulletIdx]->getPosition();
        
        if( bulletPosition.x > (width + SQUARE_SIZE) || bulletPosition.y > (height + SQUARE_SIZE) ||
            bulletPosition.x < (-SQUARE_SIZE) || bulletPosition.y < (-SQUARE_SIZE) || 
            bulletInstances[bulletIdx]->checkFullCollision(bulletInstances[bulletIdx]->getVelocity()))
        {
            bulletsToDestroy.push_back(bulletIdx);
        }
    }

    // Remove player bullets
    removeBullets(bulletsToDestroy, false);


    std::shared_ptr<Area> rocketArea = std::make_shared<Area>(rocket->getPosition().y,
                                                              rocket->getPosition().y + SQUARE_SIZE,
                                                              rocket->getPosition().x,
                                                              rocket->getPosition().x + SQUARE_SIZE);

    // Enemy Bullet Collisions
    for(unsigned int bulletIdx = 0; bulletIdx < enemyBulletInstances.size(); ++bulletIdx){
        Vector2<float> bulletPosition = enemyBulletInstances[bulletIdx]->getPosition();

        std::shared_ptr<Area> enemyBulletArea = std::make_shared<Area>(bulletPosition.y, 
                                                                 bulletPosition.y + SQUARE_SIZE,
                                                                 bulletPosition.x,
                                                                 bulletPosition.x + SQUARE_SIZE
                                                                );
        // Sniper bullet & player colllision
        if(checkObjectCollision(enemyBulletArea, rocketArea)){
            bulletsToDestroy.push_back(bulletIdx);
            rocket->setHealth(rocket->getHealth() - 10);
            
            if(DEBUG_LOG)
                std::cout << "Current Player Health [BULLET] : " << rocket->getHealth() << std::endl;  
            
            continue;
        }

        // Tile Collision and Map Check
        if( bulletPosition.x > (width + SQUARE_SIZE) || bulletPosition.y > (height + SQUARE_SIZE) ||
            bulletPosition.x < (-SQUARE_SIZE) || bulletPosition.y < (-SQUARE_SIZE) || 
            enemyBulletInstances[bulletIdx]->checkFullCollision(enemyBulletInstances[bulletIdx]->getVelocity()))
        {
            bulletsToDestroy.push_back(bulletIdx);
        }
    }

    // Remove sniper bullets
    removeBullets(bulletsToDestroy, true);

    // Monster Enemy Collisions [Necessary !]
    std::vector<unsigned int> enemiesToDestroy;
    for(unsigned int enemyIdx = 0; enemyIdx < enemyInstances.size(); ++enemyIdx){

        Vector2<float> enemyPosition = enemyInstances[enemyIdx]->getPosition();

        // Player Collision
        std::shared_ptr<Area> enemyArea = std::make_shared<Area>(enemyPosition.y, 
                                                                 enemyPosition.y + SQUARE_SIZE,
                                                                 enemyPosition.x,
                                                                 enemyPosition.x + SQUARE_SIZE
                                                                );

        if(checkObjectCollision(enemyArea, rocketArea)){
            enemiesToDestroy.push_back(enemyIdx);
            rocket->setHealth(rocket->getHealth() - 10);

            // Check type and increment respawnable enemy count
            switch (enemyInstances[enemyIdx]->getType())
            {
                case Chaser:
                {
                    ++m_EnemyConfig->chaserCount;    
                }
                break;
                case Sniper:
                {
                    ++m_EnemyConfig->sniperCount;
                }
                break;
                case SuicideBomber:
                {
                    ++m_EnemyConfig->suicideBomberCount;
                }
                break;
            }
                           
            if(DEBUG_LOG)
                std::cout << "Current Player Health : " << rocket->getHealth() << std::endl;  
            
            continue;
        }

        // Bullet Collision and Cleanup
        for(unsigned int bulletIdx = 0; bulletIdx < bulletInstances.size(); ++bulletIdx){

            Vector2<float> bulletPosition = bulletInstances[bulletIdx]->getPosition();
            std::shared_ptr<Area> bulletArea =  std::make_shared<Area>(bulletPosition.y, 
                                                                       bulletPosition.y + SQUARE_SIZE,
                                                                       bulletPosition.x,
                                                                       bulletPosition.x + SQUARE_SIZE
                                                                    );
            if(checkObjectCollision(bulletArea, enemyArea)){
                rocket->addScore(ENEMY_DESTROY_POINTS);
                std::cout << "Current Player Score : " << rocket->getScore() << std::endl;
                
                // Remove a bullet
                bulletInstances[bulletIdx] = std::move(bulletInstances.back());
                bulletInstances.pop_back();
                
                enemiesToDestroy.push_back(enemyIdx);
                break;
            }
        }
    }

    removeEnemies(enemiesToDestroy);
}

void GameWindow::setGameStatus(bool status){
    this->status = status;
}

bool GameWindow::getGameStatus(){
    return status;
}

void GameWindow::updateGameStatus(GLFWwindow* window){
    setGameStatus((bool)(!glfwWindowShouldClose(window)));
    glfwGetFramebufferSize(window, &width, &height); //GetCurrent Buffer Size
}

void GameWindow::render(GLFWwindow* window){
    glClear(GL_COLOR_BUFFER_BIT);

    for(const auto& instance : bulletInstances){
        instance->render();
    }

    for(const auto& enemy : enemyInstances){
        enemy->render();
    }

    for(const auto& enemyBullet: enemyBulletInstances){
        enemyBullet->render();
    }

    for(const auto& tile : tiles){
        tile->render();
    }


    rocket->render();
    glfwSwapBuffers(window);

    glfwPollEvents();//Get All Drawing Events
}

void GameWindow::update(GLFWwindow* window){
    handleCollisions();
    handleSpawns(window);
    
    // Update Instances
    for(const auto& instance : bulletInstances){
        instance->update(window);
    }

    for(const auto& enemy : enemyInstances){
        // std::cout << enemy->getPosition().x << " " << enemy->getPosition().y << " - Update!" << std::endl;
        if(enemy->getType() == Sniper && enemy->onAction()){
            
            Vector2<float> bulletDirection = (rocket->getPosition() - enemy->getPosition()).normalize();

            fireEnemyBullet(enemy->getPosition(), bulletDirection);
            
            enemy->stopAction();
        }
        enemy->update(rocket->getPosition(), this->grid);
    }

    for(const auto& enemyBullet : enemyBulletInstances){
        enemyBullet->update(window);
    }

    for(const auto& tile : tiles){
        tile->update(window);
    }

    rocket->update(window);
}

void GameWindow::removeBullets(std::vector<unsigned int>& bulletsToDestroy, bool enemy){
    // Remove bullets from the render list
    if(bulletsToDestroy.size() < 1)
        return;

    if(enemy){
        for(auto bulletIdx : bulletsToDestroy){
            enemyBulletInstances[bulletIdx] = std::move(enemyBulletInstances.back());
            enemyBulletInstances.pop_back();
        }
    }
    else{
        for(auto bulletIdx : bulletsToDestroy){
            bulletInstances[bulletIdx] = std::move(bulletInstances.back());
            bulletInstances.pop_back();
        }
    }

    bulletsToDestroy.clear();
}

void GameWindow::removeEnemies(std::vector<unsigned int>& enemiesToDestroy){
    // Remove collided enemies
    if(enemiesToDestroy.size() < 1)
        return;

    for(auto enemyIdx : enemiesToDestroy){
        enemyInstances[enemyIdx] = std::move(enemyInstances.back());
        enemyInstances.pop_back();
    }

    enemiesToDestroy.clear();
}

void GameWindow::close(GLFWwindow* window){
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GameWindow::test_path_finding(Vector2<float> target_pos){
    maze->print();

    Vector2<int> player_pos_tile = maze->pos2MtrCoord(target_pos);
    Vector2<int> random_target(3, 3);
    
    std::vector<Vector2<int>> route = utility::findRoute(maze->matrix, grid, player_pos_tile, random_target);
    
    std::cout << "Player row : " << player_pos_tile.x << " col : " << player_pos_tile.y << std::endl;
    std::cout << route.size() << std::endl;
    
    for(auto tile : route){
        std::cout << "T row : " << tile.x << " col : " << tile.y << std::endl;
        
        std::unique_ptr<Sprite> rand_tile = std::make_unique<Sprite>(textureTpID, Vector2<float>(SQUARE_SIZE * tile.y, height - SQUARE_SIZE * tile.x));

        tiles.push_back(std::move(rand_tile));
    }

    std::cout << std::endl;
}
