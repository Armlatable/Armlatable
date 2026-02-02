
/**
 * @file algorithm_teleop.hpp
 * @brief テレオペレーションアルゴリズム
 */

#ifndef ALGORITHM_TELEOP_HPP
#define ALGORITHM_TELEOP_HPP

#include "algorithm_base.hpp"

namespace locomo {
namespace algorithm {

/**
 * @brief WASDキー入力による手動制御クラス
 */
class AlgorithmTeleop : public AlgorithmBase {
public:
    AlgorithmTeleop(float max_vx = 0.5f, float max_wz = 1.0f);
    virtual ~AlgorithmTeleop() = default;

    /**
     * @brief キー入力処理 (W/A/S/D/X)
     */
    virtual protocol::VelocityCommand update(char input) override;

private:
    float max_vx_;
    float max_wz_;
};

} // namespace algorithm
} // namespace locomo

#endif // ALGORITHM_TELEOP_HPP
