#pragma once
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <unordered_map>

#include "dry_core/dry_time.h"

namespace dry {

/// 通用对象池（借出-归还模型，按 Key 缓存空闲对象）
/// @tparam Key     池的索引类型（需支持 hash 和 ==）
/// @tparam value   池中对象类型（通常是 shared_ptr<XxxClient>）
/// @tparam Hash    Key 的 hash 函数（默认 std::hash<Key>）
/// @tparam Equal   Key 的相等比较函数
///
/// 核心语义：
/// - acquire(key)：从空闲池中借出一个对象（移除），或通过 Factory 创建新对象
/// - Release(key, value)：将对象归还到空闲池中
/// - 池中只存放空闲对象，活跃对象由调用者持有引用
///
/// 内部使用 unordered_map<Key, deque<IdleEntry>> 实现：
/// - LIFO 复用：归还时 push_front，借出时 pop_front（最近归还的先复用）
/// - 超时淘汰：从 deque 尾部扫描（尾部是最早归还的，更可能超时）
/// - 全局淘汰：遍历找第一个非空 key 的尾部（近似 LRU）
///
/// @note 非线程安全，需要在同一线程中使用（或外部加锁）
template <typename Key, typename value, typename Hash = std::hash<Key>,
          typename Equal = std::equal_to<Key>>
class ObjectPool {
 public:
  using Ptr = std::shared_ptr<ObjectPool>;
  using Factory = std::function<value(const Key&)>;
  using Validator = std::function<bool(const value&)>;
  using Destroyer = std::function<void(value&)>;

  struct Config {
    size_t max_idle_per_key{4};      // 每个 key 最大空闲对象数
    size_t max_idle_total{20};       // 全局最大空闲对象总数
    int64_t idle_timeout_ms{60000};  // 空闲超时（ms）
  };

 private:
  struct IdleEntry {
    value m_value;
    int64_t idle_since_ms{0};  // 放入空闲池的时间戳
  };

 public:
  ObjectPool(Factory factory, Validator validator = nullptr, Destroyer destroyer = nullptr,
             Config config = {})
      : m_factory(std::move(factory)),
        m_validator(std::move(validator)),
        m_destroyer(std::move(destroyer)),
        m_config(config) {}

  ~ObjectPool() { Clear(); }

  /// 从空闲池中借出一个对象（移除），或创建新对象
  /// @param key 对象的索引 key
  /// @return 可用的对象（调用者持有所有权，用完需 Release 归还）
  ///
  /// 流程：
  /// 1. 懒淘汰超时对象
  /// 2. 从 key 对应的 deque 头部取出（LIFO）
  /// 3. 验证可用性，不可用则销毁并继续取下一个
  /// 4. 没有可用的空闲对象 → factory 创建新对象（不阻塞、不等待）
  value Acquire(const Key& key) {
    CleanupIdle();

    auto it = m_idle_map.find(key);
    if (it != m_idle_map.end()) {
      auto& deque = it->second;
      while (!deque.empty()) {
        auto entry = std::move(deque.front());
        deque.pop_front();
        --m_total_idle;

        // 如果 deque 空了，先清理 map entry
        if (deque.empty()) {
          m_idle_map.erase(it);
          it = m_idle_map.end();
        }

        // 验证对象是否可用
        if (!m_validator || m_validator(entry.m_value)) {
          return std::move(entry.m_value);
        }
        // 不可用，销毁
        if (m_destroyer) {
          m_destroyer(entry.m_value);
        }

        // deque 已被 erase 的情况下跳出
        if (it == m_idle_map.end()) {
          break;
        }
      }
    }

    // 没有可用的空闲对象，工厂创建新的
    return m_factory(key);
  }

