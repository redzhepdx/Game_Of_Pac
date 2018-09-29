#include "GameWindow.h"

GameWindow::GameWindow(bool status, int width, int height): status(status), width(width), height(height),
                                                            vertexBufferID(0){
    this->setupGL();

    this->textureBufferID = loadAndBufferImage("files/dick.png", 100, 100);
    this->textureBulletID = loadAndBufferImage("files/dick.png", 100, 100);
    this->textureEnemyID  = loadAndBufferImage("files/small_janjan.png", 100, 100);
    this->textureTileID   = loadAndBufferImage("files/tile.png", 100, 100);
    this->textureTpID     = loadAndBufferImage("files/blue_tile.png", 100, 100);

    this->game_area = std::make_unique<Area>(0.0f, this->height, 0.0f, this->width);
    this->maze      = std::make_unique<Maze>(static_cast<int>(this->height / Square_Size),
                                             static_cast<int>(this->width / Square_Size));
    
    //Init Grid for PathFinding
    for(size_t row = 0; row < this->maze->height + 1; ++row)
    {
        std::vector<std::shared_ptr<Point>> inner_vec;
        for(size_t col = 0; col < this->maze->height + 1; ++col)
        {
            inner_vec.push_back(std::make_shared<Point>(Vector2<int>(row, col)));
        }
        grid.push_back(inner_vec);
    }
    
    Vector2<float> player_pos = this->generateMap();

    /*TEST A* */
    this->maze->print();
    Vector2<int> player_pos_tile = this->maze->pos2MtrCoord(player_pos);
    Vector2<int> target(3, 3);
    std::vector<Vector2<int>> route = findRoute(this->maze->matrix, this->grid, player_pos_tile, target);
    std::cout << "Player row : " << player_pos_tile.x << " col : " << player_pos_tile.y << std::endl;
    std::cout << route.size() << std::endl;
    for(auto tile : route){
        std::cout << "T row : " << tile.x << " col : " << tile.y << std::endl;
        std::unique_ptr<Sprite> rand_tile = std::make_unique<Sprite>(this->textureTpID, Vector2<float>(Square_Size * tile.y, this->height - Square_Size * tile.x));
        this->tiles.push_back(std::move(rand_tile));
    }
    std::cout << std::endl;
    /*TEST A* */
    this->enemyInstances.reserve(this->enemyCount);

    this->rocket = std::make_unique<PlayerSprite>(this->textureBufferID, Vector2<float>(player_pos.x, player_pos.y));
    this->rocket->setArea(this->game_area->Copy());
    this->rocket->setMaze(this->maze->Copy());
    this->rocket->setVelocity(Vector2<float>(3.0f, 3.0f));
    this->rocket->setHealth(100);
    this->rocket->setOffset(5.0f);
}

GameWindow::~GameWindow(){
    this->tiles.clear();
    this->enemyInstances.clear();
    this->renderInstances.clear();

    glDeleteBuffers( 1, &this->vertexBufferID);
    glDeleteTextures(1, &this->textureBufferID);
    glDeleteTextures(1, &this->textureBulletID);
    glDeleteTextures(1, &this->textureEnemyID);

    free(this->game_area->Box);
}

void GameWindow::setupGL(){
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, this->width, this->height); //Set Size

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, this->width, 0, this->height); //Origin
    glMatrixMode(GL_MODELVIEW);

    glGenBuffers(1, &this->vertexBufferID);

    glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), this->vertices, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VertexData), (GLvoid *) offsetof(VertexData, positionCoordinates));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VertexData), (GLvoid *)offsetof(VertexData, textureCoordinates));

    this->maxTileCount = static_cast<int>(((this->width * this->height) / (Square_Size * Square_Size)) * this->tile_percent);

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

