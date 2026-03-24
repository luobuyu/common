# common

公共基础库，提供日志、配置解析、命令行参数解析、常用工具函数等通用能力。采用 C++17 标准，CMake 构建。

## 目录结构

```
common/
├── CMakeLists.txt          # 顶层构建文件
├── dry_core/               # 核心工具函数（字符串、时间、随机数）
├── config/                 # INI 配置文件解析库
├── logger/                 # 高性能日志库（支持同步/异步）
├── argparse/               # 命令行参数解析库
└── dry/                    # 对外统一入口（日志初始化 + 便捷宏）
```

**模块依赖关系：**

```
dry  ──▶  logger  ──▶  dry_core
 │          │
 ▼          ▼
config   log_format / log_event
```

---

## 模块详解

### 1. dry_core — 核心工具函数

头文件位于 `dry_core/` 目录，提供 header-only 的常用工具：

| 头文件         | 命名空间 | 功能                                                                           |
| -------------- | -------- | ------------------------------------------------------------------------------ |
| `dry_string.h` | `dry`    | 字符串工具：`strTo<T>()` 类型转换、`stringSplit()` 分割、`stringTrim()` 去空白 |
| `dry_time.h`   | `dry`    | 时间工具：`getTime()` 格式化时间、`getTimeWithMs()` 带毫秒、`Timer` 计时器类   |
| `dry_random.h` | `dry`    | 随机数工具：`Random::rand()` 支持整数/浮点/权重采样                            |

**使用示例：**

```cpp
#include "dry_core/dry_string.h"
#include "dry_core/dry_time.h"
#include "dry_core/dry_random.h"

// 字符串转换
int val = dry::strTo<int>("42");
bool flag = dry::strTo<bool>("true");

// 字符串分割
auto parts = dry::stringSplit("a,b,c", ",");

// 获取当前时间
std::string now = dry::getTime("%Y-%m-%d %H:%M:%S");

// 计时器
dry::Timer timer;
// ... 执行耗时操作 ...
double elapsed_ms = timer.getDurationMs();

// 随机数
int dice = dry::Random::rand(1, 7);         // [1, 7)
double prob = dry::Random::rand();           // [0, 1)
```

---

### 2. config — INI 配置文件解析

解析标准 INI 格式配置文件，支持 `[section]` 分组和 `key = value` 键值对。

**使用示例：**

```cpp
#include "config/config.h"

config::Config conf;
conf.initConfig("./config.ini");

// 带默认值获取
std::string log_path = conf.getString("logger", "log_path", "../log");
int log_level = conf.getInt("logger", "log_level", 2);

// 判断配置项是否存在
if (conf.exist("logger", "log_format")) {
    // ...
}
```

**支持的数据类型：** `std::string`、`int`、`double`，均提供带默认值和不带默认值两种接口。

---

### 3. logger — 高性能日志库

功能完整的日志系统，支持同步和异步两种模式。

#### 核心组件

| 组件            | 说明                                                         |
| --------------- | ------------------------------------------------------------ |
| `LogManager`    | 全局单例，管理日志系统初始化、级别控制                       |
| `SyncLogger`    | 同步日志器，写入时加锁，适合低频场景                         |
| `AsyncLogger`   | 异步日志器，后台线程 + 阻塞队列批量消费，高吞吐              |
| `FileSink`      | 文件输出，按时间和大小自动轮转                               |
| `StdoutSink`    | 控制台输出                                                   |
| `LoggerFormat`  | 可自定义的格式化引擎                                         |
| `BlockingQueue` | 无锁环形队列 + 条件变量，支持 `batchPopWithTimeout` 批量弹出 |

#### 日志级别

| 级别    | 值  | 说明     |
| ------- | --- | -------- |
| `DEBUG` | 1   | 调试信息 |
| `INFO`  | 2   | 常规信息 |
| `WARN`  | 3   | 警告信息 |
| `ERROR` | 4   | 错误信息 |
| `OFF`   | 5   | 关闭日志 |

#### 日志格式 pattern

默认格式：`%d{%Y-%m-%d %H:%M:%S}%T[%L]%T[%p,%t,%c]%T[%M]%T[%F:%f:%l]%T%m%n`

