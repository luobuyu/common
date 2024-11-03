# common

common库，里面包含

- 日志库
- 配置解析库
- dry库

## 日志库

支持配置

``` ini
[logger]
log_path = ../log
file_sink = 1
std_sink = 1
module_name = prckit

# DEBUG = 1,
# INFO = 2,
# WARN = 3,
# ERROR = 4,
# OFF = 5 // don't print log
log_level = 4
# 默认format %d{%Y-%m-%d %H:%M:%S}%T[%L]%T[%p,%t,%c]%T[%M]%T[%F:%f:%l]%T%m%n
# 日期 [level] [进程ID，线程ID，协程ID] [模块名] [文件名:函数名:行号] 日志行 换行
log_format = %d{%Y-%m-%d %H:%M:%S}%T%m%n
```

## dry库

提供公共对外接口
提供一些基础操作