Vector2<float> GameWindow::generateMap(){
    std::unique_ptr<std::stack<Vector2<int>>> stack = std::make_unique<std::stack<Vector2<int>>>();
    std::unique_ptr<std::vector<Vector2<int>>> selectedTiles = std::make_unique<std::vector<Vector2<int>>>();

    int min_row = 1;
    int min_col = 0;

    int max_row = static_cast<int>(this->width / Square_Size);
    int max_col = static_cast<int>(this->height / Square_Size) - 1;

    //First Random Tile
    Vector2<int> firstRandomPos(arc4random() % max_row + min_row,
                                arc4random() % max_col + min_col);

    while(!this->isPositionValid(firstRandomPos, min_row, max_row, min_col, max_col)){
        firstRandomPos.x = arc4random() % max_row + min_row;
        firstRandomPos.y = arc4random() % max_col + min_col;
    }

    //DFS Based Procedural Genaration
    stack->push(firstRandomPos);
    selectedTiles->push_back(firstRandomPos);
    //DFS GENERATION
    while(selectedTiles->size() <= this->maxTileCount){
        Vector2<int> curr = stack->top();
        //Select Random Point Around
        std::unique_ptr<std::vector<Vector2<int>>> neighbour_tiles = std::make_unique<std::vector<Vector2<int>>>();
        for(int n_r = -1; n_r <= 1; ++n_r){
            for(int n_c = -1; n_c <= 1; ++n_c){
                if((n_r != 0 || n_c != 0) && (std::abs(n_r) != std::abs(n_c))){
                    Vector2<int> neighbour_tile(curr.x + n_r, curr.y + n_c);
                    if(this->isPositionValid(neighbour_tile, min_row, max_row, min_col, max_col)){
                        neighbour_tiles->push_back(neighbour_tile);
                    }
                }
            }
        }
        if(neighbour_tiles->size() > 0){
            stack->push(neighbour_tiles->at(arc4random() % neighbour_tiles->size()));
            selectedTiles->push_back(stack->top());
            this->maze->matrix[stack->top().x][stack->top().y] = Empty;
        }
        else{
            stack->pop();
        }
        neighbour_tiles->clear();
    }

    /*TODO : NEEDS Automatization*/

    std::unique_ptr<std::vector<Vector2<int>>> exit_tiles  = std::make_unique<std::vector<Vector2<int>>>();
    std::unique_ptr<std::vector<Vector2<int>>> close_tiles = std::make_unique<std::vector<Vector2<int>>>();

    Vector2<int> up_exit_tile(min_row, static_cast<int>(arc4random() % (uint32_t)max_col));
    Vector2<int> down_exit_tile(max_row, up_exit_tile.y);

    Vector2<int> left_exit_tile(static_cast<int>(arc4random() % (uint32_t)max_row), min_col);
    Vector2<int> right_exit_tile(left_exit_tile.x, max_col);

    exit_tiles->push_back(up_exit_tile);
    exit_tiles->push_back(down_exit_tile);
    exit_tiles->push_back(right_exit_tile);
    exit_tiles->push_back(left_exit_tile);

    for(auto i = 0; i < exit_tiles->size(); ++i){
        Vector2<int> tile = exit_tiles->at(i);
        std::vector<float> distances;
        std::for_each(selectedTiles->begin(), selectedTiles->end(),
                      [&](Vector2<int> selected_tile){
                          distances.push_back(dist_euc<int>(tile, selected_tile));
                      }
                     );
        close_tiles->push_back(selectedTiles->at(std::min_element(distances.begin(), distances.end()) - distances.begin()));
    }

    for(auto i = 0; i < close_tiles->size(); ++i){
        int row_difference = exit_tiles->at(i).x - close_tiles->at(i).x;
        int col_difference = exit_tiles->at(i).y - close_tiles->at(i).y;

        if(std::abs(col_difference) > 0){
            for(auto step = 1; step <= std::abs(col_difference); ++step){
                (col_difference > 0) ? this->maze->matrix[close_tiles->at(i).x][close_tiles->at(i).y + step] = Empty :
                                       this->maze->matrix[exit_tiles->at(i).x][exit_tiles->at(i).y + step]   = Empty;
            }
        }

        if(std::abs(row_difference) > 0){
            int col_tile_pos = (exit_tiles->at(i).y > close_tiles->at(i).y) ? exit_tiles->at(i).y : close_tiles->at(i).y;
            for(auto step = 1; step <= std::abs(row_difference); ++step){
                (row_difference > 0) ? this->maze->matrix[close_tiles->at(i).x + step][col_tile_pos] = Empty :
                                       this->maze->matrix[exit_tiles->at(i).x + step][col_tile_pos]   = Empty;
            }
        }
    }

    /*Setup Exit Points*/
    this->maze->matrix[up_exit_tile.x][up_exit_tile.y]           = Teleport;
    this->maze->matrix[up_exit_tile.x - 1][up_exit_tile.y]       = Empty;
    this->maze->matrix[down_exit_tile.x][down_exit_tile.y]       = Teleport;
    this->maze->matrix[left_exit_tile.x][left_exit_tile.y]       = Teleport;
    this->maze->matrix[right_exit_tile.x][right_exit_tile.y]     = Teleport;
    this->maze->matrix[right_exit_tile.x][right_exit_tile.y + 1] = Empty;

    for(int row = 0; row < static_cast<int>(this->height / Square_Size) + 1; ++row){
        for(int col = 0; col < static_cast<int>(this->width / Square_Size) + 1; ++col){
            if(this->maze->matrix[row][col] == Wall){
                std::unique_ptr<Sprite> rand_tile = std::make_unique<Sprite>(this->textureTileID, Vector2<float>(Square_Size * col, this->height - Square_Size * row));
                this->tiles.push_back(std::move(rand_tile));
            }
            else if(this->maze->matrix[row][col] == Teleport){
                std::unique_ptr<Sprite> rand_tile = std::make_unique<Sprite>(this->textureTpID, Vector2<float>(Square_Size * col, this->height - Square_Size * row));
                this->tiles.push_back(std::move(rand_tile));
            }
        }
    }

    //Get Random Point
    Vector2<int> randomPosition = selectedTiles->at(arc4random() % selectedTiles->size());

    /*Cleanup*/
    selectedTiles->clear();
    exit_tiles->clear();
    close_tiles->clear();

    return Vector2<float>(randomPosition.y * Square_Size, this->width - randomPosition.x * Square_Size);
}

