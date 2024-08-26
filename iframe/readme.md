> 「iframe 版」主题相关说明

# 预览主题

在iframe目录下执行

```
python -m http.server
```
在 localhost 下访问 views/ 目录即可预览主题

# 目录说明

```
- res/             # 静态资源目录
  - adminui/       # layuiAdmin 主题核心代码目录（重要：一般升级时主要替换此目录）
    - dist/        # 主题核心代码构建后的目录（为主要引用）
      - modules/   # 主题核心 JS 模块
      - css/       # 主题核心 CSS 样式
    - src/         # 主题核心源代码目录（不推荐引用，除非要改动核心代码），结构同 dist 目录
  - json/          # 用于演示的模拟数据
  - layui/         # layui 组件库（重要：若升级 layui 直接替换该目录即可）
  - modules/       # 业务 JS 模块（可按照实际的业务需求进行修改）
  - style/         # 业务 CSS 图片等资源目录
  - views/         # 业务动态模板视图碎片目录
  - config.js      # 初始化配置文件
  - index.js       # 初始化主题入口模块
- views/           # 业务视图目录，可放置在项目任意目录（注意修改里面的 css、js 相关路径即可） 
```