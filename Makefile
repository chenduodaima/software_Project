aod:main.cc
	g++ -g -o $@ $^ -ljsoncpp -lmariadb -std=c++14 -L../cpphttplib/

.PHONY:clean
clean:
	rm -rf aod project.sql

#登录数据库
.PHONY:login
login:
	mysql -u root -proot

#导出数据库
.PHONY:mysql
mysql:
	mysqldump --defaults-file=~/.my.cnf sw_pj > project.sql

# 定义需要的文件类型
FILE_TYPES = -name "Makefile" -o -name "*.html" -o -name "*.css" -o -name "*.js" -o -name "*.json" \
             -o -name "*.hpp" -o -name "*.cc" -o -name "*.h" -o -name "*.md" \
             -o -name "*.txt" -o -name "*.htm" -o -name "*.eot" -o -name "*.ttf"

# 定义需要的目录，使用完整路径
NEED_DIRS = ./test ./web ./web/css ./web/font ./web/fonts ./web/js .

# 找到所有符合条件的文件：只在指定目录的第一层
NEED_FILES = $(foreach dir,$(NEED_DIRS),$(shell find $(dir) -maxdepth 1 \( $(FILE_TYPES) \) ))

.PHONY: git-reset-all
git-reset-all:
	@echo "Completely resetting remote repository..."
	# 创建一个全新的分支（不包含任何历史）
	git checkout --orphan temp_branch
	# 删除所有文件的跟踪
	git rm -rf --cached .
	# 添加我们想要的文件
	git add $(NEED_FILES)
	# 提交更改
	git commit -m "complete repository reset"
	# 删除主分支
	git branch -D main
	# 将临时分支重命名为主分支
	git branch -m main
	# 强制推送到远程，完全替换远程仓库
	git push -f git@github.com:chenduodaima/software_Project.git main

.PHONY:git-normal-push
git-normal-push:
	git add $(NEED_FILES)
	git commit -m "update"
	git push