bool GameWindow::isPositionValid(Vector2<int> &position, int min_row, int max_row, int min_col, int max_col){
    return (position.x > min_row) && (position.x < max_row) &&
           (position.y > min_col) && (position.y < max_col) &&
           this->maze->matrix[position.x][position.y] == Wall;
}

void GameWindow::fireBullet(){
    std::unique_ptr<Sprite> bullet = std::make_unique<Sprite>(this->textureBulletID,
                                                              this->rocket->getPosition() + Vector2<float>(20.0f, 0.0f));
    Vector2<float> bullet_relative_speed(std::cos(angle2Rad(this->rocket->getRotation())) * 4.0f,
                                         std::sin(angle2Rad(this->rocket->getRotation())) * 4.0f);

    bullet->setVelocity(bullet_relative_speed);
    bullet->setRotation(this->rocket->getRotation());
    bullet->setArea(this->game_area->Copy());
    bullet->setMaze(this->maze->Copy());

    this->renderInstances.push_back(std::move(bullet));

}

void GameWindow::spawnEnemies(){
    Vector2<float> position(static_cast<float>(this->width),
                            static_cast<float>(arc4random() % (unsigned int)this->height));
    std::unique_ptr<Sprite> enemy = std::make_unique<Sprite>(this->textureEnemyID, position);
    enemy->setArea(this->game_area->Copy());
    enemy->setVelocity(Vector2<float>(-5.0f, 0.0f));
    this->enemyInstances.push_back(std::move(enemy));
}

