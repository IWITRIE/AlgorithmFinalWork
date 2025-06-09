#!/bin/bash

# 配置
SOURCE_FILES=("berlekamp_massey.cpp" "gaussian_elimination.cpp")
ALGORITHM_NAMES=("BM+NTT算法" "高斯消元法")
INPUT_DIR="in"
TEMP_OUTPUT_DIR="tmp_compare"
ANSWER_DIR="out"
COMPILER="g++"
CPP_FLAGS="-std=c++11 -O2 -Wno-unused-result"
TIMEOUT_SECONDS=30

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 统计变量
declare -A total_tests
declare -A passed_tests
declare -A failed_tests
declare -A timeout_tests
declare -A total_time

# 初始化统计
for algorithm in "${ALGORITHM_NAMES[@]}"; do
    total_tests["$algorithm"]=0
    passed_tests["$algorithm"]=0
    failed_tests["$algorithm"]=0
    timeout_tests["$algorithm"]=0
    total_time["$algorithm"]=0
done

# 清理函数
cleanup() {
    echo -e "\n${YELLOW}清理中...${NC}"
    for source_file in "${SOURCE_FILES[@]}"; do
        executable="${source_file%.cpp}_exec"
        if [ -f "$executable" ]; then
            rm "$executable"
        fi
    done
    
    if [ -d "$TEMP_OUTPUT_DIR" ]; then
        rm -rf "$TEMP_OUTPUT_DIR"
    fi
}

# 设置信号处理
trap cleanup EXIT INT TERM

# 清理旧文件并创建临时目录
cleanup
mkdir -p "$TEMP_OUTPUT_DIR"

echo -e "${CYAN}========================================="
echo "线性递推算法对比评测系统"
echo "=========================================${NC}"
echo -e "${BLUE}对比算法: ${ALGORITHM_NAMES[0]} vs ${ALGORITHM_NAMES[1]}${NC}"
echo

echo -e "${YELLOW}编译算法实现...${NC}"
echo "-----------------------------------------"

# 编译代码
compile_success=true
for i in "${!SOURCE_FILES[@]}"; do
    source_file="${SOURCE_FILES[$i]}"
    algorithm_name="${ALGORITHM_NAMES[$i]}"
    executable="${source_file%.cpp}_exec"
    
    echo -n "编译 $algorithm_name ... "
    if $COMPILER $CPP_FLAGS "$source_file" -o "$executable" 2>/dev/null; then
        echo -e "${GREEN}成功${NC}"
    else
        echo -e "${RED}失败${NC}"
        compile_success=false
    fi
done

if [ "$compile_success" = false ]; then
    echo -e "${RED}编译失败，退出程序${NC}"
    exit 1
fi

echo

# 检查目录
if [ ! -d "$INPUT_DIR" ]; then
    echo -e "${RED}错误：输入目录 '$INPUT_DIR' 未找到${NC}"
    exit 1
fi

if [ ! -d "$ANSWER_DIR" ]; then
    echo -e "${RED}错误：答案目录 '$ANSWER_DIR' 未找到${NC}"
    exit 1
fi

echo -e "${YELLOW}开始对比测试...${NC}"
echo "========================================="