| 占位符    | 含义                 | 示例输出              |
| --------- | -------------------- | --------------------- |
| `%d{fmt}` | 时间戳（自定义格式） | `2025-03-24 19:26:43` |
| `%T`      | 空格分隔符           | ` `                   |
| `%L`      | 日志级别             | `INFO`                |
| `%p`      | 进程 ID              | `12345`               |
| `%t`      | 线程 ID              | `140257…`             |
| `%c`      | 协程 ID              | `0`                   |
| `%M`      | 模块名               | `rpckit`              |
| `%F`      | 文件名               | `main.cpp`            |
| `%f`      | 函数名               | `handleRequest`       |
| `%l`      | 行号                 | `42`                  |
| `%m`      | 日志内容             | `connect ok`          |
| `%n`      | 换行符               | `\n`                  |

#### 异步日志器工作流程

```
生产者线程                   BlockingQueue              后台消费线程
    │                            │                          │
    │── push(LogEvent) ─────────▶│                          │
    │                            │◀── batchPopWithTimeout ──│
    │                            │        (最多64条)         │
    │                            │──── 批量返回 ───────────▶│
    │                            │                          │── 写入所有 Sink
    │                            │                          │
    │                            │   (超时无数据时 flush)    │
```

---

### 4. argparse — 命令行参数解析

功能丰富的 C++ 命令行参数解析库，支持三种参数类型和子命令。

#### 参数类型

| 类型                    | 说明             | 示例                   |
| ----------------------- | ---------------- | ---------------------- |
| `FlagArgument`          | 开关参数（bool） | `--verbose`, `-v`      |
| `OptionArgument<T>`     | 选项参数（带值） | `--port 8080`, `-n 10` |
| `PositionalArgument<T>` | 位置参数         | `input.txt`            |

#### 特性

- 支持短选项和长选项（`-v` / `--verbose`）
- 支持值绑定到外部变量或 `std::vector<T>`
- 支持子命令（subcommand）
- 自动生成 `--help` 帮助信息
- 模板类型安全

**使用示例：**

```cpp
#include "argparse/argument_parser.h"

ArgumentParser parser("myapp", "我的应用程序");

// 开关参数
bool verbose = false;
parser.addFlagArgument({"-v", "--verbose"}, verbose, "启用详细输出");

// 选项参数（绑定到变量）
int port = 8080;
parser.addOptionArgument<int>({"-p", "--port"}, port, "监听端口");

// 位置参数
std::string input_file;
parser.addPositionalArgument<std::string>({"input"}, input_file, "输入文件");

// 解析
if (!parser.parse(argc, argv)) {
    return 0;  // --help 已打印
}

// 使用解析后的值
std::cout << "port=" << port << ", file=" << input_file << std::endl;
```

---

### 5. dry — 对外统一入口

封装日志系统的初始化和便捷宏，是大多数业务代码唯一需要 `#include` 的头文件。

**使用示例：**

```cpp
#include "dry/dry_log.h"

// 方式一：从配置文件初始化
config::Config conf;
conf.initConfig("config.ini");
dry::initLogger(conf);

// 方式二：手动指定参数
dry::initLogger("my_module", logger::LogLevel::INFO, "../log", true);

// 打日志（printf 风格）
LOG_DEBUG("user_id=%d, name=%s", uid, name.c_str());
LOG_INFO("server started on port %d", port);
LOG_WARN("retry count exceeded: %d", retry);
LOG_ERROR("connect failed, ret=%d", ret);
```

---

## 配置文件说明

日志系统支持通过 INI 配置文件初始化：

```ini
[logger]
# 日志文件存放路径
log_path = ../log

# 是否开启文件输出（1=开启，0=关闭）
file_sink = 1

# 是否开启控制台输出（1=开启，0=关闭）
std_sink = 1

# 模块名称，会出现在日志中
module_name = rpckit

# 日志级别：DEBUG=1, INFO=2, WARN=3, ERROR=4, OFF=5
log_level = 2

# 日志器类型：sync=同步, async=异步（默认异步）
async = 1

# 自定义日志格式（可选，有默认值）
# 默认: %d{%Y-%m-%d %H:%M:%S}%T[%L]%T[%p,%t,%c]%T[%M]%T[%F:%f:%l]%T%m%n
log_format = %d{%Y-%m-%d %H:%M:%S}%T[%L]%T[%M]%T%m%n
```

---

## 构建方式

使用 CMake 构建，要求 C++17：

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

也可作为子项目被上层 CMake 项目通过 `add_subdirectory(common)` 引入。
