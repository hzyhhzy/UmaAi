# 使用GitHub参与开发

## 我只想修改Json，添加一些新的马娘/支援卡（纯网页端PR教程）

[纯网页端PR教程](web-base.md)

## 我会编程(C++)，并且想参加代码开发

安装必要的工具，例如Visual Studio 2022及Git Bash。

从本仓库中Fork一份代码；如果曾经Fork过，请在自己Fork的仓库内点击Sync Fork同步最新更改。

将master分支克隆(Clone)到本地。

假如你不熟悉`git`的使用，你可能需要新建一个分支进行更改，而不是直接提交在`master`上。

```bash
git clone <你的仓库 git 链接> -b master
```

在本地仓库创建一个Dev分支并检出。

```bash
git branch dev
git checkout dev
```


双击打开 `UmaSimulator.sln` 文件。Visual Studio 会自动加载整个项目。

接下来就可以进行更改和测试了。

别忘了定期提交`commit`并写好message。

完成开发和测试后，向GitHub远程仓库提交你的修改（以master为例）。

```bash
git push origin master
```

回到你Fork的仓库，提交一个Pull Request，等待通过或修改意见。

另外，Visual Studio 2022也提供了提交Git更改的工具。

也可以使用SourceTree等可视化工具管理你的Git仓库。