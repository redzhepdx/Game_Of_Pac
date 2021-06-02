#include "Player.h"

Player::Player() {
    m_ControlType = MANUAL;

    m_Actions.resize(ACTION_SIZE);
}

Player::Player(uint32_t textureBufferID, const Vector2<float> &position, PlayerControl controlType) : Sprite(
        textureBufferID,
        position) {
    m_ControlType = controlType;
    m_ObservationType = Internal;

    m_Actions.resize(ACTION_SIZE);
    m_RecentScoreHistory.set_capacity(100);

    if (m_ControlType == AI_AGENT) {
        if (torch::cuda::is_available()) {
            m_Agent = std::make_unique<Agent>(STATE_SIZE, ACTION_SIZE, torch::kCUDA, m_ObservationType);
        } else {
            m_Agent = std::make_unique<Agent>(STATE_SIZE, ACTION_SIZE, torch::kCPU, m_ObservationType);
        }
    }
}

Player::Player(uint32_t textureBufferID, const Vector2<float> &position, PlayerControl controlType,
               ObservationType observationType) : Sprite(textureBufferID, position) {
    m_ControlType = controlType;
    m_ObservationType = observationType;

    m_Actions.resize(ACTION_SIZE);
    m_RecentScoreHistory.set_capacity(100);

    if (m_ControlType == AI_AGENT) {
        if (torch::cuda::is_available()) {
            m_Agent = std::make_unique<Agent>(STATE_SIZE, ACTION_SIZE, torch::kCUDA, m_ObservationType);
        } else {
            m_Agent = std::make_unique<Agent>(STATE_SIZE, ACTION_SIZE, torch::kCPU, m_ObservationType);
        }
    }
}

Vector2<float> Player::getPosition() {
    return m_Position;
}

void Player::setHealth(int32_t new_health) {
    m_Health = new_health;
}

int32_t Player::getHealth() const {
    return m_Health;
}

uint32_t Player::getRemainingBulletCount() const {
    return m_BulletCount;
}

uint32_t Player::getTimeToTeleport() const {
    return m_TeleportTicks;
}

void Player::addScore(int32_t points) {
    m_Score += points;
}

void Player::setScore(int32_t points) {
    m_Score = points;
}

int32_t Player::getScore() const {
    return m_Score;
}

bool Player::getFireStatus() const {
    return m_FireOrder;
}

void Player::setFireStatus(bool status) {
    m_FireOrder = status;
}

void Player::setTextureBufferID(uint32_t textureBufferID) {
    Sprite::setTextureBufferID(textureBufferID);
}

void Player::reset() {
    spdlog::info("Player Reset");
    m_Health = INITIAL_PLAYER_HEALTH;
    m_PrevHealth = INITIAL_PLAYER_HEALTH;
    m_Score = 0;
    m_PrevScore = 0;
    m_PrevAction = -1;
    m_PrevReward = -1;
    m_BulletCount = 10;
    m_PrevGameState.reset();
}

uint32_t Player::TextureBufferID() {
    return Sprite::getTextureBufferID();
}

std::unique_ptr<Sprite> Player::Copy() {
    std::unique_ptr<Player> copy = std::make_unique<Player>();
    copy->setTextureBufferID(Sprite::getTextureBufferID());

    copy->m_Position = m_Position;
    copy->m_Velocity = m_Velocity;

    copy->setRotation(Sprite::getRotation());

    return copy;
}

void Player::update(GLFWwindow *window,
                    std::unique_ptr<GameState> currentState,
                    const std::unique_ptr<Area> &gameArea,
                    const std::unique_ptr<Maze> &gameMaze) {
    // Reset Action Vector
    std::fill(m_Actions.begin(), m_Actions.end(), false);

    if (m_ControlType == MANUAL) {
        if (m_Health <= 0) {
            // glfwSetWindowShouldClose(window, GLFW_TRUE);
            return;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            // Shaking algorithm for fun
            Vector2<float> newPos = (m_Position + m_Velocity);
            newPos.x = m_Position.x + std::sin(newPos.x) * getVelocity().x;
            newPos.y = m_Position.y + std::sin(newPos.y) * getVelocity().y;
            m_Position = newPos;
        }

        manualControlPlayer(window);
    } else {
        updateAgent(std::move(currentState));
    }

    executeActions(gameArea, gameMaze);

    if (m_TeleportTicks > 0) {
        --m_TeleportTicks;
    }
}

