# Git 仓库迁移与 LFS 幽灵文件清理记录

## 一、 问题场景
项目 `hlos` 从 GitCode 迁移至 GitHub (`Terrix-OS`)，需保留全部 commit 记录。
1. `git push` 报错：`GH008: unknown Git LFS objects` / `(missing) master.img`。
2. `git lfs fetch --all` 报错：源仓库 GitCode 返回 `404`，历史 LFS 实体文件已丢失。
3. `git filter-repo` 清理后，`git log` 仍显示 `master.img` 残留在历史中。

## 二、 原因分析
* **源头缺失**：GitCode 服务器丢失了旧版 `master.img` 的 LFS 实体。
* **指针残留**：本地历史 commit 中保留了 LFS 指针，GitHub 推送时严格校验指针与实体对应关系，报错拦截。
* **清理不当**：普通 `filter-repo` 未正确重写历史。

## 三、 解决办法（保留 Commit，抹除幽灵文件）

**第一步：备份项目（必须）**

**第二步：彻底重写历史，抹除文件**
```bash
# --invert-paths 剔除路径，--force 强制重写
git filter-repo --path master.img --invert-paths --force