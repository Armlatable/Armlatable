
/**
 * @file algorithm_teleop.cpp
 * @brief テレオペレーションアルゴリズム実装
 */

#include <algorithm/algorithm_teleop.hpp>

namespace locomo {
namespace algorithm {

AlgorithmTeleop::AlgorithmTeleop(float max_vx, float max_wz)
    : max_vx_(max_vx), max_wz_(max_wz)
{
}

protocol::VelocityCommand AlgorithmTeleop::update(char input) {
    protocol::VelocityCommand cmd = {0.0f, 0.0f};

    switch (input) {
        case 'w': cmd.vx = max_vx_; break;
        case 's': cmd.vx = -max_vx_; break;
        case 'a': cmd.wz = max_wz_; break;
        case 'd': cmd.wz = -max_wz_; break;
        case 'x': cmd.vx = 0.0f; cmd.wz = 0.0f; break;
        default: break; // Stop or Ignore
    }
    return cmd;
}

} // namespace algorithm
} // namespace locomo
