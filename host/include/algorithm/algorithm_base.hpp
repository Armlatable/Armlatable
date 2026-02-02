
/**
 * @file algorithm_base.hpp
 * @brief アルゴリズム基底クラス
 */

#ifndef ALGORITHM_BASE_HPP
#define ALGORITHM_BASE_HPP

#include <protocol/protocol.h>

namespace locomo {
namespace algorithm {

/**
 * @brief ホスト側制御アルゴリズムの基底インターフェース
 */
class AlgorithmBase {
public:
    virtual ~AlgorithmBase() = default;

    /**
     * @brief 入力に基づいて速度指令を計算する
     * @param input 入力コード (例: キーボード入力)
     * @return 速度指令パケット
     */
    virtual protocol::VelocityCommand update(char input) = 0;
};

} // namespace algorithm
} // namespace locomo

#endif // ALGORITHM_BASE_HPP
