#include "Enemy.h"

Enemy::Enemy() = default;

Enemy::Enemy(uint32_t textureBufferID, const Vector2<float>& position) : Sprite(textureBufferID, position) {
    m_Type = Chaser;

    m_PrevPlayerTilePos.x = -1;
    m_PrevPlayerTilePos.y = -1;

    m_NextTilePos.x = -1.0f;
    m_NextTilePos.y = -1.0f;

    m_Stuck = false;
    m_OnAction = true;
    m_ActionTicks = 0;
}

Enemy::Enemy(uint32_t textureBufferID, const Vector2<float>& position, EnemyType type) : Sprite(textureBufferID, position) {
    m_Type = type;

    m_PrevPlayerTilePos.x = -1;
    m_PrevPlayerTilePos.y = -1;

    m_NextTilePos.x = -1.0f;
    m_NextTilePos.y = -1.0f;

    m_Stuck = false;
    m_OnAction = true;
    m_ActionTicks = 0;
}

Enemy::~Enemy() = default;

void Enemy::setArea(std::unique_ptr<Area> area) {
    m_Area = std::move(area);
}

Vector2<float> Enemy::getPosition() {
    return m_Position;
}

void Enemy::setHealth(int32_t new_health) {
    m_Health = new_health;
}

int32_t Enemy::getHealth() const {
    return m_Health;
}

void Enemy::setTextureBufferID(uint32_t textureBufferID) {
    Sprite::setTextureBufferID(textureBufferID);
}

uint32_t Enemy::TextureBufferID() {
    return Sprite::getTextureBufferID();
}

EnemyType Enemy::getType() {
    return m_Type;
}

void Enemy::setType(EnemyType targetType) {
    m_Type = targetType;
}

bool Enemy::onAction() const {
    return m_OnAction;
}

void Enemy::resetAction() {
    m_OnAction = true;
    m_ActionTicks = 0;
}

void Enemy::stopAction() {
    m_OnAction = false;
}

std::unique_ptr<Sprite> Enemy::Copy() {
    std::unique_ptr<Enemy> copy = std::make_unique<Enemy>();
    copy->setTextureBufferID(Sprite::getTextureBufferID());

    copy->m_Position = m_Position;
    copy->m_Velocity = m_Velocity;

    copy->setRotation(Sprite::getRotation());
    copy->setArea(m_Area->Copy());
    return copy;
}

void Enemy::update(const Vector2<float> &playerPos, const shared_point_matrix &grid) {
    switch (m_Type) {
        case Chaser: {
            // Chase and Bite
            chaserUpdate(playerPos, grid);
        }
            break;

        case Sniper: {
            // Shoot twice when it is in a valid range
            sniperUpdate(playerPos, grid);
        }
            break;

        case SuicideBomber: {
            // Pick a random spot and explode if player is in range
            suicideBomberUpdate(playerPos, grid);
            spdlog::info("Boom Exploded!");
        }
            break;

        case AI: {
            // Duplicate of player but controlled by an algorithm
            aiUpdate(playerPos, grid);
            spdlog::info("AI My Friend! Artificial Intelligence");
        }
            break;
    }
}

