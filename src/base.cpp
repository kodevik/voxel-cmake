#include "base.h"
#include <math.h>
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

const glm::vec3 up = {0.0f, 1.0f, 0.0f};

void Engine::cursorMoved(double xpos, double ypos)
{
    //Adapted from Learn OpenGL by Joey de Vries
    float fXPos = static_cast<float>(xpos);
    float fYPos = static_cast<float>(ypos);
    if (!_mouseMoved) {
        _mouseMoved = true;
        _lastMouseX = fXPos;
        _lastMouseY = fYPos;
    }

    float xOffs = (xpos - _lastMouseX) * _initData.mouseSensitivity;
    float yOffs = (_lastMouseY - ypos) * _initData.mouseSensitivity;
    
    _lastMouseX = fXPos;
    _lastMouseY = fYPos;

    _player.incrementYaw(xOffs);
    _camPitch = glm::clamp(_camPitch + yOffs, -89.0f, 89.0f);
}

void Engine::setPlayerMoving(PlayerMovement direction, bool moving)
{
    _player.setMoving(direction, moving);
}

void Engine::update()
{
    _player.applyGravity(1.0f / MAX_FPS);
    /*Fall detection, gets bottom corners of player's bounding box and checks whether 
    it intersects the map.*/
    glm::vec3 playerFeetPos = _player.getCurrentPosition();
    playerFeetPos.y -= _player.getDimensions().y;
    glm::vec2 playerXZDims;
    playerXZDims.x = _player.getDimensions().x;
    playerXZDims.y = _player.getDimensions().z;
    bool currentlyFalling = !_map.planeIntersectsMap(playerFeetPos, playerXZDims);

    //if and else if are verbose as to avoid unnecessarily setting variables.
    if (_player.getFalling() && !currentlyFalling) _player.setFalling(false);
    else if (!_player.getFalling() && currentlyFalling) _player.setFalling(true);

    //Move player if collision not detected
    if (!_map.cuboidIntersectsMap(_player.getNextPosition(), _player.getDimensions()))
        _player.move();
    
    //Calculate view matrix from player rotation & camera pitch
    glm::vec3 direction;
    direction.x = cos(glm::radians(_player.getYaw())) * cos(glm::radians(_camPitch));
    direction.y = sin(glm::radians(_camPitch));
    direction.z = sin(glm::radians(_player.getYaw())) * cos(glm::radians(_camPitch));
    _camera = glm::lookAt(_player.getCurrentPosition(), glm::normalize(direction) + _player.getCurrentPosition(), up);
}

void Engine::loadHeightmap(float *heightmap, float maxY)
{
    _map.fromHeightmap(heightmap, maxY);
}

void Player::move()
{
    _position = getNextPosition();
}

glm::vec3 Player::getNextPosition()
{
    //Start with current position
    glm::vec3 newPos = _position;
    //Calculate players horizontal bearing
    glm::vec3 direction = glm::normalize(glm::vec3{cos(glm::radians(_yaw)), 0.0f, sin(glm::radians(_yaw))});
    //Increment position based on set movement flags
    if ((_movementFlags >> PlayerMovement::Forward) & 1)
        newPos += direction * _speed;
    if ((_movementFlags >> PlayerMovement::Backwards) & 1)
        newPos -= direction * _speed;
    if ((_movementFlags >> PlayerMovement::Left) & 1)
        newPos -= glm::normalize(glm::cross(direction, up)) * _speed;
    if ((_movementFlags >> PlayerMovement::Right) & 1)
        newPos += glm::normalize(glm::cross(direction, up)) * _speed;
    return newPos;
}

void Player::setMoving(PlayerMovement direction, bool moving)
{
    if (moving)
        _movementFlags |= 1 << direction; //OR to set true
    else
        _movementFlags &= ~(1 << direction); //AND to set false
}