  /// 将对象归还到空闲池
  /// @param key 对象的索引 key
  /// @param value 要归还的对象
  /// @return true 成功归还；false 池已满或对象不可用，已销毁
  ///
  /// 流程：
  /// 1. 验证对象是否仍可用
  /// 2. 检查该 key 的空闲数是否已满
  /// 3. 检查全局空闲总数是否已满，满则淘汰最老的
  /// 4. 放入 deque 头部（LIFO）
  bool Release(const Key& key, value val) {
    // 验证对象是否仍可用
    if (m_validator && !m_validator(val)) {
      if (m_destroyer) {
        m_destroyer(val);
      }
      return false;
    }

    // 检查该 key 的空闲数是否已满
    auto& deque = m_idle_map[key];
    if (deque.size() >= m_config.max_idle_per_key) {
      if (m_destroyer) {
        m_destroyer(val);
      }
      return false;
    }

    // 检查全局空闲总数是否已满，满则淘汰最老的
    if (m_total_idle >= m_config.max_idle_total) {
      EvictOldest();
    }

    // 放入 deque 头部（LIFO：最近归还的最先被复用）
    deque.push_front({std::move(val), dry::GetNowMs()});
    ++m_total_idle;
    return true;
  }

  /// 清理空闲超时的对象
  /// @return 清理的对象数量
  /// @note acquire()
  /// 内部已包含懒淘汰调用。如需主动回收（如定时器），也可外部调用。
  size_t CleanupIdle() {
    int64_t now_ms = dry::GetNowMs();
    size_t count = 0;

    for (auto map_it = m_idle_map.begin(); map_it != m_idle_map.end();) {
      auto& deque = map_it->second;
      // 从尾部扫描（尾部是最早归还的，更可能超时）
      while (!deque.empty()) {
        auto& back = deque.back();
        if (now_ms - back.idle_since_ms >= m_config.idle_timeout_ms) {
          if (m_destroyer) {
            m_destroyer(back.m_value);
          }
          deque.pop_back();
          --m_total_idle;
          ++count;
        } else {
          break;  // 前面的更新，不用继续
        }
      }
      // 清理空 deque
      if (deque.empty()) {
        map_it = m_idle_map.erase(map_it);
      } else {
        ++map_it;
      }
    }
    return count;
  }

  /// 移除指定 key 的所有空闲对象
  void Remove(const Key& key) {
    auto it = m_idle_map.find(key);
    if (it != m_idle_map.end()) {
      for (auto& entry : it->second) {
        if (m_destroyer) {
          m_destroyer(entry.m_value);
        }
      }
      m_total_idle -= it->second.size();
      m_idle_map.erase(it);
    }
  }

  /// 清空所有空闲对象
  void Clear() {
    if (m_destroyer) {
      for (auto& [key, deque] : m_idle_map) {
        for (auto& entry : deque) {
          m_destroyer(entry.m_value);
        }
      }
    }
    m_idle_map.clear();
    m_total_idle = 0;
  }

  // ====== 状态查询 ======

  /// 当前空闲对象总数
  size_t IdleCount() const { return m_total_idle; }
  /// 指定 key 的空闲对象数
  size_t IdleCount(const Key& key) const {
    auto it = m_idle_map.find(key);
    return it != m_idle_map.end() ? it->second.size() : 0;
  }
  /// 是否为空
  bool Empty() const { return m_total_idle == 0; }

  // ====== 配置 ======

  void SetMaxIdlePerKey(size_t n) { m_config.max_idle_per_key = n; }
  void SetMaxIdleTotal(size_t n) { m_config.max_idle_total = n; }
  void SetIdleTimeout(int64_t ms) { m_config.idle_timeout_ms = ms; }
  void SetConfig(Config config) { m_config = std::move(config); }

 private:
  /// 淘汰全局最老的一个空闲对象（近似 LRU）
  /// 遍历 map 找第一个非空 key 的尾部元素淘汰
  /// unordered_map 的遍历顺序本身是伪随机的，提供了足够的近似随机性
  void EvictOldest() {
    for (auto it = m_idle_map.begin(); it != m_idle_map.end(); ++it) {
      if (!it->second.empty()) {
        auto& deque = it->second;
        if (m_destroyer) {
          m_destroyer(deque.back().m_value);
        }
        deque.pop_back();
        --m_total_idle;
        if (deque.empty()) {
          m_idle_map.erase(it);
        }
        return;
      }
    }
  }

 private:
  Factory m_factory;
  Validator m_validator;
  Destroyer m_destroyer;
  Config m_config;

  // key → 空闲对象队列（头部是最近归还的，尾部是最早归还的）
  std::unordered_map<Key, std::deque<IdleEntry>, Hash, Equal> m_idle_map;
  size_t m_total_idle{0};
};

}  // namespace dry