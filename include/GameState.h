#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <vector>

#include <torch/torch.h>
#include <torch/script.h>
#include "spdlog/spdlog.h"

#include "Vector2.h"
#include "config.h"

using namespace torch::nn::functional;

class GameState
{
public:
    GameState() {}

    virtual torch::Tensor toTensor() const { return torch::Tensor(); }
    virtual std::unique_ptr<GameState> copy() const { return std::make_unique<GameState>(); }
    virtual void printState() const {}
    virtual uint32_t getObjectSize() const { return sizeof(this); }

    ~GameState() {}
};

class GameStateInternal : public GameState
{
private:
    float m_AvailableProjectiles;
    float m_TimeToTeleport;
    float m_PlayerHealth;
    float m_PlayerRotation;
    Vector2<float> m_PlayerPos;

    std::vector<Vector2<float>> m_NormalTilePositions;
    std::vector<Vector2<float>> m_TeleportTilePositions;
    std::vector<Vector2<float>> m_EnemyPositions;
    std::vector<Vector2<float>> m_ActiveBulletPositions;
    std::vector<float> m_ActiveBulletDirections;

public:
    GameStateInternal(Vector2<float> player_pos,
                      float player_rotation,
                      int available_projectiles, int time_to_teleport, int player_health,
                      std::vector<Vector2<float>> normal_tile_positions,
                      std::vector<Vector2<float>> teleport_tile_positions,
                      std::vector<Vector2<float>> enemy_positions,
                      std::vector<Vector2<float>> bullet_positions,
                      std::vector<float> bullet_directions)
    {
        m_PlayerPos = std::move(player_pos);
        m_PlayerRotation = std::move(player_rotation);
        m_AvailableProjectiles = std::move(available_projectiles);
        m_TimeToTeleport = std::move(time_to_teleport);
        m_PlayerHealth = std::move(player_health);
        m_NormalTilePositions = std::move(normal_tile_positions);
        m_TeleportTilePositions = std::move(teleport_tile_positions);
        m_EnemyPositions = std::move(enemy_positions);
        m_ActiveBulletPositions = std::move(bullet_positions);
        m_ActiveBulletDirections = std::move(bullet_directions);
    }

    torch::Tensor toTensor() const override
    {
        std::vector<float> stateVector;

        stateVector.push_back(m_PlayerPos.x / HEIGHT);
        stateVector.push_back(m_PlayerPos.y / WIDTH);

        stateVector.push_back(m_PlayerRotation / 360.0f);

        stateVector.push_back(m_AvailableProjectiles / INITIAL_PLAYER_BULLETS);
        stateVector.push_back(m_TimeToTeleport / PLAYER_TP_TICKS);
        stateVector.push_back(m_PlayerHealth / INITIAL_PLAYER_HEALTH);

        auto VectorInsert = [&](Vector2<float> pos)
        {
            stateVector.push_back(pos.x / HEIGHT);
            stateVector.push_back(pos.y / WIDTH);
        };

        auto DirectionNormalization = [&](float direction)
        {
            stateVector.push_back(direction / 360.0f);
        };

        // Empty Bullets
        std::vector<float> remainingEmptyEnemy((MAX_ENEMY_COUNT - m_EnemyPositions.size()) * 2, 0);
        std::vector<float> remainingEmptyBullet((MAX_ACTIVE_BULLET_COUNT - m_ActiveBulletPositions.size()) * 2, 0);

        std::for_each(m_EnemyPositions.begin(), m_EnemyPositions.end(), VectorInsert);
        stateVector.insert(stateVector.end(), remainingEmptyEnemy.begin(), remainingEmptyEnemy.end());

        std::for_each(m_ActiveBulletPositions.begin(), m_ActiveBulletPositions.end(), VectorInsert);
        stateVector.insert(stateVector.end(), remainingEmptyBullet.begin(), remainingEmptyBullet.end());

        std::for_each(m_ActiveBulletDirections.begin(), m_ActiveBulletDirections.end(), DirectionNormalization);
        stateVector.insert(stateVector.end(), remainingEmptyBullet.begin(), remainingEmptyBullet.begin() + MAX_ACTIVE_BULLET_COUNT - m_ActiveBulletPositions.size());

        std::for_each(m_TeleportTilePositions.begin(), m_TeleportTilePositions.end(), VectorInsert);
        std::for_each(m_NormalTilePositions.begin(), m_NormalTilePositions.end(), VectorInsert);

        torch::Tensor stateTensor = torch::tensor(stateVector).to(torch::kFloat32).unsqueeze(0);
        return stateTensor;
    }