void Player::executeActions(const std::unique_ptr<Area> &gameArea, const std::unique_ptr<Maze> &gameMaze) {
    if (m_Actions[0]) {
        //Go Right
        Vector2<float> direction(m_Velocity.x, 0.0f);
        if (!checkDirectionCollision(Right, gameMaze)) {
            if (m_Position.x < gameArea->m_Box->right - SQUARE_SIZE / 1.0f) {
                m_Position += direction;
            } else if (gameMaze->pos2Tile(Vector2<float>(gameArea->m_Box->left, m_Position.y)) == Teleport &&
                       m_TeleportTicks == 0) {
                m_Position.x = gameArea->m_Box->left + (float) (SQUARE_SIZE * 0.75);
                m_Position.y = (float) (m_Position.y / SQUARE_SIZE) * SQUARE_SIZE;

                m_TeleportTicks = PLAYER_TP_TICKS;
            }
        }
    }

    if (m_Actions[1]) {
        //Go Up
        Vector2<float> direction(0.0f, m_Velocity.y);
        if (!checkDirectionCollision(Up, gameMaze)) {
            if (m_Position.y < gameArea->m_Box->top - SQUARE_SIZE / 1.0f) {
                m_Position += direction;
            } else if (gameMaze->pos2Tile(Vector2<float>(m_Position.x, gameArea->m_Box->bottom)) == Teleport &&
                       m_TeleportTicks == 0) {
                m_Position.y = gameArea->m_Box->bottom + (float) (SQUARE_SIZE * 0.75);
                m_Position.x = (float) (m_Position.x / SQUARE_SIZE) * SQUARE_SIZE;

                m_TeleportTicks = PLAYER_TP_TICKS;
            }
        }
    }

    if (m_Actions[2]) {
        //Go Down
        Vector2<float> direction(0.0f, m_Velocity.y);
        if (!checkDirectionCollision(Down, gameMaze)) {
            if (m_Position.y > gameArea->m_Box->bottom + SQUARE_SIZE / 1.0f) {
                m_Position -= direction;
            } else if (gameMaze->pos2Tile(Vector2<float>(m_Position.x, gameArea->m_Box->top)) == Teleport &&
                       m_TeleportTicks == 0) {
                m_Position.y = gameArea->m_Box->top - (float) (SQUARE_SIZE * 0.75);
                m_Position.x = (float) (m_Position.x / SQUARE_SIZE) * SQUARE_SIZE;

                m_TeleportTicks = PLAYER_TP_TICKS;
            }
        }
    }

    if (m_Actions[3]) {
        //Go Left
        Vector2<float> direction(m_Velocity.x, 0.0f);
        if (!checkDirectionCollision(Left, gameMaze)) {
            if (m_Position.x > gameArea->m_Box->left + SQUARE_SIZE / 1.0f) {
                m_Position -= direction;
            } else if (gameMaze->pos2Tile(Vector2<float>(gameArea->m_Box->right, m_Position.y)) == Teleport &&
                       m_TeleportTicks == 0) {
                m_Position.x = gameArea->m_Box->right - (float)(SQUARE_SIZE * 0.75);
                m_Position.y = (float) (m_Position.y / SQUARE_SIZE) * SQUARE_SIZE;

                m_TeleportTicks = PLAYER_TP_TICKS;
            }
        }
    }

    //Rotation
    if (m_Actions[4]) {
        m_RotationAngle = (float) ((int) (m_RotationAngle - m_Velocity.x) % 360);
    }

    if (m_Actions[5]) {
        m_RotationAngle = (float) ((int) (m_RotationAngle + m_Velocity.x) % 360);
    }

    //Reload
    if (m_Actions[6]) {
        m_BulletCount = 10;
    }

    // Shoot
    if (m_Actions[7]) {
        setFireStatus(true);
    }
}

void Player::manualControlPlayer(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        m_Actions[0] = true;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m_Actions[1] = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m_Actions[2] = true;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        m_Actions[3] = true;
    }

    //Rotation
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        m_Actions[4] = true;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        m_Actions[5] = true;
    }

    //Reload
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_TRUE) {
        m_Actions[6] = true;
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        m_Actions[7] = true;
    }
}

void Player::updateAgent(std::unique_ptr<GameState> currentState) {
    auto action = (int32_t) m_Agent->act(currentState);

    // if there is no score gain, punish little. If player gets a hit increase the punishment
    // float reward = (m_Score - m_PrevScore - ENEMY_DESTROY_POINTS / 5.0f) + (m_Health - m_PrevHealth) / 5.0f;
    float reward = (float) (m_Score - m_PrevScore) / 10.0f + (float) (m_Health - m_PrevHealth) / 10.0f;
    bool done = (m_Health <= 0);

    // There is no observation
    if (m_PrevAction != -1) {
        // TODO : reward or prev_reward ?????
        m_Agent->step(std::move(m_PrevGameState), currentState->copy(), m_PrevAction, reward, done);
    }

    m_PrevGameState = std::move(currentState);
    m_PrevAction = action;
    m_PrevReward = reward;
    m_PrevHealth = m_Health;
    m_PrevScore = m_Score;

    if (m_Agent->totalStepCount() % 200 == 0) {
        spdlog::info("[Player]Current Player Score : {}", m_PrevScore);
    }

    // Execute a single Action
    m_Actions[action] = true;

    if (done) {
        m_Agent->updateEpsilon();
        m_RecentScoreHistory.push_back(m_Score);

        showStatistics();
    }
}

void Player::showStatistics() {
    spdlog::info("------------------Results-------------------");
    spdlog::info("Last Score : {}", m_Score);
    auto average_score = std::accumulate(m_RecentScoreHistory.begin(), m_RecentScoreHistory.end(), 0.0) /
                         (double) m_RecentScoreHistory.size();
    spdlog::info("Average Score : {}", average_score);
    spdlog::info("--------------------------------------------");
}
