我在V5核的文件夹中创建了一个MAIN文件夹，

里面是信号量、互斥量、邮箱、消息队列、软件定时器、事件、FinSH命令的基础学习步骤

将想要学习的内容添加到V5核的user文件即可正常编译

这里小灯分别对应的是PC7、PC8、PC9；按键是PA8、PA9、PA10、PA15

后面会用的PWM来调正小灯亮度

pwm要用到的函数在V5核的HAL文件夹的LED的文件

---

## Git 版本控制介绍

本项目使用 Git 进行版本管理，已关联远程仓库：[XiaoN675/CH32H417-RTT-Study](https://github.com/XiaoN675/CH32H417-RTT-Study)

### 常用 Git 命令

| 命令 | 说明 |
|------|------|
| `git status` | 查看当前文件状态（修改/新增/删除） |
| `git add <文件>` | 将更改添加到暂存区（用 `.` 添加所有） |
| `git commit -m "说明"` | 提交暂存区的更改 |
| `git push` | 将本地提交推送到 GitHub |
| `git pull` | 从 GitHub 拉取最新更改 |
| `git log --oneline` | 查看提交历史 |

### 日常流程

```bash
# 1. 查看有哪些文件被修改了
git status

# 2. 添加所有更改
git add .

# 3. 提交并写一条说明（用中文说明改了啥）
git commit -m "你的修改说明"

# 4. 推送到 GitHub
git push
```

### 注意事项

- **不要提交编译产物**：`.o`、`.elf`、`Debug/`、`obj/` 等已通过 `.gitignore` 自动排除
- **每次提交写清楚说明**：方便以后回看历史
- **推送前先 `git pull`**：如果远程有别人（或另一台电脑）的提交，先拉取合并再推送