# 测试每个输入文件
test_count=0
for input_file in "$INPUT_DIR"/*.in; do
    if [ ! -f "$input_file" ]; then
        echo -e "${RED}没有找到输入文件${NC}"
        continue
    fi
    
    test_case_name=$(basename "$input_file" .in)
    answer_file="$ANSWER_DIR/$test_case_name.out"
    
    ((test_count++))
    echo -e "${PURPLE}测试用例 $test_count: $test_case_name${NC}"
    echo "-----------------------------------------"
    
    # 检查答案文件
    if [ ! -f "$answer_file" ]; then
        echo -e "${YELLOW}跳过 (未找到答案文件)${NC}"
        echo
        continue
    fi
    
    # 读取输入信息用于显示
    read -r n m < "$input_file"
    echo "序列长度: $n, 目标项: $m"
    
    # 测试两个算法并进行对比
    declare -A results
    declare -A times
    declare -A status
    
    for i in "${!SOURCE_FILES[@]}"; do
        source_file="${SOURCE_FILES[$i]}"
        algorithm_name="${ALGORITHM_NAMES[$i]}"
        executable="${source_file%.cpp}_exec"
        temp_output_file="$TEMP_OUTPUT_DIR/${test_case_name}_${source_file%.cpp}.out"
        
        total_tests["$algorithm_name"]=$((total_tests["$algorithm_name"] + 1))
        
        echo -n "  $algorithm_name: "
        
        # 运行程序并计时
        start_time=$(date +%s.%N)
        timeout $TIMEOUT_SECONDS "./$executable" < "$input_file" > "$temp_output_file" 2> "${temp_output_file}.err"
        program_exit_code=$?
        end_time=$(date +%s.%N)
        
        execution_time=$(echo "$end_time - $start_time" | bc)
        times["$algorithm_name"]=$execution_time
        total_time["$algorithm_name"]=$(echo "${total_time["$algorithm_name"]} + $execution_time" | bc)
        
        if [ $program_exit_code -eq 124 ]; then
            echo -e "${RED}超时 (>${TIMEOUT_SECONDS}s)${NC}"
            status["$algorithm_name"]="timeout"
            timeout_tests["$algorithm_name"]=$((timeout_tests["$algorithm_name"] + 1))
        elif [ $program_exit_code -ne 0 ]; then
            echo -e "${RED}运行错误 (退出码: $program_exit_code)${NC}"
            status["$algorithm_name"]="error"
            failed_tests["$algorithm_name"]=$((failed_tests["$algorithm_name"] + 1))
        else
            # 检查答案正确性
            if diff -w "$temp_output_file" "$answer_file" > /dev/null 2>&1; then
                echo -e "${GREEN}通过${NC} (${execution_time}s)"
                status["$algorithm_name"]="pass"
                passed_tests["$algorithm_name"]=$((passed_tests["$algorithm_name"] + 1))
                results["$algorithm_name"]=$(cat "$temp_output_file")
            else
                echo -e "${RED}答案错误${NC} (${execution_time}s)"
                status["$algorithm_name"]="wrong"
                failed_tests["$algorithm_name"]=$((failed_tests["$algorithm_name"] + 1))
            fi
        fi
    done
    
    # 性能对比分析
    echo
    echo "  性能对比:"
    if [[ "${status["${ALGORITHM_NAMES[0]}"]}" == "pass" && "${status["${ALGORITHM_NAMES[1]}"]}" == "pass" ]]; then
        time1=${times["${ALGORITHM_NAMES[0]}"]}
        time2=${times["${ALGORITHM_NAMES[1]}"]}
        
        speedup=$(echo "scale=2; $time2 / $time1" | bc)
        if (( $(echo "$time1 < $time2" | bc -l) )); then
            echo -e "    ${GREEN}${ALGORITHM_NAMES[0]} 更快${NC} (快 ${speedup}x)"
        elif (( $(echo "$time1 > $time2" | bc -l) )); then
            speedup=$(echo "scale=2; $time1 / $time2" | bc)
            echo -e "    ${GREEN}${ALGORITHM_NAMES[1]} 更快${NC} (快 ${speedup}x)"
        else
            echo "    性能相当"
        fi
        
        # 检查结果一致性
        if [[ "${results["${ALGORITHM_NAMES[0]}"]}" == "${results["${ALGORITHM_NAMES[1]}"]}" ]]; then
            echo -e "    ${GREEN}结果一致${NC}"
        else
            echo -e "    ${RED}结果不一致！${NC}"
            echo "      ${ALGORITHM_NAMES[0]}: ${results["${ALGORITHM_NAMES[0]}"]}"
            echo "      ${ALGORITHM_NAMES[1]}: ${results["${ALGORITHM_NAMES[1]}"]}"
        fi
    else
        echo -e "    ${YELLOW}无法进行性能对比 (某个算法未通过)${NC}"
    fi
    
    echo
done

echo -e "${CYAN}========================================="
echo "对比测试完成 - 总结报告"
echo "=========================================${NC}"

# 生成详细统计报告
echo
printf "%-20s %-10s %-10s %-10s %-10s %-15s\n" "算法" "总测试" "通过" "失败" "超时" "总时间(s)"
echo "--------------------------------------------------------------------------------"

for algorithm in "${ALGORITHM_NAMES[@]}"; do
    total=${total_tests["$algorithm"]}
    passed=${passed_tests["$algorithm"]}
    failed=${failed_tests["$algorithm"]}
    timeout=${timeout_tests["$algorithm"]}
    time_total=${total_time["$algorithm"]}
    
    if [ $total -gt 0 ]; then
        success_rate=$(echo "scale=1; $passed * 100 / $total" | bc)
        avg_time=$(echo "scale=3; $time_total / $total" | bc)
        
        printf "%-20s %-10d %-10d %-10d %-10d %-15s\n" \
            "$algorithm" "$total" "$passed" "$failed" "$timeout" "$time_total"
        echo "  成功率: ${success_rate}%, 平均时间: ${avg_time}s"
    else
        printf "%-20s %-10s %-10s %-10s %-10s %-15s\n" \
            "$algorithm" "0" "0" "0" "0" "0"
    fi
done
