# 哐哐当当雀雀球 (Bouncing Birds)

一个基于 SFML 的物理弹球游戏，玩家通过发射小鸟来将目标区域中的敌方小鸟击出。

## 游戏特点

- 物理引擎：真实的碰撞检测和反弹效果
- 特殊技能：特殊球体具有范围推动效果
- 存档系统：支持游戏进度的保存与加载
- 回放功能：可以查看最终局面的回放
- 计分系统：记录当前分数和历史最高分

## 操作说明

### 基本操作
- 鼠标左键：按住蓄力，松开发射
- 数字键 1-4：选择要发射的小鸟
- R 键：加载存档
- S 键：保存游戏
- V 键：在结束画面和回放模式之间切换

### 游戏规则
- 每局游戏有 4 个可发射的小鸟
- 第 3、4 号小鸟具有特殊推动效果
- 目标是将中心区域的敌方小鸟击出
- 得分取决于成功击出区域的敌方小鸟数量

## 技术特性

### 核心组件
- 物理系统：碰撞检测、反弹和摩擦
- 渲染系统：SFML 图形渲染
- 音频系统：背景音乐和碰撞音效
- 存档系统：二进制文件存储

### 主要类
- `Game`：主游戏类，管理游戏流程
- `GameObject`：游戏对象基类
- `CollisionHandler`：碰撞处理器
- `ScoreManager`：分数管理器
- `TextureManager`：纹理资源管理器

## 系统要求

### 依赖项
- SFML 2.x
- C++11 或更高版本
- 支持的操作系统：Windows/Linux/MacOS

### 资源文件
需要以下资源文件：
- `Images/`: 游戏贴图
- `chinese.ttf`: 中文字体
- `background_music.flac`: 背景音乐
- `collision.flac`: 碰撞音效

## 构建说明

1. 确保已安装 SFML 库
2. 将所有资源文件放置在正确位置
3. 使用支持 C++11 的编译器编译
4. 链接 SFML 库（-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio）

## 开发者说明

### 代码结构
- `Game.h`: 主要游戏逻辑和类定义
- 使用面向对象设计，便于扩展
- 采用 SFML 框架处理图形、音频和输入

### 扩展建议
- 添加新的特殊球体效果
- 实现更多的游戏模式
- 添加粒子效果系统
- 优化物理引擎性能

## 许可证

[在此添加许可证信息]

## 作者

[在此添加作者信息]