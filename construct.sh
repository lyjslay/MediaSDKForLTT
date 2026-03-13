#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查是否在 MediaSDK 目录
if [[ ! -f "Makefile" ]]; then
    echo -e "${RED}❌ 错误：当前目录不是 MediaSDK（缺少 Makefile）${NC}"
    return 1
fi

# 函数：执行 CarApp-export 构建流程
_run_carapp_build() {
    echo -e "${RED}➡️ 切换到 ../CarApp-export${NC}"
    cd ../CarApp-export || { echo -e "${RED}❌ 切换目录失败！请检查路径${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：source cv181.sh${NC}"
    source cv181.sh

    echo -e "${RED}➡️ 正在执行：build clean${NC}"
    build clean || { echo -e "${RED}❌ build clean 失败！${NC}"; cd - >/dev/null; return 1; }

    echo -e "${RED}➡️ 正在执行：build platform${NC}"
    build platform || { echo -e "${RED}❌ build platform 失败！${NC}"; cd - >/dev/null; return 1; }

    echo -e "${RED}➡️ 正在执行：build carapp${NC}"
    build carapp || { echo -e "${RED}❌ build carapp 失败！${NC}"; cd - >/dev/null; return 1; }

    echo -e "${RED}➡️ 切换回 MediaSDK 目录${NC}"
    cd ../MediaSDK || { echo -e "${RED}❌ 切换回 MediaSDK 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make burn_images${NC}"
    make burn_images || { echo -e "${RED}❌ make burn_images 失败！${NC}"; return 1; }

    echo -e "${GREEN}✅ CarApp-export 构建流程完成！${NC}"
}

# 函数：基础流程（make clean → make all → make install）
build_all() {
    echo -e "${YELLOW}🟨 开始执行：全部编译${NC}"

    echo -e "${RED}➡️ 正在执行：make clean${NC}"
    make clean || { echo -e "${RED}❌ make clean 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make all${NC}"
    make all || { echo -e "${RED}❌ make all 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make install${NC}"
    make install || { echo -e "${RED}❌ make install 失败！${NC}"; return 1; }

    _run_carapp_build || return 1

    echo -e "${GREEN}✅ 基础流程完成！${NC}"
}

# 函数：applications 流程
build_applications() {
    echo -e "${YELLOW}🟨 开始执行：applications 流程${NC}"

    echo -e "${RED}➡️ 正在执行：make applications_clean${NC}"
    make applications_clean || { echo -e "${RED}❌ make applications_clean 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make applications_build${NC}"
    make applications_build || { echo -e "${RED}❌ make applications_build 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make install${NC}"
    make install || { echo -e "${RED}❌ make install 失败！${NC}"; return 1; }

    _run_carapp_build || return 1

    echo -e "${GREEN}✅ applications 流程完成！${NC}"
}

# 函数：cpsl 流程
build_cpsl() {
    echo -e "${YELLOW}🟨 开始执行：cpsl 流程${NC}"

    echo -e "${RED}➡️ 正在执行：make cpsl_clean${NC}"
    make cpsl_clean || { echo -e "${RED}❌ make cpsl_clean 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make cpsl_build${NC}"
    make cpsl_build || { echo -e "${RED}❌ make cpsl_build 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make install${NC}"
    make install || { echo -e "${RED}❌ make install 失败！${NC}"; return 1; }

    _run_carapp_build || return 1

    echo -e "${GREEN}✅ cpsl 流程完成！${NC}"
}

# 函数：framework 流程
build_framework() {
    echo -e "${YELLOW}🟨 开始执行：framework 流程${NC}"

    echo -e "${RED}➡️ 正在执行：make framework_clean${NC}"
    make framework_clean || { echo -e "${RED}❌ make framework_clean 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make framework_build${NC}"
    make framework_build || { echo -e "${RED}❌ make framework_build 失败！${NC}"; return 1; }

    echo -e "${RED}➡️ 正在执行：make install${NC}"
    make install || { echo -e "${RED}❌ make install 失败！${NC}"; return 1; }

    _run_carapp_build || return 1

    echo -e "${GREEN}✅ framework 流程完成！${NC}"
}

build_carapp() {
    echo -e "${YELLOW}🟨 开始执行：carapp 流程${NC}"

    _run_carapp_build || return 1

    echo -e "${GREEN}✅ carapp 流程完成！${NC}"
}

# 函数：显示帮助
build_help() {
    echo -e "${YELLOW}📚 可用构建函数：${NC}"
    echo "  build_applications  → applications 流程"
    echo "  build_cpsl          → cpsl 流程"
    echo "  build_framework     → framework 流程"
    echo "  build_all           → 执行所有流程"
    echo "  build_carapp        → carapp 流程"
    echo "  build_help          → 显示此帮助"
    echo ""
    echo -e "${YELLOW}💡 使用方法：${NC}"
    echo "  source construct.sh"
    echo "  build_applications"
    echo "  build_all"
}

# 自动提示帮助（可选）
echo -e "${GREEN}✅ construct.sh 已加载！输入 build_help 查看可用函数${NC}"
