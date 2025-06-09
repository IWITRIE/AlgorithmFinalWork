#!/bin/bash

# --- 配置 ---
SOURCE_FILES=("berlekamp_massey.cpp" "matrix_power.cpp" "poly_multiply.cpp" "naive.cpp")
ALGORITHM_NAMES=("NTT优化" "矩阵快速幂" "多项式乘法" "朴素算法")
INPUT_DIR="in"
TEMP_OUTPUT_DIR="tmp"
ANSWER_DIR="out"
COMPILER="g++"
CPP_FLAGS="-std=c++11 -O2 -Wno-unused-result"

# --- 脚本开始 ---

# 清理旧的可执行文件和临时输出目录
for source_file in "${SOURCE_FILES[@]}"; do
    executable="${source_file%.cpp}_exec"
    rm -f "$executable"
done
mkdir -p "$TEMP_OUTPUT_DIR"

echo "========================================="
echo "编译所有算法实现..."
echo "========================================="

# 编译所有 C++ 代码
for i in "${!SOURCE_FILES[@]}"; do
    source_file="${SOURCE_FILES[$i]}"
    algorithm_name="${ALGORITHM_NAMES[$i]}"
    executable="${source_file%.cpp}_exec"
    
    echo "编译 $source_file ($algorithm_name)..."
    $COMPILER $CPP_FLAGS "$source_file" -o "$executable"
    if [ $? -ne 0 ]; then
        echo "编译失败！"
        exit 1
    fi
    echo "编译成功。"
done

echo "========================================="
echo "开始测试..."
echo "========================================="

# 检查输入目录是否存在
if [ ! -d "$INPUT_DIR" ]; then
    echo "错误：输入目录 '$INPUT_DIR' 未找到。"
    exit 1
fi

# 检查答案目录是否存在
if [ ! -d "$ANSWER_DIR" ]; then
    echo "错误：答案目录 '$ANSWER_DIR' 未找到。"
    exit 1
fi

# 遍历所有输入文件
find "$INPUT_DIR" -type f -name "*.in" | sort -V | while read -r test_case_path; do
    test_case_name=$(basename "$test_case_path" .in)
    input_file="$test_case_path"
    answer_file="$ANSWER_DIR/$test_case_name.out"

    echo "测试用例: $test_case_name"
    echo "-----------------------------------------"

    # 检查对应的答案文件是否存在
    if [ ! -f "$answer_file" ]; then
        echo "跳过 (未找到答案文件: $answer_file)"
        echo
        continue
    fi

    # 测试所有算法
    for i in "${!SOURCE_FILES[@]}"; do
        source_file="${SOURCE_FILES[$i]}"
        algorithm_name="${ALGORITHM_NAMES[$i]}"
        executable="${source_file%.cpp}_exec"
        temp_output_file="$TEMP_OUTPUT_DIR/${test_case_name}_${source_file%.cpp}.out"

        printf "  %-15s: " "$algorithm_name"

        # 运行程序
        start_time=$(date +%s%N)
        timeout 10s ./"$executable" < "$input_file" > "$temp_output_file" 2> "$temp_output_file.err"
        program_exit_code=$?
        end_time=$(date +%s%N)
        runtime_ms=$(( (end_time - start_time) / 1000000 ))

        if [ $program_exit_code -eq 124 ]; then
            echo "超时 (耗时: ${runtime_ms}ms, >10s)"
            continue
        elif [ $program_exit_code -ne 0 ]; then
            echo "运行时错误 (退出码: $program_exit_code) (耗时: ${runtime_ms}ms)"
            continue
        fi

        # 比较临时输出文件和答案文件
        if diff -wBq "$temp_output_file" "$answer_file" > /dev/null; then
            echo "通过 (耗时: ${runtime_ms}ms)"
        else
            echo "答案错误 (耗时: ${runtime_ms}ms)"
        fi
    done
    echo
done

echo "========================================="
echo "测试完成。"
echo "========================================="

# 清理临时输出目录和可执行文件
rm -rf "$TEMP_OUTPUT_DIR"
for source_file in "${SOURCE_FILES[@]}"; do
    executable="${source_file%.cpp}_exec"
    rm -f "$executable"
done