void Player::setFalling(bool isFalling)
{
    _falling = isFalling;
    //reset gravity if no longer falling
    if (!isFalling) {
        resetGravity();
        //Move player out of block, minus an offset to ensure collision detection works properly
        _position.y += 1 - (_position.y - (int)_position.y) - _blockBaseOffset;
    }
}

void Player::applyGravity(float timeStep)
{
    //Velocity verlet, adapted from Wikipedia
    float newYPos = _position.y + _yVelocity * timeStep + _yAcceleration * (timeStep * timeStep * 0.5f);
    float newYAcc = _falling ? -_gravity : 0.0;
    if ((_movementFlags >> PlayerMovement::Jump) & 1) {
        setMoving(PlayerMovement::Jump, false);
        if (!_falling) newYAcc += _jumpForce;
    }
    float newYVel = _yVelocity + (_yAcceleration + newYAcc) * (timeStep * 0.5f);
    _position.y = newYPos;
    _yVelocity = newYVel;
    _yAcceleration = newYAcc;
}

void Player::resetGravity()
{
    _yAcceleration = 0.0f;
    _yVelocity = 0.0f;
}

void Player::incrementYaw(float amount)
{
    float newYaw = _yaw + amount;
    if (newYaw <= 360.0f && newYaw >= -360.0f) _yaw = newYaw;
    else if (newYaw > 360.0f) _yaw = newYaw - 360.0f;
    else _yaw = 360.0f + newYaw;
}

void Map::fromHeightmap(float *heightmap, float maxY)
{
    for (int x = 0; x < _xDim; x++)
    {
        for (int z = 0; z < _zDim; z++){
            for (int y = 0; y < (int)(heightmap[x + z * _xDim] * maxY); y++) {
                setAt(x, y, z, true);
            }
        }
    }
}

bool Map::planeIntersectsMap(glm::vec3 position, glm::vec2 dimensions)
{
    glm::vec3 corner = position;
    //Center around position
    corner.x -= dimensions.x / 2; 
    corner.z -= dimensions.y / 2;
    for (int i = 0; i < 4; i++) {
        glm::vec3 curPoint = corner + glm::vec3{((i / 2) % 2) * dimensions.x, 0, (i % 2)*dimensions.y};
        if (at(curPoint)) return true;
    }
    return false;
}

bool Map::cuboidIntersectsMap(glm::vec3 position, glm::vec3 dimensions)
{
    glm::vec2 xzDims = {dimensions.x, dimensions.z};
    for (int i = 0; i < dimensions.y; i++)
    {
        if (planeIntersectsMap(position - glm::vec3{0.0f, static_cast<float>(i), 0.0f}, xzDims))
            return true;
    }
    return false;
}

bool Map::at(int x, int y, int z) const
{
    if (x >= _xDim || y >= _yDim || z >= _zDim
        || x < 0 || y < 0 || z < 0) return false;
    int idx = x + y * _xDim * _zDim + z * _xDim;
    return _map[idx];
}

bool Map::at(glm::vec3 position) const
{
    if (position.x >= _xDim || position.y >= _yDim || position.z >= _zDim
        || position.x < 0 || position.y < 0 || position.z < 0) return false;
    int idx = (int)position.x + (int)position.y * _xDim * _zDim + (int)position.z * _xDim;
    return _map[idx];
}

void Map::setAt(int x, int y, int z, bool value)
{
    if (x >= _xDim || y >= _yDim || z >= _zDim
        || x < 0 || y < 0 || z < 0) return;
    int idx = x + y * _xDim * _zDim + z * _xDim;
    _map[idx] = value;
}

std::bitset<6> Map::surroundingBlocks(int x, int y, int z) const
{    
    std::bitset<6> result;
    result[5] = at(x, y, z - 1); //Block in back
    result[4] = at(x, y, z + 1); //Block in front
    result[3] = at(x + 1, y, z); //Block to left
    result[2] = at(x, y + 1, z); //Block above
    result[1] = at(x, y - 1, z); //Block below
    result[0] = at(x - 1, y, z); //Block to right

    return result;
}
