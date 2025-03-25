#pragma once
#include <memory>
#include <bitset>
#include "glm/glm.hpp"

#define MAX_FPS 60.0

typedef char MovementFlags;

enum PlayerMovement {
    Left,
    Right,
    Forward,
    Backwards,
    Jump
};

//Structure for initialising engine & engine members
struct EngineInitData {
    unsigned int mapDimensionsXYZ[3];
    float mouseSensitivity = 0.1;
    float playerSpeed = 0.2;
    float gravity = 9.81;
    float jumpForce = 700;
    float blockBaseOffset = 0.01;
    glm::vec3 spawnPoint = {0.0f, 0.0f, 0.0f};
    glm::vec3 playerDimensions;
};

class Player {
public:
    Player(glm::vec3 spawnPosition, glm::vec3 dimensions) 
            : _movementFlags(), _falling(true), _yaw(), _yVelocity(),
            _yAcceleration(), _position(spawnPosition), _dimensions(dimensions) {}
    void move();
    void setMoving(PlayerMovement direction, bool moving);
    void setFalling(bool falling);
    void applyGravity(float timeStep);
    void resetGravity();
    void incrementYaw(float amount);
    inline void setGravity(float gravity) { _gravity = gravity; }
    inline void setSpeed(float speed) { _speed = speed; }
    inline void setJumpForce(float jumpForce) {_jumpForce = jumpForce; }
    inline void setBlockBaseOffset(float blockBaseOffset) { _blockBaseOffset = blockBaseOffset; }
    inline glm::vec3 getCurrentPosition() const { return _position; }
    glm::vec3 getNextPosition();
    inline glm::vec3 getDimensions() const { return _dimensions; }
    inline bool getFalling() { return _falling; }
    inline float getYaw() { return _yaw; }
private:
    bool _falling;
    MovementFlags _movementFlags;
    float _yaw;
    float _yVelocity;
    float _yAcceleration;
    float _speed;
    float _gravity;
    float _jumpForce;
    float _blockBaseOffset;
    glm::vec3 _position;
    glm::vec3 _dimensions;
};

class Map {
public:
    Map(unsigned int xDimensions, unsigned int yDimensions, unsigned int zDimensions) :
        _xDim(xDimensions), _yDim(yDimensions), _zDim(zDimensions), 
        _map(new bool[xDimensions*yDimensions*zDimensions]) {}
    void fromHeightmap(float *heightmap, float maxY);
    bool planeIntersectsMap(glm::vec3 position, glm::vec2 dimensions);
    bool cuboidIntersectsMap(glm::vec3 position, glm::vec3 dimensions);
    bool at(int x, int y, int z) const;
    bool at(glm::vec3 pos) const;
    void setAt(int x, int y, int z, bool value);
    std::bitset<6> surroundingBlocks(int x, int y, int z) const;
private:
    std::unique_ptr<bool[]> _map;
    unsigned int _xDim, _yDim, _zDim;
};

class Engine {
public:
    Engine(EngineInitData e) :
            _map(e.mapDimensionsXYZ[0], e.mapDimensionsXYZ[1], e.mapDimensionsXYZ[2]),
            _player(e.spawnPoint, e.playerDimensions), _initData(e) {
        _player.setGravity(e.gravity);
        _player.setJumpForce(e.jumpForce);
        _player.setSpeed(e.playerSpeed);
        _player.setBlockBaseOffset(e.blockBaseOffset);
        //initial location update after player members set, avoid undefined behaviour.
        update();
    }
    void cursorMoved(double xpos, double ypos);
    void setPlayerMoving(PlayerMovement direction, bool moving);
    void update();
    void loadHeightmap(float *heightmap, float maxY);
    const Map& getMap() const { return _map; }
    const glm::mat4& getCamera() const { return _camera; }
private:
    bool _mouseMoved;
    float _lastMouseX, _lastMouseY, _camPitch;
    Map _map;
    Player _player;
    EngineInitData _initData;
    glm::mat4 _camera;
};