bool GameWindow::checkCollision(std::unique_ptr<Area> obj_1, std::unique_ptr<Area> obj_2){
    //Intersection of Two Rectangle
    bool res =  !(obj_1->Box->left   >= obj_2->Box->right ||
                  obj_1->Box->right  <= obj_2->Box->left ||
                  obj_1->Box->top    <= obj_2->Box->bottom  ||
                  obj_1->Box->bottom >= obj_2->Box->top);

    //Avoid Memory Leak
    free(obj_1->Box);
    free(obj_2->Box);
    return res;
}

void GameWindow::setGameStatus(bool status){
    this->status = status;
}

bool GameWindow::getGameStatus(){
    return this->status;
}

void GameWindow::updateGameStatus(GLFWwindow* window){
    this->setGameStatus((bool)(!glfwWindowShouldClose(window)));
    glfwGetFramebufferSize(window, &this->width, &this->height); //GetCurrent Buffer Size
}

void GameWindow::render(GLFWwindow* window){
    glClear(GL_COLOR_BUFFER_BIT);

    for(const auto& instance : this->renderInstances){
        instance->render();
    }

    for(const auto& enemy : this->enemyInstances){
        enemy->render();
    }

    for(auto& tile : this->tiles){
        tile->render();
    }


    this->rocket->render();
    glfwSwapBuffers(window);

    glfwPollEvents();//Get All Drawing Events
}


