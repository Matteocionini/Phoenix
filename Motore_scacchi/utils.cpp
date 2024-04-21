#include "engineUtils.h"

bool Utils::debug = false;
Board Utils::board = Board();
bool Utils::isReady = true;
std::mutex Utils::mtx;