    std::unique_ptr<GameState> copy() const override
    {
        return std::make_unique<GameStateInternal>(m_PlayerPos,
                                                   m_PlayerRotation,
                                                   m_AvailableProjectiles,
                                                   m_TimeToTeleport,
                                                   m_PlayerHealth,
                                                   m_NormalTilePositions,
                                                   m_TeleportTilePositions,
                                                   m_EnemyPositions,
                                                   m_ActiveBulletPositions,
                                                   m_ActiveBulletDirections);
    }

    void printState() const override
    {
        spdlog::info("State Description");
        spdlog::info("Player Position : {} - {}", m_PlayerPos.x, m_PlayerPos.y);
        spdlog::info("Player Rotation : {} - {}", m_PlayerRotation);
        spdlog::info("Time to Teleport : {} - {}", m_TimeToTeleport);
        spdlog::info("m_AvailableProjectiles : {}", m_AvailableProjectiles);

        spdlog::info("ENEMY POSITIONS : ");
        for (Vector2<float> pos : m_EnemyPositions)
        {
            spdlog::info("{} - {}", pos.x, pos.y);
        }

        spdlog::info("BULLET POSITIONS : ");
        for (Vector2<float> pos : m_ActiveBulletPositions)
        {
            spdlog::info("{} - {}", pos.x, pos.y);
        }
    }

    uint32_t getObjectSize() const override
    {
        return sizeof(this) + sizeof(float) * (m_EnemyPositions.size() + m_ActiveBulletDirections.size() + m_ActiveBulletPositions.size());
    }
};

class GameStateImage : public GameState
{
private:
    u_int16_t m_Width = 800;
    u_int16_t m_Height = 800;
    u_int16_t m_Channels = 3;
    std::vector<BinaryImage> m_StateImages;
    InterpolateFuncOptions m_ResizeOptions = InterpolateFuncOptions().size(std::vector<int64_t>({m_Width / 8, m_Height / 8})).mode(torch::kBilinear).align_corners(true);

public:
    GameStateImage(std::vector<BinaryImage> images, u_int16_t width, u_int16_t height, u_int16_t channels) : m_StateImages(images), m_Width(width), m_Height(height), m_Channels(channels) {}

    torch::Tensor toTensor() const override
    {
        torch::Tensor outputTensor = preProcess(m_StateImages[0]);

        for (size_t idx = 1; idx < m_StateImages.size(); ++idx)
        {
            torch::Tensor currentTensor = preProcess(m_StateImages[idx]);
            outputTensor = torch::cat({outputTensor, currentTensor});
        }

        return outputTensor.unsqueeze(0);
    }

    std::unique_ptr<GameState> copy() const override
    {
        return std::make_unique<GameStateImage>(m_StateImages, m_Width, m_Height, m_Channels);
    }

    void printState() const override
    {
        spdlog::info("Num Images : {} Image Size : {}", m_StateImages.size(), m_StateImages[0].size());
    }

    uint32_t getObjectSize() const override
    {
        return sizeof(this) + sizeof(u_char) * m_StateImages[0].size() * m_StateImages.size();
    }

private:
    torch::Tensor preProcess(BinaryImage image) const
    {
        // torch::Tensor outputTensor = torch::tensor(image).view({m_Channels, m_Width, m_Height}).to(torch::kFloat32).flipud().unsqueeze(0);
        // outputTensor               = interpolate(outputTensor, m_ResizeOptions).mean(1);
        // return outputTensor;
        return torch::tensor(image).view({m_Width, m_Height}).to(torch::kFloat32).unsqueeze(0);
    }
};

#endif // GAME_STATE_H