void GameWindow::update(GLFWwindow* window){
    //Delete Bullets if they pass the borders or hit the any tile
    std::vector<std::vector<std::unique_ptr<Sprite>>::iterator> outInstanceIndexes;
    for(std::vector<std::unique_ptr<Sprite>>::iterator spriteIterator = this->renderInstances.begin();
        spriteIterator != this->renderInstances.end(); ++spriteIterator){
        if( (*spriteIterator)->getPosition().x  > (this->width + Square_Size) ||
            (*spriteIterator)->getPosition().y > (this->height + Square_Size) ||
            (*spriteIterator)->getPosition().x < (-Square_Size) ||
            (*spriteIterator)->getPosition().y < (-Square_Size) ||
            (*spriteIterator)->checkFullCollision((*spriteIterator)->getVelocity())){
            //Store the indexes
            outInstanceIndexes.push_back(spriteIterator);
        }
    }

    //Collision
    //NOTE : It is set because double collision happens and set more memory efficient
    std::set<std::vector<std::unique_ptr<Sprite>>::iterator> outEnemyInstances;
    for(std::vector<std::unique_ptr<Sprite>>::iterator spriteIterator = this->enemyInstances.begin();
        spriteIterator != this->enemyInstances.end(); ++spriteIterator){

        std::unique_ptr<Area> area_1 = std::make_unique<Area>(this->rocket->getPosition().y,
                                                              this->rocket->getPosition().y + Square_Size,
                                                              this->rocket->getPosition().x,
                                                              this->rocket->getPosition().x + Square_Size);

        std::unique_ptr<Area> area_2 = std::make_unique<Area>((*spriteIterator)->getPosition().y,
                                                              (*spriteIterator)->getPosition().y + Square_Size,
                                                              (*spriteIterator)->getPosition().x,
                                                              (*spriteIterator)->getPosition().x + Square_Size);

        if(this->checkCollision(std::move(area_1), std::move(area_2))){
            //outEnemyInstances.push_back(spriteIterator);
            outEnemyInstances.insert(spriteIterator);
            this->rocket->setHealth(this->rocket->getHealth() - 10);
            std::cout << "Current Health : " << this->rocket->getHealth() << std::endl;
        }

        for(std::vector<std::unique_ptr<Sprite>>::iterator bulletIterator = this->renderInstances.begin();
            bulletIterator != this->renderInstances.end(); ++bulletIterator){
            std::unique_ptr<Area> area_x = std::make_unique<Area>((*bulletIterator)->getPosition().y,
                                                                  (*bulletIterator)->getPosition().y + Square_Size,
                                                                  (*bulletIterator)->getPosition().x,
                                                                  (*bulletIterator)->getPosition().x + Square_Size);

            std::unique_ptr<Area> area_y = std::make_unique<Area>((*spriteIterator)->getPosition().y,
                                                                  (*spriteIterator)->getPosition().y + Square_Size,
                                                                  (*spriteIterator)->getPosition().x,
                                                                  (*spriteIterator)->getPosition().x + Square_Size);
            if(this->checkCollision(std::move(area_x), std::move(area_y))){
                //outEnemyInstances.push_back(spriteIterator);
                outEnemyInstances.insert(spriteIterator);
                outInstanceIndexes.push_back(bulletIterator);
            }
        }

        //NOTE : Enemy Tile Collision : Destroy Enenmy
        for(std::vector<std::unique_ptr<Sprite>>::iterator tileIterator = this->tiles.begin();
            tileIterator != this->tiles.end(); ++tileIterator){

            std::unique_ptr<Area> area_x = std::make_unique<Area>((*tileIterator)->getPosition().y,
                                                                  (*tileIterator)->getPosition().y + Square_Size,
                                                                  (*tileIterator)->getPosition().x,
                                                                  (*tileIterator)->getPosition().x + Square_Size);

            std::unique_ptr<Area> area_y = std::make_unique<Area>((*spriteIterator)->getPosition().y,
                                                                  (*spriteIterator)->getPosition().y + Square_Size,
                                                                  (*spriteIterator)->getPosition().x,
                                                                  (*spriteIterator)->getPosition().x + Square_Size);
            if(this->checkCollision(std::move(area_x), std::move(area_y))){
                //outEnemyInstances.push_back(spriteIterator);
                outEnemyInstances.insert(spriteIterator);
            }
        }


        if((*spriteIterator)->getPosition().x > (this->width + Square_Size) ||
           (*spriteIterator)->getPosition().y > (this->height + Square_Size) ||
           (*spriteIterator)->getPosition().x < (-Square_Size) ||
           (*spriteIterator)->getPosition().y < (-Square_Size) ){
            //Store the indexes
            //outEnemyInstances.push_back(spriteIterator);
            outEnemyInstances.insert(spriteIterator);
        }
        x++;
    }

    //Delete Objects Outside of Frame
    for(auto outInstance : outEnemyInstances){
        this->enemyInstances.erase(outInstance);
    }

    for(auto outInstance : outInstanceIndexes){
        this->renderInstances.erase(outInstance);
    }

    outEnemyInstances.clear();
    outInstanceIndexes.clear();

    /*Basic Spawn Trick*/
    static int spawnTicks = 0;
    static int fireTicks  = 0;

    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS &&
       rocket->bullet_count > 0 &&
       fireTicks >= 30){

        this->fireBullet();
        fireTicks = 0;
        --this->rocket->bullet_count;
    }

    if(spawnTicks >= 60){
        //this->spawnEnemies();
        spawnTicks = 0;
    }

    ++fireTicks;
    ++spawnTicks;


    for(auto& instance : this->renderInstances){
        instance->update(window);
    }

    for(auto& enemy : this->enemyInstances){
        enemy->update(window);
    }

    for(auto& tile : this->tiles){
        tile->update(window);
    }

    this->rocket->update(window);

    if(false){
        std::cout << "pos x : " << this->rocket->getPosition().x << " pos y : " << this->rocket->getPosition().y << std::endl;
    }
    if(false){
        std::cout << "Bullet on The Screen : " << this->renderInstances.size() << std::endl;
        std::cout << "Remain Bullet : " << this->rocket->bullet_count << std::endl;
    }
}

void GameWindow::close(GLFWwindow* window){
    glfwDestroyWindow(window);
    glfwTerminate();
}