void Enemy::chaserUpdate(const Vector2<float> &playerPos, const shared_point_matrix &grid) {
    Vector2<int32_t> player_tile_pos = m_Maze->pos2MtrCoord(playerPos);
    Vector2<int32_t> current_tile = m_Maze->pos2MtrCoord(m_Position);

    Vector2<float> current_tile_pos((float) current_tile.y * SQUARE_SIZE, WIDTH - (float) current_tile.x * SQUARE_SIZE);

    bool player_pos_changed = player_tile_pos != m_PrevPlayerTilePos;
    bool moving_to_the_next_cell = (m_Position - m_NextTilePos).magnitute() >= 1.0f;
    bool chasing_on = m_NextTilePos.x >= 0.0f;

    if ((player_pos_changed && !moving_to_the_next_cell) || !chasing_on || m_Stuck) {
        // Get the route to player
        std::vector<Vector2<int32_t>> route = utility::findRoute(m_Maze->m_Matrix, grid, current_tile, player_tile_pos);

        if (!route.empty()) {
            // Store new route to player
            m_PrevRoute = route;
            m_PrevPlayerTilePos = player_tile_pos;

            // Move to next tile in the route
            Vector2<int32_t> nextTile = m_PrevRoute.back();

            // Store next and prev tile positions
            m_NextTilePos = Vector2<float>((float) nextTile.y * SQUARE_SIZE, WIDTH - (float) nextTile.x * SQUARE_SIZE);
            m_PrevTilePos = Vector2<float>((float) current_tile.y * SQUARE_SIZE,
                                           WIDTH - (float) current_tile.x * SQUARE_SIZE);

            // Directional movement to next tile
            Vector2<float> dirToNext = (m_NextTilePos - current_tile_pos).normalize();
            setVelocity(dirToNext * ENEMY_SPEED);

            m_Stuck = false;
        }
    } else if (!moving_to_the_next_cell) {
        if (m_PrevRoute.empty())
            spdlog::critical("Empty Route");

        // Remove the previous tile
        m_PrevRoute.pop_back();

        // Move to next tile in the route
        Vector2<int32_t> nextTile = m_PrevRoute.back();

        // Store next and prev tile positions
        m_NextTilePos = Vector2<float>((float) nextTile.y * SQUARE_SIZE, WIDTH - (float) nextTile.x * SQUARE_SIZE);
        m_PrevTilePos = Vector2<float>((float) current_tile.y * SQUARE_SIZE,
                                       WIDTH - (float) current_tile.x * SQUARE_SIZE);

        // Directional movement to next tile
        Vector2<float> dirToNext = (m_NextTilePos - current_tile_pos).normalize();
        setVelocity(dirToNext * ENEMY_SPEED);
    }

    move();
}

void Enemy::move() {
    // Directional Boundary Check Otherwise Object can pass through obstacles
    Direction horizontalDirection = (getVelocity().y > 0.0f) ? Up : Down;
    Direction verticalDirection = (getVelocity().x > 0.0f) ? Right : Left;

    if (!checkDirectionCollision(horizontalDirection) || !checkDirectionCollision(verticalDirection)) {
        m_Position += getVelocity();
    } else {
        // Move to closest tile
        m_Position = Vector2<float>(std::roundf(m_Position.x / SQUARE_SIZE) * SQUARE_SIZE,
                                    std::roundf(m_Position.y / SQUARE_SIZE) * SQUARE_SIZE);
        m_Stuck = true;
    }
}

void Enemy::sniperUpdate(const Vector2<float> &playerPos, const shared_point_matrix &grid) {
    // TODO : Coming Soon
    float distance = utility::dist_euc<float>(m_Position, playerPos);

    // Action Status Check
    if (m_ActionTicks >= ENEMY_FIRE_TICKS) {
        // It can attack, but reset ticks
        resetAction();
    } else {
        // Reloading ticks
        ++m_ActionTicks;
    }

    if (distance < SNIPER_SIGHT_DISTANCE) {
        // Ray cast and check the line is free
        Vector2<int32_t> playerTilePos = m_Maze->pos2MtrCoord(playerPos);
        Vector2<int32_t> currentTilePos = m_Maze->pos2MtrCoord(m_Position);

        bool isSightOpen = utility::rayCastObstacleCheck(m_Maze->m_Matrix, currentTilePos, playerTilePos);

        if (isSightOpen) {
            // Shoot
            m_OnAction &= true;
        } else {
            m_OnAction &= false;
        }
    } else {
        m_OnAction = false;
    }
}

void Enemy::suicideBomberUpdate(const Vector2<float> &playerPos, const shared_point_matrix &grid) {
    // TODO : Coming Soon
    // Find the closest teleportation point32_t and settle at the other side (REAL MOFO)
}

void Enemy::aiUpdate(const Vector2<float> &playerPos, const shared_point_matrix &grid) {
    // TODO : Coming Soon
}
